#include <string.h>
#include "global.h"			// include our global project settings
#include "lpc2000.h"		// include LPC210x defines
#include "commands.h"
#include "molecubePacket.h"
#include "runtime.h"

int getMeasuredAngle(u08 targetID, u08 targetClass, u16 *angle)
{
	cmd_BuildPacket(targetClass, targetID, 0x07, 0, NULL);
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
		if((ext_busRxPkt.paramCnt != 2) || 
			(ext_busRxPkt.cmd != 0x07))
			return COMMANDS_ERROR_ON_TARGET_CUBE;
		*angle = (ext_busRxPkt.params[1] & 0x0F) << 8;
		*angle |= ext_busRxPkt.params[0];
		return COMMANDS_NOERROR; // no problem
	} else { // timeout has occurred
		return COMMANDS_ERROR_TIMEOUT;
	}
}

int getOrientationPins(u08 targetID, u08 targetClass, u08 *side, u08 *pin)
{
	cmd_BuildPacket(targetClass, targetID, 0x09, 0, NULL);
	cmd_Execute(EXTERNAL_SERIAL);

	u32 a = 0;
	while ((packetRxProcess(EXTERNAL_SERIAL, 
		targetClass, 
		targetID, 
		uartGetRxBuffer(EXTERNAL_SERIAL)) == FALSE) && 
		(a < CMD_RESPONSE_TIMEOUT))
	{	a++; }
	if (a < CMD_RESPONSE_TIMEOUT) // timeout has not occurred
	{
		if((ext_busRxPkt.paramCnt == 3) && 
			(ext_busRxPkt.cmd == 0x09))
		{
			u08 sum = bitCount(ext_busRxPkt.paramCnt, ext_busRxPkt.params);

			if(sum > 1)
				return COMMANDS_ERROR_INVALID_PIN_COMBO;
			else if(sum == 0)
				return COMMANDS_NO_DEVICE_FOUND;

			if(processGetOrientationPins(ext_busRxPkt.params, side, pin))
				return COMMANDS_ERROR_INVALID_PIN_COMBO;

			return COMMANDS_NOERROR; // device found
		}
		else
			return COMMANDS_ERROR_ON_TARGET_CUBE; // error has occurred on target cube
	} else // timeout has occurred
		return COMMANDS_ERROR_TIMEOUT;
}

int processGetOrientationPins(u08 *params, u08 *side, u08 *pin)
{
	u08 tmp_pins = 0x00;

	if(params[0] & 0x0F) {
		*side = 1; // Master Right
		tmp_pins = (params[0] & 0x0F);
	} else if(params[0] & 0xF0) {
		*side = 0; // Master Main
		tmp_pins = (params[0] & 0xF0) >> 4;
	} else if(params[1] & 0x0F) {
		*side = 3; // Slave Main
		tmp_pins = (params[1] & 0x0F);
	} else if(params[1] & 0xF0) {
		*side = 2; // Master Left
		tmp_pins = (params[1] & 0xF0) >> 4;
	} else if(params[2] & 0x0F) {
		*side = 5; // Slave Left
		tmp_pins = (params[2] & 0x0F);
	} else if(params[2] & 0xF0) {
		*side = 4; // Slave Right
		tmp_pins = (params[2] & 0xF0) >> 4;
	} else
		return COMMANDS_ERROR_INVALID_PIN_COMBO;
	
	if(tmp_pins & 0x01)
		*pin = 0;
	else if(tmp_pins & 0x02)
		*pin = 1;
	else if(tmp_pins & 0x04)
		*pin = 2;
	else if(tmp_pins & 0x08)
		*pin = 3;

	return COMMANDS_NOERROR; // device found
}

int setOrientationPin(
					  u08 targetID, 
					  u08 targetClass, 
					  u08 board, 
					  u08 pin, 
					  u08 state
					  )
{
	u08 params[1];

	// handle errors
	if(board > 0x0F)
		return COMMANDS_ERROR_PARAMETERS;
	if(pin > 0x03)
		return COMMANDS_ERROR_PARAMETERS;
	if(state > 0x01)
		return COMMANDS_ERROR_PARAMETERS;

	params[0] = 0x00;
	params[0] |= (board & 0x0F) << 4;
	params[0] |= (pin & 0x03) << 2;
	params[0] |= (state & 0x01);

	cmd_BuildPacket(targetClass, targetID, 0x08, 1, params);
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
			(ext_busRxPkt.cmd == 0x08))
			return COMMANDS_NOERROR; // no problem
		else if ((ext_busRxPkt.paramCnt == 1) && 
			(ext_busRxPkt.params[0] == 0x03)) {
			return COMMANDS_SIDE_NOT_PRESENT; // side is not present
		} else {
			return COMMANDS_ERROR_ON_TARGET_CUBE; // error has occurred on target cube
		}
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

void printOrientationErrorCode(int code)
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
