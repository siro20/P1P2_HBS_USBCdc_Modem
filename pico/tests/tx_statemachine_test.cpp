#include <gtest/gtest.h>

#include "tx_statemachine.hpp"
#include "pico/types.h"

/* MOCK TIME */
static long now;
static void mock_add_usec_to_now(int delay)
{
	now += delay;
}

static void mock_reset_timebase(void)
{
	now = 0;
}

absolute_time_t make_timeout_time_us(long timeout)
{
	return now + timeout;
}

bool time_reached(absolute_time_t time)
{
	return time < now;
}

TEST(TxStateMachine, InitialValue)
{
	UARTPio Pio;
	TxStateMachine SM(Pio);

	EXPECT_EQ(SM.IsIdle(), true);
	EXPECT_EQ(SM.IsTransmitting(), false);
	EXPECT_EQ(SM.Done(), false);
	EXPECT_EQ(SM.Error(), false);
	EXPECT_EQ(SM.TxMsg.Length, 0);
	EXPECT_EQ(SM.RxMsg.Length, 0);
}

TEST(TxStateMachine, UARTError)
{
	UARTPio Pio;
	TxStateMachine SM(Pio);
	Message Msg;

	EXPECT_EQ(SM.IsIdle(), true);
	EXPECT_EQ(SM.IsTransmitting(), false);
	EXPECT_EQ(SM.Done(), false);
	Pio.MokSetError(true);
	SM.WakeAndTransmit(Msg);
	SM.Update(false, false, false, 0);
	EXPECT_EQ(SM.Error(), true);
	EXPECT_EQ(SM.TxMsg.Length, 0);
	EXPECT_EQ(SM.RxMsg.Length, 0);
}

TEST(TxStateMachine, WakeChangesStateToTransmit)
{
	UARTPio Pio;
	TxStateMachine SM(Pio);
	Message Msg;

	EXPECT_EQ(SM.IsIdle(), true);
	EXPECT_EQ(SM.IsTransmitting(), false);
	EXPECT_EQ(SM.Done(), false);
	SM.WakeAndTransmit(Msg);
	EXPECT_EQ(SM.IsIdle(), false);
	EXPECT_EQ(SM.IsTransmitting(), true);
}

TEST(TxStateMachine, LineIsBusyForTooLong)
{
	UARTPio Pio;
	TxStateMachine SM(Pio);
	Message Msg;
	size_t cnt = 100;
	bool err = false;

	mock_reset_timebase();

	EXPECT_EQ(SM.IsTransmitting(), false);
	EXPECT_EQ(SM.Done(), false);
	SM.WakeAndTransmit(Msg);
	EXPECT_EQ(SM.IsTransmitting(), true);
	err = SM.Error();
	while (!err && cnt > 0) {
		SM.Update(true, false, false, 0);
		// Increase timebase by 1 millisecond
		mock_add_usec_to_now(1000);
		cnt--;
		err = SM.Error();
	}
	EXPECT_GT(cnt, 0);
	EXPECT_EQ(err, true);
}

TEST(TxStateMachine, States)
{
	UARTPio Pio;
	TxStateMachine SM(Pio);
	Message Msg;
	size_t i;

	mock_reset_timebase();

	EXPECT_EQ(SM.IsTransmitting(), false);
	EXPECT_EQ(SM.Done(), false);
	EXPECT_EQ(SM.GetState(), TxState::IDLE);

	SM.WakeAndTransmit(Msg);

	SM.Update(true, false, false, 0);
	EXPECT_EQ(SM.GetState(), TxState::IDLE_WAIT_LINEFREE);
	mock_add_usec_to_now(1000);

	SM.Update(false, false, false, 0);
	EXPECT_EQ(SM.GetState(), TxState::WAIT_POWERON);
	mock_add_usec_to_now(1000);

	SM.Update(false, false, false, 0);
	EXPECT_EQ(SM.GetState(), TxState::WAIT_POWERON);
	mock_add_usec_to_now(50000);

	SM.Update(false, false, false, 0);
	EXPECT_EQ(SM.GetState(), TxState::STARTED_WAIT_FOR_BUSY);
	mock_add_usec_to_now(100);

	SM.Update(true, false, false, 0);
	EXPECT_EQ(SM.GetState(), TxState::RUNNING_CHECK_DATA);

	for (i = 0; i < 8; i++) {
		SM.Update(true, false, true, 1);
		EXPECT_EQ(SM.GetState(), TxState::RUNNING_CHECK_DATA);
		SM.Update(true, false, false, 0);
		mock_add_usec_to_now(100);
	}
	SM.Update(true, false, false, 0);
	EXPECT_EQ(SM.GetState(), TxState::RUNNING_WAIT_FOR_IDLE);

	SM.Update(false, false, false, 0);
	EXPECT_EQ(SM.GetState(), TxState::IDLE);
}