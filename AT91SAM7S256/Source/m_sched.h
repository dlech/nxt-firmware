//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: m_sched.h                                     $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/m_sche $
//
// Platform        C
//



#define   APPNAME                       "LMS01"

#define   COPYRIGHTSTRING               "Let's samba nxt arm in arm, (c)LEGO System A/S"

#define   COPYRIGHTSTRINGLENGTH         46    /* Number of bytes checked in COPYRIGHTSTRING */


#ifndef   _WINDOWS

#define   SAM7SXX

#ifdef    SAM7SXX

 //
 // Platform ATMEL ARM7
 //
 //

#define   OSC                           48054850L
#define   SYSFREQ                       1000


#include  "..\SAM7S256\include\sam7s256.h"

#if       defined (PROTOTYPE_PCB_3) || (PROTOTYPE_PCB_4)

#define   TSTPin                        AT91C_PIO_PA27

#else

#define   TSTPin                        AT91C_PIO_PA31

#endif

#define	  TSTInit                       {\
                                          *AT91C_PIOA_PER  = TSTPin;\
                                          *AT91C_PIOA_OER  = TSTPin;\
                                        }

#define   TSTOn                         {\
                                          *AT91C_PIOA_SODR = TSTPin;\
                                        }

#define   TSTOff                        {\
                                          *AT91C_PIOA_CODR = TSTPin;\
                                        }

#define	  TSTExit                       {\
                                          *AT91C_PIOA_ODR  = TSTPin;\
                                          *AT91C_PIOA_CODR = TSTPin;\
                                        }

/* Defines related to loader */
#define   MAX_HANDLES                   16


/* Defines related to I2c   */
#define   BYTES_TO_TX                   8
#define   BYTES_TO_RX                   12

enum
{
  NOS_OF_AVR_OUTPUTS  = 4,
  NOS_OF_AVR_BTNS     = 4,
  NOS_OF_AVR_INPUTS   = 4
};

typedef   struct
{
  UWORD   AdValue[NOS_OF_AVR_INPUTS];
  UWORD   Buttons;
  UWORD   Battery;
}IOFROMAVR;

typedef   struct
{
  UBYTE   Power;
  UBYTE   PwmFreq;
  SBYTE   PwmValue[NOS_OF_AVR_OUTPUTS];
  UBYTE   OutputMode;
  UBYTE   InputPower;
}IOTOAVR;

extern    IOTOAVR IoToAvr;
extern    IOFROMAVR IoFromAvr;

#ifdef    INCLUDE_OS

#include  "..\SAM7S256\include\sam7s256.c"

IOTOAVR   IoToAvr;
IOFROMAVR IoFromAvr;

#endif

#endif

#else

 //
 // Platform PCWIN
 //
 //

#define   OSC                           1192000L
#define   SYSFREQ                       1000

#include  "Pcwin.h"

#ifdef    INCLUDE_OS

#include  "Pcwin.c"

#endif

#endif



