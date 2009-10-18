//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 17-02-06 8:45                                     $
//
// Filename        $Workfile:: c_display.h                                   $
//
// Version         $Revision:: 8                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/c_display. $
//
// Platform        C
//

#ifndef   C_DISPLAY
#define   C_DISPLAY

#ifndef   INCLUDE_OS

typedef   struct
{
  UBYTE   *DisplaySave;
  BMPMAP  *pOldBitmaps[BITMAPS];
  UBYTE   ErasePointer;
  UBYTE   UpdatePointer;
}VARSDISPLAY;

#endif


void      cDisplayInit(void* pHeader);
void      cDisplayCtrl(void);
void      cDisplayExit(void);


extern    const HEADER cDisplay;


#endif
