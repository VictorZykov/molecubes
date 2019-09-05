#include <stdlib.h>
#include <stdarg.h>
#include "runtime.h"
#include "morphology.h"
#include "telemetryChannels.h"
#include "molecubePacket.h"
#include "uartintr.h"
#include "rprintf.h"
#include "commands.h"

/// TODO: open response handler which does not compare expected addr and class type, only checks chksum

/// TODO: update uartintr copy on wiki, the local version has been modified with uartgetbuffer method
int buildMorphology(MMORPH *morphology)
{
	u08 targetID;
	u08 targetClass;
	u08 ac; // address counter
	u08 sc; // side counter
	u08 returnval;
	// send command to avr controller master, verify response of device 0x00:0xFC

	// initialize target & ac
	targetID = 0x00; // hardcoded controller
	targetClass = 0xFC; // controller master class
	ac = targetID + 1;

	morphology->cubecount = 0;
	morphology->cubes = NULL;
	morphology->statecount = 0;
	morphology->states = NULL;

	// initialize controller cube entry
	if(insertNewCubeEntry(morphology, targetID, targetClass))
		return ORIENTATION_ERROR_CANNOT_CREATE_CUBE;

	while(targetID < ac)
	{
		// update targetClass from developed morphology entry
		MCUBE *next = getCube(morphology, targetID);
		if(next == NULL)
			return ORIENTATION_NO_DEVICE_FOUND;
		targetClass = next->cubeclass;

		for(sc = 0; sc < 6; sc++)
		{
			
			rprintf("ID ");
			rprintfu08(targetID);
			rprintfCRLF();
			rprintf("Class ");
			rprintfu08(targetClass);
			rprintfCRLF();
			rprintf("Side ");
			rprintfu08(sc);
			rprintfCRLF();
			
			// is cube already defined?
			// this must always be true, otherwise we have a major problem
			// need this value to narrow search for dupe link
			MCUBE *cube = NULL;
			if((cube = getCube(morphology, targetID)) == NULL)
				return ORIENTATION_NO_DEVICE_FOUND;

			// is link already defined?
			// this is needed to prevent duplicated discovery of link
			CUBELINKS *link;
			link = getCubeLink(cube, sc);
			if(link != NULL)
			{
				// link already exists skip this side
				continue;
			}
			// link does not exist we can continue
			
			rprintf("test1\n\r");
			// set orientation pin on target sc
			returnval = setOrientationPin_AR(ORIENTATION_CMD_RETRY_COUNT, targetID, targetClass, sc, SIDEPIN0, ORIENTATION_PIN_ON);
			if(returnval == ORIENTATION_SIDE_NOT_PRESENT)
			{ // side is not valid or present
				printExternalCommand();
				printOrientationErrorCode(returnval);
				continue; // skip this side
			} else if(returnval) {
				printOrientationErrorCode(returnval);
				printExternalCommand();
				return ORIENTATION_ERROR_CANNOT_SET_PIN;
			}
			
			rprintf("test2\n\r");

			// broadcast query orientation pin details
			u08 side, pin;
			returnval = broadcastGetOrientationPins_AR(ORIENTATION_CMD_RETRY_COUNT, &side, &pin);
			
			if(returnval == ORIENTATION_NOERROR) // no timeout, update Link Entry
			{
				/*
				// This data refers to self
				targetID
				targetClass
				sc
				SIDEPIN0

				// This data refers to neighbor cube
				ext_busRxPkt.id
				ext_busRxPkt.devclass
				side
				pin
				
				both self and target links need to be created,
				because target responds, we know it has been assigned an address and
				therefore a morphology entry exists for this device

				*/
				printOrientationErrorCode(returnval);
				printExternalCommand();
				if(insertNewLinkEntry(morphology, 
						targetID, targetClass, sc, SIDEPIN0,
						ext_busRxPkt.id, ext_busRxPkt.devclass, side, pin))
					return ORIENTATION_ERROR_PARAMETERS;
				continue; // increment sc and continue

			} else if (returnval != ORIENTATION_ERROR_TIMEOUT)
			{
				printOrientationErrorCode(returnval);
				printExternalCommand();
				return ORIENTATION_ERROR_ON_TARGET_CUBE;
			}
			// timeout has occurred, no known cube at this location
			rprintf("test2.5\n\r");
			// broadcast set address with orientation pin to determine if new cube is present
			returnval = broadcastSetAddressWithOrientationPin_AR(ORIENTATION_CMD_RETRY_COUNT, ac);

			if(returnval == ORIENTATION_ERROR_TIMEOUT) // no cube is present here
			{
				// unset orientation pin and continue
				rprintf("test3\n\r");
				
				returnval = setOrientationPin_AR(ORIENTATION_CMD_RETRY_COUNT, targetID, targetClass, sc, SIDEPIN0, ORIENTATION_PIN_OFF);
				if(returnval)
				{
					return ORIENTATION_ERROR_CANNOT_SET_PIN;
				}
				rprintf("test4\n\r");
				continue;
			} else if (returnval != ORIENTATION_NOERROR)
			{
				printOrientationErrorCode(returnval);
				printExternalCommand();
				return ORIENTATION_ERROR_ON_TARGET_CUBE;
			} 
			rprintf("test5\n\r");
			u08 newCubeID = ext_busRxPkt.id;
			u08 newCubeClass = ext_busRxPkt.devclass;

			// new cube address properly set, new cube morphology entry needs to be created
			if(insertNewCubeEntry(morphology, newCubeID, newCubeClass))
				return ORIENTATION_ERROR_CANNOT_CREATE_CUBE;
			rprintf("test6\n\r");
			// query orientation pin details on new cube
			if((returnval = getOrientationPins_AR(ORIENTATION_CMD_RETRY_COUNT, newCubeID, newCubeClass, &side, &pin)))
			{
				rprintf("ID: ");
				rprintfu08(newCubeID);
				rprintfCRLF();
				rprintf("Class: ");
				rprintfu08(newCubeClass);
				rprintfCRLF();
				printOrientationErrorCode(returnval);
				printExternalCommand();
				return ORIENTATION_ERROR_ON_TARGET_CUBE;
			}
			rprintf("test7\n\r");
			// update link entry
			if((returnval = insertNewLinkEntry(morphology, 
						targetID, targetClass, sc, SIDEPIN0,
						newCubeID, newCubeClass, side, pin)))
			{
				printOrientationErrorCode(returnval);
				printExternalCommand();
				return ORIENTATION_ERROR_PARAMETERS;
			}
			rprintf("test8\n\r");
			// use device class of new cube to determine how to set initial state
			returnval = setInitialState(morphology, newCubeID, newCubeClass);
			if(returnval != ORIENTATION_UNKNOWN_CLASS &&
				returnval != ORIENTATION_NOERROR)
			{
				printOrientationErrorCode(returnval);
				printExternalCommand();
				return ORIENTATION_ERROR_CANNOT_SET_STATE;
			}
			// increment ac
			ac++;

			// unset orientation pin on target SC side
			rprintf("test9\n\r");
			if(setOrientationPin_AR(ORIENTATION_CMD_RETRY_COUNT, targetID, targetClass, sc, SIDEPIN0, ORIENTATION_PIN_OFF))
				return ORIENTATION_ERROR_CANNOT_SET_PIN;
			rprintf("test10\n\r");
		}
		targetID++; // increment target
	}

	// iterate through all cubes in mesh and activate internal control loop
	if(enableAngleTracking(morphology))
	{
		rprintf("Error: Cannot enable tracking mode\n");
		return ORIENTATION_ERROR_CANNOT_SET_STATE;
	}

	return ORIENTATION_NOERROR; // morphology successfully established
}

