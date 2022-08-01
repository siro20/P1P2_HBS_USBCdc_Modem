
#pragma once
#include "circular_buffer.hpp"

template <int N>
class LineReceiver
{
	public:
		// Implements a queue backed reader that waits for new line characters
		// Calls OnLineReceived whenever a full line is being received
		LineReceiver(void) : fifo()
		{
		}

		virtual void OnLineReceived(uint8_t *line) {};

		// Push inserts a new element at the end of queue if not full
		void Push(uint8_t val) {
			uint8_t line[N+1];
			size_t len = std::min(this->fifo.Length(), sizeof(line) - 1);

			if (val == '\n') {
				for (size_t i = 0; i < len; i++) {
					line[i] = this->fifo.At(i);
				}
				line[len] = 0;

				// Now invoke the callback
				OnLineReceived(line);

				this->fifo.Clear();
			} else {
				this->fifo.Push(val);
			}
		}

		bool Full(void) {
			return this->fifo.Full();
		}

	private:
		CircularBuffer<uint8_t, N> fifo;
};


