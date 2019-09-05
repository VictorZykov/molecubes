/*
	LPCUSB, an USB device driver for LPC microcontrollers	
	Copyright (C) 2006 Bertrik Sikken (bertrik@sikken.nl)

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright
	   notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright
	   notice, this list of conditions and the following disclaimer in the
	   documentation and/or other materials provided with the distribution.
	3. The name of the author may not be used to endorse or promote products
	   derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
	IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
	Minimal implementation of a USB serial port, using the CDC class.
	This example application simply echoes everything it receives right back
	to the host.

	Windows:
	Extract the usbser.sys file from .cab file in C:\WINDOWS\Driver Cache\i386
	and store it somewhere (C:\temp is a good place) along with the usbser.inf
	file. Then plug in the LPC214x and direct windows to the usbser driver.
	Windows then creates an extra COMx port that you can open in a terminal
	program, like hyperterminal.

	Linux:
	The device should be recognised automatically by the cdc_acm driver,
	which creates a /dev/ttyACMx device file that acts just like a regular
	serial port.

*/
#include <string.h>
#include "global.h"			// include our global project settings
#include "lpc2000.h"		// include LPC210x defines

#include "processor.h"		// include processor initialization functions
#include "timer.h"			// include timer functions
#include "buffer.h"
#include "uart.h"
#include "rprintf.h"
#include "usb.h"

#include "efs.h"
#include "ls.h"
#include "mkfs.h"
//#include "interfaces/lpc2000_dbg_printf.h"

//#define rprintf lpc2000_debug_printf
//#define BAUD 115200


EmbeddedFileSystem efs;
EmbeddedFile filer, filew;
DirList list;
unsigned short e;
unsigned char buf[513];
static char LogFileName[] = "test.txt";

/*************************************************************************
	main
	====
**************************************************************************/
int main(void)
{
	int c;
	// initialize processor
	processorInit();
	// initialize timers
	
	rprintfInit((void*)VCOM_putchar);
	usbSetup();
	
	timerInit();
	u08 res;
	int b;
	// echo any character received (do USB stuff in interrupt)
	while (1) {
		c = VCOM_getchar();
		if (c != EOF) {

			if(c == 'e')
			{
				rprintf("hello world\r");
			} else if (c == '1')
			{
				rprintf("CARD init...");
				if ( ( res = efs_init( &efs, 0 ) ) != 0 ) {
					rprintf("failed with %i\n",res);
				}
				else {
					rprintf("ok\n");
				}
			}  else if (c == '2')
			{
				rprintf("Directory of 'root':\n");
				ls_openDir( &list, &(efs.myFs) , "/");
				while ( ls_getNext( &list ) == 0 ) {
					list.currentEntry.FileName[LIST_MAXLENFILENAME-1] = '\0';
					rprintfStr(list.currentEntry.FileName); rprintf(" (");
					rprintfNum(10,10,FALSE,' ',list.currentEntry.FileSize); rprintf(" bytes)\n");
				}
			}  else if (c == '3')
			{
				if ( file_fopen( &filer, &efs.myFs , LogFileName , 'r' ) == 0 ) {
					rprintf("File "); rprintfStr(LogFileName); rprintf(" open. Content:\n");
					while ( ( e = file_read( &filer, 512, buf ) ) != 0 ) {
						buf[e]='\0';
						rprintfStr((char*)buf);
					}
					rprintf("\n");
					file_fclose( &filer );
				}
			} else if (c == '4')
			{
				if ( file_fopen( &filew, &efs.myFs , LogFileName , 'a' ) == 0 ) {
					rprintf("File "); rprintfStr(LogFileName); rprintf(" open for append. Appending...\n");
					strcpy((char*)buf, "Martin hat's angehaengt EFSL ROCKS!!!\r\n");
					if ( file_write( &filew, strlen((char*)buf), buf ) == strlen((char*)buf) ) {
						rprintf("ok\n");
					}
					else {
						rprintf("fail\n");
					}
					file_fclose( &filew );
				}
			} else if (c == '0')
			{
				fs_umount( &efs.myFs ) ;
			}
		} 
		
		b = uart1GetByte();
		if(b > -1)
		{
			VCOM_putchar(b);
		}
	}

	return 0;
}



