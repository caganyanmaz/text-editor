#include <gtest/gtest.h>
#include <limits.h>

extern "C" {
#include "../../src/dynamic_buffer.h"
}

// Test dbuf_create
TEST(DynamicBufferTest, Create) {
	DynamicBuffer *dbuf = dbuf_create();
	ASSERT_NE(dbuf, nullptr);
	ASSERT_EQ(dbuf->size, 0);
	ASSERT_EQ(dbuf->reserved, 64); // Assuming INITIAL_RESERVED is 64
	ASSERT_NE(dbuf->buf, nullptr);
	dbuf_destroy(dbuf);
}

// Test dbuf_destroy
TEST(DynamicBufferTest, Destroy) {
	DynamicBuffer *dbuf = dbuf_create();
	dbuf_destroy(dbuf);
	// No assertions here, just ensuring no segfault
}

// Test dbuf_addc
TEST(DynamicBufferTest, AddChar) {
	DynamicBuffer *dbuf = dbuf_create();
	dbuf_addc(dbuf, 'A');
	ASSERT_EQ(dbuf->size, 1);
	ASSERT_STREQ(dbuf->buf, "A");
	dbuf_destroy(dbuf);
}

TEST(DynamicBufferTest, AddMultipleChars) {
	DynamicBuffer *dbuf = dbuf_create();
	dbuf_addc(dbuf, 'H');
	dbuf_addc(dbuf, 'i');
	ASSERT_EQ(dbuf->size, 2);
	ASSERT_STREQ(dbuf->buf, "Hi");
	dbuf_destroy(dbuf);
}

TEST(DynamicBufferTest, AddCharRealloc) {
	DynamicBuffer *dbuf = dbuf_create();
	for (size_t i = 0; i < 100; ++i) {
		dbuf_addc(dbuf, 'x');
	}
	ASSERT_EQ(dbuf->size, 100);
	ASSERT_EQ(dbuf->reserved, 128); // Assuming reserved doubled after reaching 64
	dbuf_destroy(dbuf);
}

// Test dbuf_adds
TEST(DynamicBufferTest, AddString) {
	DynamicBuffer *dbuf = dbuf_create();
	dbuf_adds(dbuf, 5, "Hello");
	ASSERT_EQ(dbuf->size, 5);
	ASSERT_STREQ(dbuf->buf, "Hello");
	dbuf_destroy(dbuf);
}

TEST(DynamicBufferTest, AddStringRealloc) {
	DynamicBuffer *dbuf = dbuf_create();
	dbuf_adds(dbuf, 70, "This is a long string that will cause the buffer to reallocate.");
	ASSERT_EQ(dbuf->size, 70);
	ASSERT_EQ(dbuf->reserved, 128); // Assuming reserved doubled after 64
	ASSERT_STREQ(dbuf->buf, "This is a long string that will cause the buffer to reallocate.");
	dbuf_destroy(dbuf);
}

// Test dbuf_addi
TEST(DynamicBufferTest, AddIntegerPositive) {
	DynamicBuffer *dbuf = dbuf_create();
	dbuf_addi(dbuf, 12345);
	ASSERT_EQ(dbuf->size, 5);
	ASSERT_STREQ(dbuf->buf, "12345");
	dbuf_destroy(dbuf);
}

TEST(DynamicBufferTest, AddIntegerNegative) {
	DynamicBuffer *dbuf = dbuf_create();
	dbuf_addi(dbuf, -6789);
	ASSERT_EQ(dbuf->size, 5);
	ASSERT_STREQ(dbuf->buf, "-6789");
	dbuf_destroy(dbuf);
}

TEST(DynamicBufferTest, AddIntegerZero) {
	DynamicBuffer *dbuf = dbuf_create();
	dbuf_addi(dbuf, 0);
	ASSERT_EQ(dbuf->size, 1);
	ASSERT_STREQ(dbuf->buf, "0");
	dbuf_destroy(dbuf);
}

// Test dbuf_clear
TEST(DynamicBufferTest, Clear) {
	DynamicBuffer *dbuf = dbuf_create();
	dbuf_adds(dbuf, 5, "Hello");
	ASSERT_EQ(dbuf->size, 5);
	dbuf_clear(dbuf);
	ASSERT_EQ(dbuf->size, 0);
	ASSERT_STREQ(dbuf->buf, "");
	dbuf_destroy(dbuf);
}

// Test edge cases
TEST(DynamicBufferTest, AddEmptyString) {
	DynamicBuffer *dbuf = dbuf_create();
	dbuf_adds(dbuf, 0, "");
	ASSERT_EQ(dbuf->size, 0);
	ASSERT_STREQ(dbuf->buf, "");
	dbuf_destroy(dbuf);
}


TEST(DynamicBufferTest, AddIntegerMinValue) {
	DynamicBuffer *dbuf = dbuf_create();
	dbuf_addi(dbuf, INT_MIN);
	ASSERT_STREQ(dbuf->buf, "-2147483648");
	dbuf_destroy(dbuf);
}


TEST(DynamicBufferTest, AddNullTerminator) {
	DynamicBuffer *dbuf = dbuf_create();
	dbuf_addc(dbuf, 'A');
	dbuf_addc(dbuf, '\0'); // Assuming dbuf_addc should throw an error
	ASSERT_EQ(dbuf->size, 1);
	ASSERT_STREQ(dbuf->buf, "A");
	dbuf_destroy(dbuf);
}

