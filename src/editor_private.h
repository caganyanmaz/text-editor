#pragma once
#include <stdio.h>
#include "definitions.h"
#include "dynamic_array.h"
#include "editor.h"
/* Private data types */
typedef struct
{
	size_t index;
	size_t file_row;
	size_t file_start_col;
} LastPrintRowData;

typedef struct
{
	size_t col_count;
	LastPrintRowData *data;
} LastPrintScreenData;

typedef struct
{
	DynamicArray *darr; // Dynamic Array of Dynamic buffers
} FileData;


/* Private function declarations */
void editor_render_rows(const FileData *fd, vec2 window_size, vec2 cursor_pos, int top_file_index, const IO_Interface *io_interface, LastPrintScreenData *last_print_screen_data);
vec2 editor_move_cursor(const LastPrintScreenData *last_print_screen_data, vec2 current_cursor, vec2 change);
vec2 editor_advance_cursor(vec2 cursor, vec2 window_size);
vec2 editor_retreat_cursor(vec2 cursor, const LastPrintScreenData *last_print_screen_data);
bool editor_is_cursor_in_range(const LastPrintScreenData *last_print_screen_data, vec2 cursor_pos);



bool is_in_range(int l, int i, int r);
int ctrl_key(char c);


