#include "global.h"
#include <string.h>
#include <ctype.h>
#include "sequenceExec.h"
#include "lpc2000.h"
#include "processor.h"
#include "rprintf.h"
#include "queue.h"
#include "runtime.h"
#include "molecubePacket.h"
#include "commands.h"

u32 TimeStep;
u08 execute;

void enableSequenceExec(void)
{
	execute = TRUE;
}

void disableSequenceExec(void)
{
	execute = FALSE;
}

void sequenceExecProcess(char *filename)
{
	int out;
//	char filename[] = "test.msf";
	
	SEQHEADER rdSeq;
	u32 total_iters = 0;
	
	// echo any character received (do USB stuff in interrupt)
//	while (1) 
	{
		rprintf("Iters:");
		rprintfNum(10,10,FALSE,' ',total_iters);
		rprintfCRLF();
		total_iters++;

		// Load commands into buffers
		resetExecute();
#ifdef PRINT_DEBUG
		rprintf("Reading Sequence from ");
		rprintfStr(filename);
		rprintf("...");
#endif
		rprintf("Read Sequence File\r");
		if((out = readSequenceFile(filename, &rdSeq)))
		{
			rprintf("\nRead failure has occurred %d\n",out);
			file_fclose( &filer );
//			break;
			return;
		}
#ifdef PRINT_DEBUG
		rprintf("ok\n");
#endif

		// Execute command buffers
		rprintf("Reset Timer\r");
		resetTimestep(); // possibly a shared resource problem? dont write to resettimestamp when timer interrupt occurs
#ifdef PRINT_DEBUG
		rprintf("Running Sequence ...");
#endif
		rprintf("Run Sequence\r");
		if((out = runSequence()))
		{
			rprintf("\nSequence failure has occurred %d\n",out);
//			break;
			return;
		}

		// deallocate memory here
		free(cmesh);

#ifdef PRINT_DEBUG
		rprintf("complete\n");
#endif

	}
}


u08 isMsfFile(char *filename)
{
	int i = 0;
	while(isalnum(filename[i]) && (i < 255))
		i++;

	if(tolower(filename[i-4]) == '.' && tolower(filename[i-3]) == 'm' && 
		tolower(filename[i-2]) == 's' && tolower(filename[i-1]) == 'f')
	{
		return 1;
	}
	return 0;
}

