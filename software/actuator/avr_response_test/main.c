//*****************************************************************************
// File Name	: basiciotest.c
// 
// Title		: example usage of basic input and output functions on the AVR
//
//*****************************************************************************

//----- Include Files ---------------------------------------------------------
#include <math.h>
#include <avr/io.h>		// include I/O definitions (port names, pin names, etc)
#include <avr/interrupt.h>	// include interrupt support
#include "avr/eeprom.h"		// include eeprom function library
#include <stdlib.h>

#include "global.h"		// include our global settings
#include "uart2.h"		// include uart function library
#include "timer.h"		// include timer function library (timing, PWM, etc)
#include "a2d.h"		// include a2d function library
#include "i2c.h"		// include i2c function library

#include "runtime.h"
#include "molecubePacket.h"
#include "ledDriver.h"


//#define MASTER


u08 GetResponse(u08 serBus, u08 devClass, u08 devAddress)
{
	u32 a = 0;
	while ((packetRxProcess(serBus, devClass, devAddress, 
			uartGetRxBuffer(serBus)) == FALSE) && (a < CMD_RESPONSE_TIMEOUT))
	{	a+=1;	}
	
	if (a < CMD_RESPONSE_TIMEOUT)	// wait intil a serial timeout occurs
		return NO_ERROR;
	else
		return GET_RESPONSE_TIMEOUT_ERROR;
}

void MakeAllPinsInputs(void)
{
	DDRA &= 0b00111000; 	PORTA &= 0b00111000;
	DDRB &= 0b11111110; 	PORTB &= 0b11111110;
	DDRC &= 0b00110011; 	PORTC &= 0b00110011;
	DDRD &= 0b00111111; 	PORTD &= 0b00111111;
}

void displayErrorMessage(int code)
{
	switch(code)
	{
	case CMDPROCESSOR_PARAMETER_ERROR:
		RGB_LED_PWMx(R_PCA, 0x80, 0x00, 0x00);
		timerPause(10);
		RGB_LED_PWMx(R_PCA, 0x00, 0x00, 0x00);
		break;
	case CMDPROCESSOR_UNKNOWN_CMD_ERROR:
		RGB_LED_PWMx(R_PCA, 0x00, 0x80, 0x00);
		timerPause(10);
		RGB_LED_PWMx(R_PCA, 0x00, 0x00, 0x00);
		break;
	}

	
}

int commandProcessor(RXPKT pkt)
{
	// 0x12 - set LEDS
	if (pkt.cmd == 0x12)
	{
		u08 side;
		if(pkt.params[0] == 0)
			side = M_PCA;
		else if(pkt.params[0] == 1)
			side = R_PCA;
		else if(pkt.params[0] == 2)
			side = L_PCA;
		else
			return CMDPROCESSOR_PARAMETER_ERROR;

		RGB_LED_PWMx(side, pkt.params[1], pkt.params[2], pkt.params[3]);

		cmd_BuildPacket(myClass, myAddress, 0, pkt.cmd, NULL);
		cmd_Execute(EXTERNAL_SERIAL);

		return NO_ERROR;
	}
	return CMDPROCESSOR_UNKNOWN_CMD_ERROR;
}


//----- Begin Code ------------------------------------------------------------
int main(void)
{
	int returnval;

	myClass = controllerDuetClass;
	myAddress = 0x00;

	// initialize all libraries
	// init custom comm interface
	configCommBuses();
	EXT_RX_ENABLE;
	EXT_TX_DISABLE;
	INT_RX_ENABLE;
	INT_TX_DISABLE;

	// init command handling framework
	cmd_Init();

	// initialize the timer system
	timerInit();
	a2dInit();
	i2cInit();

	// init LEDs
	RGB_LED_init(M_PCA, 0x01, 0x01, 0x01);
	RGB_LED_init(R_PCA, 0x01, 0x01, 0x01);
	RGB_LED_init(L_PCA, 0x01, 0x01, 0x01);

	MakeAllPinsInputs();

	//--------------------------------------------------------------------------------------
	//	MAIN CONTROL LOOP STARTS HERE
	//--------------------------------------------------------------------------------------


#ifdef MASTER

	while(1)
	{
		timerPause (25);
		
		u08 params[5];
		params[0] = 0;
		params[1] = 0x0;
		params[2] = 0x0;
		params[3] = 0x40;
									
		cmd_BuildPacket(controllerDuetClass, myAddress, 4, 0x12, params);
		cmd_Execute(EXTERNAL_SERIAL);	


		timerPause (25);
		
		params[0] = 0;
		params[1] = 0x40;
		params[2] = 0x0;
		params[3] = 0x0;
									
		cmd_BuildPacket(controllerDuetClass, myAddress, 4, 0x12, params);
		cmd_Execute(EXTERNAL_SERIAL);	

	};

#endif


	while(1)
	{
		//	Check for incoming packets on INTERNAL bus
		if (packetRxProcess(
				EXTERNAL_SERIAL,
				myClass, 
				myAddress, 
				uartGetRxBuffer(EXTERNAL_SERIAL))
			) 
		{
			if((returnval = commandProcessor(ext_busRxPkt)))
				displayErrorMessage(returnval);
		}
	}
	return 0;
}
