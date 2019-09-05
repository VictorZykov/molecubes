#include "global.h"			// include our global project settings
#include "lpc2000.h"		// include LPC210x defines

#include "processor.h"		// include processor initialization functions
#include "timer.h"			// include timer functions
#include "buffer.h"
#include "uartintr.h"
#include "rprintf.h"

#include "molecubePacket.h"
#include "runtime.h"
#include "hostPacketInjection.h"

#ifdef BLUETOOTH_ENABLE
#include "bluetooth.h"
#else
#include "usb.h"
#endif


// blocking
int getByteFromUser(void)
{
	int c;
	#ifdef BLUETOOTH_ENABLE
	// returns -1 if null
	while((c = uart0GetByte()) == -1) {}
	#else
	// returns -1 if null
	while((c = VCOM_getchar())== -1) {}
	#endif

	return c;
}

void sendByteToUser(u08 byte)
{
	#ifdef BLUETOOTH_ENABLE
	uart0SendByte(byte);
	#else
	VCOM_putchar(byte);
	#endif
}

void directCommandInjection_SendCommand(u08 *devclass, u08 *id)
{
	int i;
	u08 p_count, cmd;
	u08 *params;

	*devclass = getByteFromUser();
	*id = getByteFromUser();
	p_count = getByteFromUser();
	cmd = getByteFromUser();

	params = (u08*)malloc(sizeof(u08)*p_count);
	for(i=0; i<p_count; i++)
		params[i] = getByteFromUser();

	cmd_BuildPacket(*devclass, *id, cmd, p_count, params);
	cmd_Execute(EXTERNAL_SERIAL);
	free(params);
}

void directCommandInjection_GetResponse(u08 devclass, u08 id)
{
	int i;
	u32 a = 0;
	while ((packetRxProcess(EXTERNAL_SERIAL, 
		devclass, 
		id, 
		uartGetRxBuffer(EXTERNAL_SERIAL)) == FALSE) && 
		(a < CMD_RESPONSE_TIMEOUT))
	{	a++; }
	
	if(a < CMD_RESPONSE_TIMEOUT)
	{
		// Indicate No Timeout
		sendByteToUser(0x00);
		sendByteToUser('\n');

		sendByteToUser(ext_busRxPkt.devclass);
		sendByteToUser(ext_busRxPkt.id);
		sendByteToUser(ext_busRxPkt.paramCnt);
		sendByteToUser(ext_busRxPkt.cmd);

		for(i=0; i<ext_busRxPkt.paramCnt; i++)
			sendByteToUser(ext_busRxPkt.params[i]);
		
		sendByteToUser('\n');
	} else {
		// timeout has occurred
		sendByteToUser(0x01);
		sendByteToUser('\n');
	}
}

void directCommandInjection(void)
{
	u08 devclass, id;
	directCommandInjection_SendCommand(&devclass, &id);
	directCommandInjection_GetResponse(devclass, id);
}


