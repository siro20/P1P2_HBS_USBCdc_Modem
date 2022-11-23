
#pragma once
#include "hardware/sync.h"
#include "inttypes.h"
#include <string.h>
#include <cassert>
#include <iostream>

template <class T, size_t N>
class FifoIrqSafe
{
  public:

    // If init is NULL the register will be zero'd, else the contents are initialized.
    FifoIrqSafe(const T *init) : data{}, off(0) {
        assert(init != nullptr);

        for (size_t i = 0; i < N; i++) {
            this->data[i] = init[i];
        }
    }

    FifoIrqSafe() : data{}, off(0) {
    }

    // Push returns true if new data has been successfully
    // been stored. false if there was not enough space.
    bool Push(const T in)  {
        uint32_t save = save_and_disable_interrupts();

        if (this->off < N) {
            this->data[this->off] = in;
            this->off++;
	    __dmb();
            restore_interrupts(save);

            return true;
        }

        restore_interrupts(save);
        return false;
    }

    // Push returns true if new data has been successfully
    // been stored. false if there was not enough space.
    bool Pop(T *out)  {
        uint32_t save = save_and_disable_interrupts();

        if (this->off > 0) {
            *out = this->data[0];
            for (size_t i = 1; i < this->off; i++) {
                this->data[i-1] = this->data[i];
            }
            this->off--;
	    __dmb();
            restore_interrupts(save);

            return true;
        }

        restore_interrupts(save);
        return false;
    }

    uint32_t Length(void) {
        uint32_t ret;
        uint32_t save = save_and_disable_interrupts();
        ret = this->off;
        restore_interrupts(save);
        return ret;
    }

    bool Empty(void) {
        return this->Length() == 0;
    }

    bool Full(void) {
        return this->Length() == N;
    }

     void Clear(void) {
        uint32_t save = save_and_disable_interrupts();
        this->off = 0;
        restore_interrupts(save);
    }


    // At returns the data at given position
    // 0 is the oldest entry and length-1 is the latest
    inline T At(const uint32_t i) {
        T ret;
        uint32_t save = save_and_disable_interrupts();
        if (i >= N) {
            assert(i < N);
        }

	ret = this->data[i];
        restore_interrupts(save);
        return ret;
    }

    // Data returns a pointer to the data.
    // The caller can assume that the returned buffer is N items big.
    // This method is not IRQ safe!
    inline T* Data() {
        return &this->data[0];
    }

  private:
    T data[N];
    uint32_t off;
};
