#pragma once
#include <x86_64-linux-gnu/sys/ttydefaults.h>
#define FORCE_INLINE inline __attribute__((always_inline))

#define EDITOR_WRITE_STATE  0
#define EDITOR_SEARCH_STATE 1


#define QUIT_KEY        CTRL('q')
#define ARROW_UP        1000
#define ARROW_DOWN      1001
#define ARROW_LEFT      1002
#define ARROW_RIGHT     1003
#define CARRIAGE_RETURN '\r'
#define BACKSPACE        127

#define READ_ERROR -1
#define NUL '\0'

#define TERMIOS_ERROR -1
#define TERMIONS_SUCCESS 0

#define TEXT_EDITOR_SUCCESSFUL_READ        0
#define TEXT_EDITOR_EOF                    1
#define TEXT_EDITOR_SWITCH_TO_SEARCH_STATE 2
#define TEXT_EDITOR_SWITCH_TO_WRITE_STATE  3

typedef struct
{
	int x;
	int y;
} vec2;
