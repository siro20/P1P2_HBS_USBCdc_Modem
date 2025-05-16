#pragma once
#include <inttypes.h>
#include <stddef.h>

template <class T, size_t N>
class UARTBit
{
  public:
    // Implements a faster version of uart_bit_detect.
    // Trades memory for CPU performance.
    UARTBit(T buffer[N * 2],
	    T buffer_abs[N * 2],
	    const uint32_t high_level,
	    const uint32_t low_level,
	    const uint8_t error_rate);

    void Update(const T in, T *probability);

    // Returns the length of the shift register used.
    uint32_t Length(void);

  private:
    // Shifts in new data.
    inline void ShiftIn(const T in);

    T *data;
    T *data_abs;
    int32_t receiver_level;
    uint32_t off;
};
