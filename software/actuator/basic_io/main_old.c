//*****************************************************************************
// File Name	: main.c
// 
// Title		: This software supports low level functionality of Molecube 
//				  robotic modules.
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
//#define NORTH_HALF
//#define GRIPPER
//#define CONTROLLER_TRIPLET
//#define CONTROLLER_DUET
//#define BATTERY_TRIPLET
//#define BATTERY_DUET

#define DEV_ADDR 0x00

//#define GRIPPER_INIT
//#define SERVO_CALIBRATION
//#define SERVO_INT_SERIAL_TEST

//#define DEBUG



#ifdef SOUTH_HALF
#define MASTER
#define mySlaveClass actuatorNorthClass
#endif

#ifdef NORTH_HALF
#define SLAVE
#endif

#ifdef GRIPPER
#define MASTER
#define SOLE_MASTER
#endif

#ifdef CONTROLLER_TRIPLET
#define MASTER
#define mySlaveClass controllerDuetClass
#endif

#ifdef CONTROLLER_DUET
#define SLAVE
#define NO_RIGHT_BOARD
#endif

#ifdef BATTERY_TRIPLET
#define MASTER
#define mySlaveClass batteryDuetClass
#endif

#ifdef BATTERY_DUET
#define SLAVE
#define NO_LEFT_BOARD
#endif


void RewriteCommFrequency(void);
u08 enterContinuousRotation(void);

// PCA addresses (to address individual LED drivers on each of the three triplet PCBs)

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
	u08 id;
	u08 devClass;
	u08 cmd;
	u08 params[128];
	u08 paramCnt;
} RXPKT;

	RXPKT int_busRxPkt;
	RXPKT ext_busRxPkt;
	
//	u08 broadAddress = 0xFE;


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
			((bufferGetAtIndex(rxBuffer,1) == devClass)	||	
			(bufferGetAtIndex(rxBuffer,1) == broadcastClass)))

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
				if(((bufferGetAtIndex(rxBuffer,1) == devClass) && (bufferGetAtIndex(rxBuffer,2) == devAddress))
					|| (bufferGetAtIndex(rxBuffer,1) == broadcastClass)) 
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
							trgPkt = &int_busRxPkt;
						else
							trgPkt = &ext_busRxPkt;

						trgPkt->id = bufferGetAtIndex(rxBuffer,2);
						trgPkt->devClass = bufferGetAtIndex(rxBuffer,1);
						trgPkt->cmd = bufferGetAtIndex(rxBuffer,4);
						trgPkt->paramCnt = p_count;

						for(i=0; i<p_count; i++)
							trgPkt->params[i] = bufferGetAtIndex(rxBuffer,5+i);

						foundpacket = TRUE;
						bufferFlush(rxBuffer);
					} else
						bufferGetFromFront(rxBuffer);
				} else
					bufferGetFromFront(rxBuffer);
			}
		}
	}
	else if(rxBuffer->datalength >= rxBuffer->size)
	{
		// if we found no packet, and the buffer is full
		// we're logjammed, flush entire buffer
		bufferFlush(rxBuffer);
	}

	return foundpacket;
}

u08 GetResponse(u08 serBus, u08 devClass, u08 devAddress)
{
	u16 TimeoutCtr = COMM_TIMEOUT;
	u16 a = 0;
	while ((packetRxProcess(serBus, devClass, devAddress, 
			uartGetRxBuffer(serBus)) == FALSE) && (a < TimeoutCtr))
	{	a+=1;	};						// wait intil a serial timeout occurs

	if (a < TimeoutCtr)  return TRUE;		// if no timeout - confirm reception
	else  return FALSE;						// otherwise report an error: serial communication timeout occurred 
};

u08 GetServoAngle(u16* ServoAngle)
// sends out a request to the AX-12 servo to report its current position
{
	u08 params[10];
	u16 a;
	params[0] = 0x24;
	params[1] = 0x02;		// AX-12 instruction READ

	cmd_BuildPacket(motorClass, 0x01, 2, 0x02, params);
	cmd_Execute(INTERNAL_SERIAL);

	a = 0;
	while ((packetRxProcess(INTERNAL_SERIAL, motorClass, 0x01, 
			uartGetRxBuffer(INTERNAL_SERIAL)) == FALSE) && (a < 20000))
	{	a+=1;	}
	
	if (a < 20000)		// wait intil a serial timeout occurs
	{					// check if the right command is confirmed
		if (int_busRxPkt.paramCnt == 0x02)
		{				// both conditions checked - save the rx'd main pot reading
			*ServoAngle = (int_busRxPkt.params[0] | (int_busRxPkt.params[1]<<8));
			return TRUE;

		}				// report an error: parameter count
	}					// report an error: serial communication timeout occurred 
						// when requesting ADC value from the north half
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
	cmd_BuildPacket(motorClass, 0x01, 0x03, 3, params);
	cmd_Execute(INTERNAL_SERIAL);	
	return GetResponse(INTERNAL_SERIAL, 0xFF, 0x01);
}

#ifdef SOUTH_HALF

u08 GetMainPotAngle(u16* MainPotAngle)
// this function should be used with the South half only - 
// sends out a request to the North half to report ADC readout, 
// measuring the main axis potentiometer output
{
	u08 params[10];
	u16 a;
	cmd_BuildPacket(mySlaveClass, myAddress, 0, 0x14, params);
	cmd_Execute(INTERNAL_SERIAL);		// send ADC readout request
	a = 0;								// wait for response packet
	
	while ((packetRxProcess(INTERNAL_SERIAL, mySlaveClass, myAddress, 
				uartGetRxBuffer(INTERNAL_SERIAL)) == FALSE) && (a < 10000))
	{	a+=1; }

	if (a < 10000)	// wait intil a serial timeout occurs
	{	// check if the right command is confirmed
		if (int_busRxPkt.cmd == 0x14)
		{	// both conditions checked - save received main pot reading
			*MainPotAngle = (int_busRxPkt.params[0] | (int_busRxPkt.params[1]<<8));
			return TRUE;
		}	// report an error: wrong command confirmed
	}	// report an error: serial communication timeout occurred 
		// when requesting ADC value from the north half   
	return FALSE;
}

u08 enterContinuousRotation()
{
	// enter continuous rotation mode
	u08 params[10];
	params[0] = 8;		// CCW angle limit address
	params[1] = 0;		// CCW angle limit (L)
	params[2] = 0;		// CCW angle limit (H)
	cmd_BuildPacket(motorClass, 0x01, 0x03, 3, params);
	cmd_Execute(INTERNAL_SERIAL);	
	return GetResponse(INTERNAL_SERIAL, 0xFF, 0x01);
}


