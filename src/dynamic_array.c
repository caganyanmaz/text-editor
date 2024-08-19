#include <string.h>
#include "error_handling.h"
#include "dynamic_array.h"

/* Definitions */
#define INITIAL_RESERVED 64

/* Private Functions */
void darr_realloc(DynamicArray *obj);
void *darr_get_byte(DynamicArray *obj, size_t byte_index);
const void *darr_get_bytec(const DynamicArray *obj, size_t byte_index);
size_t darr_get_byte_index(const DynamicArray *obj, size_t index);

DynamicArray *darr_create(size_t unit_size)
{
	DynamicArray *obj = malloc(sizeof(DynamicArray));
	obj->unit_size = unit_size;
	obj->length = 0;
	obj->reserved_length = 64;
	obj->arr = malloc(obj->unit_size * obj->reserved_length);
	return obj;
}

void darr_destroy(DynamicArray *obj)
{
	tassert(obj, "darr_destroy: obj is NULL");

	free(obj->arr);
	free(obj);
}

const void *darr_getc(const DynamicArray *obj, size_t i)
{
	tassert(obj, "darr_getc: obj is NULL");
	tassert(i < obj->length, "darr_getc: index out of range");

	size_t byte_index = darr_get_byte_index(obj, i);
	return darr_get_bytec(obj, byte_index);
}

void *darr_get(DynamicArray *obj, size_t i)
{
	tassert(obj, "darr_get: obj is NULL");
	tassert(i < obj->length, "darr_get: index out of range");

	size_t byte_index = darr_get_byte_index(obj, i);
	return darr_get_byte(obj, byte_index);
}

void darr_add_single(DynamicArray *obj, const void *val)
{
	tassert(obj, "darr_add_single: obj is NULL");
	tassert(val, "darr_add_single: Tried adding nullptr");

	darr_add_multiple(obj, 1, val);
}

void darr_add_multiple(DynamicArray *obj, size_t count, const void *vals)
{
	tassert(obj, "darr_add_multiple: obj is NULL");
	tassert(vals, "darr_add_single: vals is NULL");

	while (obj->length + count > obj->reserved_length)
	{
		obj->reserved_length <<= 1;
	}
	darr_realloc(obj);
	size_t dest_byte_index = darr_get_byte_index(obj, obj->length);
	memcpy(darr_get_byte(obj, dest_byte_index), vals, count * obj->unit_size);
	obj->length += count;
}

void darr_pop(DynamicArray *obj)
{
	tassert(obj, "darr_pop: obj is NULL");
	tassert(obj->length > 0, "darr_pop: length is empty");

	obj->length--;
}

void darr_clear(DynamicArray *obj)
{
	tassert(obj, "darr_clear: obj is NULL");

	obj->length = 0;
}

size_t darr_get_byte_index(const DynamicArray *obj, size_t index)
{
	tassert(obj, "darr_get_byte_index: obj is NULL");

	return index * obj->unit_size;
}

void darr_realloc(DynamicArray *obj)
{
	tassert(obj, "darr_realloc: obj is NULL");
	tassert(obj->length <= obj->reserved_length, "darr_realloc: reserved_length smaller than data length");

	obj->arr = realloc(obj->arr, obj->reserved_length * obj->unit_size);
}

size_t darr_get_size(const DynamicArray *obj)
{
	tassert(obj, "darr_get_size: obj is NULL");

	return obj->length;
}

void darr_insert_to(DynamicArray *obj, size_t pos, const void *val)
{
	tassert(obj, "darr_insert_to: obj is NULL");
	tassert(0 <= pos && pos <= darr_get_size(obj), "darr_insert_to: pos is out of range");
	tassert(val, "darr_insert_to: val is NULL");

	if (pos == darr_get_size(obj))
	{
		darr_add_single(obj, val);
		return;
	}
	darr_shift_right(obj, pos);
	memcpy(darr_get(obj, pos), val, obj->unit_size);
}

void darr_shift_right(DynamicArray *obj, size_t pos)
{
	tassert(obj, "darr_shift_right: obj is NULL");
	tassert(0 <= pos && pos < darr_get_size(obj), "darr_shfit_right: position out of range");
	darr_add_single(obj, darr_getc(obj, darr_get_size(obj) - 1));
	for (int i = obj->length - 1; i > pos; i--)
	{
		memcpy(darr_get(obj, i), darr_getc(obj, i-1), obj->unit_size);
	}
}

void *darr_get_byte(DynamicArray *obj, size_t byte_index)
{
	return &((char *)obj->arr)[byte_index];
}

const void *darr_get_bytec(const DynamicArray *obj, size_t byte_index)
{	
	return &((const char *)obj->arr)[byte_index];
}
