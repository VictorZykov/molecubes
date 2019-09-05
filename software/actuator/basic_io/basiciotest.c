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

#include "global.h"		// include our global settings
#include "uart2.h"		// include uart function library
#include "timer.h"		// include timer function library (timing, PWM, etc)
#include "a2d.h"		// include a2d function library
#include "i2c.h"		// include i2c function library

#include "molecubePacket.h"

#define SOUTH_HALF
#define DEV_ADDR	1
//#define CALIBRATE

u08 enterContinuousRotation(void);

void turnMotorLEDon(void)
{
	u08 params[2];
	params[0] = 0x19;
	params[1] = 0x00;
	cmd_BuildPacket(0xFF, 0x01, 0x03, 2, params);
	cmd_Execute(INTERNAL_SERIAL);
}

void turnMotorLEDoff(void)
{
	u08 params[2];
	params[0] = 0x19;
	params[1] = 0x01;
	cmd_BuildPacket(0xFF, 0x01, 0x03, 2, params);
	cmd_Execute(INTERNAL_SERIAL);	
}
/*
void blinkLed(u16 delay_msec)
{

	turnMotorLEDon();
	timerPause(delay_msec);
	turnMotorLEDoff();
	timerPause(delay_msec);

}
*/
// PCA addresses

#define     M_PCA   0b11000000
#define     L_PCA   0b11000010
#define     R_PCA   0b11000100
#define     All_PCA 0b11100000

void RGB_LED_init(u08 addr, u08 red, u08 green, u08 blue)
{
	u08 PCAstart[10];
    
    PCAstart[0] =  0b10000000;  // Control REG Auto increment for all 
								// PCA9633 registers, starting with 0000
    PCAstart[1] =  0b10000001;  // 00h MODE 1      ?: bit 4
    PCAstart[2] =  0b00000101;  // 01h MODE 2       ?: bit 2
    PCAstart[3] =  red;         // 02h PMW0  RED
    PCAstart[4] =  green;       // 03h PMW1  GREEN
    PCAstart[5] =  blue;        // 03h PMW2  BLUE
    PCAstart[6] =  0x00;        // 05h PMW3  Not used
    PCAstart[7] =  0xFF;        // 06h GRPPWM Overall Brightness
    PCAstart[8] =  0x00;        // 07h GRPFREQ Blinking rate
    PCAstart[9] =  0b10101010;  // 08h LEDOUT LED output state

	i2cMasterSend(addr, 10, PCAstart);

}

void RGB_LED_PWMx(u08 addr, u08 red, u08 green, u08 blue)
{

	u08 PCAstart[4];
    
    PCAstart[0] =  0b10100010;  // Control REG: Auto increment for PWMx 
								// PCA9633 registers, starting with 0010
    PCAstart[1] =  red;			// 02h PMW0  RED
    PCAstart[2] =  green;       // 03h PMW1  GREEN
    PCAstart[3] =  blue;        // 03h PMW2  BLUE

	i2cMasterSend(addr, 4, PCAstart);

}

typedef struct {
	u08 cmd;
	u08 params[128];
	u08 paramCnt;
} RXPKT;

	RXPKT int_busRxPkt;
	RXPKT ext_busRxPkt;
	u08 broadAddress = 0xFE;


