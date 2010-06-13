//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: c_button.h                                    $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/c_butt $
//
// Platform        C
//

#ifndef   C_BUTTON
#define   C_BUTTON

#ifdef    INCLUDE_OS
extern    const HEADER cButton;
#endif

#include  "c_button.iom"


typedef   struct
{
  UWORD   Cnt[NO_OF_BTNS];
  UBYTE   OldState;
}VARSBUTTON;

void      cButtonInit(void* pHeader);
void      cButtonCtrl(void);
void      cButtonExit(void);

extern    const HEADER cButton;

#endif
