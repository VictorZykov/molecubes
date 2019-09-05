#include "global.h"

#define INT_RADIO_ISOLATE	20
#define RS232_INT_RADIO_ENABLE IO1SET|=(1<<INT_RADIO_ISOLATE)
#define RS232_INT_RADIO_DISABLE IO1CLR|=(1<<INT_RADIO_ISOLATE)

void initBluetooth(void);