void CalibrateAngles(void)
{
	
	u08 ErrorMargin = 4;
	u08 ApproachDistance = 80;
	u08 ServoPotGapSize = 80;
	u16 HighSpeed = 500;
	u08 ApproachSpeed = 100;

	RewriteCommFrequency();
	
	u16 MainPotAngle;

	while (enterContinuousRotation() == FALSE);				// start moving CCW and continue until the magnet is found
															// magnet determines the servo angle origin location 
	while (SetServoSpeed(HighSpeed, CCW) == FALSE);				// persistant motion command (until motor confirms motion)
	while (MagnetFound() == FALSE);							// once the magnet is found (or if it's already at the magnet) reverse and move 
															// high speed CW until servo angle reading drops down to a small number

/*	while (GetServoAngle(&ServoAngle) == FALSE);
	while(SetServoSpeed(100, CCW) == FALSE);		
	while ((GetServoAngle(&ServoAngle) == FALSE) ||
		(GetServoAngle(&ServoAngle)&& (ServoAngle > 300)));
*/
	
	while(SetServoSpeed(HighSpeed, CW) == FALSE);	

	while ((GetServoAngle(&ServoAngle) == FALSE)					// get servo angle
		|| (GetServoAngle(&ServoAngle) && (ServoAngle > ApproachDistance)));		// keep moving until the angle satisfies the condition

	while(SetServoSpeed(ApproachSpeed, CW) == FALSE);					// reduce speed in proximity to servo sensing range,

	while ((GetServoAngle(&ServoAngle) == FALSE) 
		|| (GetServoAngle(&ServoAngle) && (ServoAngle > ErrorMargin)));


	// Now we are at the beginning of the servo sensing range: store both Servo Readout and Main Pot Readout for this point
	// in EEPROM, this will be one of the six switching points between measurement reqions

	while (GetMainPotAngle(&MainPotAngle) == FALSE);
	u16 CalibP1 = MainPotAngle;									// Save P1

	u16 MainPotGoal = CalibP1 - ServoPotGapSize;								// cross the servo insensitivity gap using the main axis pot:
	while(SetServoSpeed(HighSpeed, CW) == FALSE);						// move CW relative to P1Main by 30 degrees (0x5A) over 20 degrees gap
	while ((GetMainPotAngle(&MainPotAngle) == FALSE) || 
		(GetMainPotAngle(&MainPotAngle) && (MainPotAngle > MainPotGoal)));


	while(SetServoSpeed(ApproachSpeed, CCW) == FALSE);					// move slowly CCW to find and record the sensitivity edge P2

	while ((GetServoAngle(&ServoAngle) == FALSE) ||
		(GetServoAngle(&ServoAngle)&& (ServoAngle < (0x03FF - ErrorMargin))));


//	while(SetServoSpeed(0, CW) == FALSE);	
//	while(1);


	while (GetMainPotAngle(&MainPotAngle) == FALSE);
	u16 CalibP2 = MainPotAngle;									// Save P2



	while(SetServoSpeed(HighSpeed, CW) == FALSE);						// Move towards P3
	while ((GetServoAngle(&ServoAngle) == FALSE) ||
		(GetServoAngle(&ServoAngle) && (ServoAngle > ApproachDistance)));
	while(SetServoSpeed(ApproachSpeed, CW) == FALSE);						// reduce speed in proximity to servo sensing range,
	while ((GetServoAngle(&ServoAngle) == FALSE) ||
		(GetServoAngle(&ServoAngle) && (ServoAngle > ErrorMargin)));
	while (GetMainPotAngle(&MainPotAngle) == FALSE);
	u16 CalibP3 = MainPotAngle;									// Save P3
	MainPotGoal = CalibP3 - ServoPotGapSize;						
	while(SetServoSpeed(HighSpeed, CW) == FALSE);
	while ((GetMainPotAngle(&MainPotAngle) == FALSE) ||
		(GetMainPotAngle(&MainPotAngle) && (MainPotAngle > MainPotGoal)));
	while(SetServoSpeed(ApproachSpeed, CCW) == FALSE);					// move slowly CCW to find and record the sensitivity edge P4
	while ((GetServoAngle(&ServoAngle) == FALSE) ||
		(GetServoAngle(&ServoAngle) && (ServoAngle < (0x03FF - ErrorMargin))));
	while (GetMainPotAngle(&MainPotAngle) == FALSE);
	u16 CalibP4 = MainPotAngle;									// Save P4

//	u16 CalibP4 = GetMainPotAngle();

	// approach P5

	while(SetServoSpeed(HighSpeed, CW) == FALSE);
	while ((GetServoAngle(&ServoAngle) == FALSE) ||
		(GetServoAngle(&ServoAngle) && (ServoAngle > ApproachDistance)));
//	ServoAngle = GetServoAngle();	   // get servo position
//	while (ServoAngle > 80) 	ServoAngle = GetServoAngle();
	
	// reduce speed in proximity to servo sensing range,
	// stop at the servo sensing edge

	while(SetServoSpeed(ApproachSpeed, CW) == FALSE);
	while ((GetServoAngle(&ServoAngle) == FALSE) ||
		(GetServoAngle(&ServoAngle) && (ServoAngle > ErrorMargin)));
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

	MainPotGoal = CalibP5 - ServoPotGapSize;
//	u16 MainPotGoal = P1Main - 0x5A;

	while(SetServoSpeed(HighSpeed, CW) == FALSE);
	while ((GetMainPotAngle(&MainPotAngle) == FALSE) ||
		(GetMainPotAngle(&MainPotAngle) && (MainPotAngle > MainPotGoal)));

	// move slowly CCW to find and record the sensitivity edge P2

	while(SetServoSpeed(ApproachSpeed, CCW) == FALSE);
	while ((GetServoAngle(&ServoAngle) == FALSE) ||
		(GetServoAngle(&ServoAngle) && (ServoAngle < (0x03FF - ErrorMargin))));

	while (GetMainPotAngle(&MainPotAngle) == FALSE);
	u16 CalibP6 = MainPotAngle;							// Save P6


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
	cbi(DDRA, PA5);			// set Port A pin 5 as input

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
		if (ServoAngle < 0x01ff)
		{	Angle = 3600 - ((511 - ServoAngle) - ((511 - ServoAngle) / 50));	}
		else
		{	Angle = (ServoAngle - 511) - (ServoAngle - 511) / 50;	}
	}
	else
	{
		while (GetMainPotAngle(&MainPotAngle) == FALSE);
		
		if (MainPotAngle >= CalibP1)
		// for angles greater than 280.0 degrees
		{
			while (GetServoAngle(&ServoAngle) == FALSE);
			Angle = 3600 - ((511 - ServoAngle) - ((511 - ServoAngle) / 50));
		}
		else if ((MainPotAngle < CalibP1) && (MainPotAngle > CalibP2))
		{
			Angle = 2899 + 3 * (MainPotAngle - CalibP2) + 
				(MainPotAngle - CalibP2) / 3 + (MainPotAngle - CalibP2) / 120 ;
		}
		else if ((MainPotAngle <= CalibP2) && (MainPotAngle >= CalibP3))
		{
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
			Angle = (ServoAngle - 511) - (ServoAngle - 511) / 50;
		}
	}
	return Angle;
}
#endif

int bitCount(u08 param_count, u08 *params)
{
	int sum = 0;
    u08 i,j;
    for(i=0; i<param_count; i++)
    {
		for(j=0; j<8; j++)
        {
			sum += ((params[i] >> j) & 0x01);
        }
    }
    return sum;
}
 

void MakeAllPinsInputs(void)
{
	DDRA &= 0b00111000; 	PORTA &= 0b00111000;
	DDRB &= 0b11111110; 	PORTB &= 0b11111110;
	DDRC &= 0b00110011; 	PORTC &= 0b00110011;
	DDRD &= 0b00111111; 	PORTD &= 0b00111111;
				
	OutputPinSet = FALSE;
};



