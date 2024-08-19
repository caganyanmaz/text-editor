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

typedef struct _editor Editor;

Editor *editor_init(vec2 window_size, IO_Interface io_interface);
void editor_read_file(Editor *obj, const char *filename);
void editor_write_file(Editor *obj, const char *filename);
void editor_terminate(Editor *obj);
void editor_clear_screen(const Editor *obj);
void editor_render_screen(const Editor *obj);
int editor_process_tick(Editor *obj);

