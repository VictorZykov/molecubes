#include "global.h"			// include our global project settings
#include "runtime.h"
#include "processor.h"		// include processor initialization functions
#include "lpc2000.h"		// include LPC210x defines
#include "buffer.h"
#include "usb.h"

static unsigned char uartRxData[UART_RX_BUFFER_SIZE];
static unsigned char uartTxData[UART_TX_BUFFER_SIZE];
cBuffer uartRxBuffer;				///< uart receive buffer
cBuffer uartTxBuffer;				///< uart transmit buffer
unsigned short uartRxOverflow;		///< receive overflow counter

// data structure for GET_LINE_CODING / SET_LINE_CODING class requests
typedef struct {
	U32		dwDTERate;
	U8		bCharFormat;
	U8		bParityType;
	U8		bDataBits;
} TLineCoding;

static TLineCoding LineCoding = {115200, 0, 0, 8};
static U8 abBulkBuf[UART_TX_BUFFER_SIZE];
//static TLineCoding LineCoding = {1000000, 0, 0, 8};
static U8 abClassReqData[8];
static volatile BOOL fBulkInBusy;
static volatile BOOL fChainDone;

// forward declaration of interrupt handler
static void USBIntHandler(void) __attribute__ ((interrupt("IRQ")));
static void SendNextBulkIn(U8 bEP, BOOL fFirstPacket);

static const U8 abDescriptors[] = {

// device descriptor
	0x12,
	DESC_DEVICE,
	LE_WORD(0x0101),			// bcdUSB
	0x02,						// bDeviceClass
	0x00,						// bDeviceSubClass
	0x00,						// bDeviceProtocol
	MAX_PACKET_SIZE0,			// bMaxPacketSize
	LE_WORD(0xFFFF),			// idVendor
	LE_WORD(0x0005),			// idProduct
	LE_WORD(0x0100),			// bcdDevice
	0x01,						// iManufacturer
	0x02,						// iProduct
	0x03,						// iSerialNumber
	0x01,						// bNumConfigurations

// configuration descriptor
	0x09,
	DESC_CONFIGURATION,
	LE_WORD(67),				// wTotalLength
	0x02,						// bNumInterfaces
	0x01,						// bConfigurationValue
	0x00,						// iConfiguration
	0xC0,						// bmAttributes
	0x32,						// bMaxPower
// control class interface
	0x09,
	DESC_INTERFACE,
	0x00,						// bInterfaceNumber
	0x00,						// bAlternateSetting
	0x01,						// bNumEndPoints
	0x02,						// bInterfaceClass
	0x02,						// bInterfaceSubClass
	0x01,						// bInterfaceProtocol, linux requires value of 1 for the cdc_acm module
	0x00,						// iInterface
// header functional descriptor
	0x05,
	CS_INTERFACE,
	0x00,
	LE_WORD(0x0110),
// call management functional descriptor
	0x05,
	CS_INTERFACE,
	0x01,
	0x01,						// bmCapabilities = device handles call management
	0x01,						// bDataInterface
// ACM functional descriptor
	0x04,
	CS_INTERFACE,
	0x02,
	0x02,						// bmCapabilities
// union functional descriptor
	0x05,
	CS_INTERFACE,
	0x06,
	0x00,						// bMasterInterface
	0x01,						// bSlaveInterface0
// notification EP
	0x07,
	DESC_ENDPOINT,
	INT_IN_EP,					// bEndpointAddress
	0x03,						// bmAttributes = intr
	LE_WORD(8),					// wMaxPacketSize
	0x0A,						// bInterval
// data class interface descriptor
	0x09,
	DESC_INTERFACE,
	0x01,						// bInterfaceNumber
	0x00,						// bAlternateSetting
	0x02,						// bNumEndPoints
	0x0A,						// bInterfaceClass = data
	0x00,						// bInterfaceSubClass
	0x00,						// bInterfaceProtocol
	0x00,						// iInterface
// data EP OUT
	0x07,
	DESC_ENDPOINT,
	BULK_OUT_EP,				// bEndpointAddress
	0x02,						// bmAttributes = bulk
	LE_WORD(MAX_PACKET_SIZE),	// wMaxPacketSize
	0x00,						// bInterval
// data EP in
	0x07,
	DESC_ENDPOINT,
	BULK_IN_EP,					// bEndpointAddress
	0x02,						// bmAttributes = bulk
	LE_WORD(MAX_PACKET_SIZE),	// wMaxPacketSize
	0x00,						// bInterval
	
	// string descriptors
	0x04,
	DESC_STRING,
	LE_WORD(0x0409),

	0x0E,
	DESC_STRING,
	'L', 0, 'P', 0, 'C', 0, 'U', 0, 'S', 0, 'B', 0,

	0x14,
	DESC_STRING,
	'U', 0, 'S', 0, 'B', 0, 'S', 0, 'e', 0, 'r', 0, 'i', 0, 'a', 0, 'l', 0,

	0x12,
	DESC_STRING,
	'D', 0, 'E', 0, 'A', 0, 'D', 0, 'C', 0, '0', 0, 'D', 0, 'E', 0,

// terminating zero
	0
};


