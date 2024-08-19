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


/* Function definitions */
Editor *editor_init(vec2 window_size, IO_Interface _io_interface)
{
	Editor *obj = malloc(sizeof(*obj));
	obj->screen_data.cursor_pos = (vec2){ .x = 0, .y = 0};
	obj->screen_data.top_file_row = 0;
	obj->screen_data.window_size = window_size;
	obj->file_data.darr = darr_create(sizeof(DynamicBuffer*));
	obj->io_interface = _io_interface;
	obj->print_text_data.col_count = obj->screen_data.window_size.y;
	obj->print_text_data.data = calloc(obj->print_text_data.col_count, sizeof(PrintRowData));
	return obj;
}

void editor_terminate(Editor *obj)
{
	for (size_t i = 0; i < obj->file_data.darr->length; i++)
	{
		dbuf_destroy(*(DynamicBuffer**)darr_get(obj->file_data.darr, i));
	}
	darr_destroy(obj->file_data.darr);
	free(obj->print_text_data.data);
}

void editor_read_file(Editor *obj, const char *filename)
{
	tassert(filename, "editor_read_file: filename is NULL");
	// Opening file
	FILE *fp = fopen(filename, "r");
	if (fp == NULL)
	{
		DynamicBuffer *dbuf = dbuf_create();
		darr_add_single(obj->file_data.darr, &dbuf);
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
		darr_add_single(obj->file_data.darr, &dbuf);
	}
	free(line);
	// Closing file
	fclose(fp);
}

void editor_write_file(Editor *obj, const char *filename)
{
	FILE *fp = fopen(filename, "w");
	for (size_t i = 0; i < darr_get_size(obj->file_data.darr); i++)
	{
		DynamicBuffer *dbuf = *(DynamicBuffer**)darr_get(obj->file_data.darr, i);
		// Dashes are for debugging: To clearly see where each file ends
		dbuf_addc(dbuf, '\n');
		fputs(dbuf_get_with_nulc(dbuf, 0), fp);
		dbuf_popc(dbuf);
	}
	fclose(fp);
}

void editor_render_screen(const Editor *obj)
{
	obj->io_interface.hide_cursor();
	obj->io_interface.clear_screen();
	editor_render_rows(&obj->file_data, &obj->print_text_data, &obj->io_interface);
	obj->io_interface.reveal_cursor();
	obj->io_interface.flush_output();
	obj->io_interface.set_cursor_position(obj->screen_data.cursor_pos.x, obj->screen_data.cursor_pos.y);
}


void editor_clear_screen(const Editor *obj)
{
	obj->io_interface.clear_screen();
	obj->io_interface.flush_output();
}

int ctrl_key(char c)
{
	return c & (0x1f);
}

void editor_render_rows(const FileData *fd, const PrintTextData *print_text_data, const IO_Interface *io_interface)
{
	for (size_t i = 0; i < print_text_data->col_count; i++)
	{
		if (print_text_data->data[i].index == -1)
		{
			io_interface->render_row(i, 1, "~");
			continue;
		}
		const DynamicBuffer *dbuf = *(DynamicBuffer**)darr_getc(fd->darr, print_text_data->data[i].file_row);
		const char *row_data      = dbuf_get_with_nulc(dbuf, print_text_data->data[i].file_start_col);
		size_t row_size           = print_text_data->data[i].index;
		io_interface->render_row(i, row_size, row_data);
	}
}


void editor_update_print_text_data(PrintTextData *print_text_data, const FileData *fd, const ScreenData *sd)
{
	size_t file_row = sd->top_file_row;
	size_t file_col = 0;
	for (int i = 0; i < sd->window_size.y; i++)
	{
		if (file_row >= darr_get_size(fd->darr) && sd->cursor_pos.y != i)
		{
			print_text_data->data[i] = editor_update_out_of_range_row_data();
			continue;
		}
		if (file_row >= darr_get_size(fd->darr))
		{
			print_text_data->data[i] = editor_update_empty_cursor_row_data(fd, file_row);
			continue;
		}
		print_text_data->data[i] = editor_update_normal_row_data(fd, sd, &file_row, &file_col);
	}
}