void RewriteCommFrequency(void)
{
		UBRR0L = 0x00;				// Baud Rate 1M 	/	2M
		timerPause(3);
		u08 params[7];
		params[0] = 0x04;			//	baud rate memory address
		params[1] = 0x05;			//  baud rate value, 333k
		cmd_BuildPacket(motorClass, 0x01, 2, 0x03, params);
		cmd_Execute(INTERNAL_SERIAL);
		timerPause(3);
		cmd_BuildPacket(motorClass, 0x01, 2, 0x03, params);
		cmd_Execute(INTERNAL_SERIAL);
		timerPause(3);
		cmd_BuildPacket(motorClass, 0x01, 2, 0x03, params);
		cmd_Execute(INTERNAL_SERIAL);
		UBRR0L = 0x02;				// Baud Rate 333k	/	667k
		timerPause(3);
		params[0] = 0x05;			//	response delay memory address
		params[1] = 0x00;			//  response delay value
		cmd_BuildPacket(motorClass, 0x01, 2, 0x03, params);
		cmd_Execute(INTERNAL_SERIAL);
		timerPause(3);
		cmd_BuildPacket(motorClass, 0x01, 2, 0x03, params);
		cmd_Execute(INTERNAL_SERIAL);
		timerPause(3);
		cmd_BuildPacket(motorClass, 0x01, 2, 0x03, params);
		cmd_Execute(INTERNAL_SERIAL);
		timerPause(3);
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

	MakeAllPinsInputs();


	// read in myAddress from EEPROM
	eeprom_busy_wait();		
	myAddress = eeprom_read_word((const uint16_t*)AddrPtr);


	// write myAddress to EEPROM
#ifdef DEV_ADDR
	if (myAddress != DEV_ADDR)
	{
		eeprom_busy_wait();		
		eeprom_write_word((uint16_t*)AddrPtr, DEV_ADDR);
	};
#endif	
	


	// init parameter array for serial data exchange
	u08 params[100];
	u08 AddressAssigned = FALSE;


// -------------------------------------------------------------------------
// ONE-TIME initializations, which write nonvolatile parameters to EEPROM
// -------------------------------------------------------------------------

#ifdef GRIPPER_INIT		// MANUAL ADJUSTMENT OF CLOSED GRIPPER ANGLE - to avoid gripper damage
	GripperCorrection = 0;

	eeprom_busy_wait();		eeprom_write_word((uint16_t*)GripCorrPtr, GripperCorrection);
	RGB_LED_PWMx(M_PCA, 0x01, 0x8f, 0x01);
	while(1);
#endif


// -------------------------------------------------------------------------
// EVERY POWER-UP initializations, which read parameters from EEPROM
// -------------------------------------------------------------------------
	
#ifdef SOUTH_HALF					

#ifdef SERVO_CALIBRATION			// Calibrate continuous angle calculation, set servo to continuous rotation
	CalibrateAngles();		
	while(1);
#endif

	myClass = actuatorSouthClass;	// assign class
	// read in calibration values from EEPROM

	eeprom_busy_wait();		CalibP1 = eeprom_read_word((const uint16_t*)P1ptr);
	eeprom_busy_wait();		CalibP2 = eeprom_read_word((const uint16_t*)P2ptr);
	eeprom_busy_wait();		CalibP3 = eeprom_read_word((const uint16_t*)P3ptr);
	eeprom_busy_wait();		CalibP4 = eeprom_read_word((const uint16_t*)P4ptr);
	eeprom_busy_wait();		CalibP5 = eeprom_read_word((const uint16_t*)P5ptr);
	eeprom_busy_wait();		CalibP6 = eeprom_read_word((const uint16_t*)P6ptr);
						
	
	u16 GoalAngle = 2400;		// init variables used for servo mode control
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

#ifdef SERVO_INT_SERIAL_TEST
	ServoModeOn = TRUE;	
#endif

	Angle = CalculateAngle();
#endif

#ifdef NORTH_HALF
	myClass = actuatorNorthClass;		
#endif

#ifdef GRIPPER						// read in gripper max angle correction from EEPROM - protects from gripper self-damage
	myClass = gripperClass;			
	eeprom_busy_wait();		GripperCorrection = eeprom_read_word((const uint16_t*)GripCorrPtr);
#endif

#ifdef CONTROLLER_TRIPLET
	myClass = controllerTripletClass;	
	AddressAssigned = TRUE;
#endif

#ifdef CONTROLLER_DUET
	myClass = controllerDuetClass;		
#endif

#ifdef BATTERY_TRIPLET
	myClass = batteryTripletClass;	
#endif

#ifdef BATTERY_DUET
	myClass = batteryDuetClass;		
#endif


#ifdef DEBUG


	while(1)
	{

		timerPause (25);
		
		u08 params[5];
		params[0] = 0;
		params[1] = 0x0;
		params[2] = 0x0;
		params[3] = 0x40;
									
		cmd_BuildPacket(controllerDuetClass, myAddress, 4, 0x12, params);
		cmd_Execute(INTERNAL_SERIAL);	


		timerPause (25);
		
		params[0] = 0;
		params[1] = 0x40;
		params[2] = 0x0;
		params[3] = 0x0;
									
		cmd_BuildPacket(controllerDuetClass, myAddress, 4, 0x12, params);
		cmd_Execute(INTERNAL_SERIAL);	


	};

#endif




	//--------------------------------------------------------------------------------------
	//	MAIN CONTROL LOOP STARTS HERE
	//--------------------------------------------------------------------------------------

	while(1)
	{
		//	Check for incoming packets on EXTERNAL bus
		if (packetRxProcess(EXTERNAL_SERIAL, myClass, myAddress, 
										uartGetRxBuffer(EXTERNAL_SERIAL))) 
		{
			// *****************************************************
			// 0x04 - conditional address assignment (if any pin high)
			if ((ext_busRxPkt.cmd == 0x04) && (!OutputPinSet))
			{
#ifdef MASTER

				u08 ActivePinFound = FALSE;
				u08 SlaveCommError = FALSE;
				u08 SlaveCommTimeout = FALSE;

				// check all master pins assuming that pins are assigned to output 
				// on a temporary basis only, otherwise they are input

				// checking if any input pin is active
				if ((PINA & 0b11000111) || (PINB & 0b00000001) || (PINC & 0b11001100) || (PIND & 0b11000000))			
				{		ActivePinFound = TRUE; 		}

				if (ActivePinFound == FALSE)
				{

#ifndef SOLE_MASTER		// master does not have any active pins - check if slave has any active pins

					cmd_BuildPacket(mySlaveClass, myAddress, 0, 0x09, params);
					cmd_Execute(INTERNAL_SERIAL);	
	
					if (GetResponse(INTERNAL_SERIAL, mySlaveClass, myAddress))
					{
						if ((int_busRxPkt.cmd == 0x09) && (int_busRxPkt.paramCnt == 2))
						{
							if (int_busRxPkt.params[0] > 0) ActivePinFound = TRUE;
							if (int_busRxPkt.params[1] > 0) ActivePinFound = TRUE;
						}
						else SlaveCommError = TRUE;
					}
					else  SlaveCommTimeout = TRUE;
#endif

				};

				if (ActivePinFound)
				{
					u08 WriteToEEPROM = FALSE;
					if (myAddress != ext_busRxPkt.params[0])		// only re-write EEPROM if the new address is different from the old one
					{
						WriteToEEPROM = TRUE;

#ifndef SOLE_MASTER
						// assign address unconditionally to slave
						params[0] = ext_busRxPkt.params[0];

						cmd_BuildPacket(mySlaveClass, myAddress, 1, 0x03, params);
						cmd_Execute(INTERNAL_SERIAL);	

						// verify that the slave received address 
						if (GetResponse(INTERNAL_SERIAL, mySlaveClass, ext_busRxPkt.params[0]))
						{
							if ((int_busRxPkt.cmd == 0x03) && (int_busRxPkt.paramCnt == 0))
							{
								// address has been received
							}	else SlaveCommError = TRUE;
						}	else  SlaveCommTimeout = TRUE;

#endif

					// active pin found, self-assign rx'd address
					
					if ((SlaveCommTimeout || SlaveCommError) == FALSE)
							myAddress = ext_busRxPkt.params[0];
					}

					AddressAssigned = TRUE;

					// confirm execution by returning own class, address, and command ID    OR...
					u08 paramCnt = 0;
	
					if (SlaveCommTimeout) 	{paramCnt = 1; 	params[0] = 1;}
					if (SlaveCommError) 	{paramCnt = 1; 	params[0] = 2;}
	
					cmd_BuildPacket(myClass, myAddress, paramCnt, ext_busRxPkt.cmd, params);
					cmd_Execute(EXTERNAL_SERIAL);


					if (WriteToEEPROM)
					{
						eeprom_busy_wait();							
						eeprom_write_word((uint16_t*)AddrPtr, myAddress);
					};

				};
#endif
			}

#ifdef MASTER
			
#ifdef SOUTH_HALF
			// *****************************************************
			// 0x06 - report raw angle calculation data
			else if ((ext_busRxPkt.cmd == 0x06) && (AddressAssigned))
			{
				params[0] = MainPotAngle & 0x00ff ;
				params[1] = (MainPotAngle & 0xff00)>>8 ;
				params[2] = ServoAngle & 0x00ff ;
				params[3] = (ServoAngle & 0xff00)>>8 ;
				params[4] = HallSensor ;
				cmd_BuildPacket(myClass, myAddress, 5, ext_busRxPkt.cmd, params);
				cmd_Execute(EXTERNAL_SERIAL);
			}


			//*****************************************************
			// 0x07 - report measured angle
			else if ((ext_busRxPkt.cmd == 0x07) && (AddressAssigned))
			{
				params[0] = Angle & 0x00ff ;
				params[1] = (Angle & 0xff00)>>8 ;
				cmd_BuildPacket(myClass, myAddress, 2, ext_busRxPkt.cmd, params);
				cmd_Execute(EXTERNAL_SERIAL);
			}
#endif


			// *****************************************************
			// 0x08 - ORIENTATION OUTPUT CONTROL; controls one pin at a time
			//		All pins are set to inputs at all times, unless they have to be set to OUTPUT HIGH
			//		In this way, we can check the state of the opposing connected pin, and if it's LOW, 
			//		we know that it is an input and it is save to set cube's own pin to OUTPUT HIGH
			
			//		u08 InSide = RightPCB;		// 0..5 Side ID: Main = 0, Right = 1, Left = 2
			//		u08 InPin = 3;				// 0..4 - individual pins
			//		u08 InState = 0;			// 0..1 OUTPUT HIGH On = 1 / Off = 0 toggle, desired state of pin
			//
			//		ext_busRxPkt.params[0] = (InSide<<4) | (InPin<<2) | InState;  		
												// Compressed command, contains data on Side, Pin, and desired State

			else if ((ext_busRxPkt.cmd == 0x08) && (AddressAssigned))
			{

				u08 Side = (ext_busRxPkt.params[0] & 0b11110000)>>4;	// 0..5 Side ID: per module convention
				u08 Pin = (ext_busRxPkt.params[0] & 0b00001100)>>2;		// 0..3 - individual pins on a side
				u08 State = (ext_busRxPkt.params[0] & 0b00000001);		// 0..1 OUTPUT HIGH On = 1 / Off = 0 toggle, desired state of pin
				u08 ActivePinFound = FALSE;								// to report errors
				u08 OutputAlreadyHigh = FALSE;
				u08 BoardAbscent = FALSE;
				u08 SlaveCommTimeout = FALSE;
				u08 SlaveCommError = FALSE;
				
				if (Side > 5)
				{
					BoardAbscent = TRUE;		// no modules yet desined with more than 5 connectors
				}

				else if ((Side > 2) && (Side < 6))
				{

#ifdef SOLE_MASTER
					BoardAbscent = TRUE; 
#endif
#ifndef SOLE_MASTER

					u08 InSide = Side - 3; 	// 0..2 Slave Side ID: Main = 0, Right = 1, Left = 2
					u08 InPin = Pin;		// 0..4 - individual pins
					u08 InState = State;	// 0..1 OUTPUT HIGH On = 1 / Off = 0 toggle, desired state of pin
		
					params[0] = (InSide<<4) | (InPin<<2) | InState;  		
											// Compressed command, contains data on Side, Pin, and desired State

					cmd_BuildPacket(mySlaveClass, myAddress, 1, ext_busRxPkt.cmd, params);
					cmd_Execute(INTERNAL_SERIAL);	

					if (GetResponse(INTERNAL_SERIAL, mySlaveClass, myAddress))
					{
						if (int_busRxPkt.cmd == ext_busRxPkt.cmd)
						{
							if (int_busRxPkt.paramCnt == 1)
							{
								if (int_busRxPkt.params[0] == 1)	OutputAlreadyHigh = TRUE;
								else if (int_busRxPkt.params[0] == 2) ActivePinFound = TRUE;
								else if (int_busRxPkt.params[0] == 3) BoardAbscent = TRUE;
								else SlaveCommError = TRUE;
							}
							else
							{
									// no errors from slave!
							};
						}
						else SlaveCommError = TRUE;
					}
					else  SlaveCommTimeout = TRUE;

#endif
				}

				else if (Side == MainPCB)
				{
					if (Pin == 0)
					{	if (State == 0)												{	DDRA &= 0b11111011; 	PORTA &= 0b11111011;	}
						else if ((DDRA & 0b00000100) && (PORTA & 0b00000100))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRA & 0b00000100) && (PINA & 0b00000100))		{	ActivePinFound = TRUE;		}
						else if (!(DDRA & 0b00000100) && ~(PINA & 0b00000100))		{	DDRA |= 0b00000100;		PORTA |= 0b00000100;	};
					}
					else if (Pin == 1)
					{	
						if (State == 0)												{	DDRB &= 0b11111110; 	PORTB &= 0b11111110;	}
						else if ((DDRB & 0b00000001) && (PORTB & 0b00000001))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRB & 0b00000001) && (PINB & 0b00000001))		{	ActivePinFound = TRUE;		}
						else if (!(DDRB & 0b00000001) && ~(PINB & 0b00000001))		{	DDRB |= 0b00000001;		PORTB |= 0b00000001;	};
					}
					else if (Pin == 2)
					{	if (State == 0)												{	DDRA &= 0b11111110; 	PORTA &= 0b11111110;	}
						else if ((DDRA & 0b00000001) && (PORTA & 0b00000001))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRA & 0b00000001) && (PINA & 0b00000001))		{	ActivePinFound = TRUE;		}
						else if (!(DDRA & 0b00000001) && ~(PINA & 0b00000001))		{	DDRA |= 0b00000001;		PORTA |= 0b00000001;	};
					}
					else if (Pin == 3)
					{	if (State == 0)												{	DDRA &= 0b11111101; 	PORTA &= 0b11111101;	}
						else if ((DDRA & 0b00000010) && (PORTA & 0b00000010))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRA & 0b00000010) && (PINA & 0b00000010))		{	ActivePinFound = TRUE;		}
						else if (!(DDRA & 0b00000010) && ~(PINA & 0b00000010))		{	DDRA |= 0b00000010;		PORTA |= 0b00000010;	};
					};
				}
				else if (Side == LeftPCB)
				{

#ifdef NO_LEFT_BOARD

					BoardAbscent = TRUE;					

#endif

#ifndef NO_LEFT_BOARD

					if (Pin == 0)
					{	if (State == 0)												{	DDRD &= 0b01111111; 	PORTD &= 0b01111111;	}
						else if ((DDRD & 0b10000000) && (PORTD & 0b10000000))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRD & 0b10000000) && (PIND & 0b10000000))		{	ActivePinFound = TRUE;		}
						else if (!(DDRD & 0b10000000) && ~(PIND & 0b10000000))		{	DDRD |= 0b10000000;		PORTD |= 0b10000000;	};
					}
					else if (Pin == 1)
					{	if (State == 0)												{	DDRC &= 0b11110111; 	PORTC &= 0b11110111;	}
						else if ((DDRC & 0b00001000) && (PORTC & 0b00001000))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRC & 0b00001000) && (PINC & 0b00001000))		{	ActivePinFound = TRUE;		}
						else if (!(DDRC & 0b00001000) && ~(PINC & 0b00001000))		{	DDRC |= 0b00001000;		PORTC |= 0b00001000;	};
					}
					else if (Pin == 2)
					{	if (State == 0)												{	DDRC &= 0b11111011; 	PORTC &= 0b11111011;	}
						else if ((DDRC & 0b00000100) && (PORTC & 0b00000100))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRC & 0b00000100) && (PINC & 0b00000100))		{	ActivePinFound = TRUE;		}
						else if (!(DDRC & 0b00000100) && ~(PINC & 0b00000100))		{	DDRC |= 0b00000100;		PORTC |= 0b00000100;	};
					}
					else if (Pin == 3)
					{	if (State == 0)												{	DDRD &= 0b10111111; 	PORTD &= 0b10111111;	}
						else if ((DDRD & 0b01000000) && (PORTD & 0b01000000))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRD & 0b01000000) && (PIND & 0b01000000))		{	ActivePinFound = TRUE;		}
						else if (!(DDRD & 0b01000000) && ~(PIND & 0b01000000))		{	DDRD |= 0b01000000;		PORTD |= 0b01000000;	};
					};

