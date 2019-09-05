#include "stdafx.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#define TIMESTEP	1000

#pragma pack(1)
typedef struct seqHeader {
	char name[25]; // 8bit*25
	int topologyHash; // 32bit // LSB is cubecount
	int cmdCount; // 32bit
	int size; // in bytes 32bit
	unsigned char header_checksum; // 8bit
} SEQHEADER;

#pragma pack(1)
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

typedef struct cubestate {
	bool preloaded;
	CMDPKT *cmd;
} CUBESTATE;

typedef struct cubemesh {
	int topologyHash;
	int cmdCount;
	QUEUE *queues;
} CUBEMESH;

int loadSequenceFile(char *filename);
int readSequenceFile(char* filename, SEQHEADER* trgseq);

void printHeader(SEQHEADER* trg);
void printCommand(CMDPKT* cmd);

unsigned char calcPacketChecksum(CMDPKT *cmd);
bool validateHeaderChecksum(SEQHEADER *header);
unsigned char calcHeaderChecksum(SEQHEADER *header);

int runSequence(void);
void initCubeState(CUBESTATE *c);

void sleep(unsigned int mseconds);

CUBEMESH *cmesh;
