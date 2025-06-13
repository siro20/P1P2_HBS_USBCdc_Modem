

// Sample rate settings

// Oversample the signal by 16 times
#define UART_OVERSAMPLING_RATE 16
// FIR filtered reduces samplerate by 0.5
#define FIR_OVERSAMPLING_RATE (UART_OVERSAMPLING_RATE * 2)
// ADC operates on the FIR filter input sample rate
#define ADC_OVERSAMPLING_RATE FIR_OVERSAMPLING_RATE

// P1P2 bus settings
#define UART_BAUD_RATE 9600
#define BUS_HIGH_MV 1400
#define BUS_LOW_MV 600

// ADC settings
#define ADC_REF_VOLTAGE_MV 3000
#define ADC_DMA_BUFFER_SIZE 256
#define ADC_EXTERNAL_GAIN 1.666

// TX power safe mode and high impedance mode in micro seconds
// The LM4871 has a startup time of 15 msec.
// The TS4990 has a startup time of 32 msec.
// The P1P2 Daikin bus has a minimum 25msec silence between packets.
#define TX_POWERON_TIMEOUT_US 32000
#define TX_INTER_PACKET_DELAY 5000
#define TX_ONE_CHAR_TIMEOUT_US 2000
#define TX_RX_TIMEOUT_US 2000

// Max line busy time in milli seconds
#define LINE_BUSY_TIMEOUT_MS 500