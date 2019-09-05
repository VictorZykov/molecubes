#include <string.h>
#include "global.h"
#include "lpc2000.h"		// include LPC210x defines
#include "processor.h"		// include processor initialization functions
#include "buffer.h"
#include "uartintr.h"
#include "rprintf.h"

#include "molecubePacket.h"

void HD232_Init(void)
{
	uart0Init(UART_BAUD(1000000), UART_8N1, UART_FIFO_8);
	uart1Init(UART_BAUD(1000000), UART_8N1, UART_FIFO_8);
	
	// Configure Enhanced baud rate at 500kbaud External
	// Enhanced uses FDR register allowing for lower error
	U1LCR = ULCR_DLAB_ENABLE;
	U1FDR = 0x41; //0x41 -> 1M
	U1DLL = 9; // 3 -> 1M
	U1DLM = 0;
	U1LCR = UART_8N1;
	
	// Configure Enhanced baud rate at 500kbaud Internal
	// Enhanced uses FDR register allowing for lower error
	U0LCR = ULCR_DLAB_ENABLE;
	U0FDR = 0x41;
	U0DLL = 9;
	U0DLM = 0;
	U0LCR = UART_8N1;

	// uart0 internal serial bus buffer pin config
	IO1DIR |= (1<<INT_TXDIRCTL);
	IO1CLR |= (1<<INT_TXDIRCTL);
	IO1DIR |= (1<<INT_RXDIRCTL);
	IO1CLR |= (1<<INT_RXDIRCTL);
	
	// uart0 internal serial bus buffer pin config
	IO1DIR |= (1<<EXT_TXDIRCTL);
	IO1CLR |= (1<<EXT_TXDIRCTL);
	IO1DIR |= (1<<EXT_RXDIRCTL);
	IO1CLR |= (1<<EXT_RXDIRCTL);

	IO1DIR |= (1<<INT_RADIO_ISOLATE);
	IO1CLR |= (1<<INT_RADIO_ISOLATE);
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
	int (*uartSendFxn)(int) = NULL;
	unsigned char c;
	if(uart == INTERNAL_SERIAL)
	{
		uartSendFxn = &uart0SendByte;
		RS232_INT_DISABLE_RX;
		RS232_INT_ENABLE_TX;
	} else {
		uartSendFxn = &uart1SendByte;
		RS232_EXT_DISABLE_RX;
		RS232_EXT_ENABLE_TX;
	}
	while((MAX_COMMAND_PACKET_SIZE - bufferIsNotFull(&cmdBuffer)) > 0)
	{
		c = bufferGetFromFront(&cmdBuffer);
		uartSendFxn(c);
	}
	
	if(uart == INTERNAL_SERIAL)
	{
		while( !(U0LSR & ULSR_TEMT) );
		RS232_INT_DISABLE_TX;
		RS232_INT_ENABLE_RX;
	} else {
		while( !(U1LSR & ULSR_TEMT) );
		RS232_EXT_DISABLE_TX;
		RS232_EXT_ENABLE_RX;
	}
	cmd_Flush();
}

void cmd_BuildPacket(u08 dev_class, u08 id, u08 cmd, u08 p_count, u08 *params)
{
	u08 i, chksum = 0;
	cmd_Flush();
	cmd_AddByteToEnd(0xFF);
	cmd_AddByteToEnd(dev_class);
	cmd_AddByteToEnd(id);
	cmd_AddByteToEnd(p_count);
	cmd_AddByteToEnd(cmd);

	chksum += id;
	chksum += (p_count);
	chksum += (cmd);
	/*
	rprintfu08(0xff);
	rprintfu08(dev_class);
	rprintfu08(id);
	rprintfu08(p_count);
	rprintfu08(cmd);
	*/
	for(i=0; i<p_count; i++)
	{
		cmd_AddByteToEnd(params[i]);
		chksum+=params[i];
		//rprintfu08(params[i]);
	}
	cmd_AddByteToEnd(chksum);
	//rprintfu08(chksum);
	//rprintfCRLF();
	
}

