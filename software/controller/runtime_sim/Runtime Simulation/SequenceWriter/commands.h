#ifndef CMDS_H
#define CMDS_H

#include "stdafx.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "SequenceWriter.h"

// 11 packet bytes CMD0x06
CMDPKT* getRawAngle(unsigned char id, int ts, unsigned char bus);

// 11 packet bytes CMD0x07
CMDPKT* getMeasuredAngle(unsigned char devclass, unsigned char id, int ts, unsigned char bus);

// 15 packet bytes CMD0x12
CMDPKT* setLeds(unsigned char devclass, unsigned char id, int ts, unsigned char bus, 
				unsigned char side, unsigned char red, unsigned char green, unsigned char blue);

// 11 packet bytes CMD0x14
CMDPKT* getADCReadout(unsigned char devclass, unsigned char id, int ts, unsigned char bus);

// 15 packet bytes CMD0x15
CMDPKT* setAngleAndSpeed(unsigned char devclass, unsigned char id, int ts, unsigned char bus, 
						 unsigned short int angle, unsigned short int speed);

// 17 packet bytes CMD0x17
CMDPKT* setGripperConfig(unsigned char devclass, unsigned char id, int ts, unsigned char bus, 
						 unsigned short int pos, unsigned short int speed, unsigned short int force);


#endif
