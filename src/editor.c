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
Editor *editor_create(vec2 window_size, IO_Interface _io_interface)
{
	Editor *obj = malloc(sizeof(*obj));
	obj->screen_data.cursor_pos = (vec2){ .x = 0, .y = 0};
	obj->screen_data.top_file_row = 0;
	obj->screen_data.window_size = window_size;
	obj->screen_data.window_size.y--;
	obj->file_data.darr = darr_create(sizeof(DynamicBuffer*));
	obj->io_interface = _io_interface;
	obj->print_text_data.col_count = obj->screen_data.window_size.y;
	obj->print_text_data.data = calloc(obj->print_text_data.col_count, sizeof(PrintRowData));
	obj->state = EDITOR_WRITE_STATE;
	obj->search_data.searched_text_index = 0;
	obj->search_data.match_index = 0;
	obj->search_data.searched_text[0] = NUL;
	obj->search_data.matches = darr_create(sizeof(vec2));
	return obj;
}

void editor_destroy(Editor *obj)
{
	for (size_t i = 0; i < obj->file_data.darr->length; i++)
	{
		dbuf_destroy(*(DynamicBuffer**)darr_get(obj->file_data.darr, i));
	}
	darr_destroy(obj->file_data.darr);
	free(obj->print_text_data.data);
	free(obj->search_data.matches);
	free(obj);
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
	if (obj->state == EDITOR_SEARCH_STATE)
	{
		editor_render_search_bar(&obj->search_data, &obj->print_text_data, &obj->io_interface);
	}
	obj->io_interface.reveal_cursor();
	obj->io_interface.flush_output();
	vec2 real_cursor_position = get_real_cursor_position(&obj->screen_data, &obj->print_text_data);
	obj->io_interface.set_cursor_position(real_cursor_position.x, real_cursor_position.y);
}

