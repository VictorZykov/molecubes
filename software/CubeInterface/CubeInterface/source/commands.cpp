#include "stdafx.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "SequenceWriter.h"
#include "commands.h"

// 11 packet bytes CMD0x06
CMDPKT* getRawAngle(unsigned char devclass, unsigned char id, int ts, unsigned char bus)
{
	CMDPKT* pkt;
	pkt = (CMDPKT*)malloc(sizeof(CMDPKT));
	
	pkt->timestamp = ts;
	pkt->bus = bus;
	pkt->header = 0xff;
	pkt->h_class = devclass;
	pkt->id = id;
	pkt->len = 0x00;
	pkt->instruct = 0x06;
	pkt->chksum = (pkt->id + pkt->len + pkt->instruct);
	
	return pkt;
}

// 11 packet bytes CMD0x07
CMDPKT* getMeasuredAngle(unsigned char devclass, unsigned char id, int ts, unsigned char bus)
{
	CMDPKT* pkt;
	pkt = (CMDPKT*)malloc(sizeof(CMDPKT));
	
	pkt->timestamp = ts;
	pkt->bus = bus;
	pkt->header = 0xff;
	pkt->h_class = devclass;
	pkt->id = id;
	pkt->len = 0x00;
	pkt->instruct = 0x07;
	pkt->chksum = (pkt->id + pkt->len + pkt->instruct);
	
	return pkt;
}

// 15 packet bytes CMD0x12
CMDPKT* setLeds(unsigned char devclass, unsigned char id, int ts, unsigned char bus, 
				unsigned char side, unsigned char red, unsigned char green, unsigned char blue)
{
	CMDPKT* pkt;
	pkt = (CMDPKT*)malloc(sizeof(CMDPKT));
	
	pkt->timestamp = ts;
	pkt->bus = bus;
	pkt->header = 0xff;
	pkt->h_class = devclass;
	pkt->id = id;
	pkt->len = 0x04;
	pkt->instruct = 0x12;
	
	pkt->params = (unsigned char *)malloc(sizeof(pkt->len));
	pkt->params[0] = side;
	pkt->params[1] = red;
	pkt->params[2] = green;
	pkt->params[3] = blue;

	pkt->chksum = (pkt->id + pkt->len + pkt->instruct + pkt->params[0] + pkt->params[1] + pkt->params[2] + pkt->params[3]);
	return pkt;
}

// 11 packet bytes CMD0x14
CMDPKT* getADCReadout(unsigned char devclass, unsigned char id, int ts, unsigned char bus)
{
	CMDPKT* pkt;
	pkt = (CMDPKT*)malloc(sizeof(CMDPKT));
	
	pkt->timestamp = ts;
	pkt->bus = bus;
	pkt->header = 0xff;
	pkt->h_class = devclass;
	pkt->id = id;
	pkt->len = 0x00;
	pkt->instruct = 0x14;
	pkt->chksum = (pkt->id + pkt->len + pkt->instruct);
	
	return pkt;
}

// 15 packet bytes CMD0x15
// devclass - type of device, for motor always SOUTH_CLASS
// id - cube address, address 0x00 is controller, cube addresses start at 0x01
// ts - time in msec of cmd execution
// bus - Command target bus - always EXTERNAL_BUS for cube commands
// angle - joint angle in tenths of degrees 360deg = 3600
// speed - angular velocity in tenths of degrees per second
CMDPKT* setAngleAndSpeed(unsigned char devclass, unsigned char id, int ts, unsigned char bus, 
						 unsigned short int angle, unsigned short int speed)
{
	CMDPKT* pkt;
	pkt = (CMDPKT*)malloc(sizeof(CMDPKT));
	
	pkt->timestamp = ts;
	pkt->bus = bus;
	pkt->header = 0xff;
	pkt->h_class = devclass;
	pkt->id = id;
	pkt->len = 0x04;
	pkt->instruct = 0x15;

	pkt->params = (unsigned char *)malloc(sizeof(pkt->len));
	pkt->params[0] = angle & 0x00ff;
	pkt->params[1] = (angle & 0xff00)>>8;
	pkt->params[2] = speed & 0x00ff;
	pkt->params[3] = (speed & 0xff00)>>8;

	pkt->chksum = (pkt->id + pkt->len + pkt->instruct + pkt->params[0] + pkt->params[1] + pkt->params[2] + pkt->params[3]);	
	return pkt;
}
