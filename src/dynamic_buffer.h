#pragma once
#include <stdlib.h>
#include "dynamic_array.h"

typedef struct
{
	DynamicArray *darr;
} DynamicBuffer;

DynamicBuffer *dbuf_create();
void dbuf_destroy(DynamicBuffer *obj);

void dbuf_addc(DynamicBuffer *obj, char c);
void dbuf_adds(DynamicBuffer *obj, size_t size, const char *s);
void dbuf_addi(DynamicBuffer *obj, int i);
void dbuf_clear(DynamicBuffer *obj);

void dbuf_shift_right(DynamicBuffer *obj, size_t pos);
void dbuf_shift_left(DynamicBuffer *obj, size_t start_pos);
void dbuf_insertc_to(DynamicBuffer *obj, size_t pos, char c);

void dbuf_popc(DynamicBuffer *obj);
void dbuf_popm(DynamicBuffer *obj, size_t count);

char *dbuf_get(DynamicBuffer *obj, size_t index);
const char *dbuf_getc(const DynamicBuffer *obj, size_t index);
char *dbuf_get_with_nul(DynamicBuffer *obj, size_t index);
const char *dbuf_get_with_nulc(const DynamicBuffer *obj, size_t index);

size_t dbuf_get_size(const DynamicBuffer *obj);
