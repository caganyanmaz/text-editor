#include <string.h>
#include "error_handling.h"
#include "dynamic_array.h"

/* Definitions */
#define INITIAL_RESERVED 64

/* Private Functions */
void darr_realloc(DynamicArray *obj);
size_t darr_get_byte_index(const DynamicArray *obj, size_t index);

DynamicArray *darr_create(size_t unit_size)
{
	checkpoint()
	debugi((int)unit_size);
	checkpoint()
	debugi((int)sizeof(DynamicArray))
	DynamicArray *obj = malloc(sizeof(DynamicArray));
	debugp(obj);
	checkpoint()
	obj->unit_size = unit_size;
	checkpoint()
	obj->length = 0;
	checkpoint()
	obj->reserved_length = 64;
	checkpoint()
	obj->arr = malloc(obj->unit_size * obj->reserved_length);
	debugi((int)(obj->unit_size * obj->reserved_length))
	debugp(obj->arr);
	checkpoint()
	return obj;
}

void darr_destroy(DynamicArray *obj)
{
	free(obj->arr);
	free(obj);
}

const void *darr_getc(const DynamicArray *obj, size_t i)
{
	tassert(i < obj->length, "darr_getc: index out of range");
	return &((const char *)(obj->arr))[i];
}

void *darr_get(DynamicArray *obj, size_t i)
{
	tassert(i < obj->length, "darr_get: index out of range");
	return &((char *)obj->arr)[i]; 
}

void darr_add_single(DynamicArray *obj, const void *val)
{
	checkpoint()
	tassert(val, "darr_add_single: Tried adding nullptr");
	checkpoint()
	darr_add_multiple(obj, 1, val);
	checkpoint()
}

void darr_add_multiple(DynamicArray *obj, size_t count, const void *vals)
{
	tassert(vals, "darr_add_single: Tried adding nullptr");
	checkpoint()
	while (obj->length + count > obj->reserved_length)
	{
		obj->reserved_length <<= 1;
	}
	checkpoint()
	darr_realloc(obj);
	checkpoint()
	memcpy(&((char *)obj->arr)[darr_get_byte_index(obj, obj->length)], vals, count * obj->unit_size);
	obj->length += count;
	checkpoint()
}

void darr_pop(DynamicArray *obj)
{
	obj->length--;
}

void darr_clear(DynamicArray *obj)
{
	obj->length = 0;
}

size_t darr_get_byte_index(const DynamicArray *obj, size_t index)
{
	return index * obj->unit_size;
}

void darr_realloc(DynamicArray *obj)
{
	tassert(obj->length <= obj->reserved_length, "darr_realloc: reserved_length smaller than data length");
	obj->arr = realloc(obj->arr, obj->reserved_length * obj->unit_size);
}

