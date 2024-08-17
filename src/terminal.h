#pragma once
#include <stdlib.h>
#include "definitions.h"
void terminal_init();
void terminal_terminate();
vec2 get_window_size();

void terminal_clear_screen();
void terminal_render_row(int row_id, size_t size, const char *row);
int  terminal_read_key();
void terminal_flush_output();
void terminal_set_cursor_position(int x, int y);
void terminal_hide_cursor();
void terminal_reveal_cursor();

