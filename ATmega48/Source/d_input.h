//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 16-06-05 14:32                                    $
//
// Filename        $Workfile:: d_input.h                                     $
//
// Version         $Revision:: 3                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/d_input. $
//
// Platform        C
//


#ifndef   D_INPUT
#define   D_INPUT

void      dInputInit(void);
void      dInputSelect(UBYTE No);
void      dInputConvert(UBYTE No);
void      dInputDeselect(UBYTE No);
void      dInputExit(void);

#endif
