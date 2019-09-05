#ifndef _MCUBE_PKT_H_
#define _MCUBE_PKT_H_

#include <avr/io.h>		// include I/O definitions (port names, pin names, etc)
#include <avr/interrupt.h>	// include interrupt support

#include "global.h"
#include "buffer.h"
#include "uart2.h"
#include "runtime.h"

#define INTERNAL_SERIAL		0
#define EXTERNAL_SERIAL		1

#define INT_SERIAL_CTL_PORT		PORTB
#define INT_SERIAL_CTL_DDR		DDRB
#define INT_SERIAL_CTL_RXPIN	PB1
#define INT_SERIAL_CTL_TXPIN	PB2

#define INT_TX_ENABLE	sbi(INT_SERIAL_CTL_PORT, INT_SERIAL_CTL_TXPIN)
#define INT_TX_DISABLE	cbi(INT_SERIAL_CTL_PORT, INT_SERIAL_CTL_TXPIN)
#define INT_RX_ENABLE	sbi(INT_SERIAL_CTL_PORT, INT_SERIAL_CTL_RXPIN)
#define INT_RX_DISABLE	cbi(INT_SERIAL_CTL_PORT, INT_SERIAL_CTL_RXPIN)

#define EXT_SERIAL_CTL_PORT		PORTD
#define EXT_SERIAL_CTL_DDR		DDRD
#define EXT_SERIAL_CTL_RXPIN	PD5
#define EXT_SERIAL_CTL_TXPIN	PD4

#define EXT_TX_ENABLE	sbi(EXT_SERIAL_CTL_PORT, EXT_SERIAL_CTL_TXPIN)
#define EXT_TX_DISABLE	cbi(EXT_SERIAL_CTL_PORT, EXT_SERIAL_CTL_TXPIN)
#define EXT_RX_ENABLE	sbi(EXT_SERIAL_CTL_PORT, EXT_SERIAL_CTL_RXPIN)
#define EXT_RX_DISABLE	cbi(EXT_SERIAL_CTL_PORT, EXT_SERIAL_CTL_RXPIN)

#define MAX_COMMAND_PACKET_SIZE	128

unsigned char cmdBufferData[MAX_COMMAND_PACKET_SIZE];
cBuffer cmdBuffer;

void cmd_Init(void);
u08 cmd_AddByteToEnd(u08 b);
void cmd_Flush(void);
void cmd_Execute(u08 uart);
void cmd_BuildPacket(u08 dev_class, u08 id, u08 cmd, u08 p_count, u08 *params);
void configCommBuses(void);
void delay_micro(u08 us);

u08 packetRxProcess(u08 serBus, u08 devClass, u08 devAddress, cBuffer* rxBuffer);

#endif
