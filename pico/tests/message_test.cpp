#include <gtest/gtest.h>

#include "message.hpp"

TEST(Message, Ctor)
{
	uint8_t test_data[3] = {1,2,3};
	Message m(0, 0, test_data, sizeof(test_data));

	EXPECT_EQ(m.Data[0], 1);
	EXPECT_EQ(m.Data[1], 2);
	EXPECT_EQ(m.Data[2], 3);
	EXPECT_EQ(m.Length, sizeof(test_data));
}

TEST(Message, cstr)
{
	uint8_t test_data[3] = {1,2,3};
	Message m1(1, 2, test_data, sizeof(test_data));
	EXPECT_EQ(strcmp("0000000000000001:00000002:010203", m1.c_str()), 0);

	Message m2(100000000, 0, test_data, sizeof(test_data));
	EXPECT_EQ(strcmp("0000000100000000:00000000:010203", m2.c_str()), 0);

	Message m3(100000000, 0xffffffff, test_data, sizeof(test_data));
	EXPECT_EQ(strcmp("0000000100000000:ffffffff:010203", m3.c_str()), 0);
}

TEST(Message, parsing)
{
	char buf1[] = "0000000100000000:ffffffff:010203";
	char buf2[] = "::010203";
	char buf3[] = "010203";
	char buf4[] = ";010203";
	char buf5[] = "010203;04";
	char buf6[] = "01 02  03  ;04";

	Message m1(buf1);
	EXPECT_EQ(m1.Data[0], 1);
	EXPECT_EQ(m1.Data[1], 2);
	EXPECT_EQ(m1.Data[2], 3);
	EXPECT_EQ(m1.Length, 3);
	EXPECT_EQ(m1.Status, 0xffffffff);
	EXPECT_EQ(m1.Time, 100000000);

	Message m2(buf2);
	EXPECT_EQ(m2.Status, 0);
	EXPECT_EQ(m2.Time, 0);

	Message m3(buf3);
	EXPECT_EQ(m3.Status, 0);
	EXPECT_EQ(m3.Time, 0);
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
}