#pragma once
#include <stdlib.h>
#include "definitions.h"

typedef struct
{
	int (*read_key) ();
	void (*render_row) (int row_index, size_t row_size, const char *data);
	void (*flush_output) ();
	void (*set_cursor_position) (int x, int y);
	void (*hide_cursor) ();
	void (*reveal_cursor) ();
	void (*clear_screen) ();
} IO_Interface;

void editor_init(vec2 window_size, IO_Interface io_interface);
void editor_read_file(const char *filename);
void editor_terminate();
void editor_clear_screen();
void editor_render_screen();
int editor_process_tick();

