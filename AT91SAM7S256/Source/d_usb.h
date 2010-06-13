//
// Programmer
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dkandlun                                        $
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: d_usb.h                                       $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_usb. $
//
// Platform        C
//

#ifndef   D_USB
#define   D_USB

//* public constants

// LOW level commands

#define   OPENFILEWRITE 0x01
#define   OPENFILEREAD  0x02
#define   WRITEFILE     0x03
#define   CLOSEFILE     0x04

// Low level direct command escape

#define   DIRECTCOMMAND 0x80		// Escape sent to the Loader
#define   USB_TIMEOUT   0x0BB8      // Equals approx. 1 mS. Used for recover a "broken" cable situation

//* external function description

void      dUsbInit(void);
void      dUsbExit(void);
ULONG     dUsbRead(UBYTE *pData, ULONG Length);
ULONG     dUsbWrite( const UBYTE *pData, ULONG Length);
UBYTE     dUsbIsConfigured(void);

void      dUsbInsertHandle(UBYTE Handle);
void      dUsbRemoveHandle(UBYTE Handle);
UWORD     dUsbGetFirstHandle(void);
UWORD     dUsbGetNextHandle(void);
UWORD     dUsbCheckConnection(void);
void      dUsbResetConfig(void);
void      dUsbStoreBtAddress(UBYTE *pBtAddress);
#endif
