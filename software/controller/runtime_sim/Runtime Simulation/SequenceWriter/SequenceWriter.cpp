// SequenceWriter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "SequenceWriter.h"
#include "commands.h"

// Device Classes
#define ACTUATOR_CLASS		0xFA
#define CONTROLLER_CLASS	0xFC
#define BATTERY_CLASS		0xF7
#define GRIPPER_CLASS		0xF8


#define SOUTH_MAIN_SIDE		0x00
#define SOUTH_RIGHT_SIDE	0x01
#define SOUTH_LEFT_SIDE		0x02
#define NORTH_MAIN_SIDE		0x03
#define NORTH_RIGHT_SIDE	0x04
#define NORTH_LEFT_SIDE		0x05

// Controller bus
#define EXTERNAL_BUS 0x01

int main(int argc, char* argv[])
{
	int out;
	char filename[] = "./test.msf";

	printf("Writing sequence to %s...",filename);

	if((out = writeSequenceFile(filename)))
	{
		fprintf(stderr, "\nWrite failure has occurred %d\n",out);
		return -1;
	}
	printf("ok\n");
	return 0;
}


int writeSequenceFile(char* filename)
{
	FILE *seqFile;
	SEQHEADER *trgseq;
	//CMDPKT *pkt;
	
	// cubecount, packet count, total packet bytes
	//trgseq = generateSampleSequenceHeader(6, 8, 120);
	//trgseq = generateSampleSequenceHeader(10, 47, 705);
	trgseq = generateSampleSequenceHeader(10, 94, 1410);
	if(!(seqFile = fopen(filename, "wb")))
		return 1;

	fwrite(trgseq,sizeof(SEQHEADER),1,seqFile);

	/*
	writePacket(setAngleAndSpeed(ACTUATOR_CLASS, 0x05, 1000, EXTERNAL_BUS, 3000, 300), seqFile); // 15 bytes

	writePacket(setAngleAndSpeed(ACTUATOR_CLASS, 0x02, 2000, EXTERNAL_BUS, 1200, 583), seqFile); // 15 bytes
	writePacket(setAngleAndSpeed(ACTUATOR_CLASS, 0x03, 2000, EXTERNAL_BUS, 2400, 583), seqFile); // 15 bytes
	
	writePacket(setAngleAndSpeed(ACTUATOR_CLASS, 0x02, 4000, EXTERNAL_BUS, 2400, 583), seqFile); // 15 bytes
	writePacket(setAngleAndSpeed(ACTUATOR_CLASS, 0x03, 4000, EXTERNAL_BUS, 1200, 583), seqFile); // 15 bytes

	writePacket(setAngleAndSpeed(ACTUATOR_CLASS, 0x02, 6000, EXTERNAL_BUS, 0, 583), seqFile); // 15 bytes
	writePacket(setAngleAndSpeed(ACTUATOR_CLASS, 0x03, 6000, EXTERNAL_BUS, 0, 583), seqFile); // 15 bytes
*/
/*
	writePacket(setLeds(CONTROLLER_CLASS, 0x00, 500, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x80, 0x80, 0x80), seqFile); // 15 bytes
	writePacket(setLeds(CONTROLLER_CLASS, 0x00, 500, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x80, 0x80, 0x80), seqFile);
	writePacket(setLeds(CONTROLLER_CLASS, 0x00, 500, EXTERNAL_BUS, NORTH_MAIN_SIDE, 0x80, 0x80, 0x80), seqFile);
	writePacket(setLeds(CONTROLLER_CLASS, 0x00, 500, EXTERNAL_BUS, NORTH_LEFT_SIDE, 0x80, 0x80, 0x80), seqFile);

	writePacket(setGripperConfig(GRIPPER_CLASS, 0x06, 1000, EXTERNAL_BUS, 500, 300, 300), seqFile); // 17 bytes
	writePacket(setGripperConfig(GRIPPER_CLASS, 0x06, 2000, EXTERNAL_BUS, 700, 300, 300), seqFile); // 17 bytes
*/

	// 47 commands 705 bytes
	writePacket(setLeds(CONTROLLER_CLASS, 0x00, 500, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x00, 0x00, 0x80), seqFile); // 15 bytes
	writePacket(setLeds(CONTROLLER_CLASS, 0x00, 500, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(CONTROLLER_CLASS, 0x00, 500, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(CONTROLLER_CLASS, 0x00, 500, EXTERNAL_BUS, NORTH_MAIN_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(CONTROLLER_CLASS, 0x00, 500, EXTERNAL_BUS, NORTH_LEFT_SIDE, 0x00, 0x00, 0x80), seqFile);

	writePacket(setLeds(ACTUATOR_CLASS, 0x01, 500, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x00, 0x00, 0x80), seqFile); // 15 bytes
	writePacket(setLeds(ACTUATOR_CLASS, 0x01, 500, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x01, 500, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x01, 500, EXTERNAL_BUS, NORTH_MAIN_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x01, 500, EXTERNAL_BUS, NORTH_RIGHT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x01, 500, EXTERNAL_BUS, NORTH_LEFT_SIDE, 0x00, 0x00, 0x80), seqFile);

	writePacket(setLeds(GRIPPER_CLASS, 0x02, 500, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x00, 0x00, 0x80), seqFile); // 15 bytes
	writePacket(setLeds(GRIPPER_CLASS, 0x02, 500, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(GRIPPER_CLASS, 0x02, 500, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x00, 0x00, 0x80), seqFile);

	writePacket(setLeds(ACTUATOR_CLASS, 0x03, 500, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x00, 0x00, 0x80), seqFile); // 15 bytes
	writePacket(setLeds(ACTUATOR_CLASS, 0x03, 500, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x03, 500, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x03, 500, EXTERNAL_BUS, NORTH_MAIN_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x03, 500, EXTERNAL_BUS, NORTH_RIGHT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x03, 500, EXTERNAL_BUS, NORTH_LEFT_SIDE, 0x00, 0x00, 0x80), seqFile);

	writePacket(setLeds(GRIPPER_CLASS, 0x04, 500, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x00, 0x00, 0x80), seqFile); // 15 bytes
	writePacket(setLeds(GRIPPER_CLASS, 0x04, 500, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(GRIPPER_CLASS, 0x04, 500, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x00, 0x00, 0x80), seqFile);

	writePacket(setLeds(ACTUATOR_CLASS, 0x06, 500, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x00, 0x00, 0x80), seqFile); // 15 bytes
	writePacket(setLeds(ACTUATOR_CLASS, 0x06, 500, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x06, 500, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x06, 500, EXTERNAL_BUS, NORTH_MAIN_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x06, 500, EXTERNAL_BUS, NORTH_RIGHT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x06, 500, EXTERNAL_BUS, NORTH_LEFT_SIDE, 0x00, 0x00, 0x80), seqFile);
	
	writePacket(setLeds(ACTUATOR_CLASS, 0x07, 500, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x00, 0x00, 0x80), seqFile); // 15 bytes
	writePacket(setLeds(ACTUATOR_CLASS, 0x07, 500, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x07, 500, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x07, 500, EXTERNAL_BUS, NORTH_MAIN_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x07, 500, EXTERNAL_BUS, NORTH_RIGHT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x07, 500, EXTERNAL_BUS, NORTH_LEFT_SIDE, 0x00, 0x00, 0x80), seqFile);

	writePacket(setLeds(ACTUATOR_CLASS, 0x08, 500, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x00, 0x00, 0x80), seqFile); // 15 bytes
	writePacket(setLeds(ACTUATOR_CLASS, 0x08, 500, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x08, 500, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x08, 500, EXTERNAL_BUS, NORTH_MAIN_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x08, 500, EXTERNAL_BUS, NORTH_RIGHT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x08, 500, EXTERNAL_BUS, NORTH_LEFT_SIDE, 0x00, 0x00, 0x80), seqFile);

	writePacket(setLeds(ACTUATOR_CLASS, 0x09, 500, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x00, 0x00, 0x80), seqFile); // 15 bytes
	writePacket(setLeds(ACTUATOR_CLASS, 0x09, 500, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x09, 500, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x09, 500, EXTERNAL_BUS, NORTH_MAIN_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x09, 500, EXTERNAL_BUS, NORTH_RIGHT_SIDE, 0x00, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x09, 500, EXTERNAL_BUS, NORTH_LEFT_SIDE, 0x00, 0x00, 0x80), seqFile);





	writePacket(setLeds(CONTROLLER_CLASS, 0x00, 1000, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x80, 0x00, 0x80), seqFile); // 15 bytes
	writePacket(setLeds(CONTROLLER_CLASS, 0x00, 1000, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x80, 0x00, 0x80), seqFile);
	writePacket(setLeds(CONTROLLER_CLASS, 0x00, 1000, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x80, 0x00, 0x80), seqFile);
	writePacket(setLeds(CONTROLLER_CLASS, 0x00, 1000, EXTERNAL_BUS, NORTH_MAIN_SIDE, 0x80, 0x00, 0x80), seqFile);
	writePacket(setLeds(CONTROLLER_CLASS, 0x00, 1000, EXTERNAL_BUS, NORTH_LEFT_SIDE, 0x80, 0x00, 0x80), seqFile);

	writePacket(setLeds(ACTUATOR_CLASS, 0x01, 1000, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x80, 0x00, 0x80), seqFile); // 15 bytes
	writePacket(setLeds(ACTUATOR_CLASS, 0x01, 1000, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x80, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x01, 1000, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x80, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x01, 1000, EXTERNAL_BUS, NORTH_MAIN_SIDE, 0x80, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x01, 1000, EXTERNAL_BUS, NORTH_RIGHT_SIDE, 0x80, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x01, 1000, EXTERNAL_BUS, NORTH_LEFT_SIDE, 0x80, 0x00, 0x80), seqFile);

	writePacket(setLeds(GRIPPER_CLASS, 0x02, 1000, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x80, 0x00, 0x80), seqFile); // 15 bytes
	writePacket(setLeds(GRIPPER_CLASS, 0x02, 1000, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x80, 0x00, 0x80), seqFile);
	writePacket(setLeds(GRIPPER_CLASS, 0x02, 1000, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x80, 0x00, 0x80), seqFile);

	writePacket(setLeds(ACTUATOR_CLASS, 0x03, 1000, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x80, 0x00, 0x80), seqFile); // 15 bytes
	writePacket(setLeds(ACTUATOR_CLASS, 0x03, 1000, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x80, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x03, 1000, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x80, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x03, 1000, EXTERNAL_BUS, NORTH_MAIN_SIDE, 0x80, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x03, 1000, EXTERNAL_BUS, NORTH_RIGHT_SIDE, 0x80, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x03, 1000, EXTERNAL_BUS, NORTH_LEFT_SIDE, 0x80, 0x00, 0x80), seqFile);

	writePacket(setLeds(GRIPPER_CLASS, 0x04, 2000, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x80, 0x00, 0x80), seqFile); // 15 bytes
	writePacket(setLeds(GRIPPER_CLASS, 0x04, 2000, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x80, 0x00, 0x80), seqFile);
	writePacket(setLeds(GRIPPER_CLASS, 0x04, 2000, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x80, 0x00, 0x80), seqFile);

	writePacket(setLeds(ACTUATOR_CLASS, 0x06, 2000, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x80, 0x00, 0x80), seqFile); // 15 bytes
	writePacket(setLeds(ACTUATOR_CLASS, 0x06, 2000, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x80, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x06, 2000, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x80, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x06, 2000, EXTERNAL_BUS, NORTH_MAIN_SIDE, 0x80, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x06, 2000, EXTERNAL_BUS, NORTH_RIGHT_SIDE, 0x80, 0x00, 0x80), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x06, 2000, EXTERNAL_BUS, NORTH_LEFT_SIDE, 0x80, 0x00, 0x80), seqFile);
	
	writePacket(setLeds(ACTUATOR_CLASS, 0x07, 2000, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x00, 0x80, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(ACTUATOR_CLASS, 0x07, 2000, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x00, 0x80, 0x00), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x07, 2000, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x00, 0x80, 0x00), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x07, 2000, EXTERNAL_BUS, NORTH_MAIN_SIDE, 0x00, 0x80, 0x00), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x07, 2000, EXTERNAL_BUS, NORTH_RIGHT_SIDE, 0x00, 0x80, 0x00), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x07, 2000, EXTERNAL_BUS, NORTH_LEFT_SIDE, 0x00, 0x80, 0x00), seqFile);

	writePacket(setLeds(ACTUATOR_CLASS, 0x08, 3000, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x00, 0x80, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(ACTUATOR_CLASS, 0x08, 3000, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x00, 0x80, 0x00), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x08, 3000, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x00, 0x80, 0x00), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x08, 3000, EXTERNAL_BUS, NORTH_MAIN_SIDE, 0x00, 0x80, 0x00), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x08, 3000, EXTERNAL_BUS, NORTH_RIGHT_SIDE, 0x00, 0x80, 0x00), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x08, 3000, EXTERNAL_BUS, NORTH_LEFT_SIDE, 0x00, 0x80, 0x00), seqFile);

	writePacket(setLeds(ACTUATOR_CLASS, 0x09, 3000, EXTERNAL_BUS, SOUTH_MAIN_SIDE, 0x00, 0x80, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(ACTUATOR_CLASS, 0x09, 3000, EXTERNAL_BUS, SOUTH_RIGHT_SIDE, 0x00, 0x80, 0x00), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x09, 3000, EXTERNAL_BUS, SOUTH_LEFT_SIDE, 0x00, 0x80, 0x00), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x09, 3000, EXTERNAL_BUS, NORTH_MAIN_SIDE, 0x00, 0x80, 0x00), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x09, 3000, EXTERNAL_BUS, NORTH_RIGHT_SIDE, 0x00, 0x80, 0x00), seqFile);
	writePacket(setLeds(ACTUATOR_CLASS, 0x09, 3000, EXTERNAL_BUS, NORTH_LEFT_SIDE, 0x00, 0x80, 0x00), seqFile);




	fclose(seqFile);
	
	return 0;
}

