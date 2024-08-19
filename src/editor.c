/* Includes */
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include "definitions.h"
#include "error_handling.h"
#include "dynamic_buffer.h"
#include "terminal.h"
#include "editor.h"
#include "editor_private.h" /* Global data */

static LastPrintScreenData g_last_print_screen_data;
static int g_top_file = 0; static FileData g_fd;
static vec2 g_window_size;
static vec2 g_cursor_pos;
static IO_Interface g_io_interface;

/* Function definitions */
void editor_init(vec2 _window_size, IO_Interface _io_interface)
{
	g_window_size = _window_size;
	g_cursor_pos = (vec2){ .x = 0, .y = 0 };
	g_fd.darr = darr_create(sizeof(DynamicBuffer*));
	g_io_interface = _io_interface;
	g_last_print_screen_data.col_count = g_window_size.y;
	g_last_print_screen_data.data = calloc(g_last_print_screen_data.col_count, sizeof(LastPrintRowData));
}

void editor_terminate()
{
	for (size_t i = 0; i < g_fd.darr->length; i++)
	{
		dbuf_destroy(*(DynamicBuffer**)darr_get(g_fd.darr, i));
	}
	darr_destroy(g_fd.darr);
	free(g_last_print_screen_data.data);
}

void editor_read_file(const char *filename)
{
	tassert(filename, "editor_read_file: filename is NULL");
	g_fd.filename = filename;
	// Opening file
	FILE *fp = fopen(g_fd.filename, "r");
	tassert(fp, "editor_read_file: file couldn't be opened");
	// Reading line by line and 
	char *line = NULL;
	size_t len;
	while (getline(&line, &len, fp) != EOF)
	{
		DynamicBuffer *dbuf = dbuf_create();
		size_t line_size = strlen(line);
		if (line[line_size-1] == '\n')
		{
			line[--line_size] = NUL;
		}
		dbuf_adds(dbuf, line_size, line);
		darr_add_single(g_fd.darr, &dbuf);
	}
	free(line);
	// Closing file
	fclose(fp);
}

int ctrl_key(char c)
{
	return (c & 0x1f);
}

void editor_render_screen()
{
	g_io_interface.hide_cursor();
	g_io_interface.clear_screen();
	editor_render_rows(&g_fd, g_window_size, g_cursor_pos, g_top_file, &g_io_interface, &g_last_print_screen_data);
	//editor_add_set_cursor_to_start_to_buffer(global_buf);
	g_io_interface.reveal_cursor();
	g_io_interface.flush_output();
	g_io_interface.set_cursor_position(g_cursor_pos.x, g_cursor_pos.y);
}

void editor_render_rows(const FileData *fd, vec2 window_size, vec2 cursor_pos, int top_file_index, const IO_Interface *io_interface, LastPrintScreenData *last_print_screen_data)
{
	size_t file_row = top_file_index;
	size_t file_col = 0;
	for (int i = 0; i < window_size.y; i++)
	{
		if (file_row >= darr_get_size(fd->darr) && cursor_pos.y != i)
		{
			io_interface->render_row(i, 1, "~");
			last_print_screen_data->data[i] = (LastPrintRowData) { .index = 0, .file_row = -1, .file_start_col = -1 };
			continue;
		}
		if (file_row >= darr_get_size(fd->darr))
		{
			io_interface->render_row(i, 0, "");
			last_print_screen_data->data[i] = (LastPrintRowData) { .index = 0, .file_row = -1, .file_start_col = -1 };
			continue;
		}
		const DynamicBuffer *current_line_data = *(DynamicBuffer **)darr_getc(fd->darr, file_row);
		size_t row_render_size = dbuf_get_size(current_line_data) - file_col;
		// Continue printing current file
		if (row_render_size > window_size.x) 
		{
			row_render_size = window_size.x;
		}
		io_interface->render_row(i, row_render_size, dbuf_getc(current_line_data, file_col));
		last_print_screen_data->data[i] = (LastPrintRowData) { .index = row_render_size, .file_row = file_row, .file_start_col = file_col };
		file_col += row_render_size;
		if (file_col == dbuf_get_size(current_line_data))
		{
			file_row++;
			file_col = 0;
		}
	}
}

void editor_clear_screen()
{
	g_io_interface.clear_screen();
	g_io_interface.flush_output();
}

int editor_process_tick()
{
	tassert(editor_is_cursor_in_range(&g_last_print_screen_data, g_cursor_pos), "editor_process_tick: cursor not in range");

	int c = g_io_interface.read_key();
#ifdef DEBUGGING
	if (c == 'q')
#else
	if (c == ctrl_key('q'))
#endif
	{
		return TEXT_EDITOR_EOF;
	}
	if (c == NUL)
	{
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	if (c == ARROW_UP)
	{
		g_cursor_pos = editor_move_cursor(&g_last_print_screen_data, g_cursor_pos, (vec2) {.x = 0, .y = -1});
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	if (c == ARROW_DOWN)
	{
		g_cursor_pos = editor_move_cursor(&g_last_print_screen_data, g_cursor_pos, (vec2) {.x = 0, .y = 1});
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	if (c == ARROW_LEFT)
	{
		g_cursor_pos = editor_move_cursor(&g_last_print_screen_data, g_cursor_pos, (vec2) {.x = -1, .y = 0});
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	if (c == ARROW_RIGHT)
	{
		g_cursor_pos = editor_move_cursor(&g_last_print_screen_data, g_cursor_pos, (vec2) {.x = 1, .y = 0});
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	if (iscntrl(c))
	{
		// For control shit
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	size_t file_row = g_last_print_screen_data.data[g_cursor_pos.y].file_row;
	size_t file_col = g_cursor_pos.x + g_last_print_screen_data.data[g_cursor_pos.y].file_start_col;
	if (file_row == -1)
	{
		DynamicBuffer *new_dbuf = dbuf_create();
		dbuf_addc(new_dbuf, c);
		darr_add_single(g_fd.darr, &new_dbuf);
	}
	else
	{
		dbuf_insertc_to(*(DynamicBuffer**)darr_get(g_fd.darr, file_row), file_col, c);
	}
	if( g_cursor_pos.x == g_window_size.x - 1 && g_cursor_pos.y == g_window_size.y - 1)
	{
		// TODO: Need to add g_top_file change
		// Right now do nothing
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	if (g_cursor_pos.x == g_window_size.x - 1)
	{
		g_cursor_pos.x = 0;
		g_cursor_pos.y++;
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	g_cursor_pos.x++;
	return TEXT_EDITOR_SUCCESSFUL_READ;
}


vec2 editor_move_cursor(const LastPrintScreenData *last_print_screen_data, vec2 current_cursor, vec2 change)
{
	vec2 new_cursor = 
	{ 
		.x = current_cursor.x + change.x,
		.y = current_cursor.y + change.y
	};
	if (!editor_is_cursor_in_range(last_print_screen_data, new_cursor))
	{
		return current_cursor;

	}
	return new_cursor;
}

bool editor_is_cursor_in_range(const LastPrintScreenData *last_print_screen_data, vec2 cursor_pos)
{
	if (!is_in_range(0, cursor_pos.y, last_print_screen_data->col_count))
	{
		return false;
	}
	return last_print_screen_data->data[cursor_pos.y].index >= cursor_pos.x;
}

bool is_in_range(int l, int i, int r)
{
	return l <= i && i < r;
}

