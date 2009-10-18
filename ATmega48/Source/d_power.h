//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 2-09-05 14:37                                     $
//
// Filename        $Workfile:: d_power.h                                     $
//
// Version         $Revision:: 4                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/d_power. $
//
// Platform        C
//


#ifndef   D_POWER
#define   D_POWER

void      dPowerInit(void);
void      dPowerRechargeable(UBYTE Mounted);
UBYTE     dPowerReadOn(void);
UBYTE     dPowerReadBoot(void);
void      dPowerWriteOn(UBYTE On);
void      dPowerSelect(void);
UWORD     dPowerConvert(void);
void      dPowerDeselect(void);
void      dPowerHigh(void);
void      dPowerFloat(void);
void      dPowerExit(void);

#endif
