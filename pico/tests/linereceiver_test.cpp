#include <gtest/gtest.h>

#include "line_receiver.hpp"

static char test_result[256];

class LineReceiverTester : public LineReceiver<128>
{
	public:
		LineReceiverTester() : LineReceiver()
		{
		}
	
		void OnLineReceived(uint8_t *line)
		{
			memcpy(test_result, line, strlen((char*)line));
		}
};

TEST(LineReceiver, TestOnLineReceived)
{
	LineReceiverTester t;
	const char *test = "LineReceiver";

	for (size_t i = 0; i < strlen(test); i++) {
		t.Push(test[i]);
		EXPECT_EQ(strlen(test_result), 0);
	}
	t.Push('\n');
	EXPECT_EQ(strlen(test_result), strlen(test));

	for (size_t i = 0; i < strlen(test); i++) {
		EXPECT_EQ(test_result[i], test[i]);
	}
}
