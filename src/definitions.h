#pragma once
#define FORCE_INLINE inline __attribute__((always_inline))

#define ARROW_UP    1000
#define ARROW_DOWN  1001
#define ARROW_LEFT  1002
#define ARROW_RIGHT 1003

#define READ_ERROR -1
#define NUL '\0'

#define TERMIOS_ERROR -1
#define TERMIONS_SUCCESS 0

#define TEXT_EDITOR_SUCCESSFUL_READ 0
#define TEXT_EDITOR_EOF 1

typedef struct
{
	int x;
	int y;
} vec2;
