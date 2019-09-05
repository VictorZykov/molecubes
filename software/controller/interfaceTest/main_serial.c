#include <string.h>
#include <ctype.h>
#include "global.h"			// include our global project settings
#include "lpc2000.h"		// include LPC210x defines

#include "processor.h"		// include processor initialization functions
#include "timer.h"			// include timer functions
#include "buffer.h"
#include "uartintr.h"
#include "rprintf.h"
#include "usb.h"

#include "buttons.h"
#include "bluetooth.h"
#include "runtime.h"
#include "molecubePacket.h"


void printButton2(void)
{
	rprintf("Button2\n\r");
}

void printButton1(void)
{
	rprintf("Button1\n\r");
}

int main(void)
{
	// initialize processor
	processorInit();
	
	HD232_Init();
	timerInit();

	#ifdef BLUETOOTH_ENABLE
	initBluetooth();
	rprintfInit((void*)uart0SendByte);
	#else
	usbSetup();
	rprintfInit((void*)VCOM_putchar);
	#endif
	
	initButtons();
	buttonAttach(BUTTON1, (void*)printButton1);
	buttonAttach(BUTTON2, (void*)printButton2);
	enableButtons();

	timerPause(5000); // short delay before sequence is executed
	rprintf("Molecubes Controller Online!\r");

	while(1)
	{
		buttonPressService();
	}

	return 0;
}
