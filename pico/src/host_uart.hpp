#include "circular_buffer_spinlock.hpp"
#include "line_receiver.hpp"
#include "message.hpp"

#define MAX_PACKET_SIZE 32

// High level abstraction of UART
class HostUART : public LineReceiver<128>
{
	public:
		HostUART(void);
		~HostUART(void);

		static HostUART& getInstance(void)
		{
			static HostUART instance;
			return instance;
		}

		HostUART(HostUART const&) = delete;
		void operator=(HostUART const&) = delete;

		enum UART_PARITY {
			PARITY_NONE = 0,
			PARITY_EVEN,
			PARITY_ODD,
		};

		void OnLineReceived(uint8_t *line);

	void UpdateAndSend(Message& m);
	void Send(Message& m);

	void SetTime(uint64_t t);
	uint64_t GetTime(void);

	void Check(void);
	bool HasData(void);

	private:
		void CheckRXFIFO(void);
		void CheckTXFIFO(void);

		// time is the offset to host time. can be negative.
		int64_t time;
		// error is true on buffer overrun. Should never happen.
		bool error;
		CircularBufferSpinlock<uint8_t, 128> tx_fifo;

		CircularBufferSpinlock<Message, 8> rx_fifo;

};