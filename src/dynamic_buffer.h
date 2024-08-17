#pragma once
#include <stdlib.h>

typedef struct
{
	size_t size;
	size_t reserved;
	char *buf;
} DynamicBuffer;

DynamicBuffer *dbuf_create();
void dbuf_destroy(DynamicBuffer *obj);

void dbuf_addc(DynamicBuffer *obj, char c);
void dbuf_adds(DynamicBuffer *obj, size_t size, const char *s);
void dbuf_addi(DynamicBuffer *obj, int i);
void dbuf_clear(DynamicBuffer *obj);