int broadcastGetOrientationPins_AR(u08 rcount, u08 *side, u08 *pin)
{
	u08 i;
	int retval;
	for(i=0; i<rcount; i++)
	{
		retval = broadcastGetOrientationPins(side, pin);
		if(retval != ORIENTATION_ERROR_TIMEOUT)
			return retval;
	}
	return ORIENTATION_ERROR_TIMEOUT;
}

int broadcastGetOrientationPins(u08 *side, u08 *pin)
{
	cmd_BuildPacket(BROADCAST_CLASS, 0x00, 0x09, 0, NULL);
	cmd_Execute(EXTERNAL_SERIAL);

	u32 a = 0;
	while ((packetRxProcess_NoAddrClassCheck(EXTERNAL_SERIAL, 
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
				return ORIENTATION_ERROR_INVALID_PIN_COMBO;
			else if(sum == 0)
				return ORIENTATION_NO_DEVICE_FOUND; // device should not respond in the first place here

			if(processGetOrientationPins(ext_busRxPkt.params, side, pin))
				return ORIENTATION_ERROR_INVALID_PIN_COMBO;

			return ORIENTATION_NOERROR; // device found
		}
		else
		{
			printExternalCommand();
			return ORIENTATION_ERROR_ON_TARGET_CUBE; // error has occurred on target cube
		}
	} else { // timeout has occurred
		return ORIENTATION_ERROR_TIMEOUT;
	}
}

int getOrientationPins_AR(u08 rcount, u08 targetID, u08 targetClass, u08 *side, u08 *pin)
{
	u08 i;
	int retval;
	for(i=0; i<rcount; i++)
	{
		retval = getOrientationPins(targetID, targetClass, side, pin);
		if(retval != ORIENTATION_ERROR_TIMEOUT)
			return retval;
	}
	return ORIENTATION_ERROR_TIMEOUT;
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
				return ORIENTATION_ERROR_INVALID_PIN_COMBO;
			else if(sum == 0)
				return ORIENTATION_NO_DEVICE_FOUND;

			if(processGetOrientationPins(ext_busRxPkt.params, side, pin))
				return ORIENTATION_ERROR_INVALID_PIN_COMBO;

			return ORIENTATION_NOERROR; // device found
		}
		else
			return ORIENTATION_ERROR_ON_TARGET_CUBE; // error has occurred on target cube
	} else // timeout has occurred
		return ORIENTATION_ERROR_TIMEOUT;
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
		return ORIENTATION_ERROR_INVALID_PIN_COMBO;
	
	if(tmp_pins & 0x01)
		*pin = 0;
	else if(tmp_pins & 0x02)
		*pin = 1;
	else if(tmp_pins & 0x04)
		*pin = 2;
	else if(tmp_pins & 0x08)
		*pin = 3;

	return ORIENTATION_NOERROR; // device found
}

