#include "global.h"
#include "ledDriver.h"
#include "i2c.h"		// include i2c function library

void RGB_LED_init(u08 addr, u08 red, u08 green, u08 blue)
{
	u08 PCAstart[10];
    
    PCAstart[0] =  0b10000000;  // Control REG Auto increment for all 
								// PCA9633 registers, starting with 0000
    PCAstart[1] =  0b10000001;  // 00h MODE 1      ?: bit 4
    PCAstart[2] =  0b00000101;  // 01h MODE 2       ?: bit 2
    PCAstart[3] =  red;         // 02h PMW0  RED
    PCAstart[4] =  green;       // 03h PMW1  GREEN
    PCAstart[5] =  blue;        // 03h PMW2  BLUE
    PCAstart[6] =  0x00;        // 05h PMW3  Not used
    PCAstart[7] =  0xFF;        // 06h GRPPWM Overall Brightness
    PCAstart[8] =  0x00;        // 07h GRPFREQ Blinking rate
    PCAstart[9] =  0b10101010;  // 08h LEDOUT LED output state

	i2cMasterSend(addr, 10, PCAstart);
}

void RGB_LED_PWMx(u08 addr, u08 red, u08 green, u08 blue)
{
	u08 PCAstart[4];
    
    PCAstart[0] =  0b10100010;  // Control REG: Auto increment for PWMx 
								// PCA9633 registers, starting with 0010
    PCAstart[1] =  red;			// 02h PMW0  RED
    PCAstart[2] =  green;       // 03h PMW1  GREEN
    PCAstart[3] =  blue;        // 03h PMW2  BLUE

	i2cMasterSend(addr, 4, PCAstart);
}
