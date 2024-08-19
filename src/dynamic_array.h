#pragma once
#include <stdlib.h>

typedef struct
{
	size_t unit_size;
	size_t length;
	size_t reserved_length;
	void *arr;
} DynamicArray;

DynamicArray *darr_create(size_t unit_size);
void darr_destroy(DynamicArray *obj);

const void *darr_getc(const DynamicArray *obj, size_t i);
void *darr_get(DynamicArray *obj, size_t i);

void darr_add_single(DynamicArray *obj, const void *val);
void darr_add_multiple(DynamicArray *obj, size_t count, const void *vals);

void darr_pop(DynamicArray *obj);

void darr_clear(DynamicArray *obj);

size_t darr_get_size(const DynamicArray *obj);

void darr_shift_right(DynamicArray *obj, size_t start_pos);
void darr_shift_left(DynamicArray *obj, size_t start_pos);
void darr_insert_to(DynamicArray *obj, size_t pos, const void *val);



