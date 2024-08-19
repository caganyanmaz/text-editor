/* Includes */
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "definitions.h"
#include "error_handling.h"
#include "dynamic_buffer.h"
#include "terminal.h"

/* Global Data */
struct termios original_attributes;
static vec2 window_size;
static vec2 current_cursor_position;
static DynamicBuffer *dbuf;

/* Private Function Declarations */
void editor_add_set_cursor_to_start_to_buffer(DynamicBuffer *buf);
void editor_add_clear_screen_to_buffer(DynamicBuffer *buf);
void editor_add_reveal_cursor_to_buffer(DynamicBuffer *buf);
void update_cursor_position();

int terminal_read_ANSI_sequence();

void print_delicate();

void restore_terminal_behaviour();
void setup_terminal_behaviour();


void terminal_init()
{
	dbuf = dbuf_create();
#ifndef DEBUGGING
	setup_terminal_behaviour();
#endif
	struct winsize ws;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
	{
		throw_up("get_window_size, ioctl failed");
	}
	window_size = (vec2) { .x = ws.ws_col, .y = ws.ws_row };
	current_cursor_position = (vec2) {.x = 0, .y = 0};
}

void terminal_terminate()
{
	dbuf_destroy(dbuf);
#ifndef DEBUGGING
	restore_terminal_behaviour();
#endif
}

vec2 get_window_size()
{
	return window_size;
}

void setup_terminal_behaviour()
{
	if (tcgetattr(STDIN_FILENO, &original_attributes) == TERMIOS_ERROR) 
	{
		throw_up("tcgetattr failed");
	}
	struct termios attr = original_attributes;
	//                newline <C-S>  old shit
	attr.c_iflag &= ~(ICRNL | IXON | BRKINT | INPCK | ISTRIP);
	//             no output processing 
	attr.c_oflag &= ~(OPOST);
	// Old shit
	attr.c_cflag |= (CS8);
	//   echo	canonical mode    <C-C>   <C-V>
	attr.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
	attr.c_cc[VMIN] = 0;
	attr.c_cc[VTIME] = 1;
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &attr) == TERMIOS_ERROR)
	{
		throw_up("tcsetattr failed");
	}
}

void restore_terminal_behaviour()
{
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_attributes) == TERMIOS_ERROR)
	{
		perror("tcsettr failed");
		exit(1);
		//throw_up("tcsetattr failed"); <- We don't want infinite recursion after an error here, do we?
	}
}

void terminal_clear_screen()
{
	dbuf_adds(dbuf, 4, "\x1b[2J");
	dbuf_adds(dbuf, 3, "\x1b[H");
}

void terminal_render_row(int row_id, size_t size, const char *row)
{
	dbuf_adds(dbuf, size, row);
	if (row_id < window_size.y - 1)
	{
		dbuf_adds(dbuf, 2, "\r\n");
	}
}

int terminal_read_key()
{
	char c = NUL;
	handle_error(read(STDIN_FILENO, &c, 1), "read failed");
	if (c == '\x1b') 
	{
		return terminal_read_ANSI_sequence();
	}
	return c;
}


int terminal_read_ANSI_sequence()
{
	char seq[3];
	if (read(STDIN_FILENO, &seq[0], 1) != 1)
	{
		return '\x1b';
	}
	if (read(STDIN_FILENO, &seq[1], 1) != 1) 
	{
		return '\x1b';
	}
	if (seq[0] != '[')
	{
		return '\x1b';
	}
	switch (seq[1]) 
	{
		case 'A': return ARROW_UP;
		case 'B': return ARROW_DOWN;
		case 'C': return ARROW_RIGHT;
		case 'D': return ARROW_LEFT;
	}
	return '\x1b';
}

void terminal_flush_output()
{
	handle_error(write(STDIN_FILENO, dbuf_getc(dbuf, 0), dbuf_get_size(dbuf)), "terminal_flush_output: write failed");
	dbuf_clear(dbuf);
	update_cursor_position();
}

void terminal_set_cursor_position(int x, int y)
{
	// Some bug happens when I set the cursor with other buffers, so I'll do it seperately at each flush
	current_cursor_position = (vec2){.x = x, .y = y};
}

void terminal_hide_cursor()
{
	dbuf_adds(dbuf, 6, "\x1b[?25l");
}

void terminal_reveal_cursor()
{
	dbuf_adds(dbuf, 6, "\x1b[?25h");
}

void update_cursor_position()
{
	if (dbuf_get_size(dbuf) > 0)
	{
		throw_up("terminal_set_cursor_pos: buffer isn't empty");
	}
	dbuf_adds(dbuf, 2, "\x1b[");
	dbuf_addi(dbuf, current_cursor_position.y+1);
	dbuf_addc(dbuf, ';');
	dbuf_addi(dbuf, current_cursor_position.x+1);
	dbuf_addc(dbuf, 'H');
	handle_error(write(STDIN_FILENO, dbuf_getc(dbuf, 0), dbuf_get_size(dbuf)), "terminal_set_cursor_pos: write failed");
	dbuf_clear(dbuf);
}

void print_delicate()
{
	printf("\nDelicate buffer(%d): ", (int)dbuf_get_size(dbuf));
	for (int i = 0; i < dbuf_get_size(dbuf); i++)
	{
		printf("%d ", *dbuf_getc(dbuf, i));
	}
	printf("\n");
}
