//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 2-09-05 14:37                                     $
//
// Filename        $Workfile:: d_timer.r                                     $
//
// Version         $Revision:: 4                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/d_timer. $
//
// Platform        C
//

#ifdef    ATMEGAX8

#define   TIMERInit                     {\
                                          TCCR2A  =  0x00;\
                                          TCCR2B  =  0x06;\
                                          TCNT2   =  0x00;\
                                        }

#define   TIMERRead                     TCNT2


#define   TIMERExit                     {\
                                        }


#endif
