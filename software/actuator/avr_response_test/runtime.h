#ifndef _RUNTIME_H_
#define _RUNTIME_H_

#include "global.h"

// EEPROM pointers for nonvolatile parameter storage

#define P1ptr		0		// adresses of the angle calibration
#define P2ptr		2		// values in EEPROM
#define P3ptr		4
#define P4ptr		6
#define P5ptr		8
#define P6ptr		10

#define AddrPtr		12		// Pointer to Module address storage (0..255)

#define GripCorrPtr 14      // Pointer to Gripper max angle correction =(1023 - angle, at which jaws hit the star track)

// servo rotation directions
#define CCW 0	
#define CW 1

// PCB identifiers within a triplet
#define	MainPCB 0	
#define RightPCB 1
#define LeftPCB 2

// Module Classes

#define motorClass				0xFF
#define broadcastClass			0xFE
#define ARMClass				0xFD
#define controllerTripletClass	0xFC
#define controllerDuetClass		0xFB
#define actuatorSouthClass		0xFA
#define actuatorNorthClass		0xF9
#define gripperClass			0xF8
#define batteryClass			0xF7

#define BROADCAST_CLASS			0xFE

#define CMD_RESPONSE_TIMEOUT	60000 // 5000 -> 12ms // 60000 -> 120ms

#define NO_ERROR						0x00
#define CMDPROCESSOR_PARAMETER_ERROR	0x01
#define GET_RESPONSE_TIMEOUT_ERROR		0x02
#define CMDPROCESSOR_UNKNOWN_CMD_ERROR	0x03

// Global Variables
typedef struct {
	u08 id;
	u08 devClass;
	u08 cmd;
	u08 params[128];
	u08 paramCnt;
} RXPKT;

RXPKT int_busRxPkt;
RXPKT ext_busRxPkt;

u08 myClass;
u08 myAddress;

#endif
