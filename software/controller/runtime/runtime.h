#ifndef _RUNTIME_H_
#define _RUNTIME_H_

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "global.h"
#include "queue.h"

#include "efs.h"
#include "ls.h"
#include "mkfs.h"

#define CMD_RESPONSE_TIMEOUT		5000	// 5000 -> 12ms
#define HIGH_PRIORITY_TIMEOUT		850		// ~2ms, used in the sequence execution procedure, timeouts expected

#define BLUETOOTH_ENABLE

#ifndef BLUETOOTH_ENABLE
#define RELIABLE_USB
#endif

// Print response packets with > 0 parameters
#define PRINT_CMD_RESPONSE
#define PRINT_DEBUG

/* 
Thread on structure packing in gcc:
http://en.mikrocontroller.net/topic/64486
*/

#pragma pack(1)
typedef struct __attribute__ ((__packed__)) {
	char name[25]; // 8bit*25
	u32 topologyHash; // 32bit // LSB is cubecount
	u32 cmdCount; // 32bit
	u32 size; // in bytes 32bit
	u08 header_checksum; // 8bit
} SEQHEADER;

#pragma pack(1)
typedef struct __attribute__ ((__packed__)) { // 10 byte header, 15 bytes overall
	u32 timestamp;
	u08 bus; // internal or external
	u08 header;
	u08 h_class;
	u08 id;
	u08 len;
	u08 instruct;
	u08 *params; // 32bits
	u08 chksum; // 8 bits
} CMDPKT;

typedef struct cubestate {
	u08 preloaded;
	CMDPKT *cmd;
} CUBESTATE;

typedef struct cubemesh {
	u32 topologyHash;
	u32 cmdCount;
	QUEUE *queues;
} CUBEMESH;

u32 loadSequenceFile(char *filename);
u32 readSequenceFile(char *filename, SEQHEADER* trgseq);

void printHeader(SEQHEADER* trg);
void printCommand(CMDPKT* cmd);

u08 calcPacketChecksum(CMDPKT *cmd);
u08 validateHeaderChecksum(SEQHEADER *header);
u08 calcHeaderChecksum(SEQHEADER *header);

u08 isMsfFile(char *filename);

u32 runSequence(void);
void initCubeState(CUBESTATE *c);

void sleep(u32 mseconds);

CUBEMESH *cmesh;

void discardCommand(CMDPKT* cmd);

EmbeddedFileSystem efs;
EmbeddedFile filer, filew;
DirList list;

#endif /* _RUNTIME_H_ */
