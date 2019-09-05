#include "global.h"

int getByteFromUser(void);
void sendByteToUser(u08 byte);
void directCommandInjection_SendCommand(u08 *devclass, u08 *id);
void directCommandInjection_GetResponse(u08 devclass, u08 id);
void directCommandInjection(void);
