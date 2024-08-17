#pragma once
#include "definitions.h"
#include "editor.h"
/* Private data types */
typedef struct
{
	int index;
	char *data;
} Row;


/* Private function declarations */
void editor_render_rows(Row *row, vec2 window_size, vec2 cursor_pos, const IO_Interface *io_interface);
void row_shift_right(Row *row, vec2 window_size, int pos);
void add_normal_character(Row *text, vec2 *cursor_pos, vec2 window_size, char c);
vec2 editor_move_cursor(const Row *text, vec2 window_size, vec2 current_cursor, vec2 change);


void editor_add_to_row(Row *row, vec2 window_size, char c);

bool is_in_range(int l, int i, int r);
int ctrl_key(char c);