int broadcastSetAddressWithOrientationPin_AR(u08 rcount, u08 address)
{
	u08 i;
	int retval;
	for(i=0; i<rcount; i++)
	{
		retval = broadcastSetAddressWithOrientationPin(address);
		if(retval != ORIENTATION_ERROR_TIMEOUT)
			return retval;
	}
	return ORIENTATION_ERROR_TIMEOUT;
}

int broadcastSetAddressWithOrientationPin(u08 address)
{
	u08 params[1];
	params[0] = address;

	cmd_BuildPacket(BROADCAST_CLASS, address, 0x04, 1, params);
	cmd_Execute(EXTERNAL_SERIAL);
	
	u32 a = 0;
	while ((packetRxProcess_NoAddrClassCheck(EXTERNAL_SERIAL, 
		uartGetRxBuffer(EXTERNAL_SERIAL)) == FALSE) && 
		(a < CMD_RESPONSE_TIMEOUT))
	{	a++; }
	if (a < CMD_RESPONSE_TIMEOUT) // timeout has not occurred
	{
		if((ext_busRxPkt.paramCnt == 0) && 
			(ext_busRxPkt.cmd == 0x04))
			return ORIENTATION_NOERROR; // no problem
		else
		{
			return ORIENTATION_ERROR_ON_TARGET_CUBE; // error has occurred on target cube
		}
	} else { // timeout has occurred
		return ORIENTATION_ERROR_TIMEOUT;
	}
}

int getMeasuredAngle_AR(u08 rcount, u08 targetID, u08 targetClass, u16 *angle)
{
	u08 i;
	int retval;
	for(i=0; i<rcount; i++)
	{
		retval = getMeasuredAngle(targetID, targetClass, angle);
		if(retval != ORIENTATION_ERROR_TIMEOUT)
			return retval;
	}
	return ORIENTATION_ERROR_TIMEOUT;
}

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
			return ORIENTATION_ERROR_ON_TARGET_CUBE;
		*angle = (ext_busRxPkt.params[1] & 0x0F) << 8;
		*angle |= ext_busRxPkt.params[0];
		return ORIENTATION_NOERROR; // no problem
	} else { // timeout has occurred
		return ORIENTATION_ERROR_TIMEOUT;
	}
}

int setOrientationPin_AR(u08 rcount, u08 targetID, u08 targetClass, u08 board, u08 pin, u08 state)
{
	u08 i;
	int retval;
	for(i=0; i<rcount; i++)
	{
		retval = setOrientationPin(targetID, targetClass, board, pin, state);
		if(retval != ORIENTATION_ERROR_TIMEOUT)
			return retval;
	}
	return ORIENTATION_ERROR_TIMEOUT;
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
		return ORIENTATION_ERROR_PARAMETERS;
	if(pin > 0x03)
		return ORIENTATION_ERROR_PARAMETERS;
	if(state > 0x01)
		return ORIENTATION_ERROR_PARAMETERS;

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
			return ORIENTATION_NOERROR; // no problem
		else if ((ext_busRxPkt.paramCnt == 1) && 
			(ext_busRxPkt.params[0] == 0x03)) {
			return ORIENTATION_SIDE_NOT_PRESENT; // side is not present, no problem
		} else {
			return ORIENTATION_ERROR_ON_TARGET_CUBE; // error has occurred on target cube
		}
	} else { // timeout has occurred
		return ORIENTATION_ERROR_TIMEOUT;
	}
}

