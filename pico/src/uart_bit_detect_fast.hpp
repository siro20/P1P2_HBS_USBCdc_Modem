#pragma once
#include <inttypes.h>
#include <stddef.h>

template <class T, size_t N>
class UARTBit
{
  public:
    // Implements a faster version of uart_bit_detect.
    // Trades memory for CPU performance.
    UARTBit();

    // Update returns false if no new data is available.
    // Update returns true if new data has been placed in out.
    void Update(const T in, T *probability_high, T *probability_low);

    // Returns the length of the shift register used.
    uint32_t Length(void);

  private:
    // Shifts in new data.
    inline void ShiftIn(const T in);

    T data[N * 2];
    T data_abs[N * 2];
    uint32_t off;
};
