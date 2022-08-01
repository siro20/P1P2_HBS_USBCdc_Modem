

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
#define ADC_REF_VOLTAGE_MV 3300
#define ADC_DMA_BUFFER_SIZE 256