void printExternalCommand(void)
{
	rprintf("ID ");
	rprintfu08(ext_busRxPkt.id);
	rprintfCRLF();
	rprintf("Class ");
	rprintfu08(ext_busRxPkt.devclass);
	rprintfCRLF();
	rprintf("Cmd ");
	rprintfu08(ext_busRxPkt.cmd);
	rprintfCRLF();
	rprintf("Params ");
	rprintfu08(ext_busRxPkt.paramCnt);
	rprintfCRLF();
	int i;
	for(i=0; i<ext_busRxPkt.paramCnt; i++)
	{
		rprintfu08(ext_busRxPkt.params[i]);
	}
	rprintfCRLF();
}

// this method allows us to specify what an inital state means for various master classes
int setInitialState(MMORPH *morph, u08 id, u08 devclass)
{
	if(devclass == 0xFA) // Actuator master half (south)
	{
		u16 angle;

		// query state on newly addressed device
		if(getMeasuredAngle_AR(ORIENTATION_CMD_RETRY_COUNT, id, devclass, &angle))
		{
			return ORIENTATION_ERROR_ON_TARGET_CUBE;
		}
		// only one initial state value, actuator position
		if(insertInitialState(morph, id, devclass, 0x00, (u32)angle)) 
			return ORIENTATION_ERROR_CANNOT_SET_STATE;

		return ORIENTATION_NOERROR;
	}
	return ORIENTATION_UNKNOWN_CLASS;
}

int insertInitialState(MMORPH *morph, u08 id, u08 devclass, u08 channel, u32 value)
{
	if(getInitialState(morph, id, devclass, channel, NULL) != ORIENTATION_NO_STATE_FOUND)
		return ORIENTATION_ERROR_DUPLICATE_STATE;

	morph->statecount++;
	if((morph->states = realloc(morph->states, morph->statecount*sizeof(STATEDESC))) == NULL)
		return ORIENTATION_ERROR_MEM_ALLOC;

	STATEDESC *newState = &morph->states[morph->statecount-1];

	newState->id = id;
	newState->cubeClass = devclass;
	newState->timestamp = 0; // initial state
	newState->channel = channel;
	newState->value = value;

	return ORIENTATION_NOERROR; // no problem
}

int getInitialState(MMORPH *morph, u08 id, u08 devclass, u08 channel, STATEDESC *state)
{
	int i;
	for(i=0; i<morph->statecount; i++)
	{
		if((morph->states[i].id == id) && 
			(morph->states[i].cubeClass == devclass) &&
			(morph->states[i].channel == channel))
		{
			state = &morph->states[i];
			return ORIENTATION_NOERROR;
		}
	}
	return ORIENTATION_NO_STATE_FOUND;
}

// This performs a sequential search for a cube with a matching id and class
MCUBE *getCube(MMORPH *morph, u08 id)
{
	int i;
	for(i=0;i<morph->cubecount;i++)
	{
		if(morph->cubes[i].id == id)
		{
			return &morph->cubes[i];
		}
	}
	return NULL; // no device found
}

STATEDESC *getCubeState(MMORPH *morph, MCUBE *cube, u08 channel)
{
	int i;
	for(i=0; i<morph->statecount; i++)
	{
		if((morph->states[i].id == cube->id) &&
			(morph->states[i].cubeClass == cube->cubeclass) &&
			(morph->states[i].channel == channel))
		{
			return &morph->states[i];
		}
	}
	return NULL;
}

CUBELINKS *getCubeLink(MCUBE *cube, u08 selfSide)
{
	int i;
	for(i=0; i<cube->numlinks; i++)
	{
		if(cube->clinks[i].selfSide == selfSide)
		{
			return &cube->clinks[i];
		}
	}
	return NULL;
}

int insertNewCubeEntry(MMORPH *morph, u08 id, u08 devclass)
{
	// does this cube already exist?
	if(getCube(morph, id) != NULL)
		return ORIENTATION_ERROR_DUPLICATE_CUBE;

	// safe to create new cube
	morph->cubecount++;
	if((morph->cubes = realloc(morph->cubes, morph->cubecount*sizeof(MCUBE))) == NULL)
		return ORIENTATION_ERROR_MEM_ALLOC;

	MCUBE *newCube = &morph->cubes[morph->cubecount-1];

	newCube->id = id;
	newCube->cubeclass = devclass;
	newCube->numlinks = 0;
	newCube->clinks = NULL;

	return ORIENTATION_NOERROR;
}

