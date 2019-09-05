// SequenceWriter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "SequenceWriter.h"
#include "commands.h"




/*int main(int argc, char* argv[])
{
	int out;
	char filename[] = "C:/test.msf";

	printf("Writing sequence to %s...",filename);

	if((out = writeSequenceFile(filename)))
	{
		fprintf(stderr, "\nWrite failure has occurred %d\n",out);
		return -1;
	}
	printf("ok\n");
	return 0;
}*/


int writeSequenceFile(char* filename)
{
	FILE *seqFile;
	SEQHEADER *trgseq;
	//CMDPKT *pkt;
	
	// cubecount, packet count, total packet bytes
	trgseq = generateSampleSequenceHeader(2, 4, 60);

	if(!(seqFile = fopen(filename, "wb")))
		return 1;

	fwrite(trgseq,sizeof(SEQHEADER),1,seqFile);

	//writePacket(setAngleAndSpeed(SOUTH_CLASS, 3, 3000, EXTERNAL_BUS, 0, 500), seqFile); // 15 bytes
	writePacket(setAngleAndSpeed(SOUTH_CLASS, 2, 3000, EXTERNAL_BUS, 0, 500), seqFile); // 15 bytes

	//writePacket(setAngleAndSpeed(SOUTH_CLASS, 3, 6000, EXTERNAL_BUS, 1200, 500), seqFile); // 15 bytes
	writePacket(setAngleAndSpeed(SOUTH_CLASS, 2, 9000, EXTERNAL_BUS, 1200, 500), seqFile); // 15 bytes

	//writePacket(setAngleAndSpeed(SOUTH_CLASS, 3, 12000, EXTERNAL_BUS, 2400, 500), seqFile); // 15 bytes
	
	writePacket(setAngleAndSpeed(SOUTH_CLASS, 1, 15000, EXTERNAL_BUS, 0, 500), seqFile); // 15 bytes

	writePacket(setAngleAndSpeed(SOUTH_CLASS, 1, 18000, EXTERNAL_BUS, 2400, 500), seqFile); // 15 bytes

	//writePacket(setAngleAndSpeed(SOUTH_CLASS, 6, 21000, EXTERNAL_BUS, 2700, 500), seqFile); // 15 bytes
	//writePacket(setAngleAndSpeed(SOUTH_CLASS, 2, 24000, EXTERNAL_BUS, 2700, 500), seqFile); // 15 bytes
/*
	writePacket(setLeds(SOUTH_CLASS, 0x01, 500, EXTERNAL_BUS, MAIN_SIDE, 0x00, 0x00, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(SOUTH_CLASS, 0x01, 600, EXTERNAL_BUS, MAIN_SIDE, 0xff, 0x00, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(SOUTH_CLASS, 0x01, 700, EXTERNAL_BUS, MAIN_SIDE, 0x00, 0x00, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(SOUTH_CLASS, 0x01, 800, EXTERNAL_BUS, MAIN_SIDE, 0xff, 0x00, 0x00), seqFile); // 15 bytes
	
	writePacket(setLeds(SOUTH_CLASS, 0x03, 900, EXTERNAL_BUS, MAIN_SIDE, 0x00, 0x00, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(SOUTH_CLASS, 0x03, 1000, EXTERNAL_BUS, MAIN_SIDE, 0xff, 0x00, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(SOUTH_CLASS, 0x03, 1100, EXTERNAL_BUS, MAIN_SIDE, 0x00, 0x00, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(SOUTH_CLASS, 0x03, 1200, EXTERNAL_BUS, MAIN_SIDE, 0xff, 0x00, 0x00), seqFile); // 15 bytes

	writePacket(setLeds(SOUTH_CLASS, 0x06, 1300, EXTERNAL_BUS, MAIN_SIDE, 0x00, 0x00, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(SOUTH_CLASS, 0x06, 1400, EXTERNAL_BUS, MAIN_SIDE, 0xff, 0x00, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(SOUTH_CLASS, 0x06, 1500, EXTERNAL_BUS, MAIN_SIDE, 0x00, 0x00, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(SOUTH_CLASS, 0x06, 1600, EXTERNAL_BUS, MAIN_SIDE, 0xff, 0x00, 0x00), seqFile); // 15 bytes

	writePacket(setLeds(SOUTH_CLASS, 0x02, 1700, EXTERNAL_BUS, MAIN_SIDE, 0x00, 0x00, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(SOUTH_CLASS, 0x02, 1800, EXTERNAL_BUS, MAIN_SIDE, 0xff, 0x00, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(SOUTH_CLASS, 0x02, 1900, EXTERNAL_BUS, MAIN_SIDE, 0x00, 0x00, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(SOUTH_CLASS, 0x02, 2000, EXTERNAL_BUS, MAIN_SIDE, 0xff, 0x00, 0x00), seqFile); // 15 bytes

	writePacket(setLeds(SOUTH_CLASS, 0x05, 2100, EXTERNAL_BUS, MAIN_SIDE, 0x00, 0x00, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(SOUTH_CLASS, 0x05, 2200, EXTERNAL_BUS, MAIN_SIDE, 0xff, 0x00, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(SOUTH_CLASS, 0x05, 2300, EXTERNAL_BUS, MAIN_SIDE, 0x00, 0x00, 0x00), seqFile); // 15 bytes
	writePacket(setLeds(SOUTH_CLASS, 0x05, 2400, EXTERNAL_BUS, MAIN_SIDE, 0xff, 0x00, 0x00), seqFile); // 15 bytes
*/
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

	strcpy(seq->name, "Test Sequence!!!");
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