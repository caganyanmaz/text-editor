#pragma once
#include <stdio.h>
#include "definitions.h"
#include "dynamic_array.h"
#include "dynamic_buffer.h"
#include "editor.h"

#define MX_SEARCH_TEXT_LENGTH 1024
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

typedef struct
{
	size_t searched_text_index;
	size_t match_index;
	char searched_text[MX_SEARCH_TEXT_LENGTH];
	DynamicArray *matches;
} SearchData;

typedef struct _editor
{
	int state;
	PrintTextData print_text_data;
	FileData file_data;
	ScreenData screen_data;
	IO_Interface io_interface;
	SearchData search_data;
} Editor;

/* Private function declarations */
void editor_update_print_text_data(PrintTextData *print_text_data, const FileData *fd, const ScreenData *sd);
PrintRowData editor_update_out_of_range_row_data();
PrintRowData editor_update_empty_cursor_row_data(const FileData *fd, size_t last_file_row);
PrintRowData editor_update_normal_row_data(const FileData *fd, const ScreenData *sd, size_t *old_file_row, size_t *old_file_col);

void editor_render_rows(const FileData *fd, const PrintTextData *print_text_data, const IO_Interface *io_interface);

int editor_process_keypress_for_write_state(ScreenData *screen_data, FileData *file_data, const PrintTextData *print_text_data, int c);
int editor_process_keypress_for_search_state(SearchData *search_data, ScreenData *screen_data, const FileData *file_data, int c);

void adjust_top_file_row(ScreenData *screen_data, const FileData *file_data);


void process_backspace(ScreenData *screen_data, FileData *file_data, const PrintTextData *print_text_data);
void process_carriage_return(ScreenData *screen_data, FileData *file_data, const PrintTextData *print_text_data);
void process_printable_character(ScreenData *screen_data, FileData *file_data, const PrintTextData *print_text_data, char c);

vec2 editor_move_cursor_to_next_line_beginning(vec2 cursor_pos);
vec2 editor_move_cursor(const FileData *file_data, vec2 current_cursor, vec2 change);
bool editor_is_cursor_in_range(const FileData *file_data, vec2 cursor_pos);

vec2 editor_advance_cursor(vec2 cursor);
vec2 editor_retreat_cursor(vec2 cursor, const FileData *file_data);

vec2 get_real_cursor_position(const ScreenData *screen_data, const PrintTextData *file_data);

size_t shift_top_file_row(size_t top_file, int change, size_t file_row_count);



bool is_in_range(int l, int i, int r);
int ctrl_key(char c);

bool is_a_printable_character(int c);


int editor_process_state_tick_result(int* state, int res);

void editor_change_match_index(SearchData *search_data, ScreenData *screen_data, int change);

void editor_process_carriage_return_for_search_state(SearchData *search_data, ScreenData *screen_data, const FileData *file_data);


void editor_process_backspace_for_search_state(SearchData *search_data);

vec2 editor_get_match_pos(const SearchData* search_data);


void editor_process_arrow_for_search_state(SearchData *search_data, ScreenData *screen_data, int change);


void editor_process_line_matches(SearchData *search_data, const DynamicBuffer *line, int line_index);
