#include <math.h>
#include <assert.h>
#include "uart_bit_detect_fast.hpp"

// reverse?

template <class T, size_t N>
UARTBit<T, N>::UARTBit() : data{}, data_abs{}, off(0) {
}

template <class T, size_t N>
void UARTBit<T, N>::Update(const T in, T *probability_high, T *probability_low) {
	uint32_t result_abs, result_high, result_low;
	size_t i;

	this->ShiftIn(in);

	// Get pointers to memory with length N
	// The first element is the oldest and the last is the newest
	const T *const ptr = &this->data[this->off];
	const T *const ptr_abs = &this->data_abs[this->off];

	result_abs = -ptr_abs[0];
	result_high = 0;
	for (i = 1; i < N/2 +1; i++) {
		result_high += ptr[i];
	}
	result_low = -result_high;

	for (; i < N; i++) {
		result_abs -= ptr_abs[i];
	}
	

	*probability_high = result_abs + result_high;
	*probability_low = result_abs + result_low;
}

template <class T, size_t N>
uint32_t UARTBit<T, N>::Length(void) {
	return N;
}

template <class T, size_t N>
inline void UARTBit<T, N>::ShiftIn(const T in) {
	const T in_abs = abs(in);
	assert(this->off < this->Length());

	// Place two times in buffer to make sure reading never wraps
	this->data[this->off] = in;
	this->data[this->off + N] = in;

	// Place two times in buffer to make sure reading never wraps
	this->data_abs[this->off] = in_abs;
	this->data_abs[this->off + N] = in_abs;

	this->off++;
	if (this->off == N)
		this->off = 0;
}

template class UARTBit<int32_t, 16>;
template class UARTBit<int16_t, 16>;
template class UARTBit<int16_t, 8>;