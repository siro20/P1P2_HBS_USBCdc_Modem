#pragma once
#include <inttypes.h>

#include "message.hpp"
#include "tx_state.hpp"

#ifndef WITH_GOOGLE_TEST
#include "uart_pio.hpp"
#else
#include "uart_pio_mock.hpp"
#endif

// High level abstraction of TX state machine
class TxStateMachine
{
	public:
		TxStateMachine(UARTPio& Pio) :
		TxMsg{}, RxMsg{}, State(TxState::IDLE), TxOffset(~0), UART(&Pio), Err (false)
		{
			UART->ClearFifo();
			UART->EnableShutdown(true);
		}

		~TxStateMachine(void) {}

		static TxStateMachine& getInstance(UARTPio& Pio)
		{
			static TxStateMachine instance(Pio);
			return instance;
		}

		TxStateMachine(TxStateMachine const&) = delete;
		void operator=(TxStateMachine const&) = delete;

		// Load a new message to be transmitted. Only has effect when the
		// state machine is in the idle state.
		void WakeAndTransmit(Message &Msg) {
			if (IsIdle()) {
				TxMsg = Msg;
				TxOffset = 0;
				Err = false;
				RxMsg.Clear();
				ChangeState(TxState::IDLE_WAIT_LINEFREE);
			}
		}

		// Returns true if the TX state machine is in the idle state
		bool IsIdle(void) {
			return State.Value() == TxState::IDLE;
		}

		// Returns true if the TX state machine isn't in the idle state
		bool IsTransmitting(void) {
			return !IsIdle();
		}

		// Returns true if the TX state machine has transmitted a packet
		bool Done(void) {
			return TxOffset == TxMsg.Length;
		}

		// Returns true if the TX state machine has encountered an error
		// transmitting the packet
		bool Error(void) {
			bool state = Err;
			Err = false;
			return state;
		}

		enum TxState::STATE GetState() {
			return State.Value();
		}

		// Update performs periodic checks on the TX state machine.
		// This method will change the state by calling ChangeState.
		void Update(const bool LineIsBusy, const bool RxError,
			    const bool RxValid, const char RxChar) {
			bool BusCollision = false;
			if (State.IsError() || State.LineStateIsError(LineIsBusy)) {
				if (LineIsBusy)
					RxMsg.Status = Message::STATUS_ERR_BUS_COLLISION;
				else
					RxMsg.Status = Message::STATUS_ERR_NO_FRAMING;
				ChangeState(TxState::IDLE);
				Err = true;
				return;
			}

			if (!IsIdle() && UART->Error()) {
				RxMsg.Status = Message::STATUS_INTERNAL_ERROR;
				ChangeState(TxState::IDLE);
				Err = true;
				return;
			}

			switch (State.Value()) {
			case TxState::IDLE:
				// Nothing to do here
				break;
			case TxState::IDLE_WAIT_LINEFREE:
				if (!LineIsBusy && !UART->Transmitting() && TxOffset == 0)
					// Need to wait for the IC to power on...
					ChangeState(TxState::WAIT_POWERON);
				break;
			case TxState::WAIT_POWERON:
				if (State.TimedOut())
					ChangeState(TxState::STARTED_WAIT_FOR_BUSY);
				break;
			case TxState::STARTED_WAIT_FOR_BUSY:
				if (RxError) {
					RxMsg.Status = Message::STATUS_ERR_PARITY;
					BusCollision = true;
				} else if (LineIsBusy || RxValid)
					ChangeState(TxState::RUNNING_CHECK_DATA);

				if (!RxValid)
					break;
			case TxState::RUNNING_CHECK_DATA:
				if (RxError) {
					RxMsg.Status = Message::STATUS_ERR_PARITY;
					BusCollision = true;
				} else if (RxValid) {
					RxMsg.Append(TxMsg.Data[TxOffset]);
					if(TxMsg.Data[TxOffset] != RxChar)
						BusCollision = true;
					TxOffset++;
					if (TxOffset == TxMsg.Length)
						ChangeState(TxState::RUNNING_WAIT_FOR_IDLE);
					else
						ChangeState(TxState::RUNNING_CHECK_DATA);
				}
				break;
			case TxState::RUNNING_WAIT_FOR_IDLE:
				// The RX state machine will insert the neccessary delay between
				// two packets. No need to wait here.
				if (!LineIsBusy)
					ChangeState(TxState::IDLE);
				break;
			}

			if (BusCollision) {
				ChangeState(TxState::RUNNING_WAIT_FOR_IDLE);
				if (RxMsg.Status == 0)
					RxMsg.Status = Message::STATUS_ERR_BUS_COLLISION;
				Err = true;
			}
		}
		Message TxMsg;
		Message RxMsg;

	private:
		// Callback on state change. Updates the hardware TX UART.
		// Resets the timeout timer.
		void ChangeState(const enum TxState::STATE NewState) {
			switch (NewState) {
			case TxState::IDLE:
				// FIFO should be empty.
				UART->EnableShutdown(true);
				break;
			case TxState::IDLE_WAIT_LINEFREE:
				break;
			case TxState::WAIT_POWERON:
				UART->EnableShutdown(false);
				break;
			case TxState::STARTED_WAIT_FOR_BUSY:
				UART->Send(TxMsg);
				break;
			case TxState::RUNNING_CHECK_DATA:
				break;
			case TxState::RUNNING_WAIT_FOR_IDLE:
				// FIFO should be empty.
				// Only on error path it might still have data.
				UART->ClearFifo();
				break;
			}

			State = TxState(NewState);
		}

		TxState State;
		size_t TxOffset;
		UARTPio* UART;
		bool Err;
};
