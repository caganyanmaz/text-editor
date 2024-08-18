#include <string.h>
#include <stdbool.h>
#include "definitions.h"
#include "error_handling.h"
#include "dynamic_buffer.h"


/* Private functions */

DynamicBuffer *dbuf_create()
{
	checkpoint()
	DynamicBuffer *obj = malloc(sizeof(DynamicBuffer));
	checkpoint()
	obj->darr = darr_create(sizeof(char));
	debugp(obj->darr);
	checkpoint()
	darr_add_single(obj->darr, "\0");
	checkpoint()
	return obj;
}

void dbuf_destroy(DynamicBuffer *obj)
{
	darr_destroy(obj->darr);
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
	int prev_size = dbuf_get_size(obj);
	while (i > 0)
	{
		dbuf_addc(obj, (i % 10) + '0');
		i /= 10;
	}
	for (int i = prev_size; i < (dbuf_get_size(obj) + prev_size) / 2; i++)
	{
		int j = dbuf_get_size(obj) - 1 - i + prev_size;
		char tmp = *(char *)darr_getc(obj->darr, i);
		*(char *)darr_get(obj->darr, i) = *(char *)darr_get(obj->darr, j);
		*(char *)darr_get(obj->darr, j) = tmp;
	}
}

void dbuf_addc(DynamicBuffer *obj, char c)
{
	checkpoint()
	tassert(c != NUL, "dbuf_addc: Trying to add NUL character");
	checkpoint()
	*(char *)darr_get(obj->darr, dbuf_get_size(obj)) = c;
	checkpoint()
	darr_add_single(obj->darr, "\0");
	checkpoint()
}

void dbuf_adds(DynamicBuffer *obj, size_t size, const char *s)
{
	tassert(s[size] == NUL, "dbuf_adds: Given string must be terminated by NUL character (not included in the size)");
	darr_pop(obj->darr);
	darr_add_multiple(obj->darr, size+1, s);
}

char *dbuf_get(DynamicBuffer *obj, size_t index)
{
	return darr_get(obj->darr, index);
}

const char *dbuf_getc(const DynamicBuffer *obj, size_t index)
{
	return darr_getc(obj->darr, index);
}

void dbuf_clear(DynamicBuffer *obj)
{
	darr_clear(obj->darr);
	darr_add_single(obj->darr, "\0");
}

size_t dbuf_get_size(const DynamicBuffer *obj)
{
	return obj->darr->length - 1; // Not including the NULL character 
}
