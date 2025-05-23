#include <gtest/gtest.h>

#include "message.hpp"

TEST(Message, Ctor)
{
	uint8_t test_data[3] = {1,2,3};
	Message m(0, test_data, sizeof(test_data));

	EXPECT_EQ(m.Data[0], 1);
	EXPECT_EQ(m.Data[1], 2);
	EXPECT_EQ(m.Data[2], 3);
	EXPECT_EQ(m.Length, sizeof(test_data));
}

static void cmp(const char *str1, const char *str2)
{
	EXPECT_EQ(strcmp(str1, str2), 0) <<
		"expected " << str1 << ", but got " << str2;
}

TEST(Message, cstr)
{
	uint8_t test_data[3] = {1,2,3};
	Message m1(2, test_data, sizeof(test_data));
	cmp("010203 # 02 O", m1.c_str());

	Message m2(1, test_data, sizeof(test_data));
	cmp("010203 # 01 C", m2.c_str());

	Message m3(0, test_data, sizeof(test_data));
	cmp("010203", m3.c_str());

	Message m4(0xff, test_data, sizeof(test_data));
	cmp("010203 # ff  ", m4.c_str());
}

TEST(Message, parsing)
{
	char buf1[] = "010203";
	char buf2[] = "010203#test";
	char buf3[] = "010203;test";
	char buf4[] = ";010203";
	char buf5[] = "010203;04";
	char buf6[] = "01 02  03  ;04";
	char buf7[] = "::";

	Message m1(buf1);
	EXPECT_EQ(m1.Data[0], 1);
	EXPECT_EQ(m1.Data[1], 2);
	EXPECT_EQ(m1.Data[2], 3);
	EXPECT_EQ(m1.Length, 3);

	Message m2(buf2);
	EXPECT_EQ(m2.Data[0], 1);
	EXPECT_EQ(m2.Data[1], 2);
	EXPECT_EQ(m2.Data[2], 3);
	EXPECT_EQ(m2.Length, 3);

	Message m3(buf3);
	EXPECT_EQ(m3.Status, 0);
	EXPECT_EQ(m3.Data[0], 1);
	EXPECT_EQ(m3.Data[1], 2);
	EXPECT_EQ(m3.Data[2], 3);
	EXPECT_EQ(m3.Length, 3);

	Message m4(buf4);
	EXPECT_EQ(m4.Length, 0);

	Message m5(buf5);
	EXPECT_EQ(m5.Length, 3);

	Message m6(buf6);
	EXPECT_EQ(m6.Length, 3);
	EXPECT_EQ(m6.Data[0], 1);
	EXPECT_EQ(m6.Data[1], 2);
	EXPECT_EQ(m6.Data[2], 3);

	Message m7(buf7);
	EXPECT_EQ(m7.Length, 0);
}