// This function inserts a new link between two neighbor cubes
int insertNewLinkEntry(MMORPH *morph, 
					   u08 cubeA_id, u08 cubeA_class, u08 cubeA_side, u08 cubeA_pin,
					   u08 cubeB_id, u08 cubeB_class, u08 cubeB_side, u08 cubeB_pin)
{
	u08 orient;
	// decode orientation
	if(getPinOrientation(cubeA_pin, cubeB_pin, &orient))
		return ORIENTATION_ERROR_PARAMETERS;

	MCUBE *cubeA = NULL;
	MCUBE *cubeB = NULL;
	
	// get pointer to cube A
	cubeA = getCube(morph, cubeA_id);
	if(cubeA == NULL)
		return ORIENTATION_NO_DEVICE_FOUND;

	// get pointer to cube B
	cubeB = getCube(morph, cubeB_id);
	if(cubeB == NULL)
		return ORIENTATION_NO_DEVICE_FOUND;

	if(getCubeLink(cubeA, cubeA_side) != NULL)
		return ORIENTATION_ERROR_DUPLICATE_CUBE_LINK;

	if(getCubeLink(cubeB, cubeB_side) != NULL)
		return ORIENTATION_ERROR_DUPLICATE_CUBE_LINK;
	
	// no duplicate links exist, safe to create new link
	cubeA->numlinks++;
	if((cubeA->clinks = realloc(cubeA->clinks, cubeA->numlinks*sizeof(CUBELINKS))) == NULL)
		return ORIENTATION_ERROR_MEM_ALLOC;

	cubeB->numlinks++;
	if((cubeB->clinks = realloc(cubeB->clinks, cubeB->numlinks*sizeof(CUBELINKS))) == NULL)
		return ORIENTATION_ERROR_MEM_ALLOC;

	u08 newCubeIndexA = cubeA->numlinks-1;
	u08 newCubeIndexB = cubeB->numlinks-1;

	cubeA->clinks[newCubeIndexA].selfID = cubeA_id;
	cubeA->clinks[newCubeIndexA].selfClass = cubeA_class;
	cubeA->clinks[newCubeIndexA].selfSide = cubeA_side;
	cubeA->clinks[newCubeIndexA].selfOrient = orient;
	cubeA->clinks[newCubeIndexA].neighborID = cubeB_id;
	cubeA->clinks[newCubeIndexA].neighborClass = cubeB_class;
	cubeA->clinks[newCubeIndexA].neighborSide = cubeB_side;

	cubeB->clinks[newCubeIndexB].selfID = cubeB_id;
	cubeB->clinks[newCubeIndexB].selfClass = cubeB_class;
	cubeB->clinks[newCubeIndexB].selfSide = cubeB_side;
	cubeB->clinks[newCubeIndexB].selfOrient = orient;
	cubeB->clinks[newCubeIndexB].neighborID = cubeA_id;
	cubeB->clinks[newCubeIndexB].neighborClass = cubeA_class;
	cubeB->clinks[newCubeIndexB].neighborSide = cubeA_side;

	return ORIENTATION_NOERROR;
}

int getPinOrientation(u08 cubeA_pin, u08 cubeB_pin, u08 *orient)
{
	if((cubeA_pin == SIDEPIN0 && cubeB_pin == SIDEPIN3) ||
		(cubeA_pin == SIDEPIN1 && cubeB_pin == SIDEPIN2) ||
		(cubeA_pin == SIDEPIN2 && cubeB_pin == SIDEPIN1) ||
		(cubeA_pin == SIDEPIN3 && cubeB_pin == SIDEPIN0))
	{ // orientation 0
		*orient = 0;
		return ORIENTATION_NOERROR;
	} else if((cubeA_pin == SIDEPIN0 && cubeB_pin == SIDEPIN2) ||
		(cubeA_pin == SIDEPIN1 && cubeB_pin == SIDEPIN1) ||
		(cubeA_pin == SIDEPIN2 && cubeB_pin == SIDEPIN0) ||
		(cubeA_pin == SIDEPIN3 && cubeB_pin == SIDEPIN3))
	{ // orientation 1
		*orient = 1;
		return ORIENTATION_NOERROR;
	} else if((cubeA_pin == SIDEPIN0 && cubeB_pin == SIDEPIN1) ||
		(cubeA_pin == SIDEPIN1 && cubeB_pin == SIDEPIN0) ||
		(cubeA_pin == SIDEPIN2 && cubeB_pin == SIDEPIN3) ||
		(cubeA_pin == SIDEPIN3 && cubeB_pin == SIDEPIN2))
	{ // orientation 2
		*orient = 2;
		return ORIENTATION_NOERROR;
	} else if((cubeA_pin == SIDEPIN0 && cubeB_pin == SIDEPIN0) ||
		(cubeA_pin == SIDEPIN1 && cubeB_pin == SIDEPIN3) ||
		(cubeA_pin == SIDEPIN2 && cubeB_pin == SIDEPIN2) ||
		(cubeA_pin == SIDEPIN3 && cubeB_pin == SIDEPIN1))
	{ // orientation 3
		*orient = 3;
		return ORIENTATION_NOERROR;
	} else
		return ORIENTATION_ERROR_PARAMETERS;
}

