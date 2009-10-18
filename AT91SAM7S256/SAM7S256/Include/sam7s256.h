//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 10:08                                    $
//
// Filename        $Workfile:: sam7s256.h                                    $
//
// Version         $Revision:: 7                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Sam7s256/Include/ $
//
// Platform        C
//

#ifndef   SAM7S256_H
#define   SAM7S256_H

#include  "ioat91sam7s256.h"

#define   SAM7S256

#define   HARDWAREInit                  {\
                                          ULONG TmpReset;\
                                          *AT91C_RSTC_RMR  = 0xA5000401;\
                                          *AT91C_AIC_DCR   = 1;\
                                          *AT91C_PITC_PIMR = (0x000FFFFF | 0x01000000);\
                                          TmpReset         = *AT91C_PITC_PIVR;\
                                          TmpReset         = TmpReset;/* Suppress warning*/\
                                          *AT91C_PMC_PCER   = (1L<<AT91C_ID_PIOA);\
                                        }


#define   HARDWAREExit

#define   OSIntEnable()
#define   OSIntDisable()

#define   OSWatchdogWrite

void      mSchedReset                   (void);
void      mSchedInit                    (void);
UBYTE     mSchedCtrl                    (void);
void      mSchedExit                    (void);

#endif
