#include "uart_edge_detect.hpp"
#include "uart_bit_detect_fast.hpp"

#include "shiftreg.hpp"
#include "defines.hpp"

// High level abstraction of UART
class UART
{
	public:
		enum UART_PARITY {
			PARITY_NONE = 0,
			PARITY_EVEN,
			PARITY_ODD,
		};

	// Level sets the line level for a "high" or "low" symbol
	// p is the parity.
	// For P1P2 bus this must be 'PARITY_EVEN' to make sure the signal has a zero DC level.
	UART(uint16_t level, enum UART_PARITY p);

	// Receiving returns true as long as data is being received
	bool Receiving(void);

	// InternalUpdate returns false if no new data is available.
	// InternalUpdate returns true if new data has been placed in out.
	bool InternalUpdate(const int32_t symbol_prob, uint8_t *out, bool *err);
	bool Update(const int32_t signal, uint8_t *out, bool *err);

	private:

		bool ZeroLevelDetect(const int16_t prob);
		uint32_t ExtractDataAndParity(const uint8_t phase, uint8_t *parity, uint8_t *out, bool *err);
		uint32_t ExtractData(const uint8_t phase, uint8_t *out, bool *err);
		bool FindBestPhase(uint8_t *out, bool *err);

		enum UART_STATE {
			// Wait for the line to be idle
			WAIT_FOR_IDLE = 0,
			// Wait for a start signal on the idle line
			WAIT_FOR_START,

			// Data phase
			DATA,

			// Stop phase
			STOP
		};

		// Parity
		enum UART_PARITY parity;
		size_t counter;
		// The internal state used to decode uart data
		enum UART_STATE state;
		// receiver_level sets the threshold for a "0" symbol 
		uint8_t receiver_level;

		// Data storage for propability
		ShiftReg<int16_t, (UART_OVERSAMPLING_RATE * 10 + UART_OVERSAMPLING_RATE/2)> reg_parity;
		ShiftReg<int16_t, (UART_OVERSAMPLING_RATE * 9 + UART_OVERSAMPLING_RATE/2)> reg_no_parity;
		UARTBit<int32_t, UART_OVERSAMPLING_RATE>  bit;
};