int enableAngleTracking(MMORPH *morph)
{
	int i;
	int error_code;
	
	for(i=0; i<morph->cubecount; i++)
	{
		if(morph->cubes[i].cubeclass == 0xFA)
		{
			STATEDESC *state = getCubeState(morph, &morph->cubes[i], ACTUATOR_POSITION);
			if(state == NULL)
				return ORIENTATION_NO_STATE_FOUND;
			
			// set initial target position to position which was sensed during morphology generation
			if((error_code = setServoSpeedAndAngle(morph->cubes[i].id, morph->cubes[i].cubeclass, state->value, 0)))
			{
				printCommandErrorCode(error_code);
				return ORIENTATION_ERROR_ON_TARGET_CUBE;
			}

			// enable control loop internal to actuator cubes, this results in high packet loss and is therefore delayed
			if((error_code = setServoAngleTracking(morph->cubes[i].id, morph->cubes[i].cubeclass, ANGLE_TRACKING_ON)))
			{	
				printCommandErrorCode(error_code);
				return ORIENTATION_ERROR_ON_TARGET_CUBE;
			}
		}
	}
	return ORIENTATION_NOERROR;
}

int writeMorphologyToFile(char *filename)
{
	int i,j;
	int ret;
	u08 cube_chksum = cube_checksum_generator();
	u08 state_chksum = state_checksum_generator();

	if ( (ret = file_fopen( &filer, &efs.myFs , filename , 'w' )) )
		return ret;

	if(file_write(&filer, sizeof(u08), &cubemorph.cubecount) != sizeof(u08))
		return 2;
	for(i=0; i<cubemorph.cubecount; i++)
	{
		if(file_write(&filer, sizeof(u08), &cubemorph.cubes[i].id) != sizeof(u08))
			return 2;
		if(file_write(&filer, sizeof(u08), &cubemorph.cubes[i].cubeclass) != sizeof(u08))
			return 2;
		if(file_write(&filer, sizeof(u08), &cubemorph.cubes[i].numlinks) != sizeof(u08))
			return 2;
		
		for(j=0; j<cubemorph.cubes[i].numlinks; j++)
		{
			if(file_write(&filer, sizeof(u08), &cubemorph.cubes[i].clinks[j].selfID) != sizeof(u08))
				return 3;
			if(file_write(&filer, sizeof(u08), &cubemorph.cubes[i].clinks[j].selfClass) != sizeof(u08))
				return 3;
			if(file_write(&filer, sizeof(u08), &cubemorph.cubes[i].clinks[j].selfSide) != sizeof(u08))
				return 3;
			if(file_write(&filer, sizeof(u08), &cubemorph.cubes[i].clinks[j].selfOrient) != sizeof(u08))
				return 3;
			if(file_write(&filer, sizeof(u08), &cubemorph.cubes[i].clinks[j].neighborID) != sizeof(u08))
				return 3;
			if(file_write(&filer, sizeof(u08), &cubemorph.cubes[i].clinks[j].neighborClass) != sizeof(u08))
				return 3;
			if(file_write(&filer, sizeof(u08), &cubemorph.cubes[i].clinks[j].neighborSide) != sizeof(u08))
				return 3;
		}
	}
	
	if(file_write(&filer, sizeof(u08), &cube_chksum) != sizeof(u08))
		return 4;

	if(file_write(&filer, sizeof(u32), (u08*)&cubemorph.statecount) != sizeof(u32))
		return 5;

	for(i=0; i<cubemorph.statecount; i++)
	{
		if(file_write(&filer, sizeof(u08), &cubemorph.states[i].id) != sizeof(u08))
			return 6;
		if(file_write(&filer, sizeof(u08), &cubemorph.states[i].cubeClass) != sizeof(u08))
			return 6;
		if(file_write(&filer, sizeof(u32), (u08*)&cubemorph.states[i].timestamp) != sizeof(u32))
			return 6;
		if(file_write(&filer, sizeof(u08), &cubemorph.states[i].channel) != sizeof(u08))
			return 6;
		if(file_write(&filer, sizeof(u32), (u08*)&cubemorph.states[i].value) != sizeof(u32))
			return 6;
	}

	if(file_write(&filer, sizeof(u08), &state_chksum) != sizeof(u08))
		return 4;

	file_fclose(&filer);
	return 0;
}

