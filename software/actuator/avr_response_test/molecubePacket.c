#include <string.h>
#include <avr/io.h>		// include I/O definitions (port names, pin names, etc)
#include <avr/interrupt.h>	// include interrupt support

#include "global.h"
#include "buffer.h"
#include "uart2.h"
#include "timer.h"
#include "buffer.h"

#include "molecubePacket.h"

u16 timeout_length;
u16	timeout_tics;

#ifndef CRITICAL_SECTION_START
#define CRITICAL_SECTION_START	unsigned char _sreg = SREG; cli()
#define CRITICAL_SECTION_END	SREG = _sreg
#endif

void configCommBuses(void)
{
	// configure internal serial bus pins for output
	sbi(INT_SERIAL_CTL_DDR, INT_SERIAL_CTL_RXPIN);
	sbi(INT_SERIAL_CTL_DDR, INT_SERIAL_CTL_TXPIN);
	INT_TX_DISABLE;
	INT_RX_DISABLE;
	
	// configure external serial bus pins for output
	sbi(EXT_SERIAL_CTL_DDR, EXT_SERIAL_CTL_RXPIN);
	sbi(EXT_SERIAL_CTL_DDR, EXT_SERIAL_CTL_TXPIN);
	EXT_TX_DISABLE;
	EXT_RX_DISABLE;

	// initialize the UART (serial port)
	uartInit();
	
	uartSetBaudRate(INTERNAL_SERIAL,115200); // Internal Serial Bus (recfg 2M)
	uartSetBaudRate(EXTERNAL_SERIAL,115200); // External Serial Bus (recfg 1M)

	// Internal Serial Bus Custom Cfg
//	UCSR0A = 0x02;   // U2X0 = 1, 8 samples, double speed
	UCSR0A = 0x00;   // U2X0 = 0, 16 samples, normal speed
	UBRR0H = 0x00;
//	UBRR0L = 0x00;   // Baud Rate 1M	/	2M
//	UBRR0L = 0x01;   // Baud Rate 500k	/	1M
	UBRR0L = 0x02;   // Baud Rate 333k	/	667k
//	UBRR0L = 0x03;   // Baud Rate 250k	/	500k
//	UBRR0L = 0x04;   // Baud Rate 200k	/	400k
//	UBRR0L = 0x05;   // Baud Rate 167k	/	333k

	// External Serial Bus Custom Cfg
//	UCSR1A = 0x02;   // U2X1 = 1, 8 samples, double speed
	UCSR1A = 0x00;   // U2X1 = 0, 16 samples, normal speed
	UBRR1H = 0x00;
//	UBRR1L = 0x00;   // Baud Rate 1M	/	2M
//	UBRR1L = 0x01;   // Baud Rate 500k	/	1M
	UBRR1L = 0x02;   // Baud Rate 333k	/	667k
}

void cmd_Init(void) {
	bufferInit(&cmdBuffer, cmdBufferData, MAX_COMMAND_PACKET_SIZE);
} 

u08 cmd_AddByteToEnd(u08 b) {
	return bufferAddToEnd(&cmdBuffer, b);
}

void cmd_Flush(void) {
	bufferFlush(&cmdBuffer);
}

void cmd_Execute(u08 uart) {
	void (*uartSendFxn)(unsigned char c);
	u08 c;
	if(uart == INTERNAL_SERIAL)
	{
		uartSendFxn = &uart0SendByte;
		INT_RX_DISABLE;
		INT_TX_ENABLE;
	} else {
		uartSendFxn = &uart1SendByte;
		EXT_RX_DISABLE;
		EXT_TX_ENABLE;
	}
	//cli(); // disables interrupts
	CRITICAL_SECTION_START;
	while((MAX_COMMAND_PACKET_SIZE - bufferIsNotFull(&cmdBuffer)) > 0)
	{
		c = bufferGetFromFront(&cmdBuffer);
		uartSendFxn(c);
	}
	//sei(); // enables interrupts
	CRITICAL_SECTION_END;
	if(uart == INTERNAL_SERIAL)
	{
		delay_micro(20); // delay to allow final byte in buffer to TX
		INT_TX_DISABLE;
		INT_RX_ENABLE;
	} else {
		delay_micro(20); // delay to allow final byte in buffer to TX
		EXT_TX_DISABLE;
		EXT_RX_ENABLE;
	}

	// here: listen to the servo response packet 
	// and check if it's an error. If error persists, report to ARM

	cmd_Flush();
}

