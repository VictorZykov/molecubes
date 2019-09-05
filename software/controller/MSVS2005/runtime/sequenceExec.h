#include "global.h"
#include "runtime.h"

void sequenceExecProcess(char *filename);
u08 isMsfFile(char *filename);
u32 runSequence(void);
u32 readSequenceFile(char* filename, SEQHEADER* trgseq);
u32 loadSequenceFile(char *filename);
void discardCommand(CMDPKT* cmd);
void initCubeState(CUBESTATE *c);
u08 validateHeaderChecksum(SEQHEADER *header);
u08 calcPacketChecksum(CMDPKT *cmd);
u08 calcHeaderChecksum(SEQHEADER *header);
void printHeader(SEQHEADER* trg);
void printCommand(CMDPKT* cmd);
void incTimestep(void);
void resetTimestep(void);
void resetExecute(void);
void enableSequenceExec(void);
void disableSequenceExec(void);
