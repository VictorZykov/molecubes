#include "global.h"
#include "buttons.h"

#include "lpc2000.h"
#include "processor.h"

u08 button1, button2, buttonsEnabled;

typedef void (*voidFuncPtr)(void);
volatile static voidFuncPtr ButtonIntrFunc[BUTTON_NUM_INTERRUPTS];

// Interrupt Function Declaration
void __attribute__ ((interrupt("IRQ"))) EINT1_routine(void);
void __attribute__ ((interrupt("IRQ"))) EINT2_routine(void);

void __attribute__ ((interrupt("IRQ"))) EINT1_routine(void)
{
	VICIntEnClear |= (1<<VIC_EINT1);

	button1 = TRUE;

	EXTINT=0x02; //Clear EINT1 interrupt flag
	VICIntEnable |= (1<<VIC_EINT1);
	VICVectAddr=0; //Dummy write to signal end of interrupt
}

void __attribute__ ((interrupt("IRQ"))) EINT2_routine(void)
{
	VICIntEnClear |= (1<<VIC_EINT2);

	button2 = TRUE;

	EXTINT=0x04; //Clear EINT1 interrupt flag
	VICIntEnable |= (1<<VIC_EINT2);
	VICVectAddr=0; //Dummy write to signal end of interrupt
}

void initButtons(void)
{
	// Button pins
	// P0.3 EINT1
	// P0.7 EINT2
	// catch falling edge
	
	PINSEL0 |= 0x000000C0; // set P0.3 to EINT1
	PINSEL0 |= 0x0000C000; // set P0.7 to EINT2

	// Catch falling edge on both buttons
	EXTMODE = 0x06;
	EXTINT = 0x06;
	
	// setup EINT1 for IRQ
	// set interrupt as IRQ
	VICIntSelect &= ~(1<<VIC_EINT1 || 1<<VIC_EINT2);
	// assign VIC slot
	VICVectCntl12 = VIC_ENABLE | VIC_EINT1;
	VICVectAddr12 = (u32)EINT1_routine;
	VICVectCntl13 = VIC_ENABLE | VIC_EINT2;
	VICVectAddr13 = (u32)EINT2_routine;
	// enable interrupt
	VICIntEnable |= (1<<VIC_EINT1);
	VICIntEnable |= (1<<VIC_EINT2);

	button1 = FALSE;
	button2 = FALSE;
	buttonsEnabled = FALSE;

	buttonDetach(BUTTON1);
	buttonDetach(BUTTON2);
}

void enableButtons(void)
{
	buttonsEnabled = TRUE;
}

void disableButtons(void)
{
	buttonsEnabled = FALSE;
}

void buttonAttach(u08 buttonNum, void (*userFunc)(void) )
{
	// make sure the interrupt number is within bounds
	if(buttonNum < BUTTON_NUM_INTERRUPTS)
	{
		// set the interrupt function to run
		// the supplied user's function
		ButtonIntrFunc[buttonNum] = userFunc;
	}
}

void buttonDetach(u08 buttonNum)
{
	// make sure the interrupt number is within bounds
	if(buttonNum < BUTTON_NUM_INTERRUPTS)
	{
		// set the interrupt function to run nothing
		ButtonIntrFunc[buttonNum] = 0;
	}
}

void buttonPressService(void)
{
	if(buttonsEnabled)
	{
		if(button1)
		{
			// if a user function is defined, execute it
			if(ButtonIntrFunc[BUTTON1])
				ButtonIntrFunc[BUTTON1]();
			button1 = FALSE;
		}
		if(button2)
		{
			// if a user function is defined, execute it
			if(ButtonIntrFunc[BUTTON2])
				ButtonIntrFunc[BUTTON2]();
			button2 = FALSE;
		}
	}
}

