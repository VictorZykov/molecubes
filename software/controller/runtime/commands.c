#include <string.h>
#include "global.h"			// include our global project settings
#include "lpc2000.h"		// include LPC210x defines
#include "commands.h"
#include "molecubePacket.h"
#include "runtime.h"

int setServoSpeedAndAngle(u08 targetID, u08 targetClass, u16 angle, u16 velo)
{
	u08 params[4];

	params[0] = (angle & 0x00ff);
	params[1] = (angle & 0xff00) >> 8;
	params[2] = (velo & 0x00ff);
	params[3] = (velo & 0xff00) >> 8;

	cmd_BuildPacket(targetClass, targetID, 0x15, 4, params);
	cmd_Execute(EXTERNAL_SERIAL);

	// determine response and return error codes if necessary
	u32 a = 0;
	while ((packetRxProcess(EXTERNAL_SERIAL, 
		targetClass, 
		targetID, 
		uartGetRxBuffer(EXTERNAL_SERIAL)) == FALSE) && 
		(a < CMD_RESPONSE_TIMEOUT))
	{	a++; }
	if (a < CMD_RESPONSE_TIMEOUT) // timeout has not occurred
	{
		if((ext_busRxPkt.paramCnt == 0) && 
			(ext_busRxPkt.cmd == 0x15))
			return COMMANDS_NOERROR; // no problem
		else
			return COMMANDS_ERROR_ON_TARGET_CUBE;
	} else { // timeout has occurred
		return COMMANDS_ERROR_TIMEOUT;
	}
}

int setServoAngleTracking(u08 targetID, u08 targetClass, u08 state)
{
	u08 params[1];
	if(state > 0x01)
		return COMMANDS_ERROR_PARAMETERS;

	params[0] = state;

	cmd_BuildPacket(targetClass, targetID, 0x0E, 1, params);
	cmd_Execute(EXTERNAL_SERIAL);

	// determine response and return error codes if necessary
	u32 a = 0;
	while ((packetRxProcess(EXTERNAL_SERIAL, 
		targetClass, 
		targetID, 
		uartGetRxBuffer(EXTERNAL_SERIAL)) == FALSE) && 
		(a < CMD_RESPONSE_TIMEOUT))
	{	a++; }
	if (a < CMD_RESPONSE_TIMEOUT) // timeout has not occurred
	{
		if((ext_busRxPkt.paramCnt == 0) && 
			(ext_busRxPkt.cmd == 0x0E))
			return COMMANDS_NOERROR; // no problem
		else
			return COMMANDS_ERROR_ON_TARGET_CUBE;
	} else { // timeout has occurred
		return COMMANDS_ERROR_TIMEOUT;
	}
}

int setLEDs(u08 targetID, u08 targetClass, u08 side, u08 red, u08 green, u08 blue)
{
	u08 params[4];
	u32 a=0;

	params[0] = side;
	params[1] = red;
	params[2] = green;
	params[3] = blue;

	cmd_BuildPacket(targetClass, targetID, 0x12, 4, params);
	cmd_Execute(EXTERNAL_SERIAL);

	while ((packetRxProcess(EXTERNAL_SERIAL, 
		targetClass, 
		targetID, 
		uartGetRxBuffer(EXTERNAL_SERIAL)) == FALSE) && 
		(a < CMD_RESPONSE_TIMEOUT))
	{	a++; }
	if (a < CMD_RESPONSE_TIMEOUT) // timeout has not occurred
	{
		if((ext_busRxPkt.paramCnt == 0) && 
			(ext_busRxPkt.cmd == 0x12))
			return COMMANDS_NOERROR; // no problem
		else if ((ext_busRxPkt.paramCnt == 1) && 
			(ext_busRxPkt.params[0] == 0x01)) {
			return COMMANDS_SIDE_NOT_PRESENT; // right side is not present
		} else if ((ext_busRxPkt.paramCnt == 1) && 
			(ext_busRxPkt.params[0] == 0x02)) {
			return COMMANDS_SIDE_NOT_PRESENT; // left side is not present
		} else {
			return COMMANDS_ERROR_ON_TARGET_CUBE; // error has occurred on target cube
		}
	} else { // timeout has occurred
		return COMMANDS_ERROR_TIMEOUT;
	}
}
/*
int bitCount(u08 param_count, u08 *params)
{
	int sum = 0;
	u08 i,j;
	for(i=0; i<param_count; i++)
	{
		for(j=0; j<8; j++)
		{
			sum += ((params[i] >> j) & 0x01);
		}
	}
	return sum;
}
*/
void printCommandErrorCode(int code)
{
	switch(code) {
		case COMMANDS_NOERROR:
			rprintf("COMMANDS_NOERROR\n\r"); 
			break;
		case COMMANDS_ERROR_CANNOT_SET_PIN:
			rprintf("COMMANDS_ERROR_CANNOT_SET_PIN\n\r"); 
			break;
		case COMMANDS_ERROR_ON_TARGET_CUBE:
			rprintf("COMMANDS_ERROR_ON_TARGET_CUBE\n\r"); 
			break;
		case COMMANDS_ERROR_TIMEOUT:
			rprintf("COMMANDS_ERROR_TIMEOUT\n\r"); 
			break;
		case COMMANDS_ERROR_PARAMETERS:
			rprintf("COMMANDS_ERROR_PARAMETERS\n\r"); 
			break;
		case COMMANDS_ERROR_MEM_ALLOC:
			rprintf("COMMANDS_ERROR_MEM_ALLOC\n\r"); 
			break;
		case COMMANDS_ERROR_INVALID_PIN_COMBO:
			rprintf("COMMANDS_ERROR_INVALID_PIN_COMBO\n\r"); 
			break;
		case COMMANDS_NO_DEVICE_FOUND:
			rprintf("COMMANDS_NO_DEVICE_FOUND\n\r"); 
			break;
		case COMMANDS_NO_LINK_FOUND:
			rprintf("COMMANDS_NO_LINK_FOUND\n\r"); 
			break;
		case COMMANDS_ERROR_DUPLICATE_CUBE_LINK:
			rprintf("COMMANDS_ERROR_DUPLICATE_CUBE_LINK\n\r"); 
			break;
		case COMMANDS_ERROR_DUPLICATE_CUBE:
			rprintf("COMMANDS_ERROR_DUPLICATE_CUBE\n\r"); 
			break;
		case COMMANDS_ERROR_CANNOT_CREATE_CUBE:
			rprintf("COMMANDS_ERROR_CANNOT_CREATE_CUBE\n\r"); 
			break;
		case COMMANDS_NO_STATE_FOUND:
			rprintf("COMMANDS_NO_STATE_FOUND\n\r"); 
			break;
		case COMMANDS_ERROR_DUPLICATE_STATE:
			rprintf("COMMANDS_ERROR_DUPLICATE_STATE\n\r"); 
			break;
		case COMMANDS_ERROR_CANNOT_SET_STATE:
			rprintf("COMMANDS_ERROR_CANNOT_SET_STATE\n\r"); 
			break;
		case COMMANDS_UNKNOWN_CLASS:
			rprintf("COMMANDS_UNKNOWN_CLASS\n\r"); 
			break;
		case COMMANDS_SIDE_NOT_PRESENT:
			rprintf("COMMANDS_SIDE_NOT_PRESENT\n\r"); 
			break;
		default:
			rprintf("COMMANDS - UNKNOWN ERROR CODE\n\r");
			break;
	}
}
