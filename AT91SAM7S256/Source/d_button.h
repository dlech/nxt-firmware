//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 9:58                                     $
//
// Filename        $Workfile:: d_button.h                                    $
//
// Version         $Revision:: 4                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_button.h $
//
// Platform        C
//

#ifndef   D_BUTTON
#define   D_BUTTON

void      dButtonInit(UBYTE Prescaler);
void      dButtonExit(void);

void      dButtonRead(UBYTE *pButton);


#endif
