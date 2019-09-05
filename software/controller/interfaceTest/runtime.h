#ifndef _RUNTIME_H_
#define _RUNTIME_H_

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "global.h"

#include "efs.h"
#include "ls.h"
#include "mkfs.h"

#define TIMESTEP					1000
#define CMD_RESPONSE_TIMEOUT		10000 // 5000 -> 12ms // 60000 -> 120ms

#define BLUETOOTH_ENABLE

#ifndef BLUETOOTH_ENABLE
#define RELIABLE_USB
#endif

EmbeddedFileSystem efs;
EmbeddedFile filer, filew;
DirList list;

#endif /* _RUNTIME_H_ */
