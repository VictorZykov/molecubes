
//*****************************************************************************
//
// File Name	: 'global.h'
// Title		: AVR project global include 
// Author		: Pascal Stang
// Created		: 7/12/2001
// Revised		: 9/30/2002
// Version		: 1.1
// Target MCU	: Atmel AVR series
// Editor Tabs	: 4
//
//	Description : This include file is designed to contain items useful to all
//					code files and projects.
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#ifndef GLOBAL_H
#define GLOBAL_H

// global AVRLIB defines
#include "avrlibdefs.h"
// global AVRLIB types definitions
#include "avrlibtypes.h"

// project/system dependent defines

// CPU clock speed
#define F_CPU        16000000               		// 16MHz processor
//#define F_CPU        14745000               		// 14.745MHz processor
//#define F_CPU        8000000               		// 8MHz processor
//#define F_CPU        7372800               		// 7.37MHz processor
//#define F_CPU        4000000               		// 4MHz processor
//#define F_CPU        3686400               		// 3.69MHz processor


// EEPROM pointers for nonvolatile parameter storage

#define P1ptr		0		// adresses of the angle calibration
#define P2ptr		2		// values in EEPROM
#define P3ptr		4
#define P4ptr		6
#define P5ptr		8
#define P6ptr		10

#define AddrPtr		12		// Pointer to Module address storage (0..255)

#define GripCorrPtr 14      // Pointer to Gripper max angle correction =(1023 - angle, at which jaws hit the star track)


u16 CalibP1;				// global calibration values
u16 CalibP2;				// for the Main Pot Angle
u16 CalibP3;
u16 CalibP4;
u16 CalibP5;
u16 CalibP6;

u16 GripperCorrection;		// Gripper max angle correction =(1023 - angle, at which jaws hit the star track)

// servo rotation directions
#define CCW 0	
#define CW 1

u16 MainPotAngle;
u16 ServoAngle;
u08 HallSensor;

u08 myClass;
u08 myAddress;


u08 OutputPinSet;

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
#define batteryTripletClass		0xF7
#define batteryDuetClass		0xF6

//#define broadcastAddress		0xFE

#define COMM_TIMEOUT			200

#define CYCLES_PER_US ((F_CPU+500000)/1000000) 	// cpu cycles per microsecond

#endif
