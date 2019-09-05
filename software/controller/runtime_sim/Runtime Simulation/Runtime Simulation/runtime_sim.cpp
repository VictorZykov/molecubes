// Runtime Simulation.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "queue.h"
#include <time.h>
#include "runtime_sim.h"

// msf - Molecube Seqence Format

int main(int argc, char* argv[])
{
	int out;
	char filename[] = "test.msf";

	printf("Reading Sequence from %s...", filename);
	if((out = loadSequenceFile(filename)))
	{
		fprintf(stderr, "\nRead failure has occurred %d\n",out);
		return -1;
	}
	printf("ok\n");

	printf("Running Sequence Sequence...");
	if((out = runSequence()))
	{
		fprintf(stderr, "\nSequence failure has occurred %d\n",out);
		return -1;
	}
	printf("complete\n");
	
	return 0;
}

void initCubeState(CUBESTATE *c)
{
	c->preloaded = false;
	c->cmd = NULL;
}

int runSequence(void)
{
	int timestep = 0, i;
	int totalCmdsExec = 0;
	int cubeCount = cmesh->topologyHash & 0x000000ff;
	CUBESTATE *cstate = (CUBESTATE *)malloc(sizeof(CUBESTATE)*cubeCount);

	for(i=0; i<cubeCount; i++)
		initCubeState(&cstate[i]);

	while(totalCmdsExec < cmesh->cmdCount)
	{
		for(i=0; i<cubeCount; i++) {
			if(cstate[i].preloaded == false)
			{ // if next in queue is due and is not a preload cmd
				if( cmesh->queues[i].count > 0 &&
					((CMDPKT*)cmesh->queues[i].head->ptr)->timestamp <= timestep) {
					//execute command and dequeue
					printf("Executing Command %d:\n", totalCmdsExec);
					printCommand((CMDPKT*)cmesh->queues[i].head->ptr);
					dequeue(&cmesh->queues[i]);
					totalCmdsExec++;
				} // else if next command is of preload type (regardless of ts) preload it


			}
			if(cstate[i].preloaded == true) {
				// if preloaded cmd ts (found in cubestate) is due, request heartbeat
			}
		}
		// if heartbeat is requested, execute

		sleep(TIMESTEP);
		timestep++;
	}
	return 0;
}

int loadSequenceFile(char *filename)
{
	SEQHEADER rdSeq;
	int out;

	if((out = readSequenceFile(filename, &rdSeq)))
		return -1;
	
	return 0;
}

void printHeader(SEQHEADER* trg)
{
	printf("Name: %s\nTopology: %d\nCommand Count: %d\nSize: %d bytes\n",
		trg->name, trg->topologyHash, trg->cmdCount, trg->size);
	printf("\n");
}

void printCommand(CMDPKT* cmd)
{
	int i;
	printf("Timestamp: %d\nBus: %d\nHeader: %d\nClass: %d\nID: %d\nLength: %d\nInstruction: %.2x\n",
		cmd->timestamp, cmd->bus, cmd->header, cmd->h_class, cmd->id, cmd->len, cmd->instruct);
	for(i=0;i<cmd->len;i++)
		printf("Param%d: %.2x\n",i,cmd->params[i]);
	printf("\n");
}






int readSequenceFile(char* filename, SEQHEADER* trgseq)
{
	FILE *seqFile;
	CMDPKT* cmd_header;
	
	unsigned char *cmd_params;
	unsigned char param_chksum;
	int i;

	if(!(seqFile = fopen(filename, "rb")))
		return 1;

	fread(trgseq,sizeof(SEQHEADER),1,seqFile);
	
	if(!validateHeaderChecksum(trgseq))
		return 2;
	
	QUEUE *cubequeue = (QUEUE*)malloc(sizeof(QUEUE)*(trgseq->topologyHash & 0x000000ff));
	initQueue(cubequeue, (trgseq->topologyHash & 0x000000ff));

	cmesh = (CUBEMESH *)malloc(sizeof(CUBEMESH));
	cmesh->cmdCount = trgseq->cmdCount;
	cmesh->topologyHash = trgseq->topologyHash;
	cmesh->queues = cubequeue;
	
	for(i=0; i<trgseq->cmdCount; i++)
	{
		cmd_header = (CMDPKT*)malloc(sizeof(CMDPKT));
		fread(cmd_header, sizeof(CMDPKT)-5, 1, seqFile);
		
		cmd_params = (unsigned char*)malloc(sizeof(char)*cmd_header->len);
		cmd_header->params = cmd_params;
		fread(cmd_params, sizeof(unsigned char), cmd_header->len, seqFile);
		fread(&param_chksum, sizeof(unsigned char), 1, seqFile);
		
		if(calcPacketChecksum(cmd_header) != param_chksum)
			return 3;
		
		NODE *n = (NODE*)malloc(sizeof(NODE));
		n->ptr = cmd_header;
		enqueue(&cmesh->queues[cmd_header->id-1], n);
		
		//printCommand(cmd_header);
	}
	fclose(seqFile);
	
	return 0;
}

bool validateHeaderChecksum(SEQHEADER *header)
{
	return (calcHeaderChecksum(header) == header->header_checksum);
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
	return ~chksum;
}
 
void sleep(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}