u08 packetRxProcess(u08 serBus, u08 devClass, u08 devAddress, cBuffer* rxBuffer)
{
	u08 foundpacket = FALSE;
	u08 startFlag = FALSE;
	u08 rxchksum, chksum = 0;
	RXPKT *trgPkt;
	u16 i;

	// process the receive buffer
	// go through buffer looking for packets
	while((rxBuffer->datalength) > 0)
	{
		// look for a start of NMEA packet
		if((bufferGetAtIndex(rxBuffer,0) == 0xFF) &&
			(bufferGetAtIndex(rxBuffer,1) > 0xFC))

			// comparing two consecutive bits helps avoid nasty bugs 
			// when FF is a part of a packet addressed to a different
			// microprocessor. MAKE SURE there are no sequences that include
			// 2 bytes FF, FA..FF, this will result in bad glitches.
			// Can't get on-the fly frequency switching to work reliably.
		{

			// found start
			startFlag = TRUE;
			// when start is found, we leave it intact in the receive buffer
			// in case the full NMEA string is not completely received.  The
			// start will be detected in the next nmeaProcess iteration.

			// done looking for start
			break;
		}
		else
			bufferGetFromFront(rxBuffer);
	}
	
	// if we detected a start, look for end of packet
	if(startFlag)
	{
		if(rxBuffer->datalength >= 4) // enough of packet has been read to determine length
		{
			u08 p_count = 0;
			if (devClass == 0xFF)	p_count = bufferGetAtIndex(rxBuffer,3) - 2;
			else					p_count = bufferGetAtIndex(rxBuffer,3);
			
			u08 max_rx_buffer_size;
			if(serBus == EXTERNAL_SERIAL)
				max_rx_buffer_size = UART1_RX_BUFFER_SIZE;
			else 
				max_rx_buffer_size = UART0_RX_BUFFER_SIZE;

			if(p_count + 6 > max_rx_buffer_size)
				bufferFlush(rxBuffer);
			else if(rxBuffer->datalength >= p_count + 6)
				// entire packet has been rxed
			{
				// Class and Address match
				if((bufferGetAtIndex(rxBuffer,1) == devClass) && 
					((bufferGetAtIndex(rxBuffer,2) == devAddress) || 
					(bufferGetAtIndex(rxBuffer,2) == broadAddress)))
	     		{

					// grab checksum

					rxchksum = bufferGetAtIndex(rxBuffer,p_count  + 5);
					chksum = 0;
					// calc checksum
					chksum += bufferGetAtIndex(rxBuffer,2); // id
					chksum += bufferGetAtIndex(rxBuffer,3); // p_count
					chksum += bufferGetAtIndex(rxBuffer,4); // cmd
					for(i=0; i<p_count; i++)
					{
						chksum += bufferGetAtIndex(rxBuffer,5+i); // params
					}

					if (devClass == 0xFF)	chksum = ~chksum;

					if(rxchksum == chksum)
					{

						// command successfully received!
						if(serBus == INTERNAL_SERIAL)
						{
							trgPkt = &int_busRxPkt;
						} else {
							trgPkt = &ext_busRxPkt;
						}

						trgPkt->cmd = bufferGetAtIndex(rxBuffer,4);
						trgPkt->paramCnt = p_count;
						for(i=0; i<p_count; i++)
						{
							trgPkt->params[i] = bufferGetAtIndex(rxBuffer,5+i);
						}
						foundpacket = TRUE;
						bufferFlush(rxBuffer);
					} else
						bufferGetFromFront(rxBuffer);
				} else
					bufferGetFromFront(rxBuffer);
			};
		}
/*		else if (CommTimeoutCounter < 1)
		{
			CommTimeoutCounter += 1;
		} 
		else 
		{
			bufferGetFromFront(rxBuffer);
//			bufferFlush(rxBuffer);
			CommTimeoutCounter = 0;
		};
*/	}
	else if(rxBuffer->datalength >= rxBuffer->size)
	{
		// if we found no packet, and the buffer is full
		// we're logjammed, flush entire buffer
		bufferFlush(rxBuffer);
	}

	return foundpacket;
}

u08 GetMainPotAngle(u16* MainPotAngle)
// this function should be used with the South half only - 
// sends out a request to the North half to report ADC readout, 
// measuring the main axis potentiometer output
{
	u08 params[10];
	u16 a;
	cmd_BuildPacket(0xFD, myAddress, 0, 0x14, params);
	cmd_Execute(INTERNAL_SERIAL);		// send ADC readout request
	a = 0;								// wait for response packet

	
	while ((packetRxProcess(INTERNAL_SERIAL, 0xFD, myAddress, 
				uartGetRxBuffer(INTERNAL_SERIAL)) == FALSE) && (a < 10000))
	{	a+=1; }


	if (a < 10000)	// wait intil a serial timeout occurs
	{	// check if the right command is confirmed
		if (int_busRxPkt.cmd == 0x14)
		{	// both conditions checked - save 
			// the rx'd main pot reading
//			return (int_busRxPkt.params[0] | (int_busRxPkt.params[1]<<8));
			*MainPotAngle = (int_busRxPkt.params[0] | (int_busRxPkt.params[1]<<8));
			return TRUE;
		}	// report an error: wrong command confirmed
	}	// report an error: serial communication timeout occurred 
		// when requesting ADC value from the north half   
	return FALSE;
}

u08 GetServoAngle(u16* ServoAngle)
//u16 GetServoAngle(void)
// sends out a request to the AX-12 servo to report its current position
{
	u08 params[10];
	u16 a;
	params[0] = 0x24;
	params[1] = 0x02;		// AX-12 instruction READ

	cmd_BuildPacket(0xFF, 0x01, 2, 0x02, params);
	cmd_Execute(INTERNAL_SERIAL);

	a = 0;
	while ((packetRxProcess(INTERNAL_SERIAL, 0xFF, 0x01, 
			uartGetRxBuffer(INTERNAL_SERIAL)) == FALSE) && (a < 20000))
	{	a+=1;	}
	
	if (a < 20000)	// wait intil a serial timeout occurs
	{				// check if the right command is confirmed
		if (int_busRxPkt.paramCnt == 0x02)
		{	// both conditions checked - save the rx'd main pot reading
//			return (int_busRxPkt.params[0] | (int_busRxPkt.params[1]<<8));
	
			*ServoAngle = (int_busRxPkt.params[0] | (int_busRxPkt.params[1]<<8));
			return TRUE;

//			RGB_LED_PWMx(M_PCA, 0x01, 0xFF, 0x01);
//			timerPause(3);
//			RGB_LED_PWMx(M_PCA, 0.01, 0x01, 0x01);
		}	// report an error: parameter count
	}	// report an error: serial communication timeout occurred 
						// when requesting ADC value from the north half
	return FALSE;
};


