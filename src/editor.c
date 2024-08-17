/* Includes */
#include <ctype.h>
#include <stdbool.h>
#include "definitions.h"
#include "error_handling.h"
#include "dynamic_buffer.h"
#include "terminal.h"
#include "editor.h"
#include "editor_private.h"

/* Defines */
#define SPECIAL_SEQUENCE_CHAR '\x1b'
/* Global data */
static Row *g_text;
static vec2 g_window_size;
static vec2 g_cursor_pos;
static IO_Interface g_io_interface;


/* Function definitions */
void editor_init(vec2 _window_size, IO_Interface _io_interface)
{
	g_window_size = _window_size;
	g_text = malloc(g_window_size.y * sizeof(Row));
	g_cursor_pos = (vec2){ .x = 0, .y = 0 };
	for (int i = 0; i < g_window_size.y; i++)
	{
		g_text[i].index = 0;
		g_text[i].data = calloc(g_window_size.x, sizeof(char));
	}
	g_io_interface = _io_interface;
}

void editor_terminate()
{
	for (int i = 0; i < g_window_size.y; i++)
	{
		free(g_text[i].data);
	}
	free(g_text);
}

void editor_add_to_row(Row *row, vec2 window_size, char c)
{
	tassert(c, "editor_add_to_row: tried adding NUL");
	tassert(row->index <= window_size.x - 1, "editor_add_to_row: window size is too big");
	row->data[row->index++] = c;
}

int ctrl_key(char c)
{
	return (c & 0x1f);
}

void editor_render_screen()
{
	g_io_interface.hide_cursor();
	g_io_interface.clear_screen();
	editor_render_rows(g_text, g_window_size, g_cursor_pos, &g_io_interface);
	//editor_add_set_cursor_to_start_to_buffer(global_buf);
	g_io_interface.reveal_cursor();
	g_io_interface.flush_output();
	g_io_interface.set_cursor_position(g_cursor_pos.x, g_cursor_pos.y);
}

void editor_render_rows(Row *text, vec2 window_size, vec2 cursor_pos, const IO_Interface *io_interface)
{
	for (int i = 0; i < window_size.y; i++)
	{
		if (text[i].index == 0 && cursor_pos.y != i)
		{
			io_interface->render_row(i, 1, "~");
			continue;
		}
		io_interface->render_row(i, text[i].index, text[i].data);
	}
}

void editor_clear_screen()
{
	g_io_interface.clear_screen();
	g_io_interface.flush_output();
}

int editor_process_tick()
{
	int c = g_io_interface.read_key();
	if (c == ctrl_key('q'))
	{
		return TEXT_EDITOR_EOF;
	}
	if (c == NUL)
	{
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	if (c == ARROW_UP)
	{
		g_cursor_pos = editor_move_cursor(g_text, g_window_size, g_cursor_pos, (vec2) {.x = 0, .y = -1});
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	if (c == ARROW_DOWN)
	{
		g_cursor_pos = editor_move_cursor(g_text, g_window_size, g_cursor_pos, (vec2) {.x = 0, .y = 1});
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	if (c == ARROW_LEFT)
	{
		g_cursor_pos = editor_move_cursor(g_text, g_window_size, g_cursor_pos, (vec2) {.x = -1, .y = 0});
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	if (c == ARROW_RIGHT)
	{
		g_cursor_pos = editor_move_cursor(g_text, g_window_size, g_cursor_pos, (vec2) {.x = 1, .y = 0});
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	if (!iscntrl(c))
	{
		add_normal_character(g_text, &g_cursor_pos, g_window_size, c);
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	return TEXT_EDITOR_SUCCESSFUL_READ;
}


vec2 editor_move_cursor(const Row *text, vec2 window_size, vec2 current_cursor, vec2 change)
{
	int nx = current_cursor.x + change.x;
	int ny = current_cursor.y + change.y;
	if (!is_in_range(0, nx, window_size.x) || !is_in_range(0, ny, window_size.y) || nx > text[ny].index)
	{
		return current_cursor;
	}
	return (vec2) { .x = nx, .y = ny };
}

void add_normal_character(Row *text, vec2 *current_cursor, vec2 window_size, char c)
{
	if (current_cursor->x > text[current_cursor->y].index)
	{
		throw_up("editor_process_tick: cursor position is out of bounds");
	}
	if (current_cursor->x == text[current_cursor->y].index)
	{
		text[current_cursor->y].data[current_cursor->x] = c;
		text[current_cursor->y].index++;
	}
	else
	{
		row_shift_right(&text[current_cursor->y], window_size, current_cursor->x);
		text[current_cursor->y].data[current_cursor->x] = c;
		text[current_cursor->y].data[text[current_cursor->y].index] = '\0';
	}
	current_cursor->x++;
}

void row_shift_right(Row *row, vec2 window_size, int pos)
{
	if (row->index >= window_size.x)
	{
		throw_up("row_shift_right: row is filled");
	}
	if (!is_in_range(0, pos, row->index))
	{
		throw_up("row_shift_right: pos is not in range");
	}
	for (int i = row->index; i > pos; i--)
	{
		row->data[i] = row->data[i-1];
	}
	row->index++;
}

bool is_in_range(int l, int i, int r)
{
	return l <= i && i < r;
}