#endif

				}
				else if (Side == RightPCB)
				{

#ifdef NO_RIGHT_BOARD

					BoardAbscent = TRUE;					

#endif

#ifndef NO_RIGHT_BOARD

					if (Pin == 0)
					{	if (State == 0)												{	DDRA &= 0b10111111; 	PORTA &= 0b10111111;	}
						else if ((DDRA & 0b01000000) && (PORTA & 0b01000000))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRA & 0b01000000) && (PINA & 0b01000000))		{	ActivePinFound = TRUE;		}
						else if (!(DDRA & 0b01000000) && ~(PINA & 0b01000000))		{	DDRA |= 0b01000000;		PORTA |= 0b01000000;	};
					}
					else if (Pin == 1)
					{	if (State == 0)												{	DDRC &= 0b01111111; 	PORTC &= 0b01111111;	}
						else if ((DDRC & 0b10000000) && (PORTC & 0b10000000))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRC & 0b10000000) && (PINC & 0b10000000))		{	ActivePinFound = TRUE;		}
						else if (!(DDRC & 0b10000000) && ~(PINC & 0b10000000))		{	DDRC |= 0b10000000;		PORTC |= 0b10000000;	};
					}
					else if (Pin == 2)
					{	if (State == 0)												{	DDRC &= 0b10111111; 	PORTC &= 0b10111111;	}
						else if ((DDRC & 0b01000000) && (PORTC & 0b01000000))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRC & 0b01000000) && (PINC & 0b01000000))		{	ActivePinFound = TRUE;		}
						else if (!(DDRC & 0b01000000) && ~(PINC & 0b01000000))		{	DDRC |= 0b01000000;		PORTC |= 0b01000000;	};
					}
					else if (Pin == 3)
					{	if (State == 0)												{	DDRA &= 0b01111111; 	PORTA &= 0b01111111;	}
						else if ((DDRA & 0b10000000) && (PORTA & 0b10000000))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRA & 0b10000000) && (PINA & 0b10000000))		{	ActivePinFound = TRUE;		}
						else if (!(DDRA & 0b10000000) && ~(PINA & 0b10000000))		{	DDRA |= 0b10000000;		PORTA |= 0b10000000;	};
					};
