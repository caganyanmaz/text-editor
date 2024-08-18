#include <string.h>
#include <stdbool.h>
#include "definitions.h"
#include "error_handling.h"
#include "dynamic_buffer.h"


/* Private functions */

DynamicBuffer *dbuf_create()
{
	DynamicBuffer *obj = malloc(sizeof(DynamicBuffer));
	obj->darr = darr_create(sizeof(char));
	darr_add_single(obj->darr, "\0");
	return obj;
}

void dbuf_destroy(DynamicBuffer *obj)
{
	tassert(obj, "dbuf_destroy: obj is NULL");

	darr_destroy(obj->darr);
	free(obj);
}

void dbuf_addi(DynamicBuffer *obj, int i)
{
	tassert(obj, "dbuf_addi: obj is NULL");

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
	tassert(obj, "dbuf_addc: obj is NULL");
	tassert(c != NUL, "dbuf_addc: Trying to add NUL character");

	*(char *)darr_get(obj->darr, dbuf_get_size(obj)) = c;
	darr_add_single(obj->darr, "\0");
}

void dbuf_adds(DynamicBuffer *obj, size_t size, const char *s)
{
	tassert(obj, "dbuf_adds: obj is NULL");
	tassert(s[size] == NUL, "dbuf_adds: Given string must be terminated by NUL character (not included in the size)");

	darr_pop(obj->darr);
	darr_add_multiple(obj->darr, size+1, s);
}

void dbuf_insertc_to(DynamicBuffer *obj, size_t pos, char c)
{
	tassert(obj, "dbuf_insert_to: obj is NULL");
	tassert(0 <= pos && pos <= dbuf_get_size(obj), "dbuf_shift_right: not in range");
	tassert(c != NUL, "dbuf_insert_to: c is NUL");

	darr_insert_to(obj->darr, pos, &c);
}

void dbuf_shift_right(DynamicBuffer *obj, size_t pos)
{
	tassert(obj, "dbuf_shift_right: obj is NULL");
	tassert(0 <= pos && pos < dbuf_get_size(obj), "dbuf_shift_right: not in range");

	darr_shift_right(obj->darr, pos);
}

char *dbuf_get(DynamicBuffer *obj, size_t index)
{
	tassert(obj, "dbuf_get: obj is NULL");

	return darr_get(obj->darr, index);
}

const char *dbuf_getc(const DynamicBuffer *obj, size_t index)
{
	tassert(obj, "dbuf_getc: obj is NULL");

	return darr_getc(obj->darr, index);
}

void dbuf_clear(DynamicBuffer *obj)
{
	tassert(obj, "dbuf_clear: obj is NULL");

	darr_clear(obj->darr);
	darr_add_single(obj->darr, "\0");
}

size_t dbuf_get_size(const DynamicBuffer *obj)
{
	tassert(obj, "dbuf_get_size: obj is NULL");

	return obj->darr->length - 1; // Not including the NULL character 
}
