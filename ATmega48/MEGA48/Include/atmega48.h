//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 2-09-05 14:37                                     $
//
// Filename        $Workfile:: atmega48.h                                    $
//
// Version         $Revision:: 7                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Mega48/Include/ $
//
// Platform        C
//


#ifndef   ATMEGA88_H
#define   ATMEGA88_H

#include  "iom48.h"
#include  "inavr.h" 


#define   ATMEGAX8


#define   HARDWAREReset                 {\
                                          void (*Reset)(void);\
                                          Reset = (void*)0x0000;\
                                          Reset();\
                                        }

#define   HARDWAREInit                  {\
                                          SMCR    =  0x00;\
                                          CLKPR   =  0x80;\
                                          CLKPR   =  0x00;\
                                          __enable_interrupt();\
                                        }
                              

#define   HARDWAREExit                  {\
                                          ADCSRA  =  0x00;\
                                          SMCR    =  0x05;\
                                          __sleep();\
                                          HARDWAREReset;\
                                        } 

#define   OSIntEnable()                 {\
                                          __enable_interrupt();\
                                        }

#define   OSIntDisable()                {\
                                          __disable_interrupt();\
                                        }

#define   OSWatchdogWrite   

void      mSchedInit(void);
UBYTE     mSchedCtrl(void);
void      mSchedExit(void);


#endif
