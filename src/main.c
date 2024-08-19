/*  Includes */
#include "definitions.h"
#include "error_handling.h"
#include "terminal.h"
#include "editor.h"

static IO_Interface terminal_interface = 
{
	.read_key = terminal_read_key,
	.render_row = terminal_render_row,
	.flush_output = terminal_flush_output,
	.set_cursor_position = terminal_set_cursor_position,
	.hide_cursor = terminal_hide_cursor,
	.reveal_cursor = terminal_reveal_cursor,
	.clear_screen = terminal_clear_screen,
};

int main(int argc, char **argv) 
{
#ifdef DEBUGGING
	setvbuf(stderr, NULL, _IONBF, 0);
#endif
	if (argc < 2)
	{
		printf("You need to include the file name as the second argument\n");
		return 0;
	}
	terminal_init();
	vec2 window_size = get_window_size();
	Editor *editor = editor_create(window_size, terminal_interface);
	editor_read_file(editor, argv[1]);
	editor_clear_screen(editor);
	int user_input_res;
	do
	{
		user_input_res = editor_process_tick(editor);
		editor_render_screen(editor);
	} while (user_input_res == TEXT_EDITOR_SUCCESSFUL_READ);
	editor_write_file(editor, argv[1]);
	editor_clear_screen(editor);
	editor_destroy(editor);
	terminal_terminate();
	system("clear");
	return 0;
}