/**
	Local function to handle incoming bulk data
		
	@param [in] bEP
	@param [in] bEPStatus
 */
static void BulkOut(U8 bEP, U8 bEPStatus)
{
	int i, iLen;

	
	//if (fifo_free(&rxfifo) < MAX_PACKET_SIZE) {
	if (bufferIsNotFull(&uartRxBuffer) < MAX_PACKET_SIZE) {
		// may not fit into fifo
		return;
	}

	// get data from USB into intermediate buffer
	iLen = USBHwEPRead(bEP, abBulkBuf, sizeof(abBulkBuf));
	for (i = 0; i < iLen; i++) {
		// put into FIFO
		//if (!fifo_put(&rxfifo, abBulkBuf[i])) {
		if(!bufferAddToEnd(&uartRxBuffer, abBulkBuf[i])) {	
			// overflow... :(
			//ASSERT(FALSE);
			break;
		}
	}
}


/**
	Local function to handle outgoing bulk data
		
	@param [in] bEP
	@param [in] bEPStatus
 */
static void BulkIn(U8 bEP, U8 bEPStatus)
{
	SendNextBulkIn(bEP, FALSE);
}

/**
	Sends the next packet in chain of packets to the host
		
	@param [in] bEP
	@param [in] bEPStatus
 */
static void SendNextBulkIn(U8 bEP, BOOL fFirstPacket)
{
	int i, iLen;
	
	// this transfer is done
	fBulkInBusy = FALSE;
	
	// first packet?
	if (fFirstPacket) {
		fChainDone = FALSE;
	}

	// last packet?
	if (fChainDone) {
		return;
	}
	//transmitted data will never be over MAX_PACKET_SIZE due to buffer use
	//if there is any data, send it all
	if (uartTxBuffer.datalength>0)
	{
		// get bytes from transmit FIFO into intermediate buffer
		iLen = uartTxBuffer.datalength;
		for (i = 0; i < iLen; i++) {
			abBulkBuf[i] = bufferGetFromFront(&uartTxBuffer);
		}
		
		// send over USB
		USBHwEPWrite(bEP, abBulkBuf, iLen);
		
		fBulkInBusy = TRUE;

		// was this a short packet?
		// it will always be less than or equal to MAX_PACKET_SIZE
		//   there will never be another packet to send
		//if (iLen < MAX_PACKET_SIZE) {
			fChainDone = TRUE;
		//}
	}
}

void SendData2Host( BYTE str[], BYTE len )
{
	register char i=0;

	// check to make sure we have a good pointer
	if (!str) return;
	
	// then print exactly len characters
	for(i=0; i<len; i++)
	{
		VCOM_putchar(*str++);
	}
}

/**
	Local function to handle the USB-CDC class requests
		
	@param [in] pSetup
	@param [out] piLen
	@param [out] ppbData
 */