u08 GetResponse(u08 serBus, u08 devClass, u08 devAddress)
{
	u16 a = 0;
	while ((packetRxProcess(serBus, devClass, devAddress, 
			uartGetRxBuffer(serBus)) == FALSE) && (a < 20000))
	{	a+=1;	};
	
	if (a < 20000)	// wait intil a serial timeout occurs
	{				// check if the right command is confirmed
		return TRUE;
//		RGB_LED_PWMx(M_PCA, 0x01, 0xFF, 0x01);
//		timerPause(5);
//		RGB_LED_PWMx(M_PCA, 0.01, 0x01, 0x01);
	} // report an error: serial communication timeout occurred 
	return FALSE;
};

u08 SetServoSpeed(u16 goalSpeed, u08 goalDirection)
{
	if (goalSpeed & 0xfc00) return FALSE;

	u08 params[10];
	params[0] = 0x20;
	params[1] = goalSpeed & 0x00ff ;
	params[2] = (goalSpeed & 0xff00)>>8 ;
	if (goalDirection) params[2] = (params[2] | 0b00000100);
	cmd_BuildPacket(0xFF, 0x01, 0x03, 3, params);
	cmd_Execute(INTERNAL_SERIAL);	
	return GetResponse(INTERNAL_SERIAL, 0xFF, 0x01);
}

u08 enterContinuousRotation()
{
	// enter continuous rotation mode
	u08 params[10];
	params[0] = 8;		// CCW angle limit address
	params[1] = 0;		// CCW angle limit (L)
	params[2] = 0;		// CCW angle limit (H)
	cmd_BuildPacket(0xFF, 0x01, 0x03, 3, params);
	cmd_Execute(INTERNAL_SERIAL);	
	return GetResponse(INTERNAL_SERIAL, 0xFF, 0x01);
}


