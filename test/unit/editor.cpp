#include <gtest/gtest.h>
#include <vector>
#include <cstring>

extern "C"
{
#include "editor.h"
#include "editor_private.h"
}
TEST(add_to_row, normal_characters)
{
	char data[10];
	Row row = { .index = 0, .data = data };
	editor_add_to_row(&row, {10, 1}, 'H');
	ASSERT_EQ(row.index, 1);
	ASSERT_EQ(row.data[0], 'H');
	editor_add_to_row(&row, {10, 1}, 'E');
	ASSERT_EQ(row.index, 2);
	ASSERT_EQ(row.data[1], 'E');
	editor_add_to_row(&row, {10, 1}, 'L');
	ASSERT_EQ(row.index, 3);
	ASSERT_EQ(row.data[2], 'L');
	editor_add_to_row(&row, {10, 1}, 'L');
	ASSERT_EQ(row.index, 4);
	ASSERT_EQ(row.data[3], 'L');
	editor_add_to_row(&row, {10, 1}, 'O');
	ASSERT_EQ(row.index, 5);
	ASSERT_EQ(row.data[4], 'O');
	row.data[5] = '\0';
	ASSERT_STREQ(row.data, "HELLO");

}

TEST(add_to_row, just_below_capacity)
{
	char data[10];
	Row row = (Row){ .index = 2, .data = data };
	editor_add_to_row(&row, {10, 1}, 'a');
	ASSERT_EQ(row.data[2], 'a');
}

TEST(ctrl_key, normal_checks)
{
	for (char i = 'a'; i <= 'z'; i++)
	{
		ASSERT_EQ(ctrl_key(i), i - 'a' + 1);
	}
}

std::vector<std::string> rows;

void mock_render_row(int row_id, size_t size, const char *data)
{
	rows.push_back(std::string(data, size)); 
}


char g_data[10][10];
void generate_text(Row text[10])
{
	for (int i = 0; i < 10; i++)
	{
		text[i].data = g_data[i];
		for (int j = 0; j < i; j++)
		{
			text[i].data[j] = 'a';
		}
		text[i].index = i;
	}
}

TEST(editor_render_rows, normal_checks)
{
	rows.clear();
	Row text[10];
	generate_text(text);
	IO_Interface io_interface;
	io_interface.render_row = mock_render_row;
	editor_render_rows(text, {10, 10}, {1, 1}, &io_interface);
	ASSERT_EQ(rows[0], "~");
	for (int i = 1; i < 10; i++)
	{
		ASSERT_STREQ(rows[i].c_str(), std::string(i, 'a').c_str());
	}
	
}

TEST(editor_move_cursor, normal_checks)
{
	Row text[10];
	generate_text(text);
	vec2 res = editor_move_cursor(text, {10, 10}, {0, 0}, {0, 1});
	ASSERT_EQ(res.x, 0);
	ASSERT_EQ(res.y, 1);
	res = editor_move_cursor(text, {10, 10}, {0, 0}, {0, 2});
	ASSERT_EQ(res.x, 0);
	ASSERT_EQ(res.y, 2);
	res = editor_move_cursor(text, {10, 10}, {0, 0}, {0, 3});
	ASSERT_EQ(res.x, 0);
	ASSERT_EQ(res.y, 3);
	res = editor_move_cursor(text, {10, 10}, {2, 8}, {5, 0});
	ASSERT_EQ(res.x, 7);
	ASSERT_EQ(res.y, 8);
	res = editor_move_cursor(text, {10, 10}, {2, 3}, {1, 2});
	ASSERT_EQ(res.x, 3);
	ASSERT_EQ(res.y, 5);
	
}

TEST(editor_move_cursor, always_ends_up_same_or_invalid)
{
	Row text[10];
	generate_text(text);
	for (int i = 0; i < 1000; i++)
	{
		int y = rand() % 10;
		int x = rand() % (y + 1);
		int dy = (rand() % 10) - y;
		int dx = (rand() % 10) - x;
		vec2 res = editor_move_cursor(text, {10, 10}, {x, y}, {dx, dy});
		if (res.x == x && res.y == y)
			continue;
		res = editor_move_cursor(text, {10, 10}, res, {-dx, -dy});
		ASSERT_EQ(res.x, x);
		ASSERT_EQ(res.y, y);
	}
	

}


TEST(add_normal_character, cursor_always_moves_one_right)
{
	Row text[10];
	generate_text(text);
	for (int i = 0; i < 1000; i++)
	{
		int y = rand() % 10;
		int x = rand() % (y + 1);
		vec2 current_cursor = {x, y};
		generate_text(text);
		add_normal_character(text, &current_cursor, {10, 10}, (rand() % 26) + 'a');
		ASSERT_EQ(current_cursor.x, x+1);
		ASSERT_EQ(current_cursor.y, y);
	}
	
}


TEST(add_normal_character, normal_checks)
{
	Row text[10];

	generate_text(text);
	text[0].data[0] = 'A';
	text[0].data[1] = 'B';
	text[0].data[2] = 'C';
	text[0].data[3] = 'D';
	text[0].data[4] = 'E';
	text[0].data[5] = 'F';
	text[0].index = 6;
	vec2 cursor = {6, 0};

	add_normal_character(text, &cursor, {10, 10}, 'G');

	text[0].data[7] = '\0';

	ASSERT_STREQ(text[0].data, "ABCDEFG");
	ASSERT_EQ(cursor.x, 7);
	ASSERT_EQ(cursor.y, 0);
	
	generate_text(text);
	text[3].data[0] = 'A';
	text[3].data[1] = 'B';
	text[3].data[2] = 'C';
	text[3].data[3] = 'D';
	text[3].data[4] = 'E';
	text[3].data[5] = 'F';
	text[3].index = 6;
	cursor = {2, 3};

	add_normal_character(text, &cursor, {10, 10}, 'Z');

	text[3].data[7] = '\0';

	ASSERT_STREQ(text[3].data, "ABZCDEF");
	ASSERT_EQ(cursor.x, 3);
	ASSERT_EQ(cursor.y, 3);
}


TEST(row_shift_right, normal_checks)
{
	Row text[10];
	generate_text(text);
	text[0].index = 7;
	strncpy(text[0].data, "1234567", 7);
	row_shift_right(&text[0], {10, 10}, 2);
	ASSERT_EQ(text[0].index, 8);
	ASSERT_EQ(text[0].data[7], '7');
	ASSERT_EQ(text[0].data[6], '6');
	ASSERT_EQ(text[0].data[5], '5');
	ASSERT_EQ(text[0].data[0], '1');
	ASSERT_EQ(text[0].data[1], '2');
}

TEST(is_in_range, normal_checks)
{
	ASSERT_TRUE(is_in_range(0, 1, 2));
	ASSERT_TRUE(is_in_range(0, 0, 2));
	ASSERT_FALSE(is_in_range(0, 2, 2));
	ASSERT_TRUE(is_in_range(-5, 0, 2));
	ASSERT_TRUE(is_in_range(-5, -3, -2));
	ASSERT_FALSE(is_in_range(-5, -3, -3));
	ASSERT_TRUE(is_in_range(-5, -5, -2));
}
