#include "global.h"

#define BUTTON1		0
#define BUTTON2		1
#define BUTTON_NUM_INTERRUPTS	2

void initButtons(void);

void buttonPressService(void);

void enableButtons(void);
void disableButtons(void);

void buttonAttach(u08 buttonNum, void (*userFunc)(void));
void buttonDetach(u08 buttonNum);
