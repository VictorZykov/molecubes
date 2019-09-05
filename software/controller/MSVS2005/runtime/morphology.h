#include "global.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "runtime.h"

#define ORIENTATION_PIN_ON	0x01
#define ORIENTATION_PIN_OFF	0x00

#define SIDEPIN0	0x00
#define SIDEPIN1	0x01
#define SIDEPIN2	0x02
#define SIDEPIN3	0x03

#define ORIENTATION_NOERROR						0x00
#define ORIENTATION_ERROR_CANNOT_SET_PIN		0x01
#define ORIENTATION_ERROR_ON_TARGET_CUBE		0x02
#define ORIENTATION_ERROR_TIMEOUT				0x03
#define ORIENTATION_ERROR_PARAMETERS			0x04
#define ORIENTATION_ERROR_MEM_ALLOC				0x05
#define ORIENTATION_ERROR_INVALID_PIN_COMBO		0x06
#define ORIENTATION_NO_DEVICE_FOUND				0x07
#define ORIENTATION_NO_LINK_FOUND				0x08
#define ORIENTATION_ERROR_DUPLICATE_CUBE_LINK	0x09
#define ORIENTATION_ERROR_DUPLICATE_CUBE		0x0A
#define ORIENTATION_ERROR_CANNOT_CREATE_CUBE	0x0B
#define ORIENTATION_NO_STATE_FOUND				0x0C
#define ORIENTATION_ERROR_DUPLICATE_STATE		0x0D
#define ORIENTATION_ERROR_CANNOT_SET_STATE		0x0E
#define ORIENTATION_UNKNOWN_CLASS				0x0F
#define ORIENTATION_SIDE_NOT_PRESENT			0x10

#define ORIENTATION_CMD_RETRY_COUNT				5

typedef struct statedesc {
	u08 id;
	u08 cubeClass;
	u32 timestamp;
	u08 channel;
	u32 value;
} STATEDESC;

typedef struct clink {
	u08 selfID;
	u08 selfClass;
	u08 selfSide;
	u08 selfOrient;
	u08 neighborID;
	u08 neighborClass;
	u08 neighborSide;
} CUBELINKS;

typedef struct mcube {
	u08 id;
	u08 cubeclass;
	u08 numlinks;
	CUBELINKS *clinks;
} MCUBE;

typedef struct mmorphology {
	u08 cubecount;
	MCUBE *cubes;
	u32 statecount;
	STATEDESC *states;
} MMORPH;

MMORPH cubemorph;

int createTestMorphology(void);
void deleteMorphology(void);
int buildMorphologyFromFile(char *filename);
int writeMorphologyToFile(char *filename);
u08 cube_checksum_generator(void);
u08 state_checksum_generator(void);

// Functions which uses functions which communicate with molecubes (need to use AutoRetry) 

int buildMorphology(MMORPH *morphology);
int setInitialState(MMORPH *morph, u08 id, u08 devclass);

// Functions which communicate externally with the molecubes

int setOrientationPin(u08 targetID, u08 targetClass, u08 board, u08 pin, u08 state);
int broadcastSetAddressWithOrientationPin(u08 address);
int getOrientationPins(u08 targetID, u08 targetClass, u08 *side, u08 *pin);
int broadcastGetOrientationPins(u08 *side, u08 *pin);
int getMeasuredAngle(u08 targetID, u08 targetClass, u16 *angle);

int enableAngleTracking(MMORPH *morph);

// AUTO RETRY External Comm Function Wrappers
int setOrientationPin_AR(u08 rcount, u08 targetID, u08 targetClass, u08 board, u08 pin, u08 state);
int broadcastSetAddressWithOrientationPin_AR(u08 rcount, u08 address);
int getOrientationPins_AR(u08 rcount, u08 targetID, u08 targetClass, u08 *side, u08 *pin);
int broadcastGetOrientationPins_AR(u08 rcount, u08 *side, u08 *pin);
int getMeasuredAngle_AR(u08 rcount, u08 targetID, u08 targetClass, u16 *angle);

// Functions which Modify the Morphology datastructure

MCUBE *getCube(MMORPH *morph, u08 id);
CUBELINKS *getCubeLink(MCUBE *cube, u08 selfSide);
STATEDESC *getCubeState(MMORPH *morph, MCUBE *cube, u08 channel);

int insertNewCubeEntry(MMORPH *morph, u08 id, u08 devclass);
int insertNewLinkEntry(MMORPH *morph, 
					   u08 cubeA_id, u08 cubeA_class, u08 cubeA_side, u08 cubeA_pin,
					   u08 cubeB_id, u08 cubeB_class, u08 cubeB_side, u08 cubeB_pin);

int getInitialState(MMORPH *morph, u08 id, u08 devclass, u08 channel, STATEDESC *state);
int insertInitialState(MMORPH *morph, u08 id, u08 devclass, u08 channel, u32 value);

// Utility Functions

void printOrientationErrorCode(int code);
void printExternalCommand(void);
int getPinOrientation(u08 cubeA_pin, u08 cubeB_pin, u08 *orient);
int processGetOrientationPins(u08 *params, u08 *side, u08 *pin);
int bitCount(u08 param_count, u08 *params);
