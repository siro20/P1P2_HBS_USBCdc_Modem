
#pragma once
#include <cstddef>
#include <algorithm>
#include <pico/critical_section.h>

template <class T, int N>
class CircularBufferSpinlock
{
	public:
		// Implements a queue based on circular fixed size buffer.
		// Can insert N elements at max.
		// Holds a spinlock to prevent the other core or interrupts to happen
		CircularBufferSpinlock(void) :
		off(0), len(0)
		{
			critical_section_init (&this->spinlock);
		}

		~CircularBufferSpinlock(void)
		{
			critical_section_deinit (&this->spinlock);
		}

		// Front returns the element at queue head
		T Front(void) {
			T ret = 0;
			if (this->Empty())
				return 0;
			critical_section_enter_blocking(&this->spinlock);
			ret = this->_array[this->off];
			critical_section_exit(&this->spinlock);

			return ret;
		}

		T At(size_t idx) {
			T ret = 0;
			critical_section_enter_blocking(&this->spinlock);

			if (idx < this->len) {
				idx = (idx + this->off) % N;
				ret = this->_array[idx];
			}
			critical_section_exit(&this->spinlock);
			return ret;
		}

		// Push inserts a new element at the end of queue if not full
		void Push(T val) {
			critical_section_enter_blocking(&this->spinlock);

			if (this->len < N) {
				int idx = (this->off + this->len) % N;
				this->_array[idx] = val;
				this->len++;
			}
			critical_section_exit(&this->spinlock);
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
			critical_section_enter_blocking(&this->spinlock);
			this->len = 0;
			this->off = 0;
			critical_section_exit(&this->spinlock);
		}

		// Pop removes an emelement from the queue head
		void Pop(void) {
			critical_section_enter_blocking(&this->spinlock);
			if (this->len > 0) {
				this->len--;
				this->off = (this->off + 1) % N;
			}
			critical_section_exit(&this->spinlock);
		}

	private:
		T _array[N];
		size_t off;
		size_t len;
		critical_section_t spinlock;
};


