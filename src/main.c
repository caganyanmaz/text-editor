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
	setvbuf(stdout, NULL, _IONBF, 0);
#endif

	terminal_init();
	vec2 window_size = get_window_size();
	editor_init(window_size, terminal_interface);
	if (argc > 1)
	{
		printf("%s\n", argv[1]);
		editor_read_file(argv[1]);
	}
	editor_clear_screen();
	int user_input_res;
	do
	{
		user_input_res = editor_process_tick();
		editor_render_screen();
	} while (user_input_res == TEXT_EDITOR_SUCCESSFUL_READ);
	editor_clear_screen();
	editor_terminate();
	terminal_terminate();
	system("clear");
	return 0;
}

