#ifndef MPK_H
#define MPK_H

#include "global.h"
#include "lpc2000.h"		// include LPC210x defines
#include "processor.h"		// include processor initialization functions
#include "buffer.h"
#include "uartintr.h"
#include "rprintf.h"

#define INTERNAL_SERIAL	0 // uart0
#define EXTERNAL_SERIAL	1 // uart1

#define INT_RADIO_ISOLATE	20
#define RS232_INT_RADIO_ISOLATE_ENABLE IO1SET|=(1<<INT_RADIO_ISOLATE)
#define RS232_INT_RADIO_ISOLATE_DISABLE IO1CLR|=(1<<INT_RADIO_ISOLATE)

#define INT_TXDIRCTL	16
#define INT_RXDIRCTL	17
#define RS232_INT_ENABLE_TX IO1SET|=(1<<INT_TXDIRCTL)
#define RS232_INT_DISABLE_TX IO1CLR|=(1<<INT_TXDIRCTL)
#define RS232_INT_ENABLE_RX IO1SET|=(1<<INT_RXDIRCTL)
#define RS232_INT_DISABLE_RX IO1CLR|=(1<<INT_RXDIRCTL)

#define EXT_TXDIRCTL	18
#define EXT_RXDIRCTL	19
#define RS232_EXT_ENABLE_TX IO1SET|=(1<<EXT_TXDIRCTL)
#define RS232_EXT_DISABLE_TX IO1CLR|=(1<<EXT_TXDIRCTL)
#define RS232_EXT_ENABLE_RX IO1SET|=(1<<EXT_RXDIRCTL)
#define RS232_EXT_DISABLE_RX IO1CLR|=(1<<EXT_RXDIRCTL)

#define BROADCAST_CLASS 0xFE

#define MAX_COMMAND_PACKET_SIZE	128
unsigned char cmdBufferData[MAX_COMMAND_PACKET_SIZE];
cBuffer cmdBuffer;

typedef struct {
	u08 id;
	u08 devclass;
	u08 cmd;
	u08 params[128];
	u08 paramCnt;
} RXPKT;

RXPKT int_busRxPkt;
RXPKT ext_busRxPkt;

void cmd_Init(void);
u08 cmd_AddByteToEnd(u08 b);
void cmd_Flush(void);
void cmd_Execute(u08 uart);
void cmd_BuildPacket(u08 dev_class, u08 id, u08 cmd, u08 p_count, u08 *params);
void cmd_PrintPacket(void);
void cmd_PrintRxPacket(RXPKT *pkt);
void HD232_Init(void);
u08 packetRxProcess(u08 serBus, u08 devClass, u08 devAddress, cBuffer* rxBuffer);
u08 packetRxProcess_NoAddrClassCheck(u08 serBus, cBuffer* rxBuffer);

#endif
