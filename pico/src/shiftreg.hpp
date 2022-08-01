
#pragma once
#include "inttypes.h"
#include <string.h>
#include <cassert>
#include <iostream>

template <class T, size_t N>
class ShiftReg
{
  public:

    // Implements a shift register on top of a fixed size circular buffer
    // The buffer has twice the requested size to prevent wrapping around.
    // This allows to return a pointer to a linear buffer with zero copy.
    // By pushing a new value into the shift register the oldest value is poped
    // If init is NULL the register will be zero'd, else the contents are initialized.
    ShiftReg(const T *init) :  data{}, off(0) {
        assert(init != nullptr);

        for (size_t i = 0; i < N; i++)
            this->data[i] = init[i];
    }

    ShiftReg() : data{}, off(0) {
    }
    // Update returns false if no new data is available.
    // Update returns true if new data has been placed in out.
    bool Update(const T in, T *out)  {
        if (out)
            *out = this->data[this->off];
        assert(this->off < N);

        this->data[this->off] = in;
        this->data[this->off + N] = in;

        this->off++;
        if (this->off == N)
            this->off = 0;
        return true;
    }

    // Update returns false if no new data is available.
    // Update returns true if new data has been placed in out.
    bool Update(const T in)  {
        this->data[this->off] = in;
        this->data[this->off + N] = in;

        this->off++;
        if (this->off == N)
            this->off = 0;
        return true;
    }

    uint32_t Length(void) {
        return N;
    }

    // At returns the data at given position
    // 0 is the oldest entry and length-1 is the latest
    inline T At(const uint32_t i) {
        if (i >= N) {
            assert(i < N);
        }
        // Get offset into circular buffer
        // As it has twice the specified size it never overflows
        // and doesn't need to wrap around.
        return this->data[i + this->off];
    }

    // Data returns a pointer to the data.
    // The caller can assume that the returned buffer is N items big.
    // The first item is the oldest entry and length-1 is the latest.
    inline T* Data() {
        // Get offset into circular buffer
        // As it has twice the specified size it never overflows
	// and doesn't need to wrap around.
        return &this->data[this->off];
    }

    // Folds the shift register with another shift register.
    // If the length of both shift register is not equal, it only folds the
    // smaller register full set of data.
    //
    // The caller must make sure that the result doesn't overflow, i.e. by reducing
    // the signal amplitude before calling this function.
    template<typename A, typename B, size_t X, size_t Y>
    static int32_t Convolute(ShiftReg<A, X>& a, ShiftReg<B, Y>& b)
    {
        uint32_t min, i;
        int32_t result = 0;
	A *ptr_a = a.Data();
	B *ptr_b = b.Data();

        min = (a.Length() < b.Length()) ? a.Length() : b.Length();

        for (i = 0; i < min; i++) {
            result += ptr_a[i] * ptr_b[i];
        }
        return result;
    }

  private:
    T data[N * 2];
    uint32_t off;
};
