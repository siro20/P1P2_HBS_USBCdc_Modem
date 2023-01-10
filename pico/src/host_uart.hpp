#include "fifo_irqsafe.hpp"
#include "line_receiver_irqsafe.hpp"

#include "message.hpp"

#define MAX_PACKET_SIZE 32

// High level abstraction of UART
class HostUART
{
	public:
		HostUART(void);
		~HostUART(void);

		static HostUART& getInstance(void)
		{
			__scratch_y("host_uart_instance") static HostUART instance;
			return instance;
		}

		HostUART(HostUART const&) = delete;
		void operator=(HostUART const&) = delete;

		enum UART_PARITY {
			PARITY_NONE = 0,
			PARITY_EVEN,
			PARITY_ODD,
		};

		void OnLineReceived(char *line);

		void UpdateAndSend(Message& m);
		void Send(Message& m);
		Message Pop(void);

		void SetTime(uint64_t t);
		uint64_t GetTime(void);

		void Check(void);
		bool HasData(void);

		void CheckRXFIFO(void);
		void CheckTXFIFO(void);
	private:

		void CheckRxLine(void);

		// time is the offset to host time. can be negative.
		int64_t time;
		// error is true on buffer overrun. Should never happen.
		bool error;
		FifoIrqSafe<uint8_t, 128> tx_fifo;
		LineReceiverIrqSafe<char, 128> rx_fifo;
		FifoIrqSafe<Message, 8> rx_msgs;
};
