//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 29-08-05 11:26                                    $
//
// Filename        $Workfile:: d_display.h                                   $
//
// Version         $Revision:: 5                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_display. $
//
// Platform        C
//

#ifndef   D_DISPLAY
#define   D_DISPLAY

void      dDisplayInit(void);
void      dDisplayOn(UBYTE On);
UBYTE     dDisplayUpdate(UWORD Height,UWORD Width,UBYTE *pImage);
void      dDisplayExit(void);



typedef   struct    
{
  UBYTE   StartX;
  UBYTE   StartY;
  UBYTE   PixelsX;
  UBYTE   PixelsY;
}
SCREEN_CORDINATE;


#endif
