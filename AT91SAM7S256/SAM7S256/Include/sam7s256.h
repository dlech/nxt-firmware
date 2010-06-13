//
// Date init       14.12.2004
//
// Revision date   $Date:: 24-04-08 14:33                                    $
//
// Filename        $Workfile:: sam7s256.h                                    $
//
// Version         $Revision:: 5                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Sam7s256/Incl $
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
                                          ADSetup;    /* ADC used in several modules */\
                                        }


#define   HARDWAREExit

#define   OSIntEnable()
#define   OSIntDisable()

#define   OSWatchdogWrite

#define   ADCCLOCK                      (5000000L)  /* 5MHz */
#define   ADCPRESCALER                  (((OSC + ((ADCCLOCK*2)-1))/(ADCCLOCK*2)) - 1)

#define   ADCSTARTUPTIME                20          /*  uS  */
#define   ADCSTARTUP                    ((((20 * (ADCCLOCK/1000)) + 7999)/8000L) - 1)

#define   SAMPLEHOLDTIME                600         /*  nS  */
#define   SHTIM                         ((((SAMPLEHOLDTIME * (ADCCLOCK/1000)) + 999999)/1000000L)-1)

#define   ADSetup                       *AT91C_ADC_MR = (((ULONG)ADCPRESCALER <<  8) | \
                                                         ((ULONG)ADCSTARTUP   << 16) | \
                                                         ((ULONG)SHTIM        << 24))
#define   ADStart                       *AT91C_ADC_CR = AT91C_ADC_START

void      mSchedReset                   (void);
void      mSchedInit                    (void);
UBYTE     mSchedCtrl                    (void);
void      mSchedExit                    (void);

#endif