void CalibrateAngles(void)
{

	while(enterContinuousRotation() == FALSE);
	// start moving CCW and continue until the magnet is found
	// magnet determines the servo angle origin location 

	while(SetServoSpeed(500, CCW) == FALSE);
//	u08 MagnetFound = a2dConvert8bit(ADC_CH_ADC5);
//	while (MagnetFound > 128) MagnetFound = a2dConvert8bit(ADC_CH_ADC5);

	while (MagnetFound() == FALSE);



	// once the magnet is found (or if it's already at the magnet)
	// reverse and move high speed CW until servo angle reading 
	// drops down to a small numver


	


	while(SetServoSpeed(500, CW) == FALSE);
//	u16 ServoAngle = GetServoAngle();	// get servo position
	while ((GetServoAngle(&ServoAngle) == FALSE) || (GetServoAngle(&ServoAngle) && (ServoAngle > 80)));
//		ServoAngle = GetServoAngle();
//		while (GetServoAngle(&ServoAngle) == FALSE);
	
	// reduce speed in proximity to servo sensing range,
	// stop at the servo sensing edge

	while(SetServoSpeed(100, CW) == FALSE);

	while ((GetServoAngle(&ServoAngle) == FALSE) || (GetServoAngle(&ServoAngle) && (ServoAngle > 4)));

//	while (ServoAngle > 1) 	
//		while (GetServoAngle(&ServoAngle) == FALSE);
//		ServoAngle = GetServoAngle();

	// Now we are at the beginning of the servo sensing range: 
	// store both Servo Readout and Main Pot Readout for this point
	// in EEPROM, this will be one of the six switching points 
	// between measurement reqions

	u16 MainPotAngle;
	while (GetMainPotAngle(&MainPotAngle) == FALSE);
	u16 CalibP1 = MainPotAngle;
	

	// cross the servo insensitivity gap using the main axis pot:
	// move CW relative to P1Main by 30 degrees (0x5A) over 20 degrees gap

	u16 MainPotGoal = CalibP1 - 70;
//	u16 MainPotGoal = P1Main - 0x5A;

	while(SetServoSpeed(500, CW) == FALSE);

	while ((GetMainPotAngle(&MainPotAngle) == FALSE) || 
		(GetMainPotAngle(&MainPotAngle) && (MainPotAngle > MainPotGoal)));
//	u16 MainPotAngle = GetMainPotAngle();
//	while (MainPotAngle > MainPotGoal) 	
//		while (GetMainPotAngle(&MainPotAngle) == FALSE);
	//	MainPotAngle = GetMainPotAngle();

	// move slowly CCW to find and record the sensitivity edge P2

	while(SetServoSpeed(100, CCW) == FALSE);

	while ((GetServoAngle(&ServoAngle) == FALSE) ||
		(GetServoAngle(&ServoAngle)&& (ServoAngle < 1019)));
//	ServoAngle = GetServoAngle();	   // get servo position
//	while (ServoAngle < 1022) 	ServoAngle = GetServoAngle();
	
	// Save P2

	while (GetMainPotAngle(&MainPotAngle) == FALSE);
	u16 CalibP2 = MainPotAngle;

//	u16 CalibP2 = GetMainPotAngle();

	// approach P3

	while(SetServoSpeed(500, CW) == FALSE);
	while ((GetServoAngle(&ServoAngle) == FALSE) ||
		(GetServoAngle(&ServoAngle) && (ServoAngle > 80)));
//	ServoAngle = GetServoAngle();	   // get servo position
//	while (ServoAngle > 80) 	ServoAngle = GetServoAngle();
	
	// reduce speed in proximity to servo sensing range,
	// stop at the servo sensing edge

	while(SetServoSpeed(100, CW) == FALSE);
	while ((GetServoAngle(&ServoAngle) == FALSE) ||
		(GetServoAngle(&ServoAngle) && (ServoAngle > 4)));
//	while (ServoAngle > 1) 	ServoAngle = GetServoAngle();

	// Now we are at the beginning of the servo sensing range: 
	// store both Servo Readout and Main Pot Readout for this point
	// in EEPROM, this will be one of the six switching points 
	// between measurement reqions

	while (GetMainPotAngle(&MainPotAngle) == FALSE);
	u16 CalibP3 = MainPotAngle;

//	u16 CalibP3 = GetMainPotAngle();

	// cross the servo insensitivity gap using the main axis pot:
	// move CW relative to P1Main by 30 degrees (0x5A) over 20 degrees gap

	MainPotGoal = CalibP3 - 70;
//	u16 MainPotGoal = P1Main - 0x5A;

	while(SetServoSpeed(500, CW) == FALSE);

	while ((GetMainPotAngle(&MainPotAngle) == FALSE) ||
		(GetMainPotAngle(&MainPotAngle) && (MainPotAngle > MainPotGoal)));

//	MainPotAngle = GetMainPotAngle();
//	while (MainPotAngle > MainPotGoal) 	
//		while (GetMainPotAngle(&MainPotAngle) == FALSE);

//		MainPotAngle = GetMainPotAngle();

	// move slowly CCW to find and record the sensitivity edge P2

	while(SetServoSpeed(100, CCW) == FALSE);
	while ((GetServoAngle(&ServoAngle) == FALSE) ||
		(GetServoAngle(&ServoAngle) && (ServoAngle < 1019)));

//	ServoAngle = GetServoAngle();	   // get servo position
//	while (ServoAngle < 1022) 	ServoAngle = GetServoAngle();
	
	// Save P4

	while (GetMainPotAngle(&MainPotAngle) == FALSE);
	u16 CalibP4 = MainPotAngle;

//	u16 CalibP4 = GetMainPotAngle();

	// approach P5

	while(SetServoSpeed(500, CW) == FALSE);
	while ((GetServoAngle(&ServoAngle) == FALSE) ||
		(GetServoAngle(&ServoAngle) && (ServoAngle > 80)));
//	ServoAngle = GetServoAngle();	   // get servo position
//	while (ServoAngle > 80) 	ServoAngle = GetServoAngle();
	
	// reduce speed in proximity to servo sensing range,
	// stop at the servo sensing edge

	while(SetServoSpeed(100, CW) == FALSE);
	while ((GetServoAngle(&ServoAngle) == FALSE) ||
		(GetServoAngle(&ServoAngle) && (ServoAngle > 4)));
//	while (ServoAngle > 1) 	ServoAngle = GetServoAngle();

	// Now we are at the beginning of the servo sensing range: 
	// store both Servo Readout and Main Pot Readout for this point
	// in EEPROM, this will be one of the six switching points 
	// between measurement reqions

	while (GetMainPotAngle(&MainPotAngle) == FALSE);
	u16 CalibP5 = MainPotAngle;

//	u16 CalibP5 = GetMainPotAngle();

	// cross the servo insensitivity gap using the main axis pot:
	// move CW relative to P1Main by 30 degrees (0x5A) over 20 degrees gap

	MainPotGoal = CalibP5 - 70;
//	u16 MainPotGoal = P1Main - 0x5A;

	while(SetServoSpeed(500, CW) == FALSE);
	while ((GetMainPotAngle(&MainPotAngle) == FALSE) ||
		(GetMainPotAngle(&MainPotAngle) && (MainPotAngle > MainPotGoal)));
//	MainPotAngle = GetMainPotAngle();
//	while (MainPotAngle > MainPotGoal) 	
//		while (GetMainPotAngle(&MainPotAngle) == FALSE);
//		MainPotAngle = GetMainPotAngle();

	// move slowly CCW to find and record the sensitivity edge P2

	while(SetServoSpeed(100, CCW) == FALSE);
//	ServoAngle = GetServoAngle();	   // get servo position
	while ((GetServoAngle(&ServoAngle) == FALSE) ||
		(GetServoAngle(&ServoAngle) && (ServoAngle < 1019)));
//	while (ServoAngle < 1022) 	ServoAngle = GetServoAngle();
	
	// Save P4

	while (GetMainPotAngle(&MainPotAngle) == FALSE);
	u16 CalibP6 = MainPotAngle;

//	u16 CalibP6 = GetMainPotAngle();

	// save calibration values to EEPROM

	eeprom_busy_wait();		eeprom_write_word((uint16_t*)P1ptr, CalibP1);
	eeprom_busy_wait();		eeprom_write_word((uint16_t*)P2ptr, CalibP2);
	eeprom_busy_wait();		eeprom_write_word((uint16_t*)P3ptr, CalibP3);
	eeprom_busy_wait();		eeprom_write_word((uint16_t*)P4ptr, CalibP4);
	eeprom_busy_wait();		eeprom_write_word((uint16_t*)P5ptr, CalibP5);
	eeprom_busy_wait();		eeprom_write_word((uint16_t*)P6ptr, CalibP6);

	while(SetServoSpeed(0, CCW) == FALSE);

};

