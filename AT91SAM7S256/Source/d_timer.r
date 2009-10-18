//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 10:18                                    $
//
// Filename        $Workfile:: d_timer.r                                     $
//
// Version         $Revision:: 11                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_timer.r  $
//
// Platform        C
//


#ifdef    SAM7S256


#define   MS_1_TIME         ((OSC/16)/1000)

static    ULONG TimerValue;
static    ULONG Timer1mS;

/* PIT timer is used as main timer - timer interval is 1mS */

#define   TIMERInit                     TimerValue = ((*AT91C_PITC_PIIR) & AT91C_PITC_CPIV);\
                                        Timer1mS   = 0

#define   TIMERRead(V)                  if (MS_1_TIME < ((((*AT91C_PITC_PIIR) & AT91C_PITC_CPIV) - TimerValue) & AT91C_PITC_CPIV))\
                                        {\
                                          TimerValue += MS_1_TIME;\
                                          TimerValue &= AT91C_PITC_CPIV;\
                                          Timer1mS++;\
                                        }\
                                        V = Timer1mS

#define   TIMERExit



#endif //SAM7S256



#ifdef    _WINDOWS

#include <windows.h>
#include <mmsystem.h>

#define TIMERInit     timeBeginPeriod(1);

#define TIMERRead(V)  (V) = timeGetTime();

#define TIMERExit     timeEndPeriod(1);

#endif //_WINDOWS
