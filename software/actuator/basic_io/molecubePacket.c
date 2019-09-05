#include <string.h>
#include <avr/io.h>		// include I/O definitions (port names, pin names, etc)
#include <avr/interrupt.h>	// include interrupt support

#include "global.h"
#include "buffer.h"
#include "uart2.h"
#include "timer.h"

#include "molecubePacket.h"


#ifndef CRITICAL_SECTION_START
#define CRITICAL_SECTION_START	unsigned char _sreg = SREG; cli()
#define CRITICAL_SECTION_END	SREG = _sreg
#endif


u16 timeout_length;
u16	timeout_tics;


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
	//void (*uartSendFxn)(void) = NULL;
	u08 c;
	if(uart == INTERNAL_SERIAL)
	{
		uartSendFxn = &uart0SendByte;
		//uartSendFxn = &uart0AddToTxBuffer;
		INT_RX_DISABLE;
		INT_TX_ENABLE;
	} else {
		uartSendFxn = &uart1SendByte;
		//uartSendFxn = &uart1AddToTxBuffer;
		EXT_RX_DISABLE;
		EXT_TX_ENABLE;
	}

	/*
	cbi(UCSR0B, RXCIE);   //set to low
	cbi(UCSR0B, TXCIE);
	cbi(UCSR1B, RXCIE);
	cbi(UCSR1B, TXCIE);
*/
/*
	cbi(TIMSK0, TOIE0);
	cbi(TIMSK1, TOIE1);
	cbi(TIMSK2, TOIE2);
*/
	CRITICAL_SECTION_START;
	//cli(); // disables interrupts
	while((MAX_COMMAND_PACKET_SIZE - bufferIsNotFull(&cmdBuffer)) > 0)
	{
		c = bufferGetFromFront(&cmdBuffer);
		uartSendFxn(c);
//		delay_micro(1);
	}
//	sei(); // enables interrupts
	CRITICAL_SECTION_END;

/*
	sbi(TIMSK0, TOIE0);
	sbi(TIMSK1, TOIE1);
	sbi(TIMSK2, TOIE2);
	
	*/
/*
	sbi(UCSR0B, RXCIE);   //set to high
	sbi(UCSR0B, TXCIE);
	sbi(UCSR1B, RXCIE);
	sbi(UCSR1B, TXCIE);
*/

	/*
	if(uart == INTERNAL_SERIAL)
	{
		uartSendTxBuffer(INTERNAL_SERIAL);
		while(uartTransmitBufferIsEmpty(INTERNAL_SERIAL)){}
	} else {
		uartSendTxBuffer(EXTERNAL_SERIAL);
		while(uartTransmitBufferIsEmpty(EXTERNAL_SERIAL)){}
	}*/
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
	{
//		UBRR0L = 0x01;					// Baud Rate 500k	
//		UBRR0L = 0x02;					// Baud Rate 333k	
		cmd_AddByteToEnd(p_count + 2);	
	}
	else					
	{
//		UBRR0L = 0x02;					// Baud Rate 333k	
		cmd_AddByteToEnd(p_count);		
	};
	
	cmd_AddByteToEnd(cmd);

	chksum += id;

	if (dev_class == 0xFF) 	{chksum += (p_count + 2);		}
	else					{chksum += p_count;				};

	chksum += (cmd);
	
	for(i=0; i<p_count; i++)
	{
		cmd_AddByteToEnd(params[i]);
		chksum+=params[i];
	}
	
	if (dev_class == 0xFF)	{cmd_AddByteToEnd(~chksum);		}
	else					{cmd_AddByteToEnd(chksum);		};

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
