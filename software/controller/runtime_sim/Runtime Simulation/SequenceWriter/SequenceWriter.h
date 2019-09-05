#ifndef SEQWRITE_H
#define SEQWRITE_H

#include "stdafx.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>

// Header is 38 bytes
#pragma pack(push, 1)
typedef struct seqHeader {
	char name[25]; // 8bit*25
	int topologyHash; // 32bit // LSB is cubecount
	int cmdCount; // 32bit
	int size; // in bytes 32bit
	unsigned char header_checksum; // 8bit
} SEQHEADER;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct cmdPkt { // 10 byte header, 15 bytes overall
	int timestamp;
	unsigned char bus; // internal or external
	unsigned char header;
	unsigned char h_class;
	unsigned char id;
	unsigned char len;
	unsigned char instruct;
	unsigned char *params; // 32bits
	unsigned char chksum; // 8 bits
} CMDPKT;
#pragma pack(pop)

SEQHEADER* generateSampleSequenceHeader(int cubecount, int pktcount, int size);
CMDPKT* generateSP_LEDoff(unsigned char id, int ts, unsigned char bus);
CMDPKT* generateSP_LEDon(unsigned char id, int ts, unsigned char bus);
int writeSequenceFile(char* filename);
unsigned char calcHeaderChecksum(SEQHEADER *header);
unsigned char calcPacketChecksum(CMDPKT *cmd);
int writePacket(CMDPKT *pkt, FILE *seqFile);

#endif
