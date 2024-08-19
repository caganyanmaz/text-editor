#pragma once
#include <stdio.h>
#include "definitions.h"
#include "dynamic_array.h"
#include "editor.h"
/* Private data types */
typedef struct {
	size_t index;
	size_t file_row;
	size_t file_start_col;
} PrintRowData;

typedef struct
{
	size_t col_count;
	PrintRowData *data;
} PrintTextData;

typedef struct
{
	vec2 window_size;
	vec2 cursor_pos;
	size_t top_file_row;
} ScreenData;

typedef struct
{
	DynamicArray *darr; // Dynamic Array of Dynamic buffers
} FileData;

typedef struct _editor
{
	PrintTextData print_text_data;
	FileData file_data;
	ScreenData screen_data;
	IO_Interface io_interface;
} Editor;




/* Private function declarations */
void editor_update_print_text_data(PrintTextData *print_text_data, const FileData *fd, const ScreenData *sd);
PrintRowData editor_update_out_of_range_row_data();
PrintRowData editor_update_empty_cursor_row_data(const FileData *fd, size_t last_file_row);
PrintRowData editor_update_normal_row_data(const FileData *fd, const ScreenData *sd, size_t *old_file_row, size_t *old_file_col);

void editor_render_rows(const FileData *fd, const PrintTextData *print_text_data, const IO_Interface *io_interface);
//void editor_render_rows(const FileData *fd, vec2 window_size, vec2 cursor_pos, int top_file_index, const IO_Interface *io_interface, PrintTextData *last_print_screen_data);
//void editor_render_out_of_range_row(size_t i, PrintTextData *last_print_screen_data, const IO_Interface *io_interface);
//void editor_render_empty_cursor_row(size_t i, size_t file_row, const FileData *fd, PrintTextData* last_print_screen_data, const IO_Interface *io_interface);

int process_keypress(ScreenData *screen_data, FileData *file_data, const PrintTextData *print_text_data, const IO_Interface *io_interface);

void process_backspace(ScreenData *screen_data, FileData *file_data, const PrintTextData *print_text_data);
void process_carriage_return(ScreenData *screen_data, FileData *file_data, const PrintTextData *print_text_data);
void process_printable_character(ScreenData *screen_data, FileData *file_data, const PrintTextData *print_text_data, char c);


vec2 get_file_pos(const PrintTextData *print_text_data, vec2 cursor_pos);

vec2 editor_move_cursor(const PrintTextData *last_print_screen_data, vec2 current_cursor, vec2 change);
vec2 editor_advance_cursor(vec2 cursor, vec2 window_size);
vec2 editor_retreat_cursor(vec2 cursor, const PrintTextData *last_print_screen_data);
bool editor_is_cursor_in_range(const PrintTextData *last_print_screen_data, vec2 cursor_pos);



bool is_in_range(int l, int i, int r);
int ctrl_key(char c);


