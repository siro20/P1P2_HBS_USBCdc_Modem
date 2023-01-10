#include <gtest/gtest.h>

#include "fifo_irqsafe.hpp"

TEST(IRQSafeFifo, InitialLoadValue)
{
	uint8_t initdata[3] = {1, 2, 3};
	FifoIrqSafe<uint8_t, 3> f(initdata);

	EXPECT_EQ(f.At(0), 1);
	EXPECT_EQ(f.At(1), 2);
	EXPECT_EQ(f.At(2), 3);
}

TEST(IRQSafeFifo, Length)
{
	FifoIrqSafe<uint8_t, 3> f;

	EXPECT_EQ(f.Length(), 0);
	EXPECT_EQ(f.Empty(), true);
	EXPECT_EQ(f.Full(), false);

	f.Push(2);
	EXPECT_EQ(f.Length(), 1);
	EXPECT_EQ(f.Empty(), false);
	EXPECT_EQ(f.Full(), false);

	f.Push(3);
	EXPECT_EQ(f.Length(), 2);
	EXPECT_EQ(f.Empty(), false);
	EXPECT_EQ(f.Full(), false);

	f.Push(4);
	EXPECT_EQ(f.Length(), 3);
	EXPECT_EQ(f.Empty(), false);
	EXPECT_EQ(f.Full(), true);
}

TEST(IRQSafeFifo, PushPop)
{
	FifoIrqSafe<uint8_t, 3> f;
	uint8_t c;

	EXPECT_EQ(f.Pop(&c), false);

	f.Push(3);
	f.Push(4);
	f.Push(5);
	EXPECT_EQ(f.Pop(&c), true);
	EXPECT_EQ(c, 3);
	EXPECT_EQ(f.Pop(&c), true);
	EXPECT_EQ(c, 4);
	EXPECT_EQ(f.Pop(&c), true);
	EXPECT_EQ(c, 5);
	EXPECT_EQ(f.Pop(&c), false);
	f.Push(6);
	EXPECT_EQ(f.Pop(&c), true);
	EXPECT_EQ(c, 6);
	f.Push(7);
	f.Push(8);
	f.Push(9);

	EXPECT_EQ(f.Pop(&c), true);
	EXPECT_EQ(c, 7);
	EXPECT_EQ(f.Pop(&c), true);
	EXPECT_EQ(c, 8);
	EXPECT_EQ(f.Pop(&c), true);
	EXPECT_EQ(c, 9);
}
