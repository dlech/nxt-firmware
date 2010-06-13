//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: c_ioctrl.h                                    $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/c_ioct $
//
// Platform        C
//

#ifndef   C_IOCTRL
#define   C_IOCTRL

typedef  struct
{
  UBYTE   Tmp;
}VARSIOCTRL;

void      cIOCtrlInit(void* pHeader);
void      cIOCtrlCtrl(void);
void      cIOCtrlExit(void);

extern    const HEADER cIOCtrl;
#endif
