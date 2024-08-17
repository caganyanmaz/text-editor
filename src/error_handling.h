#pragma once
#include <stdbool.h>
#include <stdio.h>


#include "definitions.h"

void throw_up(const char *c);

FORCE_INLINE void handle_error(int err, const char *msg)
{
	if (err == EOF)
	{
		throw_up(msg);
	}
}

FORCE_INLINE void tassert(bool cond, const char *msg)
{
	if (!cond)
	{
		throw_up(msg);
	}
}