void editor_render_search_bar(const SearchData *search_data, const PrintTextData *print_text_data, const IO_Interface *io_interface)
{

	const char *prefix = "Search: ";
	size_t prefix_len  = strlen(prefix);
	size_t msg_len     = search_data->searched_text_index + prefix_len;
	char *msg = malloc(msg_len * sizeof(char));
	strncpy(msg, prefix, prefix_len);
	strncpy(msg + prefix_len, search_data->searched_text, search_data->searched_text_index);
	io_interface->render_row(print_text_data->col_count, msg_len, msg);
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

vec2 get_real_cursor_position(const ScreenData *screen_data, const PrintTextData *print_text_data)
{
	for (int i = 0; i < print_text_data->col_count; i++)
	{
		size_t file_row       = print_text_data->data[i].file_row;
		size_t file_start_col = print_text_data->data[i].file_start_col;
		size_t file_end_col   = file_start_col + print_text_data->data[i].index;
		if (screen_data->cursor_pos.y == file_row && is_in_range(file_start_col, screen_data->cursor_pos.x, file_end_col+1))
		{
			return (vec2) { .x = screen_data->cursor_pos.x - file_start_col, .y = i };
		}
	}
	throw_up("Couldn't find cursor");
	return (vec2) {.x = -1, .y = -1};
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
		if (file_row >= darr_get_size(fd->darr) && sd->cursor_pos.y != file_row)
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
	int res;
	int c = obj->io_interface.read_key();
	if (obj->state == EDITOR_WRITE_STATE)
	{
		res = editor_process_keypress_for_write_state(&obj->screen_data, &obj->file_data, &obj->print_text_data, c);	
	}
	else if (obj->state == EDITOR_SEARCH_STATE)
	{
		res = editor_process_keypress_for_search_state(&obj->search_data, &obj->screen_data, &obj->file_data, c);
	}
	adjust_top_file_row(&obj->screen_data, &obj->file_data);
	editor_update_print_text_data(&obj->print_text_data, &obj->file_data, &obj->screen_data);
	return editor_process_state_tick_result(&obj->state, res);
}

int editor_process_state_tick_result(int* state, int res)
{
	switch (res)
	{
		case TEXT_EDITOR_SWITCH_TO_SEARCH_STATE:
			*state = EDITOR_SEARCH_STATE;
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case TEXT_EDITOR_SWITCH_TO_WRITE_STATE:
			*state = EDITOR_WRITE_STATE;
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case TEXT_EDITOR_EOF:
			return TEXT_EDITOR_EOF;
	}
	return TEXT_EDITOR_SUCCESSFUL_READ;
}

int editor_process_keypress_for_search_state(SearchData *search_data, ScreenData *screen_data, const FileData *file_data, int c) 
{
	switch (c)
	{
		case QUIT_KEY:
			return TEXT_EDITOR_EOF;
		case NUL:
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case CTRL('X'):
			return TEXT_EDITOR_SWITCH_TO_WRITE_STATE;
		case BACKSPACE:
			editor_process_backspace_for_search_state(search_data);
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case CARRIAGE_RETURN:
			editor_process_carriage_return_for_search_state(search_data, screen_data, file_data);
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case ARROW_UP:
			editor_process_arrow_for_search_state(search_data, screen_data, -1);
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case ARROW_DOWN:
			editor_process_arrow_for_search_state(search_data, screen_data, 1);
			return TEXT_EDITOR_SUCCESSFUL_READ;
	}
	if (is_a_printable_character(c))
	{
		search_data->searched_text[search_data->searched_text_index++] = c;
	}
	return TEXT_EDITOR_SUCCESSFUL_READ;
}

void editor_process_backspace_for_search_state(SearchData *search_data)
{
	if (search_data->searched_text_index > 0)
	{
		search_data->searched_text_index--;
	}
}

void editor_process_carriage_return_for_search_state(SearchData *search_data, ScreenData *screen_data, const FileData *file_data)
{
	darr_clear(search_data->matches);
	search_data->match_index = 0;
	for (int i = 0; i < darr_get_size(file_data->darr); i++)
	{
		const DynamicBuffer *current_line = *(const DynamicBuffer**)darr_getc(file_data->darr, i);
		editor_process_line_matches(search_data, current_line, i);
	}
	if (darr_get_size(search_data->matches) == 0)
	{
		return;
	}
	screen_data->cursor_pos = editor_get_match_pos(search_data);
}

void editor_process_line_matches(SearchData *search_data, const DynamicBuffer *line, int line_index)
{		
	for (int j = 0; j + search_data->searched_text_index <= dbuf_get_size(line); j++)
	{
		int res = strncmp(dbuf_getc(line, j), search_data->searched_text, search_data->searched_text_index);
		if (!res)
		{
			darr_add_single(search_data->matches, &((vec2) {.x = j, .y = line_index}));
		}
	}
}

void editor_process_arrow_for_search_state(SearchData *search_data, ScreenData *screen_data, int change)
{
	if (darr_get_size(search_data->matches) == 0)
	{
		return;
	}
	editor_change_match_index(search_data, screen_data, change);
	screen_data->cursor_pos = editor_get_match_pos(search_data);
}

void editor_change_match_index(SearchData *search_data, ScreenData *screen_data, int change)
{
	size_t match_count = darr_get_size(search_data->matches);
	search_data->match_index = ((search_data->match_index + change) + match_count) % match_count;
}

vec2 editor_get_match_pos(const SearchData* search_data)
{
	return *(const vec2*)darr_getc(search_data->matches, search_data->match_index);
}

int editor_process_keypress_for_write_state(ScreenData *screen_data, FileData *file_data, const PrintTextData *print_text_data, int c)
{
	switch(c)
	{
		case QUIT_KEY:
			return TEXT_EDITOR_EOF;
		case NUL:
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case ARROW_UP:
			screen_data->cursor_pos = editor_move_cursor(file_data, screen_data->cursor_pos, (vec2) {.x = 0, .y = -1});
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case ARROW_DOWN:
			screen_data->cursor_pos = editor_move_cursor(file_data, screen_data->cursor_pos, (vec2) {.x = 0, .y = 1});
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case ARROW_LEFT:
			screen_data->cursor_pos = editor_move_cursor(file_data, screen_data->cursor_pos, (vec2) {.x = -1, .y = 0});
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case ARROW_RIGHT:
			screen_data->cursor_pos = editor_move_cursor(file_data, screen_data->cursor_pos, (vec2) {.x = 1, .y = 0});
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case BACKSPACE:
			process_backspace(screen_data, file_data, print_text_data);
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case CARRIAGE_RETURN:
			process_carriage_return(screen_data, file_data, print_text_data);
			return TEXT_EDITOR_SUCCESSFUL_READ;
		case CTRL('f'):
			return TEXT_EDITOR_SWITCH_TO_SEARCH_STATE;
	}
	if (is_a_printable_character(c))
	{
		process_printable_character(screen_data, file_data, print_text_data, c);
	}
	return TEXT_EDITOR_SUCCESSFUL_READ;
}


void process_backspace(ScreenData *screen_data, FileData *file_data, const PrintTextData *print_text_data)
{
	size_t file_row = screen_data->cursor_pos.y;
	size_t file_col = screen_data->cursor_pos.x;
	// If we're at the start of file, we don't do anything
	if (file_col == 0 && file_row == 0)
	{
		return;
	}
	screen_data->cursor_pos = editor_retreat_cursor(screen_data->cursor_pos, file_data);
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
	// Reverting the cursor position by one
	return;
}


void process_carriage_return(ScreenData *screen_data, FileData *file_data, const PrintTextData *print_text_data)
{
	size_t file_row = screen_data->cursor_pos.y;
	size_t file_col = screen_data->cursor_pos.x;
	// Create new line
	DynamicBuffer *current_row = *(DynamicBuffer**)darr_get(file_data->darr, file_row);
	DynamicBuffer *new_row     = dbuf_create();
	const char *current_row_text_at_cursor_right = dbuf_get_with_nulc(current_row, file_col);
	size_t current_row_text_at_cursor_right_size = dbuf_get_size(current_row) - file_col;

	dbuf_adds(new_row, current_row_text_at_cursor_right_size, current_row_text_at_cursor_right);
	dbuf_popm(current_row, current_row_text_at_cursor_right_size);
	darr_insert_to(file_data->darr, file_row + 1, &new_row);
	screen_data->cursor_pos = editor_move_cursor_to_next_line_beginning(screen_data->cursor_pos);
}

vec2 editor_move_cursor_to_next_line_beginning(vec2 cursor_pos)
{
	cursor_pos.x = 0;
	cursor_pos.y++;
	return cursor_pos;
}

void process_printable_character(ScreenData *screen_data, FileData *file_data, const PrintTextData *print_text_data, char c)
{
	size_t file_row = screen_data->cursor_pos.y;
	size_t file_col = screen_data->cursor_pos.x;
	dbuf_insertc_to(*(DynamicBuffer**)darr_get(file_data->darr, file_row), file_col, c);
	screen_data->cursor_pos = editor_advance_cursor(screen_data->cursor_pos);
}

void adjust_top_file_row(ScreenData *screen_data, const FileData *file_data)
{
	// If cursor is left behind (that's pretty easy because we just need to set the starting position to the starting line)
	if (screen_data->cursor_pos.y < screen_data->top_file_row)
	{
		screen_data->top_file_row = screen_data->cursor_pos.y;
		return;
	}
	// Add required lines until we reach cursor file (it will need to contain the entire lines)
	int lines_needed = 0;
	for (int i = screen_data->top_file_row; i <= screen_data->cursor_pos.y; i++)
	{
		const DynamicBuffer *current_line = *(const DynamicBuffer**)darr_getc(file_data->darr, i);
		lines_needed += 1 + (((int)dbuf_get_size(current_line) - 1) / screen_data->window_size.x);
	}
	for (; lines_needed > screen_data->window_size.y; screen_data->top_file_row++)
	{
		tassert(screen_data->top_file_row < screen_data->cursor_pos.y, "adjust_top_file_row: cursor line is too big"); 
		const DynamicBuffer *current_line = *(const DynamicBuffer**)darr_getc(file_data->darr, screen_data->top_file_row);
		lines_needed -= 1 + (((int)dbuf_get_size(current_line) - 1) / screen_data->window_size.x);
	}
}

size_t shift_top_file_row(size_t top_file, int change, size_t file_row_count)
{
	if (((int)top_file) + change < 0)
	{
		return 0;
	}
	// It's safe to add them up together since previous condition was false
	if (top_file + change > file_row_count)
	{
		return file_row_count;
	}
	return top_file + change;
}

vec2 editor_advance_cursor(vec2 cursor)
{
	cursor.x++;
	return cursor;
}

vec2 editor_retreat_cursor(vec2 cursor, const FileData *file_data)
{
	if (cursor.x == 0 && cursor.y == 0)
	{
		return cursor;
	}
	if (cursor.x == 0)
	{
		cursor.y--;
		DynamicBuffer *current_line = *(DynamicBuffer**)darr_getc(file_data->darr, cursor.y);
		cursor.x = dbuf_get_size(current_line);
		return cursor;
	}
	cursor.x--;
	return cursor;
}

vec2 editor_move_cursor(const FileData *file_data, vec2 current_cursor, vec2 change)
{
	vec2 new_cursor = 
	{ 
		.x = current_cursor.x + change.x,
		.y = current_cursor.y + change.y
	};
	if (!editor_is_cursor_in_range(file_data, new_cursor))
	{
		return current_cursor;

	}
	return new_cursor;
}

bool editor_is_cursor_in_range(const FileData *file_data, vec2 cursor_pos)
{
	if (!is_in_range(0, cursor_pos.y, darr_get_size(file_data->darr)))
	{
		return false;
	}
	const DynamicBuffer *cursor_line = *(DynamicBuffer**)darr_getc(file_data->darr, cursor_pos.y);
	return is_in_range(0, cursor_pos.x, dbuf_get_size(cursor_line) + 1);
}

bool is_in_range(int l, int i, int r)
{
	return l <= i && i < r;
}

bool is_a_printable_character(int c)
{
	return c >= 32 && c < 127;
}