int buildMorphologyFromFile(char *filename)
{
	int i,j;
	u08 cube_chksum, state_chksum;
	deleteMorphology();

	if ( file_fopen( &filer, &efs.myFs , filename , 'r' ) )
		return 1;
	
	if(file_read(&filer, sizeof(u08), &cubemorph.cubecount) != sizeof(u08))
		return 2;
	cubemorph.cubes = (MCUBE*)malloc(cubemorph.cubecount * sizeof(MCUBE));
	
	for(i=0; i<cubemorph.cubecount; i++)
	{
		if(file_read(&filer, sizeof(u08), &cubemorph.cubes[i].id) != sizeof(u08))
			return 2;
		if(file_read(&filer, sizeof(u08), &cubemorph.cubes[i].cubeclass) != sizeof(u08))
			return 2;
		if(file_read(&filer, sizeof(u08), &cubemorph.cubes[i].numlinks) != sizeof(u08))
			return 2;
		cubemorph.cubes[i].clinks = (CUBELINKS*)malloc(cubemorph.cubes[i].numlinks * sizeof(CUBELINKS));

		for(j=0; j<cubemorph.cubes[i].numlinks; j++)
		{
			if(file_read(&filer, sizeof(u08), &cubemorph.cubes[i].clinks[j].selfID) != sizeof(u08))
				return 3;
			if(file_read(&filer, sizeof(u08), &cubemorph.cubes[i].clinks[j].selfClass) != sizeof(u08))
				return 3;
			if(file_read(&filer, sizeof(u08), &cubemorph.cubes[i].clinks[j].selfSide) != sizeof(u08))
				return 3;
			if(file_read(&filer, sizeof(u08), &cubemorph.cubes[i].clinks[j].selfOrient) != sizeof(u08))
				return 3;
			if(file_read(&filer, sizeof(u08), &cubemorph.cubes[i].clinks[j].neighborID) != sizeof(u08))
				return 3;
			if(file_read(&filer, sizeof(u08), &cubemorph.cubes[i].clinks[j].neighborClass) != sizeof(u08))
				return 3;
			if(file_read(&filer, sizeof(u08), &cubemorph.cubes[i].clinks[j].neighborSide) != sizeof(u08))
				return 3;
		}
	}
	
	if(file_read(&filer, sizeof(u08), &cube_chksum) != sizeof(u08))
		return 4;

	if(file_read(&filer, sizeof(u32), (u08*)&cubemorph.statecount) != sizeof(u32))
		return 5;
	cubemorph.states = (STATEDESC*)malloc(cubemorph.statecount * sizeof(STATEDESC));

	for(i=0; i<cubemorph.statecount; i++)
	{
		if(file_read(&filer, sizeof(u08), &cubemorph.states[i].id) != sizeof(u08))
			return 6;
		if(file_read(&filer, sizeof(u08), &cubemorph.states[i].cubeClass) != sizeof(u08))
			return 6;
		if(file_read(&filer, sizeof(u32), (u08*)&cubemorph.states[i].timestamp) != sizeof(u32))
			return 6;
		if(file_read(&filer, sizeof(u08), &cubemorph.states[i].channel) != sizeof(u08))
			return 6;
		if(file_read(&filer, sizeof(u32), (u08*)&cubemorph.states[i].value) != sizeof(u32))
			return 6;
	}

	if(file_read(&filer, sizeof(u08), &state_chksum) != sizeof(u08))
		return 7;

	if(cube_chksum != cube_checksum_generator() || 
		state_chksum != state_checksum_generator())
	{
		deleteMorphology();
		return 8;
	}

	file_fclose(&filer);

	return 0;
}