u08 MagnetFound(void)
{
//	if (a2dConvert8bit(ADC_CH_ADC5) > 128) 	return FALSE;
//	else									return TRUE;

	cbi(DDRA, PA5); // set Port A pin 5 as input

	if (PINA && 0b00100000) HallSensor = FALSE;
	else					HallSensor = TRUE;

	return HallSensor;

};

u16 CalculateAngle(void)
// this routine calculates the global Molecube angle from sensor data
// and saved in EEPROM angle calibration information
{
	u16 Angle = 0;
	// first, check if a magnet is sensed
	
	if (MagnetFound()) 
	{
		while (GetServoAngle(&ServoAngle) == FALSE);
//		ServoAngle = GetServoAngle();
		if (ServoAngle < 0x01ff)
		{
			Angle = 3600 - ((511 - ServoAngle) - ((511 - ServoAngle) / 50));
		}
		else
		{
			Angle = (ServoAngle - 511) - (ServoAngle - 511) / 50;
		}
	}
	else
	{

		while (GetMainPotAngle(&MainPotAngle) == FALSE);
		
//		MainPotAngle = GetMainPotAngle();
		if (MainPotAngle >= CalibP1)
		// for angles greater than 280.0 degrees
		{
			while (GetServoAngle(&ServoAngle) == FALSE);
//			u16 ServoAngle = GetServoAngle();
			Angle = 3600 - ((511 - ServoAngle) - ((511 - ServoAngle) / 50));
		}
		else if ((MainPotAngle < CalibP1) && (MainPotAngle > CalibP2))
		{
			Angle = 2899 + 3 * (MainPotAngle - CalibP2) + 
				(MainPotAngle - CalibP2) / 3 + (MainPotAngle - CalibP2) / 120 ;
		}
		else if ((MainPotAngle <= CalibP2) && (MainPotAngle >= CalibP3))
		{
	//		u16 ServoAngle = GetServoAngle();
			while (GetServoAngle(&ServoAngle) == FALSE);
			Angle = 1901 + (ServoAngle-1) + (ServoAngle-1) / 50;
		}
		else if ((MainPotAngle < CalibP3) && (MainPotAngle > CalibP4))
		{
			Angle = 1699 + 3 * (MainPotAngle - CalibP4) + 
				(MainPotAngle - CalibP4) / 3 + (MainPotAngle - CalibP4) / 120 ;
		}
		else if ((MainPotAngle <= CalibP4) && (MainPotAngle >= CalibP5))
		{
			while (GetServoAngle(&ServoAngle) == FALSE);
	//		u16 ServoAngle = GetServoAngle();
			Angle = 701 + (ServoAngle-1) + (ServoAngle-1) / 50;
		}
		else if ((MainPotAngle < CalibP5) && (MainPotAngle > CalibP6))
		{
			Angle = 499 + 3 * (MainPotAngle - CalibP6) + 
				(MainPotAngle - CalibP6) / 3 + (MainPotAngle - CalibP6) / 120 ;
		}
		else if (MainPotAngle <= CalibP6)
		{
			while (GetServoAngle(&ServoAngle) == FALSE);
	//		u16 ServoAngle = GetServoAngle();
			Angle = (ServoAngle - 511) - (ServoAngle - 511) / 50;
		}
	}

	return Angle;

}

