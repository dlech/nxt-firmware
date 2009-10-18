//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 9:50                                     $
//
// Filename        $Workfile:: c_ioctrl.h                                    $
//
// Version         $Revision:: 4                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/c_ioctrl.h $
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