void deleteMorphology(void)
{
	int i;
	for(i=0; i<cubemorph.cubecount; i++)
		free(cubemorph.cubes[i].clinks);
	free(cubemorph.cubes);
	cubemorph.cubecount = 0;
	free(cubemorph.states);
	cubemorph.statecount = 0;
}

u08 checksum_calc(void *buf, u08 size)
{
	int i;
	u08 sum=0;
	unsigned char *c_buf;

	c_buf=(unsigned char *)buf;

	for(i=0 ; i<size ; i++,c_buf++){
		sum += *c_buf;
	}

	return sum;
}


u08 cube_checksum_generator(void)
{
	u08 i, j;
	u08 sum = 0;
	sum += checksum_calc(&cubemorph.cubecount, sizeof(u08));
	
	for(i=0; i<cubemorph.cubecount; i++)
	{
		sum += checksum_calc(&cubemorph.cubes[i], 3 * sizeof(u08));
		for(j=0; j<cubemorph.cubes[i].numlinks; j++)
		{
			sum += checksum_calc(&cubemorph.cubes[i].clinks[j], 7 * sizeof(u08));
		}
	}

	return sum;
}

u08 state_checksum_generator(void)
{
	u08 i;
	u08 sum = 0;
	sum += checksum_calc(&cubemorph.statecount, 4 * sizeof(u08));
	
	for(i=0; i<cubemorph.statecount; i++)
	{
		sum += checksum_calc(&cubemorph.states[i], 11 * sizeof(u08));
	}

	return sum;
}

void printOrientationErrorCode(int code)
{
	switch(code) {
		case ORIENTATION_NOERROR:
			rprintf("ORIENTATION_NOERROR\n\r"); 
			break;
		case ORIENTATION_ERROR_CANNOT_SET_PIN:
			rprintf("ORIENTATION_ERROR_CANNOT_SET_PIN\n\r"); 
			break;
		case ORIENTATION_ERROR_ON_TARGET_CUBE:
			rprintf("ORIENTATION_ERROR_ON_TARGET_CUBE\n\r"); 
			break;
		case ORIENTATION_ERROR_TIMEOUT:
			rprintf("ORIENTATION_ERROR_TIMEOUT\n\r"); 
			break;
		case ORIENTATION_ERROR_PARAMETERS:
			rprintf("ORIENTATION_ERROR_PARAMETERS\n\r"); 
			break;
		case ORIENTATION_ERROR_MEM_ALLOC:
			rprintf("ORIENTATION_ERROR_MEM_ALLOC\n\r"); 
			break;
		case ORIENTATION_ERROR_INVALID_PIN_COMBO:
			rprintf("ORIENTATION_ERROR_INVALID_PIN_COMBO\n\r"); 
			break;
		case ORIENTATION_NO_DEVICE_FOUND:
			rprintf("ORIENTATION_NO_DEVICE_FOUND\n\r"); 
			break;
		case ORIENTATION_NO_LINK_FOUND:
			rprintf("ORIENTATION_NO_LINK_FOUND\n\r"); 
			break;
		case ORIENTATION_ERROR_DUPLICATE_CUBE_LINK:
			rprintf("ORIENTATION_ERROR_DUPLICATE_CUBE_LINK\n\r"); 
			break;
		case ORIENTATION_ERROR_DUPLICATE_CUBE:
			rprintf("ORIENTATION_ERROR_DUPLICATE_CUBE\n\r"); 
			break;
		case ORIENTATION_ERROR_CANNOT_CREATE_CUBE:
			rprintf("ORIENTATION_ERROR_CANNOT_CREATE_CUBE\n\r"); 
			break;
		case ORIENTATION_NO_STATE_FOUND:
			rprintf("ORIENTATION_NO_STATE_FOUND\n\r"); 
			break;
		case ORIENTATION_ERROR_DUPLICATE_STATE:
			rprintf("ORIENTATION_ERROR_DUPLICATE_STATE\n\r"); 
			break;
		case ORIENTATION_ERROR_CANNOT_SET_STATE:
			rprintf("ORIENTATION_ERROR_CANNOT_SET_STATE\n\r"); 
			break;
		case ORIENTATION_UNKNOWN_CLASS:
			rprintf("ORIENTATION_UNKNOWN_CLASS\n\r"); 
			break;
		case ORIENTATION_SIDE_NOT_PRESENT:
			rprintf("ORIENTATION_SIDE_NOT_PRESENT\n\r"); 
			break;
		default:
			rprintf("ORIENTATION - UNKNOWN ERROR CODE\n\r");
			break;
	}
}