int writePacket(CMDPKT *pkt, FILE *seqFile)
{
	fwrite(pkt,sizeof(CMDPKT)-5,1,seqFile); // only write header
	fwrite((void*)pkt->params,sizeof(unsigned char),pkt->len,seqFile);
	fwrite((void*)&pkt->chksum,sizeof(unsigned char),1,seqFile);
	return pkt->len + 11;
}

SEQHEADER* generateSampleSequenceHeader(int cubecount, int pktcount, int size)
{
	SEQHEADER* seq;
	seq = (SEQHEADER*)malloc(sizeof(SEQHEADER));

	strcpy(seq->name, "Test Sequence");
	seq->topologyHash = cubecount;
	seq->cmdCount = pktcount;
	seq->size = size;
	
	seq->header_checksum = calcHeaderChecksum(seq);
	return seq;
}

unsigned char calcPacketChecksum(CMDPKT *cmd)
{ 
	int i;
	unsigned char chksum = 0;
	char *ptr = (char*)cmd;
	chksum += (cmd->id + cmd->len+2 + cmd->instruct);
	for(i=0; i<cmd->len; i++)
		chksum += cmd->params[i];
	return ~chksum;
}

unsigned char calcHeaderChecksum(SEQHEADER *header)
{ 
	int i;
	unsigned char chksum = 0;
	char *ptr = (char*)header;
	for(i=0; i<sizeof(SEQHEADER)-1; i++)
		chksum += ptr[i];
	return chksum;
}