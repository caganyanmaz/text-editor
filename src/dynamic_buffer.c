#include <string.h>
#include <stdbool.h>
#include "definitions.h"
#include "error_handling.h"
#include "dynamic_buffer.h"

/* Definitions */
#define INITIAL_RESERVED 64

/* Private functions */
void dbuf_realloc(DynamicBuffer *obj);

DynamicBuffer *dbuf_create()
{
	DynamicBuffer *obj = calloc(1, sizeof(DynamicBuffer));
	obj->reserved = INITIAL_RESERVED;
	obj->size = 0;
	obj->buf = malloc(obj->reserved * sizeof(char));
	return obj;
}

void dbuf_destroy(DynamicBuffer *obj)
{
	free(obj->buf);
	free(obj);
}

void dbuf_addi(DynamicBuffer *obj, int i)
{
	if (i == 0)
	{
		dbuf_addc(obj, '0');
		return;
	}
	if (i < 0)
	{
		dbuf_addc(obj, '-');
		i = -i;
	}
	int prev_size = obj->size;
	while (i > 0)
	{
		dbuf_addc(obj, (i % 10) + '0');
		i /= 10;
	}
	for (int i = prev_size; i < (obj->size + prev_size) / 2; i++)
	{
		int j = obj->size - 1 - i + prev_size;
		char tmp = obj->buf[i];
		obj->buf[i] = obj->buf[j];
		obj->buf[j] = tmp;
	}
}

void dbuf_addc(DynamicBuffer *obj, char c)
{
	if (c == NUL)
	{
		throw_up("dbuf_addc: Trying to add NUL character");
	}
	if (obj->size == obj->reserved - 1)
	{
		obj->reserved <<= 1;
		dbuf_realloc(obj);
	}
	obj->buf[obj->size++] = c;
	obj->buf[obj->size] = '\0';
}

void dbuf_adds(DynamicBuffer *obj, size_t size, const char *s)
{
	for (int i = 0; i < size; i++)
	{
		dbuf_addc(obj, s[i]);
	}
	/*
	if (!size)

	{
		return;
	}
	bool reserved_changed = false;
	while (obj->size + size > obj->reserved)
	{
		obj->reserved <<= 1;
		reserved_changed = true;
	}
	if (reserved_changed)
	{
		dbuf_realloc(obj);
	}
	memcpy(&obj->buf[obj->size], s, size * sizeof(char));
	*/
}

void dbuf_clear(DynamicBuffer *obj)
{
	obj->size = 0;
}

void dbuf_realloc(DynamicBuffer *obj)
{
	char *new_buf = realloc(obj->buf, obj->reserved * sizeof(char));
	if (!new_buf)
	{
		throw_up("dbuf_realloc: realloc failed");
	}
	obj->buf = new_buf;
}