#endif
				};

				u08 paramCnt = 0;

				if (OutputAlreadyHigh) 	{paramCnt = 1; 	params[0] = 1;}
				if (ActivePinFound) 	{paramCnt = 1; 	params[0] = 2;}
				if (BoardAbscent) 		{paramCnt = 1; 	params[0] = 3;}
				if (SlaveCommError) 	{paramCnt = 1; 	params[0] = 4;}
				if (SlaveCommTimeout) 	{paramCnt = 1; 	params[0] = 5;}

				cmd_BuildPacket(myClass, myAddress, paramCnt, ext_busRxPkt.cmd, params);
				cmd_Execute(EXTERNAL_SERIAL);

				if (State == 0) OutputPinSet = FALSE;
				if (State == 1) OutputPinSet = TRUE;
			}



			// *****************************************************
			// 0x09 - POLLING ORIENTATION INPUTS
		    //		all orientation i/os are set to digital inputs, their states are read
			//		and reported in 2 parameter bytes of response 
			//		p[0] = 0, 0, 0, 0, M3, M2, M1, M0;  p[1] = L3, L2, L1, L0, R3, R2, R1, R0

			else if ((ext_busRxPkt.cmd == 0x09) && (AddressAssigned) && (!OutputPinSet))
			{	
				MakeAllPinsInputs();

				u08 SlaveCommError = FALSE;
				u08 SlaveCommTimeout = FALSE;

				params[0] = 0x00;	// 8 bits from master inputs M3, M2, M1, M0, R3, R2, R1, R0
				params[1] = 0x00;	// 4 bits from master inputs L3, L2, L1, L0, 
									// and 4 bits from slave inputs M3, M2, M1, M0
				params[2] = 0x00;	// 8 bits from slave inputs L3, L2, L1, L0, R3, R2, R1, R0

				// check all master pins

				if (PINA & 0b00000100)   params[0] |= 0b00010000;		// read in M0
				if (PINB & 0b00000001)   params[0] |= 0b00100000;		// read in M1
				if (PINA & 0b00000001)   params[0] |= 0b01000000;		// read in M2
				if (PINA & 0b00000010)   params[0] |= 0b10000000;		// read in M3

				if (PINA & 0b01000000)   params[0] |= 0b00000001;		// read in R0
				if (PINC & 0b10000000)   params[0] |= 0b00000010;		// read in R1
				if (PINC & 0b01000000)   params[0] |= 0b00000100;		// read in R2
				if (PINA & 0b10000000)   params[0] |= 0b00001000;		// read in R3

				if (PIND & 0b10000000)   params[1] |= 0b00010000;		// read in L0
				if (PINC & 0b00001000)   params[1] |= 0b00100000;		// read in L1
				if (PINC & 0b00000100)   params[1] |= 0b01000000;		// read in L2
				if (PIND & 0b01000000)   params[1] |= 0b10000000;		// read in L3

#ifndef SOLE_MASTER

				cmd_BuildPacket(mySlaveClass, myAddress, 0, ext_busRxPkt.cmd, params);
				cmd_Execute(INTERNAL_SERIAL);	

				if (GetResponse(INTERNAL_SERIAL, mySlaveClass, myAddress))
				{
					if ((int_busRxPkt.cmd == ext_busRxPkt.cmd) && (int_busRxPkt.paramCnt == 2))
					{
						params[1] |= int_busRxPkt.params[0];
						params[2] |= int_busRxPkt.params[1];
					}
					else SlaveCommError = TRUE;
				}
				else  SlaveCommTimeout = TRUE;
#endif

				params[3] = 0;

				u08 paramCnt = 3;
				if (SlaveCommTimeout) 	{paramCnt = 4; 	params[3] = 1;}
				if (SlaveCommError) 	{paramCnt = 4; 	params[3] = 2;}

				u08 Sum = bitCount(3, params);

				if ((ext_busRxPkt.devClass != broadcastClass) ||
					((ext_busRxPkt.devClass == broadcastClass) && Sum && (params[3] == 0)))
				{
					cmd_BuildPacket(myClass, myAddress, paramCnt, ext_busRxPkt.cmd, params);
					cmd_Execute(EXTERNAL_SERIAL);
				};
			}

#ifdef SOUTH_HALF

			//*****************************************************
			// 0x0E - Servo Angle Tracking 
			else if ((ext_busRxPkt.cmd == 0x0E) && (AddressAssigned))
			{

				ServoModeOn = ext_busRxPkt.params[0];

				cmd_BuildPacket(myClass, myAddress, 0, ext_busRxPkt.cmd, params);
				cmd_Execute(EXTERNAL_SERIAL);
			}
