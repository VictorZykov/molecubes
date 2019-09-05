#include "global.h"

#define ORIENTATION_PIN_ON	0x01
#define ORIENTATION_PIN_OFF	0x00

#define SIDEID_0	0
#define SIDEID_1	1
#define SIDEID_2	2
#define SIDEID_3	3
#define SIDEID_4	4
#define SIDEID_5	5

#define SIDEPIN0	0x00
#define SIDEPIN1	0x01
#define SIDEPIN2	0x02
#define SIDEPIN3	0x03

#define COMMANDS_NOERROR						0x00
#define COMMANDS_ERROR_CANNOT_SET_PIN			0x01
#define COMMANDS_ERROR_ON_TARGET_CUBE			0x02
#define COMMANDS_ERROR_TIMEOUT					0x03
#define COMMANDS_ERROR_PARAMETERS				0x04
#define COMMANDS_ERROR_MEM_ALLOC				0x05
#define COMMANDS_ERROR_INVALID_PIN_COMBO		0x06
#define COMMANDS_NO_DEVICE_FOUND				0x07
#define COMMANDS_NO_LINK_FOUND					0x08
#define COMMANDS_ERROR_DUPLICATE_CUBE_LINK		0x09
#define COMMANDS_ERROR_DUPLICATE_CUBE			0x0A
#define COMMANDS_ERROR_CANNOT_CREATE_CUBE		0x0B
#define COMMANDS_NO_STATE_FOUND					0x0C
#define COMMANDS_ERROR_DUPLICATE_STATE			0x0D
#define COMMANDS_ERROR_CANNOT_SET_STATE			0x0E
#define COMMANDS_UNKNOWN_CLASS					0x0F
#define COMMANDS_SIDE_NOT_PRESENT				0x10

int getMeasuredAngle(u08 targetID, u08 targetClass, u16 *angle);
int getOrientationPins(u08 targetID, u08 targetClass, u08 *side, u08 *pin);
int processGetOrientationPins(u08 *params, u08 *side, u08 *pin);
int setOrientationPin(u08 targetID, u08 targetClass, u08 board, u08 pin, u08 state);
int setLEDs(u08 targetID, u08 targetClass, u08 side, u08 red, u08 green, u08 blue);
int bitCount(u08 param_count, u08 *params);
void printOrientationErrorCode(int code);
