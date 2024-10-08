#pragma once
#include <stdbool.h>
#include <stdio.h>
//#define CHECKPOINTS


#include "definitions.h"
//#define DEBUGGING

#ifdef DEBUGGING
#define debugi(x)  fprintf(stderr, "%s:%d - %s = %d\n", __FILE__, __LINE__, #x, x);
#define debugc(x)  fprintf(stderr, "%s:%d - %s = %c\n", __FILE__, __LINE__, #x, x);
#define debugs(x)  fprintf(stderr, "%s:%d - %s = %s\n", __FILE__, __LINE__, #x, x);
#define debugf(x)  fprintf(stderr, "%s:%d - %s = %f\n", __FILE__, __LINE__, #x, x);
#define debugp(x)  fprintf(stderr, "%s:%d - %s = %p\n", __FILE__, __LINE__, #x, x);
#define debuglu(x) fprintf(stderr, "%s:%d - %s = %lu\n", __FILE__, __LINE__, #x, x);
#else
#define debugi(...) {}
#define debugc(...) {}
#define debugs(...) {}
#define debugf(...) {}
#define debugp(...) {}
#define debuglu(...) {}
#endif

#ifdef CHECKPOINTS
#define checkpoint() { fprintf(stderr, "Checkpoint at %s:%d\n", __FILE__, __LINE__); }
#else
#define checkpoint() {}
#endif

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