#endif

			//*****************************************************
			// 0x12 - set LEDS color 
			else if ((ext_busRxPkt.cmd == 0x12) && (AddressAssigned))
			{


				if (ext_busRxPkt.params[0] < 3)
				{
					u08 Side = M_PCA;
					if (ext_busRxPkt.params[0] == 1) Side = R_PCA;
					if (ext_busRxPkt.params[0] == 2) Side = L_PCA;


#ifdef NO_RIGHT_BOARD

					if (Side == R_PCA) 
					{
						params[0] = 1;
						cmd_BuildPacket(myClass, myAddress, 1, ext_busRxPkt.cmd, params);
						cmd_Execute(EXTERNAL_SERIAL);
					}
					else
#endif

#ifdef NO_LEFT_BOARD

					if (Side == L_PCA) 
					{
						params[0] = 1;
						cmd_BuildPacket(myClass, myAddress, 1, ext_busRxPkt.cmd, params);
						cmd_Execute(EXTERNAL_SERIAL);
					}
					else
#endif

					{
						cmd_BuildPacket(myClass, myAddress, 0, ext_busRxPkt.cmd, params);
						cmd_Execute(EXTERNAL_SERIAL);

						RGB_LED_PWMx(Side, ext_busRxPkt.params[1], 
							ext_busRxPkt.params[2], ext_busRxPkt.params[3]);
					};
				}
				else		// pass this command to a slave
				{

					u08 paramCnt = 0;

#ifdef SOLE_MASTER
					paramCnt = 1; 	
					params[0] = 1;

#endif

#ifndef SOLE_MASTER

					u08 SlaveCommTimeout = FALSE;
					u08 SlaveCommError = FALSE;

					params[0] = ext_busRxPkt.params[0] - 3;
					params[1] = ext_busRxPkt.params[1];
					params[2] = ext_busRxPkt.params[2];
					params[3] = ext_busRxPkt.params[3];
										
					cmd_BuildPacket(mySlaveClass, myAddress, 4, ext_busRxPkt.cmd, params);
					cmd_Execute(INTERNAL_SERIAL);	


					if (GetResponse(INTERNAL_SERIAL, mySlaveClass, myAddress))
					{
						if (int_busRxPkt.cmd == ext_busRxPkt.cmd)
						{
							if (int_busRxPkt.paramCnt == 1)
							{
								paramCnt = 1;
								params[0] = int_busRxPkt.params[0];
							}
							else if (int_busRxPkt.paramCnt > 1) 		SlaveCommError = TRUE;
						}
						else SlaveCommError = TRUE;
					}
					else  SlaveCommTimeout = TRUE;

					if (SlaveCommTimeout) 	{paramCnt = 1; 	params[0] = 3;}
					if (SlaveCommError) 	{paramCnt = 1; 	params[0] = 4;}
#endif

					cmd_BuildPacket(myClass, myAddress, paramCnt, ext_busRxPkt.cmd, params);
					cmd_Execute(EXTERNAL_SERIAL);
				};
			}
#endif

			//*****************************************************
			// 0x14 - 10 bit ADC readout, converted into u16 type as follows:
			//		           7     6     5     4     3     2     1     0
			//		(0x78)  | AD7 | AD6 | AD5 | AD4 | AD3 | AD2 | AD1 | AD0		ADCL
			//		(0x79)  | --- | --- | --- | --- | --- | --- | AD9 | AD8		ADCH
			
			else if ((ext_busRxPkt.cmd == 0x14) && (AddressAssigned))
			{
				//	read in ADC value
				u16 ADCdata = a2dConvert10bit(ADC_CH_ADC5);

				//	return ADC value in response packet
				params[0] = ADCdata & 0x00ff ;
				params[1] = (ADCdata & 0xff00)>>8 ;
				cmd_BuildPacket(myClass, myAddress, 2, ext_busRxPkt.cmd, params);
				cmd_Execute(EXTERNAL_SERIAL);
			}


#ifdef SOUTH_HALF
			//*****************************************************
			// 0x15 - set servo goal speed and angle

			else if ((ext_busRxPkt.cmd == 0x15) && (AddressAssigned))
			{
				// confirm execution by returning own class, address, and command ID
				cmd_BuildPacket(myClass, myAddress, 0, ext_busRxPkt.cmd, params);
				cmd_Execute(EXTERNAL_SERIAL);

				ServoModeOn = TRUE;	

				GoalAngle = (ext_busRxPkt.params[0] | (ext_busRxPkt.params[1]<<8));
				GoalSpeed = (ext_busRxPkt.params[2] | (ext_busRxPkt.params[3]<<8));
				// include delayed response for motor commands ???
			}
#endif


#ifdef GRIPPER
			//*****************************************************
			// 0x17 - gripper actuation
			//		ext_busRxPkt.params[0] = Pos & 0x00ff;			// Position LSB
			//		ext_busRxPkt.params[1] = (Pos & 0xff00)>>8 ;	// Position MSB
			//		ext_busRxPkt.params[2] = Speed & 0x00ff;		// Speed LSB
			//		ext_busRxPkt.params[3] = (Speed & 0xff00)>>8 ;	// Speed MSB
			//		ext_busRxPkt.params[4] = Force & 0x00ff;		// Max Torque LSB
			//		ext_busRxPkt.params[5] = (Force & 0xff00)>>8 ;	// Max Torque MSB

			//  Gripper actuation parameters
			//  Gripper position 0..1023 (0 fully open, 1023 fully closed) 
			//			must be corrected by a individual gripper-specific value 
			//			to avoid damaging jaws, shell, and star track
			//  Gripper speed 0..1023 (reduced by motor at low torque limit values)
			//  Gripping torque 0..1023 (static friction barrier ~150, human pain barrier ~400, 
			//			plastic parts break at ~700)


			else if ((ext_busRxPkt.cmd == 0x17) && (AddressAssigned))
			{
				params[0] = 30;

				u16 RawPos = (ext_busRxPkt.params[0] | (ext_busRxPkt.params[1]<<8));
				u16 CorrPos = RawPos - GripperCorrection;

				params[1] = CorrPos & 0x00ff;			// Position LSB
				params[2] = (CorrPos & 0xff00)>>8;		// Position MSB
				params[3] = ext_busRxPkt.params[2];		// Speed LSB
				params[4] = ext_busRxPkt.params[3];		// Speed MSB
				params[5] = ext_busRxPkt.params[4];		// Max Torque LSB
				params[6] = ext_busRxPkt.params[5];		// Max Torque MSB
				cmd_BuildPacket(motorClass, 0x01, 7, 0x03, params);
				cmd_Execute(INTERNAL_SERIAL);	

				// confirm execution by returning own class, address, and command ID
				cmd_BuildPacket(myClass, myAddress, 0, ext_busRxPkt.cmd, params);
				cmd_Execute(EXTERNAL_SERIAL);
			}
#endif
			;
		} // if (packetRxProcess) - end of listening to external bus



