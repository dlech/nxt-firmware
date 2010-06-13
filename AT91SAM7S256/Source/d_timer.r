//
// Date init       14.12.2004
//
// Revision date   $Date:: 23-04-08 11:15                                    $
//
// Filename        $Workfile:: d_timer.r                                     $
//
// Version         $Revision:: 2                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_time $
//
// Platform        C
//


#ifdef    SAM7S256


#define   MS_1_TIME         ((OSC/16)/1000)

static    ULONG TimerValue;
static    ULONG NextTimerValue;
static    ULONG Timer1mS;

/* PIT timer is used as main timer - timer interval is 1mS */

#define   TIMERInit                     TimerValue = ((*AT91C_PITC_PIIR) & AT91C_PITC_CPIV);\
                                        NextTimerValue = (((*AT91C_PITC_PIIR) + MS_1_TIME) & AT91C_PITC_CPIV);\
                                        Timer1mS   = 0

#define   TIMERRead(V)                  if (MS_1_TIME < ((((*AT91C_PITC_PIIR) & AT91C_PITC_CPIV) - TimerValue) & AT91C_PITC_CPIV))\
                                        {\
                                          TimerValue += MS_1_TIME;\
                                          TimerValue &= AT91C_PITC_CPIV;\
                                          Timer1mS++;\
                                        }\
                                        V = Timer1mS

#define   TIMERReadAlt(V)               if((SLONG)((*AT91C_PITC_PIIR) - NextTimerValue) >= 0)\
                                        {\
                                          Timer1mS ++;\
                                          NextTimerValue += MS_1_TIME;\
                                        }\
                                        V = Timer1mS;\

#define   TIMERReadSkip(V)               diff= (((*AT91C_PITC_PIIR)) - NextTimerValue);\
                                        if (diff >= 0)\
                                        {\
                                          diff /= MS_1_TIME;\
                                          diff += 1;\
                                          Timer1mS += diff;\
                                          diff *= MS_1_TIME;\
                                          NextTimerValue += diff;\
                                        }\
                                        V = Timer1mS;\

#define   TIMERExit



#endif //SAM7S256



#ifdef    _WINDOWS

#include <windows.h>
#include <mmsystem.h>

#define TIMERInit     timeBeginPeriod(1);

#define TIMERRead(V)  (V) = timeGetTime();

#define TIMERExit     timeEndPeriod(1);

#endif //_WINDOWS