PrintRowData editor_update_normal_row_data(const FileData *fd, const ScreenData *sd, size_t *old_file_row, size_t *old_file_col)
{
	size_t file_row = *old_file_row;
	size_t file_col = *old_file_col;
	const DynamicBuffer *current_line_data = *(DynamicBuffer **)darr_getc(fd->darr, file_row);
	size_t row_render_size = dbuf_get_size(current_line_data) - file_col;
	// Continue printing current file
	if (row_render_size > sd->window_size.x) 
	{
		row_render_size = sd->window_size.x;
	}
	PrintRowData res = (PrintRowData) { .index = row_render_size, .file_row = file_row, .file_start_col = file_col };
	file_col += row_render_size;
	if (file_col == dbuf_get_size(current_line_data))
	{
		file_row++;
		file_col = 0;
	}
	*old_file_row = file_row;
	*old_file_col = file_col;
	return res;
}

PrintRowData editor_update_out_of_range_row_data()
{
	return (PrintRowData) { .index = -1, .file_row = -1, .file_start_col = -1 };
}

PrintRowData editor_update_empty_cursor_row_data(const FileData *fd, size_t last_file_row)
{
	const DynamicBuffer *last_line_data = *(DynamicBuffer**)darr_getc(fd->darr, last_file_row);
	size_t last_line_size = dbuf_get_size(last_line_data);
	return (PrintRowData) { .index = 0, .file_row = last_file_row, .file_start_col = last_line_size };
}


int editor_process_tick(Editor *obj)
{
	int res = process_keypress(&obj->screen_data, &obj->file_data, &obj->print_text_data, &obj->io_interface);	
	editor_update_print_text_data(&obj->print_text_data, &obj->file_data, &obj->screen_data);
	return res;
}

