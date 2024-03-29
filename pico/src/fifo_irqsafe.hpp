
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

    FifoIrqSafe(const T *init) : data{}, off_read(0), off_write(0), length(N) {
        assert(init != nullptr);

        for (size_t i = 0; i < N; i++) {
            this->data[i] = init[i];
        }
    }

    FifoIrqSafe() : data{}, off_read(0), off_write(0), length(0) {
    }

    // Push returns true if new data has been successfully
    // been stored. false if there was not enough space.
    bool Push(const T in)  {
        uint32_t save = save_and_disable_interrupts();

        if (this->length < N) {
            this->data[this->off_write] = in;
            this->length++;
            this->off_write++;
            if (this->off_write == N) {
                    this->off_write = 0;
            }
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

        if (this->length > 0) {
            *out = this->data[this->off_read];
            this->off_read++;
            if (this->off_read == N) {
                    this->off_read = 0;
            }
            this->length--;
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
        ret = this->length;
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
        this->length = 0;
        this->off_read = 0;
        this->off_write = 0;
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

  private:
    T data[N];
    uint32_t off_read;
    uint32_t off_write;
    uint32_t length;
};