#ifdef SLAVE


		//----------------------------------------------------------------------
		//	Check for incoming packets on INTERNAL bus

		if (packetRxProcess(INTERNAL_SERIAL, myClass, myAddress, 
										uartGetRxBuffer(INTERNAL_SERIAL))) 
		{

			//*****************************************************
			// 0x03 - this command is only used by a SOUTH (Master) half to directly re-write 
			// the address of a NORTH (Slave), and thus is always sent over internal bus

			if (int_busRxPkt.cmd == 0x03)
			{	// re-writes myAddress in EEPROM

				u08 WriteToEEPROM = FALSE;

				if (myAddress != int_busRxPkt.params[0])		// only re-write EEPROM if the new address is different from the old one
				{
					WriteToEEPROM = TRUE;
					myAddress = int_busRxPkt.params[0];
				};

				// confirm execution by returning own class, address, and command ID
				cmd_BuildPacket(myClass, myAddress, 0, int_busRxPkt.cmd, params);
				cmd_Execute(INTERNAL_SERIAL);

				if (WriteToEEPROM)
				{
					eeprom_busy_wait();
					eeprom_write_word((uint16_t*)AddrPtr, int_busRxPkt.params[0]);	
				};
			}


			//*****************************************************
			// 0x08 - ORIENTATION OUTPUT CONTROL; controls one pin at a time
			//		All pins are set to inputs at all times, unless they have to be set to OUTPUT HIGH
			//		In this way, we can check the state of the opposing connected pin, and if it's LOW, 
			//		we know that it is an input and it is save to set cube's own pin to OUTPUT HIGH
			
			//		u08 InSide = RightPCB;		// 0..2 Side ID: Main = 0, Right = 1, Left = 2
			//		u08 InPin = 3;				// 0..4 - individual pins
			//		u08 InState = 0;			// 0..1 OUTPUT HIGH On = 1 / Off = 0 toggle, desired state of pin
			//
			//		ext_busRxPkt.params[0] = (InSide<<4) | (InPin<<2) | InState;  		
												// Compressed command, contains data on Side, Pin, and desired State

			else if (int_busRxPkt.cmd == 0x08)
			{
				u08 Side = (int_busRxPkt.params[0] & 0b11110000)>>4;	// 0..2 Side ID: only Main, Right, or Left
				u08 Pin = (int_busRxPkt.params[0] & 0b00001100)>>2;		// 0..3 - individual pins on a side
				u08 State = (int_busRxPkt.params[0] & 0b00000001);		// 0..1 OUTPUT HIGH On = 1 / Off = 0 toggle, desired state of pin
				u08 ActivePinFound = FALSE;								// to report errors
				u08 OutputAlreadyHigh = FALSE;
				u08 BoardAbscent = FALSE;
				u08 WrongSideRequested = FALSE;
				
				if (Side > 2)	WrongSideRequested = TRUE;		// slaves only have 3 sides


				else if (Side == MainPCB)
				{
					if (Pin == 0)
					{	if (State == 0)												{	DDRA &= 0b11111011; 	PORTA &= 0b11111011;	}
						else if ((DDRA & 0b00000100) && (PORTA & 0b00000100))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRA & 0b00000100) && (PINA & 0b00000100))		{	ActivePinFound = TRUE;		}
						else if (!(DDRA & 0b00000100) && ~(PINA & 0b00000100))		{	DDRA |= 0b00000100;		PORTA |= 0b00000100;	};
					}
					else if (Pin == 1)
					{	
						if (State == 0)												{	DDRB &= 0b11111110; 	PORTB &= 0b11111110;	}
						else if ((DDRB & 0b00000001) && (PORTB & 0b00000001))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRB & 0b00000001) && (PINB & 0b00000001))		{	ActivePinFound = TRUE;		}
						else if (!(DDRB & 0b00000001) && ~(PINB & 0b00000001))		{	DDRB |= 0b00000001;		PORTB |= 0b00000001;	};
					}
					else if (Pin == 2)
					{	if (State == 0)												{	DDRA &= 0b11111110; 	PORTA &= 0b11111110;	}
						else if ((DDRA & 0b00000001) && (PORTA & 0b00000001))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRA & 0b00000001) && (PINA & 0b00000001))		{	ActivePinFound = TRUE;		}
						else if (!(DDRA & 0b00000001) && ~(PINA & 0b00000001))		{	DDRA |= 0b00000001;		PORTA |= 0b00000001;	};
					}
					else if (Pin == 3)
					{	if (State == 0)												{	DDRA &= 0b11111101; 	PORTA &= 0b11111101;	}
						else if ((DDRA & 0b00000010) && (PORTA & 0b00000010))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRA & 0b00000010) && (PINA & 0b00000010))		{	ActivePinFound = TRUE;		}
						else if (!(DDRA & 0b00000010) && ~(PINA & 0b00000010))		{	DDRA |= 0b00000010;		PORTA |= 0b00000010;	};
					};
				}
				else if (Side == LeftPCB)
				{

#ifdef NO_LEFT_BOARD

					BoardAbscent = TRUE;					

#endif

#ifndef NO_LEFT_BOARD

					if (Pin == 0)
					{	if (State == 0)												{	DDRD &= 0b01111111; 	PORTD &= 0b01111111;	}
						else if ((DDRD & 0b10000000) && (PORTD & 0b10000000))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRD & 0b10000000) && (PIND & 0b10000000))		{	ActivePinFound = TRUE;		}
						else if (!(DDRD & 0b10000000) && ~(PIND & 0b10000000))		{	DDRD |= 0b10000000;		PORTD |= 0b10000000;	};
					}
					else if (Pin == 1)
					{	if (State == 0)												{	DDRC &= 0b11110111; 	PORTC &= 0b11110111;	}
						else if ((DDRC & 0b00001000) && (PORTC & 0b00001000))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRC & 0b00001000) && (PINC & 0b00001000))		{	ActivePinFound = TRUE;		}
						else if (!(DDRC & 0b00001000) && ~(PINC & 0b00001000))		{	DDRC |= 0b00001000;		PORTC |= 0b00001000;	};
					}
					else if (Pin == 2)
					{	if (State == 0)												{	DDRC &= 0b11111011; 	PORTC &= 0b11111011;	}
						else if ((DDRC & 0b00000100) && (PORTC & 0b00000100))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRC & 0b00000100) && (PINC & 0b00000100))		{	ActivePinFound = TRUE;		}
						else if (!(DDRC & 0b00000100) && ~(PINC & 0b00000100))		{	DDRC |= 0b00000100;		PORTC |= 0b00000100;	};
					}
					else if (Pin == 3)
					{	if (State == 0)												{	DDRD &= 0b10111111; 	PORTD &= 0b10111111;	}
						else if ((DDRD & 0b01000000) && (PORTD & 0b01000000))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRD & 0b01000000) && (PIND & 0b01000000))		{	ActivePinFound = TRUE;		}
						else if (!(DDRD & 0b01000000) && ~(PIND & 0b01000000))		{	DDRD |= 0b01000000;		PORTD |= 0b01000000;	};
					};

#endif

				}
				else if (Side == RightPCB)
				{

#ifdef NO_RIGHT_BOARD

					BoardAbscent = TRUE;					

#endif

#ifndef NO_RIGHT_BOARD

					if (Pin == 0)
					{	if (State == 0)												{	DDRA &= 0b10111111; 	PORTA &= 0b10111111;	}
						else if ((DDRA & 0b01000000) && (PORTA & 0b01000000))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRA & 0b01000000) && (PINA & 0b01000000))		{	ActivePinFound = TRUE;		}
						else if (!(DDRA & 0b01000000) && ~(PINA & 0b01000000))		{	DDRA |= 0b01000000;		PORTA |= 0b01000000;	};
					}
					else if (Pin == 1)
					{	if (State == 0)												{	DDRC &= 0b01111111; 	PORTC &= 0b01111111;	}
						else if ((DDRC & 0b10000000) && (PORTC & 0b10000000))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRC & 0b10000000) && (PINC & 0b10000000))		{	ActivePinFound = TRUE;		}
						else if (!(DDRC & 0b10000000) && ~(PINC & 0b10000000))		{	DDRC |= 0b10000000;		PORTC |= 0b10000000;	};
					}
					else if (Pin == 2)
					{	if (State == 0)												{	DDRC &= 0b10111111; 	PORTC &= 0b10111111;	}
						else if ((DDRC & 0b01000000) && (PORTC & 0b01000000))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRC & 0b01000000) && (PINC & 0b01000000))		{	ActivePinFound = TRUE;		}
						else if (!(DDRC & 0b01000000) && ~(PINC & 0b01000000))		{	DDRC |= 0b01000000;		PORTC |= 0b01000000;	};
					}
					else if (Pin == 3)
					{	if (State == 0)												{	DDRA &= 0b01111111; 	PORTA &= 0b01111111;	}
						else if ((DDRA & 0b10000000) && (PORTA & 0b10000000))		{	OutputAlreadyHigh = TRUE;	}
						else if (!(DDRA & 0b10000000) && (PINA & 0b10000000))		{	ActivePinFound = TRUE;		}
						else if (!(DDRA & 0b10000000) && ~(PINA & 0b10000000))		{	DDRA |= 0b10000000;		PORTA |= 0b10000000;	};
					};