u32 runSequence(void)
{
	//int timestep = 0, i;
	int i;
	int a = 0;
	int totalCmdsExec = 0;
	int cubeCount = cmesh->topologyHash & 0x000000ff;
	u08 selectedBus;
	CUBESTATE *cstate = (CUBESTATE *)malloc(sizeof(CUBESTATE)*cubeCount);
	RXPKT *trgPkt;

	for(i=0; i<cubeCount; i++)
		initCubeState(&cstate[i]);

	while(execute)
	{
		// i is an index for each cube in current topology
		for(i=0; i<cubeCount; i++) {
			//if(cstate[i].preloaded == 0) // false
			{ // if next in queue is due and is not a preload cmd
				if( cmesh->queues[i].count > 0 &&
					((CMDPKT*)cmesh->queues[i].head->ptr)->timestamp <= TimeStep) 
				{
					//execute command and dequeue
					
					cmd_BuildPacket(((CMDPKT*)cmesh->queues[i].head->ptr)->h_class, 
						((CMDPKT*)cmesh->queues[i].head->ptr)->id, 
						((CMDPKT*)cmesh->queues[i].head->ptr)->instruct, 
						((CMDPKT*)cmesh->queues[i].head->ptr)->len, 
						((CMDPKT*)cmesh->queues[i].head->ptr)->params);
					

					if(((CMDPKT*)cmesh->queues[i].head->ptr)->bus == 0x00)
					{
						selectedBus = INTERNAL_SERIAL;
						trgPkt = &int_busRxPkt;
					} else { 
						selectedBus = EXTERNAL_SERIAL;
						trgPkt = &ext_busRxPkt;
					}

					printCommand(((CMDPKT*)cmesh->queues[i].head->ptr));
					
					cmd_Execute(selectedBus);
					
					a = 0;
					while ((packetRxProcess(selectedBus, 
						((CMDPKT*)cmesh->queues[i].head->ptr)->h_class, 
						((CMDPKT*)cmesh->queues[i].head->ptr)->id, 
						uartGetRxBuffer(selectedBus)) == FALSE) && 
						(a < HIGH_PRIORITY_TIMEOUT))
					{	a++; }
					
					if (a < HIGH_PRIORITY_TIMEOUT)	// wait intil a serial timeout occurs
					{	// check if the right command is confirmed
						if (trgPkt->cmd == ((CMDPKT*)cmesh->queues[i].head->ptr)->instruct)
						{	
#ifdef PRINT_CMD_RESPONSE
							if(trgPkt->paramCnt > 0)
							{
								rprintf("RxClass:");
								rprintfu08(((CMDPKT*)cmesh->queues[i].head->ptr)->h_class);
								rprintf(" RxAddr:");
								rprintfu08(((CMDPKT*)cmesh->queues[i].head->ptr)->id);
								rprintfCRLF();
								cmd_PrintRxPacket(trgPkt);
							} else {
								rprintf("RxClass:");
								rprintfu08(((CMDPKT*)cmesh->queues[i].head->ptr)->h_class);
								rprintf(" RxAddr:");
								rprintfu08(((CMDPKT*)cmesh->queues[i].head->ptr)->id);
								rprintf(" RxCmd:");
								rprintfu08(((CMDPKT*)cmesh->queues[i].head->ptr)->instruct);
								rprintfCRLF();
							}
#endif						

							discardCommand((CMDPKT*)(getHead(&cmesh->queues[i])->ptr));
							dequeue(&cmesh->queues[i]);
							totalCmdsExec++;
						} 
#ifdef PRINT_CMD_RESPONSE
						else {
							rprintf("Incorrect CMD received\n");
							rprintf("TxCmd:");
							rprintfu08(((CMDPKT*)cmesh->queues[i].head->ptr)->instruct);
							rprintf(" RxCmd:");
							rprintfu08(trgPkt->cmd);
							rprintfCRLF();
						}
#endif
					}
#ifdef PRINT_CMD_RESPONSE					
					else {

						rprintf("Timeout occurred\n");

					}
#endif
				}
			}
		}
		
		if(totalCmdsExec >= cmesh->cmdCount)
			execute = 0;
	}
	free(cmesh->queues);
	free(cstate);
	return 0;
}
// list queues
// list commands (all queues)

u32 readSequenceFile(char* filename, SEQHEADER* trgseq)
{
	//FILE *seqFile;
	CMDPKT* cmd_header;
	unsigned char *cmd_params;
	unsigned char param_chksum;
	int i;
	rprintf("Entering Read\r");
	//if(!(seqFile = fopen(filename, "rb")))
	//	return 1;
	if ( file_fopen( &filer, &efs.myFs , filename , 'r' ) )
		return 1;

	file_read( &filer, sizeof(SEQHEADER), (void*)trgseq );
	//fread(trgseq,sizeof(SEQHEADER),1,seqFile);

	if(validateHeaderChecksum(trgseq))
		return 2;

	printHeader(trgseq);
	u08 cubeCount = (u08)(trgseq->topologyHash & 0x000000ff);
	
	QUEUE *cubequeue = (QUEUE*)malloc(sizeof(QUEUE)*cubeCount);
	initQueueArray(cubequeue, cubeCount);

	cmesh = (CUBEMESH *)malloc(sizeof(CUBEMESH));
	cmesh->cmdCount = trgseq->cmdCount;
	cmesh->topologyHash = trgseq->topologyHash;
	cmesh->queues = cubequeue;
	rprintf("Counter %d\r",trgseq->cmdCount);
	for(i=0; i<trgseq->cmdCount; i++)
	{
		cmd_header = (CMDPKT*)malloc(sizeof(CMDPKT));
		
		//fread(cmd_header, sizeof(CMDPKT)-5, 1, seqFile);
		file_read( &filer, sizeof(CMDPKT)-5, (void*)cmd_header );

		cmd_params = (unsigned char*)malloc(sizeof(char)*cmd_header->len);
		cmd_header->params = cmd_params;
		
		//fread(cmd_params, sizeof(unsigned char), cmd_header->len, seqFile);
		file_read( &filer, sizeof(unsigned char) * cmd_header->len, cmd_params );
		//fread(&param_chksum, sizeof(unsigned char), 1, seqFile);
		file_read( &filer, sizeof(unsigned char), &param_chksum );

		//printCommand(cmd_header);

		if(calcPacketChecksum(cmd_header) != param_chksum)
			return 3;
		
		NODE *n = (NODE*)malloc(sizeof(NODE));
		n->ptr = cmd_header;

		int queueIndex;

		// setup queue id assignment
		if(findQueueWithAddress(cmd_header->id, cmesh->queues, cubeCount, &queueIndex) == FALSE) // if id doesn't exist
		{
			if(findQueueWithAddress(0xFE, cmesh->queues, cubeCount, &queueIndex) == TRUE) // find next unaddressed queue
				setQueueID(&cmesh->queues[queueIndex], cmd_header->id);
			else
				return 4; // error has occurred no opening found
		}
		
		findQueueWithAddress(cmd_header->id, cmesh->queues, cubeCount, &queueIndex);
		enqueue(&cmesh->queues[queueIndex], n);
		
	}
	rprintf("Exiting...");
	//fclose(seqFile);
	if(file_fclose( &filer ))
		return 5;

	return 0;
}