u08 packetRxProcess(u08 serBus, u08 devClass, u08 devAddress, cBuffer* rxBuffer)
{
	u08 foundpacket = FALSE;
	u08 startFlag = FALSE;
	u08 rxchksum, chksum = 0;
	RXPKT *trgPkt;
	u16 i;

	// process the receive buffer
	// go through buffer looking for packets
	while((rxBuffer->datalength) > 1)
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
				if((bufferGetAtIndex(rxBuffer,1) == devClass) && 
					((bufferGetAtIndex(rxBuffer,2) == devAddress) || (bufferGetAtIndex(rxBuffer,2) == BROADCAST_CLASS)))
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

					// this is specifically for communicating with the AX12 (not performed by controller)
					if (devClass == 0xFF)	chksum = ~chksum;

					if(rxchksum == chksum)
					{

						// command successfully received!
						if(serBus == INTERNAL_SERIAL)
						{
							trgPkt = &int_busRxPkt;
						} else {
							trgPkt = &ext_busRxPkt;
						}
						
						trgPkt->devclass = bufferGetAtIndex(rxBuffer,1);
						trgPkt->id = bufferGetAtIndex(rxBuffer,2);
						trgPkt->cmd = bufferGetAtIndex(rxBuffer,4);
						trgPkt->paramCnt = p_count;
						for(i=0; i<p_count; i++)
						{
							trgPkt->params[i] = bufferGetAtIndex(rxBuffer,5+i);
						}
						foundpacket = TRUE;
						bufferFlush(rxBuffer);
					} else
						bufferGetFromFront(rxBuffer);
				} else
					bufferGetFromFront(rxBuffer);
			}
		}
	}
	else if(rxBuffer->datalength >= rxBuffer->size-1)
	{
		// if we found no packet, and the buffer is full
		// we're logjammed, flush entire buffer
		bufferFlush(rxBuffer);
	}

	return foundpacket;
}

u08 packetRxProcess_NoAddrClassCheck(u08 serBus, cBuffer* rxBuffer)
{
	u08 foundpacket = FALSE;
	u08 startFlag = FALSE;
	u08 rxchksum, chksum = 0;
	RXPKT *trgPkt;
	u16 i;

	// process the receive buffer
	// go through buffer looking for packets
	while((rxBuffer->datalength) > 1)
	{

		// look for a start of NMEA packet
		if((bufferGetAtIndex(rxBuffer,0) == 0xFF) &&
			(bufferGetAtIndex(rxBuffer,1) > 0xF5))
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
			
			p_count = bufferGetAtIndex(rxBuffer,3); // assumed not to be 0xFF

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

				if(rxchksum == chksum)
				{
					// command successfully received!
					if(serBus == INTERNAL_SERIAL)
					{
						trgPkt = &int_busRxPkt;
					} else {
						trgPkt = &ext_busRxPkt;
					}
					
					trgPkt->devclass = bufferGetAtIndex(rxBuffer,1);
					trgPkt->id = bufferGetAtIndex(rxBuffer,2);
					trgPkt->cmd = bufferGetAtIndex(rxBuffer,4);
					trgPkt->paramCnt = p_count;
					for(i=0; i<p_count; i++)
					{
						trgPkt->params[i] = bufferGetAtIndex(rxBuffer,5+i);
					}
					foundpacket = TRUE;
					bufferFlush(rxBuffer);
				} else
					bufferGetFromFront(rxBuffer);
			}
		}
	}
	else if(rxBuffer->datalength >= rxBuffer->size-1)
	{
		// if we found no packet, and the buffer is full
		// we're logjammed, flush entire buffer
		bufferFlush(rxBuffer);
	}

	return foundpacket;
}

void cmd_PrintPacket(void)
{
	u08 i;
	u08 len = (MAX_COMMAND_PACKET_SIZE - bufferIsNotFull(&cmdBuffer));
	
	for(i=0; i<len; i++)
	{
		rprintfu08(bufferGetAtIndex(&cmdBuffer, i));
	}
}

void cmd_PrintRxPacket(RXPKT *pkt)
{
	u08 i;
	rprintf("PktRX Cmd:");
	rprintfu08(pkt->cmd);
	rprintfCRLF();

	for(i=0; i<pkt->paramCnt; i++)
	{
		rprintfu08(pkt->params[i]);
	}
	rprintfCRLF();
}
