#include <stdlib.h>
#include <unistd.h>

#include "terminal.h"
#include "error_handling.h"

void throw_up(const char *c)
{
	terminal_terminate();
	//write(STDIN_FILENO, "\x1b[H", 3);
	perror(c);
	exit(1);
}