u32 loadSequenceFile(char *filename)
{
	SEQHEADER rdSeq;
	int out;

	if((out = readSequenceFile(filename, &rdSeq)))
		return -1;
	
	return 0;
}

void discardCommand(CMDPKT* cmd)
{
	free(cmd->params);
	free(cmd);
}

void initCubeState(CUBESTATE *c)
{
	c->preloaded = 0;
	c->cmd = NULL;
}

u08 validateHeaderChecksum(SEQHEADER *header)
{
	return (calcHeaderChecksum(header) != header->header_checksum);
}

u08 calcPacketChecksum(CMDPKT *cmd)
{ 
	int i;
	unsigned char chksum = 0;
	//char *ptr = (char*)cmd;
	chksum += (cmd->id + cmd->len + cmd->instruct);
	for(i=0; i<cmd->len; i++)
		chksum += cmd->params[i];
	return chksum;
}

u08 calcHeaderChecksum(SEQHEADER *header)
{ 
	int i;
	unsigned char chksum = 0;
	char *ptr = (char*)header;
	for(i=0; i<sizeof(SEQHEADER)-1; i++) // dont want to include checksum
		chksum += ptr[i];
	return chksum;
}

void printHeader(SEQHEADER* trg)
{
	rprintf("\nName: ");
	rprintfStr(trg->name);
	rprintf("\nTopology: ");
	rprintfNum(10,10,FALSE,' ',trg->topologyHash);
	rprintf("\nCommand Count: ");
	rprintfNum(10,10,FALSE,' ',trg->cmdCount);
	rprintf("\nSize: ");
	rprintfNum(10,10,FALSE,' ',trg->size);
	rprintf("  bytes\n\n");
}

void printCommand(CMDPKT* cmd)
{
	int i;
	if(cmd->instruct == 0x15)
	{
		u16 velo, pos;
		pos = cmd->params[0];
		pos |= cmd->params[1] << 8;
		velo = cmd->params[2];
		velo |= cmd->params[3] << 8;

		rprintf("Set Pos Timestamp: ");
		rprintfNum(10,10,FALSE,' ',cmd->timestamp);
		rprintf("\nPos: ");
		rprintfNum(10,10,FALSE,' ',pos);
		rprintf(" Velo: ");
		rprintfNum(10,10,FALSE,' ',velo);
		rprintf("\n");
		return;
	}

	rprintf("Timestamp: ");
	rprintfNum(10,10,FALSE,' ',cmd->timestamp);
	rprintf("\nBus: ");
	rprintfNum(10,10,FALSE,' ',cmd->bus);
	rprintf("\nHeader: ");
	rprintfNum(10,10,FALSE,' ',cmd->header);
	rprintf("\nClass: ");
	rprintfNum(10,10,FALSE,' ',cmd->h_class);
	rprintf("\nID: ");
	rprintfNum(10,10,FALSE,' ',cmd->id);
	rprintf("\nLength: ");
	rprintfNum(10,10,FALSE,' ',cmd->len);
	rprintf("\nInstruction: ");
	rprintfNum(10,10,FALSE,' ',cmd->instruct);
	rprintf("\n");

	for(i=0;i<cmd->len;i++) {
		rprintf("Param%d: ",i);
		rprintfNum(16,3,FALSE,' ',cmd->params[i]);
		rprintf("\n");
	}
}

void incTimestep(void)
{
	if (TimeStep == 0xFFFFFFFF)
		execute = 0;
	TimeStep++;
}

void resetTimestep(void)
{
	TimeStep = 0;
}

void resetExecute(void)
{
	execute = 1;
}
