#include "uart_edge_detect.hpp"

#include "shiftreg.hpp"
#include "defines.hpp"

#define UART_BITS_PARITY	11
#define UART_BITS_NO_PARITY	10
#define UART_BUFFER_LEN (UART_OVERSAMPLING_RATE * UART_BITS_PARITY - UART_OVERSAMPLING_RATE/2)

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
	UART(int16_t buffer[UART_BUFFER_LEN * 2], enum UART_PARITY p);

	// Receiving returns true as long as data is being received
	bool Receiving(void);

	// Update returns false if no new data is available.
	// Update returns true if new data has been placed in out.
	bool Update(const int32_t symbol_prob, uint8_t *out, bool *err);

	// Print contents of internal shiftreg
	void PrintShiftreg(void);

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

		// Data storage for propability
		ShiftReg<int16_t, UART_BUFFER_LEN> reg;
};