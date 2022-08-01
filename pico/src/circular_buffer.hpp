
#pragma once
#include <cstddef>
#include <algorithm>

template <class T, int N>
class CircularBuffer
{
	public:
		// Implements a queue based on circular fixed size buffer.
		// Can insert N elements at max.
		CircularBuffer(void) :
		off(0), len(0)
		{
		}

		// Front returns the element at queue head
		T Front(void) {
			if (this->Empty())
				return 0;

			return this->_array[this->off];
		}

		T At(size_t idx) {
			if (idx < this->len) {
				idx = (idx + this->off) % N;
				return this->_array[idx];
			}
			return 0;
		}

		// Push inserts a new element at the end of queue if not full
		void Push(T val) {
			if (this->len < N) {
				int idx = (this->off + this->len) % N;
				this->_array[idx] = val;
				this->len++;
			}
		}

		bool Full(void) {
			return this->len == N;
		}

		bool Empty(void) {
			return this->len == 0;
		}

		size_t Length(void) {
			return this->len;
		}


		void Clear(void) {
			this->len = 0;
			this->off = 0;
		}

		// Pop removes an emelement from the queue head
		void Pop(void) {
			if (this->len > 0) {
				this->len--;
				this->off = (this->off + 1) % N;
			}
		}

	private:
		T _array[N];
		size_t off;
		size_t len;
};