static BOOL HandleClassRequest(TSetupPacket *pSetup, int *piLen, U8 **ppbData)
{
	switch (pSetup->bRequest) {

	// set line coding
	case SET_LINE_CODING:
//DBG("SET_LINE_CODING\n");
		memcpy((U8 *)&LineCoding, *ppbData, 7);
		*piLen = 7;
//DBG("dwDTERate=%u, bCharFormat=%u, bParityType=%u, bDataBits=%u\n",
//	LineCoding.dwDTERate,
//	LineCoding.bCharFormat,
//	LineCoding.bParityType,
//	LineCoding.bDataBits);
		break;

	// get line coding
	case GET_LINE_CODING:
//DBG("GET_LINE_CODING\n");
		*ppbData = (U8 *)&LineCoding;
		*piLen = 7;
		break;

	// set control line state
	case SET_CONTROL_LINE_STATE:
		// bit0 = DTR, bit = RTS
//DBG("SET_CONTROL_LINE_STATE %X\n", pSetup->wValue);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


/**
	Initialises the VCOM port.
	Call this function before using VCOM_putchar or VCOM_getchar
 */
void VCOM_init(void)
{
	bufferInit(&uartTxBuffer, uartTxData, UART_TX_BUFFER_SIZE);
	bufferInit(&uartRxBuffer, uartRxData, UART_RX_BUFFER_SIZE);
	fBulkInBusy = FALSE;
	fChainDone = TRUE;
}


/**
	Writes one character to VCOM port
	
	@param [in] c character to write
	@returns character written, or EOF if character could not be written
	though with recent modifications this will never happen. The loop waits
	until space is available in the buffer before writing, poor way to do it.
 */
int VCOM_putchar(int c)
{
	// uncomment following line for reliable terminal conn, comment for operation w/o usb conn
#ifdef RELIABLE_USB
	while(MAX_PACKET_SIZE - bufferIsNotFull(&uartTxBuffer) > 1);
#endif
	return bufferAddToEnd(&uartTxBuffer, c) ? c : EOF;
}


/**
	Reads one character from VCOM port
	
	@returns character read, or EOF if character could not be read
 */
int VCOM_getchar(void)
{
	if(uartRxBuffer.datalength == 0)
		return -1;
	return bufferGetFromFront(&uartRxBuffer);
}


/**
	Interrupt handler
	
	Simply calls the USB ISR, then signals end of interrupt to VIC
 */
static void USBIntHandler(void)
{
	USBHwISR();
	VICVectAddr = 0x00;    // dummy write to VIC to signal end of ISR 	
}

static void USBFrameHandler(U16 wFrame)
{
	if (!fBulkInBusy && uartTxBuffer.datalength) {
		// send first packet
		SendNextBulkIn(BULK_IN_EP, TRUE);
	}
}

/**
	USB device status handler
	
	Resets state machine when a USB reset is received.
 */
static void USBDevIntHandler(U8 bDevStatus)
{
	if ((bDevStatus & DEV_STATUS_RESET) != 0) {
		fBulkInBusy = FALSE;
	}
}

void usbSetup(void)
{
	
	// init DBG
	//ConsoleInit(60000000 / (16 * BAUD_RATE));

	//DBG("Initialising USB stack\n");

	// initialise stack
	USBInit();

	// register descriptors
	USBRegisterDescriptors(abDescriptors);

	// register class request handler
	USBRegisterRequestHandler(REQTYPE_TYPE_CLASS, HandleClassRequest, abClassReqData);

	// register endpoint handlers
	USBHwRegisterEPIntHandler(INT_IN_EP, NULL);
	USBHwRegisterEPIntHandler(BULK_IN_EP, BulkIn);
	USBHwRegisterEPIntHandler(BULK_OUT_EP, BulkOut);
	
	// register frame handler
	USBHwRegisterFrameHandler(USBFrameHandler);

	// register device event handler
	USBHwRegisterDevIntHandler(USBDevIntHandler);

	// initialise VCOM
	VCOM_init();

	//DBG("Starting USB communication\n");

	// set up USB interrupt
		// set up USB interrupt
	VICIntSelect &= ~(1<<22);               // select IRQ for USB
	VICIntEnable |= (1<<22);

	(*(&VICVectCntl0+INT_VECT_NUM)) = 0x20 | 22; // choose highest priority ISR slot 	
	(*(&VICVectAddr0+INT_VECT_NUM)) = (int)USBIntHandler;
	
	processorEnableInt(CPSR_MASK_IRQ);
	//enableIRQ();
	
	// connect to bus
	USBHwConnect(TRUE);
}