void cmd_BuildPacket(u08 dev_class, u08 id, u08 p_count, u08 cmd, u08 *params)
{

	u08 i, chksum = 0;
	cmd_Flush();
	
	cmd_AddByteToEnd(0xFF);
	cmd_AddByteToEnd(dev_class);
	cmd_AddByteToEnd(id);
	
	if (dev_class == 0xFF)
		cmd_AddByteToEnd(p_count + 2);
	else
		cmd_AddByteToEnd(p_count);
	
	cmd_AddByteToEnd(cmd);

	chksum += id;

	if (dev_class == 0xFF) 	{chksum += (p_count + 2);}
	else					{chksum += p_count;	}

	chksum += (cmd);
	
	for(i=0; i<p_count; i++)
	{
		cmd_AddByteToEnd(params[i]);
		chksum+=params[i];
	}
	
	if (dev_class == 0xFF)	{cmd_AddByteToEnd(~chksum);}
	else					{cmd_AddByteToEnd(chksum);}

}

void delay_micro(u08 us)
{
	u08 i;
	for(i=0; i<3*us; i++)
	{
		sbi(PORTB,PB5);
		cbi(PORTB,PB5);
	};
};

u08 packetRxProcess(u08 serBus, u08 devClass, u08 devAddress, cBuffer* rxBuffer)
{
	u08 foundpacket = FALSE;
	u08 startFlag = FALSE;
	u08 rxchksum, chksum = 0;
	RXPKT *trgPkt;
	u16 i;

	// process the receive buffer
	// go through buffer looking for packets
	while((rxBuffer->datalength) > 0)
	{
		// look for a start of NMEA packet
		if((bufferGetAtIndex(rxBuffer,0) == 0xFF) &&
			((bufferGetAtIndex(rxBuffer,1) == devClass)	||
			(bufferGetAtIndex(rxBuffer,1) == BROADCAST_CLASS)))

			// comparing two consecutive bits helps avoid nasty bugs 
			// when FF is a part of a packet addressed to a different
			// microprocessor. MAKE SURE there are no sequences that include
			// 2 bytes FF, FA..FF, this will result in bad glitches.
			// Can't get on-the fly frequency switching to work reliably.
		{
			// found start
			startFlag = TRUE;
			// when start is found, we leave it intact in the receive buffer
			// in case the full NMEA string is not completely received.  The
			// start will be detected in the next nmeaProcess iteration.

			// done looking for start
			break;
		}
		else
			bufferGetFromFront(rxBuffer);
	}
	
	// if we detected a start, look for end of packet
	if(startFlag)
	{
		if(rxBuffer->datalength >= 4) // enough of packet has been read to determine length
		{
			u08 p_count = 0;
			if (devClass == 0xFF)	p_count = bufferGetAtIndex(rxBuffer,3) - 2;
			else					p_count = bufferGetAtIndex(rxBuffer,3);
			
			u08 max_rx_buffer_size;
			if(serBus == EXTERNAL_SERIAL)
				max_rx_buffer_size = UART1_RX_BUFFER_SIZE;
			else 
				max_rx_buffer_size = UART0_RX_BUFFER_SIZE;

			if(p_count + 6 > max_rx_buffer_size)
				bufferFlush(rxBuffer);
			else if(rxBuffer->datalength >= p_count + 6)
				// entire packet has been rxed
			{
				// Class and Address match
				if(((bufferGetAtIndex(rxBuffer,1) == devClass) && (bufferGetAtIndex(rxBuffer,2) == devAddress)) || 
					(bufferGetAtIndex(rxBuffer,1) == BROADCAST_CLASS))
	     		{
					// grab checksum
					rxchksum = bufferGetAtIndex(rxBuffer,p_count  + 5);
					chksum = 0;
					// calc checksum
					chksum += bufferGetAtIndex(rxBuffer,2); // id
					chksum += bufferGetAtIndex(rxBuffer,3); // p_count
					chksum += bufferGetAtIndex(rxBuffer,4); // cmd
					for(i=0; i<p_count; i++)
					{
						chksum += bufferGetAtIndex(rxBuffer,5+i); // params
					}

					if (devClass == 0xFF)	chksum = ~chksum;

					if(rxchksum == chksum)
					{
						// command successfully received!
						if(serBus == INTERNAL_SERIAL)
							trgPkt = &int_busRxPkt;
						else
							trgPkt = &ext_busRxPkt;

						trgPkt->id = bufferGetAtIndex(rxBuffer,2);
						trgPkt->devClass = bufferGetAtIndex(rxBuffer,1);
						trgPkt->cmd = bufferGetAtIndex(rxBuffer,4);
						trgPkt->paramCnt = p_count;
						
						for(i=0; i<p_count; i++)
							trgPkt->params[i] = bufferGetAtIndex(rxBuffer,5+i);
						
						foundpacket = TRUE;
						bufferFlush(rxBuffer);
					} else
						bufferGetFromFront(rxBuffer);
				} else
					bufferGetFromFront(rxBuffer);
			}
		}
	}
	else if(rxBuffer->datalength >= rxBuffer->size)
	{
		// if we found no packet, and the buffer is full
		// we're logjammed, flush entire buffer
		bufferFlush(rxBuffer);
	}

	return foundpacket;
}
