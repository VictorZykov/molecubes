#include <string.h>
#include "global.h"			// include our global project settings
#include "lpc2000.h"		// include LPC210x defines

#include "processor.h"		// include processor initialization functions
#include "timer.h"			// include timer functions
#include "buffer.h"

#include "uartintr.h"
#include "spi.h"
#include "a2d.h"
#include "i2c.h"
#include "rprintf.h"
#include "usb.h"
#include "uartintr.h"

#include "molecubePacket.h"
#include "commands.h"

void commandTestSequence(void)
{
	int returnval;
	while(1)
	{
		if((returnval = setLEDs(0x00, 0xFC, SIDEID_0, 0xF0, 0xF0, 0x00)))
		{
			printOrientationErrorCode(returnval);
//			timerPause(500);
		}
		timerPause(50);
		if((returnval = setLEDs(0x00, 0xFC, SIDEID_0, 0x00, 0x00, 0xF0)))
		{
			printOrientationErrorCode(returnval);
//			timerPause(500);
		}
		timerPause(50);
	}
}

int main(void)
{
	// initialize processor
	processorInit();
	
	usbSetup();
	rprintfInit((void*)VCOM_putchar);
	
	HD232_Init();
	timerInit();
	
	cmd_Init();

	commandTestSequence();

	return 0;
}