#endif

				};


				if (((OutputAlreadyHigh) || (ActivePinFound) || (BoardAbscent) || (WrongSideRequested)) == FALSE)
				{
					cmd_BuildPacket(myClass, myAddress, 0, int_busRxPkt.cmd, params);
					cmd_Execute(INTERNAL_SERIAL);
				}

				// Return error code
				else 
				{
					u08 paramCount = 1;

					if (OutputAlreadyHigh)		params[0] = 1;
					if (ActivePinFound)			params[0] = 2;
					if (BoardAbscent)			params[0] = 3;
					if (WrongSideRequested)		params[0] = 4;

					cmd_BuildPacket(myClass, myAddress, paramCount, int_busRxPkt.cmd, params);
					cmd_Execute(INTERNAL_SERIAL);
				};
			}



			//*****************************************************
			// 0x09 - POLLING ORIENTATION INPUTS
		    //		all orientation i/os are set to digital inputs, their states are read
			//		and reported in 2 parameter bytes of response 
			//		p[0] = 0, 0, 0, 0, M3, M2, M1, M0;  p[1] = L3, L2, L1, L0, R3, R2, R1, R0

			else if (int_busRxPkt.cmd == 0x09)
			{	// set all pins to inpits to make sure only one at a time can be high output 


				DDRA &= 0b00111000; 	PORTA &= 0b00111000;
				DDRB &= 0b11111110; 	PORTB &= 0b11111110;
				DDRC &= 0b00110011; 	PORTC &= 0b00110011;
				DDRD &= 0b00111111; 	PORTD &= 0b00111111;

				
				// check all south pins

				params[0] = 0x00;	// 4 bits filler and 4 bits from slave inputs M3, M2, M1, M0
				params[1] = 0x00;	// 8 bits from slave inputs R3, R2, R1, R0, L3, L2, L1, L0


				// make all orientation pins INPUTS
//				DDRA &= !0b11000111;		DDRB &= !0b00000001;		DDRC &= !0b11001100;		DDRD &= !0b11000000;

				if (PINA & 0b00000100)   params[0] |= 0b00000001;		// read in M0
				if (PINB & 0b00000001)   params[0] |= 0b00000010;		// read in M1
				if (PINA & 0b00000001)   params[0] |= 0b00000100;		// read in M2
				if (PINA & 0b00000010)   params[0] |= 0b00001000;		// read in M3

#ifndef NO_RIGHT_BOARD

				if (PINA & 0b01000000)   params[1] |= 0b00010000;		// read in R0
				if (PINC & 0b10000000)   params[1] |= 0b00100000;		// read in R1
				if (PINC & 0b01000000)   params[1] |= 0b01000000;		// read in R2
				if (PINA & 0b10000000)   params[1] |= 0b10000000;		// read in R3
#endif

#ifndef NO_LEFT_BOARD

				if (PIND & 0b10000000)   params[1] |= 0b00000001;		// read in L0
				if (PINC & 0b00001000)   params[1] |= 0b00000010;		// read in L1
				if (PINC & 0b00000100)   params[1] |= 0b00000100;		// read in L2
				if (PIND & 0b01000000)   params[1] |= 0b00001000;		// read in L3
#endif

				cmd_BuildPacket(myClass, myAddress, 2, int_busRxPkt.cmd, params);
				cmd_Execute(INTERNAL_SERIAL);

			}

			//*****************************************************
			// 0x12 - set LEDS
			else if (int_busRxPkt.cmd == 0x12)
			{

				u08 Side = M_PCA;
				if (int_busRxPkt.params[0] == 1) Side = R_PCA;
				if (int_busRxPkt.params[0] == 2) Side = L_PCA;


#ifdef NO_RIGHT_BOARD

				if (Side == R_PCA) 
				{
					params[0] = 1;
					cmd_BuildPacket(myClass, myAddress, 1, int_busRxPkt.cmd, params);
					cmd_Execute(INTERNAL_SERIAL);
				}
				else
#endif

#ifdef NO_LEFT_BOARD

				if (Side == L_PCA) 
				{
					params[0] = 2;
					cmd_BuildPacket(myClass, myAddress, 1, ext_busRxPkt.cmd, params);
					cmd_Execute(EXTERNAL_SERIAL);
				}
				else
#endif

				{
					cmd_BuildPacket(myClass, myAddress, 0, int_busRxPkt.cmd, params);
					cmd_Execute(INTERNAL_SERIAL);

					RGB_LED_PWMx(Side, int_busRxPkt.params[1], 
						int_busRxPkt.params[2], int_busRxPkt.params[3]);
				};
			}
			


			//*****************************************************
			// 0x14 - 10 bit ADC readout, converted into u16 type as follows:
			//		           7     6     5     4     3     2     1     0
			//		(0x78)  | AD7 | AD6 | AD5 | AD4 | AD3 | AD2 | AD1 | AD0		ADCL
			//		(0x79)  | --- | --- | --- | --- | --- | --- | AD9 | AD8		ADCH
			
			else if (int_busRxPkt.cmd == 0x14)
			{
#ifdef NORTH_HALF


				//	read in ADC value
				u16 ADCdata = a2dConvert10bit(ADC_CH_ADC5);

				//	return ADC value in response packet
				params[0] = ADCdata & 0x00ff ;
				params[1] = (ADCdata & 0xff00)>>8 ;
				cmd_BuildPacket(myClass, myAddress, 2, int_busRxPkt.cmd, params);
				cmd_Execute(INTERNAL_SERIAL);
#endif
			};
		};		// end of listening to external bus

#endif

//			timerPause(4);


#ifdef SOUTH_HALF				// SERVO MODE CONTROLS, only used by south half
		if (ServoModeOn)
		{
			Angle = CalculateAngle();

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
#endif
	};

	return 0;
};








// -----------------------------------------------------------------------------------
//    Miscellaneous code snippets
// -----------------------------------------------------------------------------------



// writing a data value into the control table of AX-12

// setting goal position (within servo range)
	
/*	
	u08 params[3];
	params[0] = 30;
	params[1] = 100;
	params[2] = 0;
	cmd_BuildPacket(motorClass, 0x01, 0x03, 3, params);
	cmd_Execute(INTERNAL_SERIAL);	
*/

// exit continuous rotation mode

/*
	u08 params[3];
	params[0] = 8;
	params[1] = 255;
	params[2] = 3;
	cmd_BuildPacket(motorClass, 0x01, 0x03, 3, params);
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
		cmd_BuildPacket(motorClass, 0x01, 2, 0x03, params);
		cmd_Execute(INTERNAL_SERIAL);
		UBRR0L = 0x02;   // Baud Rate 333k	/	667k
		timerPause(20);
*/
		

	/*	
	GoalAngle = (ext_busRxPkt.params[0] | (ext_busRxPkt.params[1]<<8));
	params[1] = goalSpeed & 0x00ff ;
	params[2] = (goalSpeed & 0xff00)>>8 ;
*/

	// write max torque

/*
	params[0] = 34;
	params[1] = 150;	// Max Torque LSB
	params[2] = 1;		// Max Torque MSB
	cmd_BuildPacket(motorClass, 0x01, 3, 0x03, params);
	cmd_Execute(INTERNAL_SERIAL);	
*/

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
			cmd_BuildPacket(mySlaveClass, 0x01, 0x12, 2, params);
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


/*
void turnMotorLEDon(void)
{
	u08 params[2];
	params[0] = 0x19;
	params[1] = 0x00;
	cmd_BuildPacket(motorClass, 0x01, 0x03, 2, params);
	cmd_Execute(INTERNAL_SERIAL);
}

void turnMotorLEDoff(void)
{
	u08 params[2];
	params[0] = 0x19;
	params[1] = 0x01;
	cmd_BuildPacket(motorClass, 0x01, 0x03, 2, params);
	cmd_Execute(INTERNAL_SERIAL);	
}

void blinkLed(u16 delay_msec)
{

	turnMotorLEDon();
	timerPause(delay_msec);
	turnMotorLEDoff();
	timerPause(delay_msec);

}
*/

	//		params[0] = Angle & 0x00ff ;
	//		params[1] = (Angle & 0xff00)>>8 ;
	//		cmd_BuildPacket(myClass, myAddress, 2, 0x07, params);
	//		cmd_Execute(EXTERNAL_SERIAL);
	

/*


					RGB_LED_PWMx(M_PCA, 100, 0, 0); 
					delay_micro(10);
					RGB_LED_PWMx(M_PCA, 0x01, 0x01, 0x01); 
	


*/