int process_keypress(ScreenData *screen_data, FileData *file_data, const PrintTextData *print_text_data, const IO_Interface *io_interface)
{
	int c = io_interface->read_key();
	switch(c)
	{
		case QUIT_KEY:
			return TEXT_EDITOR_EOF;
		case NUL:
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case ARROW_UP:
			screen_data->cursor_pos = editor_move_cursor(print_text_data, screen_data->cursor_pos, (vec2) {.x = 0, .y = -1});
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case ARROW_DOWN:
			screen_data->cursor_pos = editor_move_cursor(print_text_data, screen_data->cursor_pos, (vec2) {.x = 0, .y = 1});
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case ARROW_LEFT:
			screen_data->cursor_pos = editor_move_cursor(print_text_data, screen_data->cursor_pos, (vec2) {.x = -1, .y = 0});
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case ARROW_RIGHT:
			screen_data->cursor_pos = editor_move_cursor(print_text_data, screen_data->cursor_pos, (vec2) {.x = 1, .y = 0});
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case BACKSPACE:
			process_backspace(screen_data, file_data, print_text_data);
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case CARRIAGE_RETURN:
			process_carriage_return(screen_data, file_data, print_text_data);
			return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	if (iscntrl(c) || c >= 256)
	{
		return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	process_printable_character(screen_data, file_data, print_text_data, c);
	return TEXT_EDITOR_SUCCESSFUL_READ;
}

void process_backspace(ScreenData *screen_data, FileData *file_data, const PrintTextData *print_text_data)
{
	vec2 file_pos = get_file_pos(print_text_data, screen_data->cursor_pos);
	size_t file_row = file_pos.y;
	size_t file_col = file_pos.x;
	// If we're at the start of file, we don't do anything
	if (file_col == 0 && file_row == 0)
	{
		return;
	}
	// If we're at the start of a line (that's not the start of file), we append the current line to the previous line
	DynamicBuffer *current_row = *(DynamicBuffer**)darr_get(file_data->darr, file_row);
	if (file_col == 0)
	{
		DynamicBuffer *prev_row  = *(DynamicBuffer**)darr_get(file_data->darr, file_row - 1);
		size_t current_row_size  = dbuf_get_size(current_row);
		const char *current_row_s = dbuf_get_with_nulc(current_row, 0);
		dbuf_adds(prev_row, current_row_size, current_row_s);
		free(current_row);
		darr_shift_left(file_data->darr, file_row);
	}
	// else we remove one character from the line (previous character)
	else
	{
		dbuf_shift_left(current_row, file_col - 1); 
	}
	screen_data->cursor_pos = editor_retreat_cursor(screen_data->cursor_pos, print_text_data);
	// Reverting the cursor position by one
	return;
}


void process_carriage_return(ScreenData *screen_data, FileData *file_data, const PrintTextData *print_text_data)
{
	vec2 file_pos = get_file_pos(print_text_data, screen_data->cursor_pos);
	size_t file_row = file_pos.y;
	size_t file_col = file_pos.x;
	// Create new line
	DynamicBuffer *current_row = *(DynamicBuffer**)darr_get(file_data->darr, file_row);
	DynamicBuffer *new_row     = dbuf_create();
	const char *current_row_text_at_cursor_right = dbuf_get_with_nulc(current_row, file_col);
	size_t current_row_text_at_cursor_right_size = dbuf_get_size(current_row) - file_col;
	dbuf_adds(new_row, current_row_text_at_cursor_right_size, current_row_text_at_cursor_right);
	dbuf_popm(current_row, current_row_text_at_cursor_right_size);
	darr_insert_to(file_data->darr, file_row + 1, &new_row);
	screen_data->cursor_pos.x = 0;
	screen_data->cursor_pos.y++;
}

void process_printable_character(ScreenData *screen_data, FileData *file_data, const PrintTextData *print_text_data, char c)
{
	vec2 file_pos = get_file_pos(print_text_data, screen_data->cursor_pos);
	size_t file_row = file_pos.y;
	size_t file_col = file_pos.x;
	dbuf_insertc_to(*(DynamicBuffer**)darr_get(file_data->darr, file_row), file_col, c);
	screen_data->cursor_pos = editor_advance_cursor(screen_data->cursor_pos, screen_data->window_size);
}

vec2 get_file_pos(const PrintTextData *print_text_data, vec2 cursor_pos)
{
	size_t file_row = print_text_data->data[cursor_pos.y].file_row;
	size_t file_col = cursor_pos.x + print_text_data->data[cursor_pos.y].file_start_col;
	return (vec2){ .x = file_col, .y = file_row };
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

vec2 editor_retreat_cursor(vec2 cursor, const PrintTextData *last_print_text_data)
{
	if (cursor.x == 0 && cursor.y == 0)
	{
		// TODO: Need to add g_top_file change when cursor hits window beginning
		// Right now doing nothing
		return cursor;
	}
	if (cursor.x == 0)
	{
		cursor.y--;
		cursor.x = last_print_text_data->data[cursor.y].index;
		return cursor;
	}
	cursor.x--;
	return cursor;

}

vec2 editor_move_cursor(const PrintTextData *last_print_text_data, vec2 current_cursor, vec2 change)
{
	vec2 new_cursor = 
	{ 
		.x = current_cursor.x + change.x,
		.y = current_cursor.y + change.y
	};
	if (!editor_is_cursor_in_range(last_print_text_data, new_cursor))
	{
		return current_cursor;

	}
	return new_cursor;
}

bool editor_is_cursor_in_range(const PrintTextData *last_print_text_data, vec2 cursor_pos)
{
	if (!is_in_range(0, cursor_pos.y, last_print_text_data->col_count))
	{
		return false;
	}
	return last_print_text_data->data[cursor_pos.y].index != -1 && last_print_text_data->data[cursor_pos.y].index >= cursor_pos.x;
}

bool is_in_range(int l, int i, int r)
{
	return l <= i && i < r;
}

