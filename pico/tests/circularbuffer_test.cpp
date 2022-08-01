#include <gtest/gtest.h>
#include <math.h>

#include "circular_buffer.hpp"


TEST(CircularBuffer, TestFullEmpty)
{
	CircularBuffer<uint8_t, 3> b;

	EXPECT_EQ(b.Empty(), true);
	EXPECT_EQ(b.Full(), false);
	b.Push(0);
	EXPECT_EQ(b.Empty(), false);
	EXPECT_EQ(b.Full(), false);
	b.Push(1);
	EXPECT_EQ(b.Empty(), false);
	EXPECT_EQ(b.Full(), false);
	b.Push(2);
	EXPECT_EQ(b.Empty(), false);
	EXPECT_EQ(b.Full(), true);
}

TEST(CircularBuffer, CheckValue)
{
	CircularBuffer<uint8_t, 4> b;

	b.Push(0);
	b.Push(1);
	b.Push(2);

	EXPECT_EQ(b.Front(), 0);
	b.Pop();
	EXPECT_EQ(b.Front(), 1);
	b.Push(3);
	b.Push(4);
	b.Push(5);	// 5 drops due to overflow

	b.Pop();
	EXPECT_EQ(b.Front(), 2);
	b.Pop();
	EXPECT_EQ(b.Front(), 3);
	b.Pop();
	EXPECT_EQ(b.Front(), 4);
	b.Pop();
	EXPECT_EQ(b.Empty(), true);

	for (int i = 6; i < 100; i++) {
		b.Push(i);
		EXPECT_EQ(b.Front(), i);
		EXPECT_EQ(b.At(0), i);
		b.Pop();
	}
	for (int i = 0; i < 100; i++) {
		b.Push(i);
		b.Push(i+1);
		b.Push(i+2);
		EXPECT_EQ(b.At(0), i);
		EXPECT_EQ(b.At(1), i+1);
		EXPECT_EQ(b.At(2), i+2);

		EXPECT_EQ(b.Front(), i);
		b.Pop();
		EXPECT_EQ(b.Front(), i+1);
		b.Pop();
		EXPECT_EQ(b.Front(), i+2);
		b.Pop();
	}
}