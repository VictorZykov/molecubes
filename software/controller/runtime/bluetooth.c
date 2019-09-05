#include "global.h"
#include "bluetooth.h"
#include "molecubePacket.h"

void initBluetooth(void)
{
	// Set control pin
	IO1DIR |= (1<<INT_RADIO_ISOLATE);
	IO1CLR |= (1<<INT_RADIO_ISOLATE);

	// Initialize Baudrate
	//uart0Init(UART_BAUD(115200), UART_8N1, UART_FIFO_8);
	U0LCR = ULCR_DLAB_ENABLE;
	U0FDR = 0xE5;
	U0DLL = 24;
	U0DLM = 0;
	U0LCR = UART_8N1;

	// Disable Internal Serial Bus
	RS232_INT_DISABLE_RX;
	RS232_INT_DISABLE_TX;
	// Enable Radio Interface
	RS232_INT_RADIO_ENABLE;
}
