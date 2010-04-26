//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dkandlun                                        $
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: d_display.h                                   $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_disp $
//
// Platform        C
//

#ifndef   D_DISPLAY
#define   D_DISPLAY

void      dDisplayInit(void);
void      dDisplayOn(UBYTE On, UBYTE Contrast);
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
