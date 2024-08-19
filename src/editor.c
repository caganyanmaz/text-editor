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

static LastPrintScreenData g_last_print_screen_data; static int g_top_file = 0; static FileData g_fd;
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
	// Opening file
	FILE *fp = fopen(filename, "r");
	if (fp == NULL)
	{
		DynamicBuffer *dbuf = dbuf_create();
		darr_add_single(g_fd.darr, &dbuf);
		return;
	}
	// Reading line by line and 
	char *line = NULL;
	size_t len;
	while (getline(&line, &len, fp) != EOF)
	{
		DynamicBuffer *dbuf = dbuf_create();
		size_t line_size = strlen(line);
		if (line[line_size-1] == '\n')
		{
			line_size--;
		}
		dbuf_adds(dbuf, line_size, line);
		darr_add_single(g_fd.darr, &dbuf);
	}
	free(line);
	// Closing file
	fclose(fp);
}

void editor_write_file(const char *filename)
{
	FILE *fp = fopen(filename, "w");
	for (size_t i = 0; i < darr_get_size(g_fd.darr); i++)
	{
		DynamicBuffer *dbuf = *(DynamicBuffer**)darr_get(g_fd.darr, i);
		// Dashes are for debugging: To clearly see where each file ends
		dbuf_addc(dbuf, '\n');
		fputs(dbuf_get_with_nulc(dbuf, 0), fp);
		dbuf_popc(dbuf);
	}
	fclose(fp);
}

int ctrl_key(char c)
{
	return c & (0x1f);
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
			last_print_screen_data->data[i] = (LastPrintRowData) { .index = -1, .file_row = -1, .file_start_col = -1 };
			continue;
		}
		if (file_row >= darr_get_size(fd->darr))
		{
			io_interface->render_row(i, 0, "");
			const DynamicBuffer *last_line_data = *(DynamicBuffer**)darr_getc(fd->darr, file_row-1);
			size_t last_line_size = dbuf_get_size(last_line_data);
			last_print_screen_data->data[i] = (LastPrintRowData) { .index = 0, .file_row = file_row-1, .file_start_col = last_line_size };
			continue;
		}
		const DynamicBuffer *current_line_data = *(DynamicBuffer **)darr_getc(fd->darr, file_row);
		size_t row_render_size = dbuf_get_size(current_line_data) - file_col;
		// Continue printing current file
		if (row_render_size > window_size.x) 
		{
			row_render_size = window_size.x;
		}
		io_interface->render_row(i, row_render_size, dbuf_get_with_nulc(current_line_data, file_col));
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
	if (c == QUIT_KEY)
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
	
	size_t file_row = g_last_print_screen_data.data[g_cursor_pos.y].file_row;
	size_t file_col = g_cursor_pos.x + g_last_print_screen_data.data[g_cursor_pos.y].file_start_col;
	tassert(file_row < darr_get_size(g_fd.darr), "editor_process_tick: file_row out of range");
	if (c == BACKSPACE)
	{
		// If we're at the start of file, we don't do anything
		if (file_col == 0 && file_row == 0)
		{
			return TEXT_EDITOR_SUCCESSFUL_READ;
		}
		// If we're at the start of a line (that's not the start of file), we append the current line to the previous line
		DynamicBuffer *current_row = *(DynamicBuffer**)darr_get(g_fd.darr, file_row);
		if (file_col == 0)
		{
			DynamicBuffer *prev_row  = *(DynamicBuffer**)darr_get(g_fd.darr, file_row - 1);
			size_t current_row_size  = dbuf_get_size(current_row);
			const char *current_row_s = dbuf_getc(current_row, 0);
			dbuf_adds(prev_row, current_row_size, current_row_s);
			free(current_row);
			darr_shift_left(g_fd.darr, file_row);
		}
		// else we remove one character from the line (previous character)
		else
		{
			dbuf_shift_left(current_row, file_col - 1); }
		g_cursor_pos = editor_retreat_cursor(g_cursor_pos, &g_last_print_screen_data);
		// Reverting the cursor position by one
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	if (c == CARRIAGE_RETURN)
	{
		// Create new line
		debugi(file_row);
		debugi(file_col);
		DynamicBuffer *current_row = *(DynamicBuffer**)darr_get(g_fd.darr, file_row);
		DynamicBuffer *new_row     = dbuf_create();
		const char *current_row_text_at_cursor_right = dbuf_get_with_nulc(current_row, file_col);
		size_t current_row_text_at_cursor_right_size = dbuf_get_size(current_row) - file_col;
		dbuf_adds(new_row, current_row_text_at_cursor_right_size, current_row_text_at_cursor_right);
		dbuf_popm(current_row, current_row_text_at_cursor_right_size);
		darr_insert_to(g_fd.darr, file_row + 1, &new_row);
		g_cursor_pos.x = 0;
		g_cursor_pos.y++;
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	if (iscntrl(c))
	{
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	tassert(file_row != -1, "editor_process_tick: Cursor not on a file");
	dbuf_insertc_to(*(DynamicBuffer**)darr_get(g_fd.darr, file_row), file_col, c);
	g_cursor_pos = editor_advance_cursor(g_cursor_pos, g_window_size);
	return TEXT_EDITOR_SUCCESSFUL_READ;
}


vec2 editor_advance_cursor(vec2 cursor, vec2 window_size)
{
	if( cursor.x == window_size.x - 1 && cursor.y == window_size.y - 1)
	{
		// TODO: Need to add g_top_file change when cursor hits window end
		// Right now doing nothing
		return cursor;
	}
	if (cursor.x == window_size.x - 1)
	{
		cursor.y++;
		cursor.x = 0;
		return cursor;
	}
	cursor.x++;
	return cursor;
}

vec2 editor_retreat_cursor(vec2 cursor, const LastPrintScreenData *last_print_screen_data)
{
	tassert(!(cursor.x == 0 && cursor.y == 0 && g_top_file == 0), "editor_retreat_cursor: at the start of file");
	if (cursor.x == 0 && cursor.y == 0)
	{
		// TODO: Need to add g_top_file change when cursor hits window beginning
		// Right now doing nothing
		return cursor;
	}
	if (cursor.x == 0)
	{
		cursor.y--;
		cursor.x = last_print_screen_data->data[cursor.y].index;
		return cursor;
	}
	cursor.x--;
	return cursor;

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
	return last_print_screen_data->data[cursor_pos.y].index != -1 && last_print_screen_data->data[cursor_pos.y].index >= cursor_pos.x;
}

bool is_in_range(int l, int i, int r)
{
	return l <= i && i < r;
}

