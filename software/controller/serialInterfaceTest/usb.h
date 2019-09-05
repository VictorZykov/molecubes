#include <string.h>
#include "global.h"			// include our global project settings
#include "lpc2000.h"		// include LPC210x defines
#include "usbapi.h"
#include "buffer.h"

#define INT_IN_EP		0x81
#define BULK_OUT_EP		0x05
#define BULK_IN_EP		0x82

#define MAX_PACKET_SIZE	64

#define LE_WORD(x)		((x)&0xFF),((x)>>8)

// CDC definitions
#define CS_INTERFACE			0x24
#define CS_ENDPOINT				0x25

#define	SET_LINE_CODING			0x20
#define	GET_LINE_CODING			0x21
#define	SET_CONTROL_LINE_STATE	0x22

#define EOF (-1)


#define UART_TX_BUFFER_SIZE	64
#define UART_RX_BUFFER_SIZE	64

#define	INT_VECT_NUM	0

#define U8	u08
#define U16	u16
#define U32	u32

void usbSetup(void);
int VCOM_getchar(void);
void VCOM_init(void);
int VCOM_putchar(int c);
void *usbPrintChar(unsigned char p);
