#include <string.h>
#include <ctype.h>
#include "global.h"			// include our global project settings
#include "lpc2000.h"		// include LPC210x defines

#include "processor.h"		// include processor initialization functions
#include "timer.h"			// include timer functions
#include "buffer.h"
#include "uartintr.h"
#include "rprintf.h"

#include "runtime.h"
#include "molecubePacket.h"
#include "morphology.h"
#include "sequenceExec.h"
#include "buttons.h"
#include "hostPacketInjection.h"

#ifdef BLUETOOTH_ENABLE
#include "bluetooth.h"
#else
#include "usb.h"
#endif

void executeSequenceProcess(void)
{
	enableSequenceExec();
	sequenceExecProcess();
}

void executeCreatemorphology(void)
{
	char *filename = "cubes.mmf"; // molecubes morphology file
	int retval;

	// init data structure
	retval = buildMorphology(&cubemorph);
	printOrientationErrorCode(retval);
	if(retval)
		return;

	// deletes previous copy of file and ignores return value in case file doesn't exist
	rmfile(&efs.myFs, (unsigned char*)filename);

	// Build mmf file
	retval = writeMorphologyToFile(filename);
	if(retval)
	{
		rprintf("write failed with ");
		rprintfNum(10,10,TRUE,' ', retval);
		rprintf("\n");
		file_fclose(&filer);
		return;
	} else { rprintf("write successful\n"); }

	// Read mmf file
	retval = buildMorphologyFromFile(filename);
	if(retval)
	{
		rprintf("build failed with ");
		rprintfNum(10,10,TRUE,' ', retval);
		rprintf("\n");
		file_fclose(&filer);
		return;
	} else { rprintf("build successful\n"); }

}

int initController(void)
{
	u08 returnval=0;
	processorInit();

	HD232_Init(); // setup external comm interfaces
	
	#ifdef BLUETOOTH_ENABLE
	rprintfInit((void*)uart0SendByte);
	initBluetooth();
	#else
	usbSetup();
	rprintfInit((void*)VCOM_putchar);
	#endif

	disableSequenceExec();
	resetTimestep();

	timerInit();
	// setup MR0 value
	timer1Match0Set(PCLK/1000); // 1ms step
	// enable timer0 interrupt and reset on MR0 match
	T1MCR |= TMCR_MR0_I | TMCR_MR0_R;
	timerAttach(TIMER1MATCH0_INT, incTimestep);
	
	cmd_Init(); // initialize command buffers

	initButtons(); // initialize buttons
	enableButtons();

	// attach functionality to buttons
	buttonAttach(BUTTON1,(void*)executeCreatemorphology);
	buttonAttach(BUTTON2,(void*)executeSequenceProcess);


	// Initialize microSD card
	returnval = efs_init(&efs, 0);
	
	return returnval;
}

void controllerMenu(void)
{
	u08 run = TRUE;
	int c;

	rprintfProgStrM("CMD>");
	while(run)
	{
		buttonPressService();

		#ifdef BLUETOOTH_ENABLE
		c = uart0GetByte(); // returns -1 if null
		#else
		c = VCOM_getchar(); // returns -1 if null
		#endif
		if(c != -1)
		{
			rprintfChar(c);
			rprintfCRLF();
			switch(c)
			{
			case 'r':
				executeCreatemorphology();
				break;
			case 'x':
				executeSequenceProcess();
				break;
			case '?':
				rprintfProgStrM("Commands:\r\n");
				rprintfProgStrM("(r) Create Morphology File\r\n");
				rprintfProgStrM("(x) Execute Sequence\r\n");
				break;
			case 0x01:
				directCommandInjection();
				break;
			default:
				rprintfProgStrM("Unknown command\r\n");
				break;
			}
			rprintfProgStrM("CMD>");
		}
	}
}

int main(void)
{
	int returnval;
	returnval = initController();

	timerPause(5000);

	if(returnval)
	{
		rprintf("Startup Error\r");
		return -1;
	}
	rprintf("Molecubes Controller Online!\r");


	controllerMenu();
	// Morphology Test Sequence
	/*
	test_morphology();
	
	enableSequenceExec();
	sequenceExecProcess();
	*/
	return 0;
}

