#include <math.h>
#include <assert.h>
#include "uart_bit_detect_fast.hpp"
#include <iostream>
#include <cstring>

template <class T, size_t N>
UARTBit<T, N>::UARTBit(T buffer[N * 2],
		       T buffer_abs[N * 2],
		       const uint32_t high_level,
		       const uint32_t low_level,
		       const uint8_t error_rate) :
	data(buffer), data_abs(buffer_abs), receiver_level(0), off(0) {
	memset(data, 0, sizeof(T) * N * 2);
	memset(data_abs, 0, sizeof(T) * N * 2);

	/* Calculate receiver level */
	this->receiver_level = (N/2 - 1) * high_level;
	this->receiver_level -= (N/2 - 1) * low_level;

	/* Now apply error rate. 0xff == no error, 0x7f == 1/2 receiver level */
	this->receiver_level *= (error_rate + 1);
	this->receiver_level >>= 8;
}

// Update returns the probability of a found symbol
// Return value:
//   = 0    line is idle
//   > 0    a positive pulse has been detected
//   < 0    a negative pulse has been detected
template <class T, size_t N>
void UARTBit<T, N>::Update(const T in, T *probability) {
	int32_t result_abs, result_high, h, l;
	size_t i;

	this->ShiftIn(in);

	// Get pointers to memory with length N
	// The first element is the oldest and the last is the newest
	const T *const ptr = &this->data[this->off];
	const T *const ptr_abs = &this->data_abs[this->off];

	result_abs = ptr_abs[0];
	result_high = 0;
	for (i = 1; i < N/2; i++) {
		result_high += ptr[i];
	}

	for (i+=2; i < N; i++) {
		result_abs += ptr_abs[i];
	}
	
	h = result_high - result_abs;
	l = -result_high - result_abs;

	if (h >= this->receiver_level)
		*probability = h;
	else if (l >= this->receiver_level)
		*probability = -l;
	else
		*probability = 0;
}

template <class T, size_t N>
uint32_t UARTBit<T, N>::Length(void) {
	return N;
}

template <class T, size_t N>
inline void UARTBit<T, N>::ShiftIn(const T in) {
	const T in_abs = abs(in);
	T* ptr;
	assert(this->off < this->Length());

	// Place two times in buffer to make sure reading never wraps
	ptr = &this->data[this->off];
	// Pointer magic generates smaller program size
	*ptr = in;
	ptr += N;
	*ptr = in;

	// Place two times in buffer to make sure reading never wraps
	ptr = &this->data_abs[this->off];
	// Pointer magic generates smaller program size
	*ptr = in_abs;
	ptr += N;
	*ptr = in_abs;

	this->off++;
	if (this->off == N)
		this->off = 0;
}

template class UARTBit<int32_t, 16>;
template class UARTBit<int32_t, 8>;
template class UARTBit<int16_t, 16>;
template class UARTBit<int16_t, 8>;