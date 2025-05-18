#pragma once
#include "defines.hpp"
#include "pico/types.h"

class TxState
{
	public:
		enum STATE {
			IDLE,
			IDLE_WAIT_LINEFREE,
			WAIT_POWERON,
			STARTED_WAIT_FOR_BUSY,
			RUNNING_CHECK_DATA,
			RUNNING_WAIT_FOR_IDLE
		};

		TxState(const enum STATE NewState) :
		State{NewState}, WaitCounter{0} {
			unsigned long t = TimeoutForState(NewState);
			if (t)
				WaitCounter = make_timeout_time_us(t);
		}
		~TxState(void) {}

		// Returns true if the timer timed out
		bool TimedOut(void) {
			return time_reached(WaitCounter);
		}

		// Returns true when the state was active for too long
		bool IsError(void) {
			return TimedOut() && IsTimeoutError(State);
		}

		// Returns true if the line busy state is unexpected in the
		// current state.
		bool LineStateIsError(const bool LineIsBusy) {
			if (LineIsBusy)
				return LineBusyIsError(State);
			else
				return LineFreeIsError(State);
		}

		enum STATE Value(void) {
			return State;
		}

		static unsigned long TimeoutForState(const enum STATE s) {
			switch (s) {
			case IDLE:
				return 0;
			case IDLE_WAIT_LINEFREE:
				return TX_ONE_CHAR_TIMEOUT_US * 16 + TX_ONE_CHAR_TIMEOUT_US;
			case WAIT_POWERON:
				return TX_POWERON_TIMEOUT_US;
			case STARTED_WAIT_FOR_BUSY:
				return TX_ONE_CHAR_TIMEOUT_US;
			case RUNNING_CHECK_DATA:
				return TX_ONE_CHAR_TIMEOUT_US * 2;
			case RUNNING_WAIT_FOR_IDLE:
				return TX_ONE_CHAR_TIMEOUT_US * 2;
			default:
				return 0;
			}
		}

		static bool IsTimeoutRequired(const enum STATE s) {
			switch (s) {
			case IDLE:
			case IDLE_WAIT_LINEFREE:
			case STARTED_WAIT_FOR_BUSY:
			case RUNNING_CHECK_DATA:
			case RUNNING_WAIT_FOR_IDLE:
				return false;

			case WAIT_POWERON:
				return true;
			default:
				return false;
			}
		}

		static bool IsTimeoutError(const enum STATE s) {
			switch (s) {
			case IDLE:
			case WAIT_POWERON:
				return false;
			case IDLE_WAIT_LINEFREE:
			case STARTED_WAIT_FOR_BUSY:
			case RUNNING_CHECK_DATA:
			case RUNNING_WAIT_FOR_IDLE:
				return true;
			}
			return false;
		}

		static bool LineBusyIsError(const enum STATE s) {
			switch (s) {
			case IDLE:
			case IDLE_WAIT_LINEFREE:
			case STARTED_WAIT_FOR_BUSY:
			case RUNNING_CHECK_DATA:
			case RUNNING_WAIT_FOR_IDLE:
				return false;

			case WAIT_POWERON:
				return true;
			}
			return false;
		}

		static bool LineFreeIsError(const enum STATE s) {
			switch (s) {
			case IDLE:
			case IDLE_WAIT_LINEFREE:
			case STARTED_WAIT_FOR_BUSY:
			case RUNNING_WAIT_FOR_IDLE:
			case WAIT_POWERON:
				return false;

			case RUNNING_CHECK_DATA:
				return true;
			}
			return false;
		}
	
	private:
		enum STATE State;
		absolute_time_t WaitCounter;
};