//----- Begin Code ------------------------------------------------------------
int main(void)
{
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

	// write myAddress to EEPROM
	eeprom_busy_wait();		eeprom_write_word((uint16_t*)AddrPtr, DEV_ADDR);
	// read in myAddress from EEPROM
	eeprom_busy_wait();		
	myAddress = eeprom_read_word((const uint16_t*)AddrPtr);


#ifdef SOUTH_HALF
	// read in calibration values from EEPROM
	eeprom_busy_wait();		CalibP1 = eeprom_read_word((const uint16_t*)P1ptr);
	eeprom_busy_wait();		CalibP2 = eeprom_read_word((const uint16_t*)P2ptr);
	eeprom_busy_wait();		CalibP3 = eeprom_read_word((const uint16_t*)P3ptr);
	eeprom_busy_wait();		CalibP4 = eeprom_read_word((const uint16_t*)P4ptr);
	eeprom_busy_wait();		CalibP5 = eeprom_read_word((const uint16_t*)P5ptr);
	eeprom_busy_wait();		CalibP6 = eeprom_read_word((const uint16_t*)P6ptr);

// writing a data value into the control table of AX-12

// setting goal position and speed (within servo range)

/*	
	u08 params[5];
	params[0] = 30;
	params[1] = 100;	// Position LSB
	params[2] = 3;		// Position MSB
	params[3] = 250;	// Speed LSB
	params[4] = 1;		// Speed MSB
	cmd_BuildPacket(0xFF, 0x01, 0x03, 5, params);
	cmd_Execute(INTERNAL_SERIAL);	

*/

// setting goal position (within servo range)
	
/*	

	u08 params[3];
	params[0] = 30;
	params[1] = 100;
	params[2] = 0;
	cmd_BuildPacket(0xFF, 0x01, 0x03, 3, params);
	cmd_Execute(INTERNAL_SERIAL);	

*/

// exit continuous rotation mode

/*
	u08 params[3];
	params[0] = 8;
	params[1] = 255;
	params[2] = 3;
	cmd_BuildPacket(0xFF, 0x01, 0x03, 3, params);
	cmd_Execute(INTERNAL_SERIAL);	

*/

/*
		// re-write communication frequency
		UBRR0L = 0x01;   // Baud Rate 500k	/	1M
		timerPause(20);
		u08 params[7];
		params[0] = 0x04;			//	baud rate memory address
//		params[1] = 0x03 ;			//  baud rate value, 500k
		params[1] = 0x05 ;			//  baud rate value, 333k
		cmd_BuildPacket(0xFF, 0x01, 2, 0x03, params);
		cmd_Execute(INTERNAL_SERIAL);
		UBRR0L = 0x02;   // Baud Rate 333k	/	667k
		timerPause(20);
*/
		
/*	// Led colors

	u08 NMRed = 25;		u08 NMGrn = 25;		u08 NMBlu = 25;
	u08 NLRed = 25;		u08 NLGrn = 25;		u08 NLBlu = 25;
	u08 NRRed = 25;		u08 NRGrn = 25;		u08 NRBlu = 25;
	u08 SMRed = 25;		u08 SMGrn = 25;		u08 SMBlu = 25;
	u08 SLRed = 25;		u08 SLGrn = 25;		u08 SLBlu = 25;
	u08 SRRed = 25;		u08 SRGrn = 25;		u08 SRBlu = 25;
		
*/

#ifdef CALIBRATE
	CalibrateAngles();		// Also sets the servo into a continuous rotation mode
	while(1);
#endif
	//-------------------------------------------------------------
	//	SOUTH HALF
	//-------------------------------------------------------------

	myClass = 0xFE; // south half class

	u16 GoalAngle = 2400;
	u16 GoalSpeed = 1000;
	u08 Precision = 4;
	u08 Ramp = 10;
	u08 Jerk = 50;

	u08 Direction;
	u16 OppositeToGoal;
	u16 FixedSpeed;
	u16 Angle;
	u16 Error;

	u08 ServoModeOn = FALSE;	

	u08 params[100];

	while (1);
	{

		// Module always tracks its angle
		Angle = CalculateAngle();

		//	Check for incoming packets on external bus
		if (packetRxProcess(EXTERNAL_SERIAL, myClass, myAddress, 
			uartGetRxBuffer(EXTERNAL_SERIAL))) 
		{

			//*****************************************************
			// 0x04 - conditional address assignment (if any pin high)
			if (ext_busRxPkt.cmd == 0x04)
			{	// check all south pins
				
				// assuming that pins are assigned to output 
				// on a temporary basis only, otherwise they are input

				u08 ActivePinFound = FALSE;

				// checking if any input pin is active
				if ((PINA && 0b11000111) || 
					(PINB && 0b00000001) ||
					(PINC && 0b11001100) ||
					(PIND && 0b11000000)) ActivePinFound = TRUE;
		



				myAddress = ext_busRxPkt.params[0];
				eeprom_busy_wait();							
				eeprom_write_word((uint16_t*)AddrPtr, myAddress);	
				
				// report success
				cmd_BuildPacket(myClass, myAddress, 0, 0x04, params);
				cmd_Execute(EXTERNAL_SERIAL);
			}

			
			
			//*****************************************************
			// 0x06 - report raw angle calculation data
			else if (ext_busRxPkt.cmd == 0x06)
			{
				params[0] = MainPotAngle & 0x00ff ;
				params[1] = (MainPotAngle & 0xff00)>>8 ;
				params[2] = ServoAngle & 0x00ff ;
				params[3] = (ServoAngle & 0xff00)>>8 ;
				params[4] = HallSensor ;
				cmd_BuildPacket(myClass, myAddress, 5, 0x07, params);
				cmd_Execute(EXTERNAL_SERIAL);
			}

			//*****************************************************
			// 0x07 - report measured angle
			else if (ext_busRxPkt.cmd == 0x07)
			{
				params[0] = Angle & 0x00ff ;
				params[1] = (Angle & 0xff00)>>8 ;
				cmd_BuildPacket(myClass, myAddress, 2, 0x07, params);
				cmd_Execute(EXTERNAL_SERIAL);
			}


			//*****************************************************
			// 0x12 - set LEDS
			else if (ext_busRxPkt.cmd == 0x12)
			{
				u08 Side = M_PCA;
				if (ext_busRxPkt.params[0] == 1) Side = R_PCA;
				if (ext_busRxPkt.params[0] == 2) Side = L_PCA;

				RGB_LED_PWMx(Side, ext_busRxPkt.params[1], 
					ext_busRxPkt.params[2], ext_busRxPkt.params[3]);

				cmd_BuildPacket(myClass, myAddress, 0, 0x12, params);
				cmd_Execute(EXTERNAL_SERIAL);
			}


			//*****************************************************
			// 0x15 - set servo goal speed and angle
			else if (ext_busRxPkt.cmd == 0x15)
			{



				ServoModeOn = TRUE;	

				GoalAngle = (ext_busRxPkt.params[0] | (ext_busRxPkt.params[1]<<8));
				GoalSpeed = (ext_busRxPkt.params[2] | (ext_busRxPkt.params[3]<<8));
				// include delayed response for motor commands ???
				cmd_BuildPacket(myClass, myAddress, 0, 0x15, params);
				cmd_Execute(EXTERNAL_SERIAL);
			}
		} // if (packetRxProcess)
			
		if (ServoModeOn)
		{
			// servo to a goal angle over the shortest path with the set speed
		
			// determine direction by comparing two chords
			if (GoalAngle <= 1800)
			{	OppositeToGoal = GoalAngle + 1800;
				if ((Angle > GoalAngle) && (Angle < OppositeToGoal))
				{	Direction = CW;		}
				else
				{	Direction = CCW;	}
			}
			else
			{	OppositeToGoal = GoalAngle - 1800;
				if ((Angle < GoalAngle) && (Angle > OppositeToGoal))
				{	Direction = CCW;	}
				else
				{	Direction = CW;		}
			};

			// determine error
			if (Direction == CW)
			{ 	if (Angle > GoalAngle)	{	Error = Angle - GoalAngle;				}
				else					{	Error = Angle + (3600 - GoalAngle);		};
			}
			else
			{	if (Angle < GoalAngle)	{	Error = GoalAngle - Angle;				}
				else					{	Error = GoalAngle + (3600 - Angle);		};
			};

			// determine speed
			if (Error > Precision)
			{	FixedSpeed = Error * Ramp + Jerk;
				if (GoalSpeed < FixedSpeed)
				{	SetServoSpeed(GoalSpeed, Direction);	}
				else	
				{	SetServoSpeed(FixedSpeed, Direction);	};
			}
			else	
			{	SetServoSpeed(0, Direction);	};
			
			
		}
			
	//		params[0] = Angle & 0x00ff ;
	//		params[1] = (Angle & 0xff00)>>8 ;
	//		cmd_BuildPacket(myClass, myAddress, 2, 0x07, params);
	//		cmd_Execute(EXTERNAL_SERIAL);
	
	}

#endif
#ifndef SOUTH_HALF
	//-------------------------------------------------------------
	//	NORTH HALF
	//-------------------------------------------------------------

	u08	myClass = 0xFD;		// north half class

	u08 params[100];

	while(1) 
	{
		//	Check for incoming packets on external bus
		if (packetRxProcess(EXTERNAL_SERIAL, myClass, myAddress, 
										uartGetRxBuffer(EXTERNAL_SERIAL))) 
		{
			//*****************************************************
			// 0x12 - set LEDS
			if (ext_busRxPkt.cmd == 0x12)
			{
				u08 Side = M_PCA;
				if (ext_busRxPkt.params[0] == 1) Side = R_PCA;
				if (ext_busRxPkt.params[0] == 2) Side = L_PCA;

				RGB_LED_PWMx(Side, ext_busRxPkt.params[1], 
					ext_busRxPkt.params[2], ext_busRxPkt.params[3]);

				cmd_BuildPacket(myClass, myAddress, 0, 0x12, params);
				cmd_Execute(EXTERNAL_SERIAL);
			}
		}


		//	Check for incoming packets on internal bus
		if (packetRxProcess(INTERNAL_SERIAL, myClass, myAddress, 
										uartGetRxBuffer(INTERNAL_SERIAL))) 
		{

			//*****************************************************
			// 0x03 - ADC readout
			if (int_busRxPkt.cmd == 0x03)
			{	// re-writes myAddress in EEPROM
				eeprom_busy_wait();							
				eeprom_write_word((uint16_t*)AddrPtr, int_busRxPkt.params[0]);	
				
				// report success
				cmd_BuildPacket(myClass, myAddress, 0, 0x03, params);
				cmd_Execute(INTERNAL_SERIAL);
			}


			//*****************************************************
			// 0x14 - ADC readout
			else 
				
				if (int_busRxPkt.cmd == 0x14)
			{
	//			RGB_LED_PWMx(M_PCA, 0x01, 0xFF, 0x01);
	//			timerPause(10);
	//			RGB_LED_PWMx(M_PCA, 0x01, 0x01, 0x01);


				// read in ADC value
				u16 MainPotAngle = 0;
				MainPotAngle = a2dConvert10bit(ADC_CH_ADC5);

				// 10 bit ADC readout (seems to correctly transfer into u16 type):
				//            7     6     5     4     3     2     1     0
				// (0x79)  | --- | --- | --- | --- | --- | --- | AD9 | AD8		ADCH
				// (0x78)  | AD7 | AD6 | AD5 | AD4 | AD3 | AD2 | AD1 | AD0		ADCL

				// return ADC value in response packet
				u08 params[4];
				params[0] = MainPotAngle & 0x00ff ;
				params[1] = (MainPotAngle & 0xff00)>>8 ;
				cmd_BuildPacket(myClass, myAddress, 2, 0x14, params);
				cmd_Execute(INTERNAL_SERIAL);


			};
		}; 

//	cmd_BuildPacket(myClass, myAddress, 0, 0x25, params);
//	cmd_Execute(EXTERNAL_SERIAL);




//		packet reception below works! at up to 500kbaud
/*
		if (packetRxProcess(EXTERNAL_SERIAL, uartGetRxBuffer(EXTERNAL_SERIAL))) 
		{
			RGB_LED_PWMx(M_PCA, 0x00, 0xFF, 0x00);
		}; 
*/

//		transmission to ARM works!
/*
			u08 params[2];
			params[0] = 0x26;
			params[1] = 0xa5;
			cmd_BuildPacket(0xFE, 0x01, 0x12, 2, params);
			cmd_Execute(EXTERNAL_SERIAL);
	
*/

//		Debugging External serial reception
/*
		if (uartReceiveByte(EXTERNAL_SERIAL, c)) d =~d;
		
		if (d) RGB_LED_PWMx(M_PCA, 0xFF, 0x00, 0x00);
		else RGB_LED_PWMx(M_PCA, 0x00, 0xFF, 0x00);

		timerPause(10);
*/


//		Blinking LEDs with period proportional to ADC input voltage

/*
		c1 = 16*(sin((a/255.0) * 360.0) / (2.0*M_PI));
		c2 = 16*(sin((a/255.0) * 360.0 + 120.0) / (2.0*M_PI));
		c3 = 16*(sin((a/255.0) * 360.0 + 240.0) / (2.0*M_PI));

		c = a2dConvert8bit(ADC_CH_ADC5);
		
		blinkLed(c);

		RGB_LED_PWMx(M_PCA, c1, c2, c3);
		RGB_LED_PWMx(L_PCA, c2, c3, c1);
		RGB_LED_PWMx(R_PCA, c3, c1, c2);

		a+=1;
*/

//  set let brightness proportional to the adc input

/*		c = a2dConvert8bit(ADC_CH_ADC5);
		RGB_LED_PWMx(M_PCA, c, 0, 0);
		RGB_LED_PWMx(L_PCA, 0, c, 0);
		RGB_LED_PWMx(R_PCA, 0, 0, c);
*/




	};
#endif
	return 0;
};
