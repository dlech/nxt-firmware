//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 28-10-05 13:46                                    $
//
// Filename        $Workfile:: m_sched.h                                     $
//
// Version         $Revision:: 15                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/m_sched. $
//
// Platform        C
//


#define   COPYRIGHTSTRING                   "Let's samba nxt arm in arm, (c)LEGO System A/S"

#define   COPYRIGHTSTRINGLENGTH   46        // Number of bytes checked in COPYRIGHTSTRING

#define   OSC                     8000000L  // Main oscillator frequency

#include  "..\mega48\include\atmega48.h"

#define   BYTES_TO_TX             8         // Number of bytes received from ARM  = sizeof(IOTOAVR)
#define   BYTES_TO_RX             12        // Number of bytes transmitted to ARM = sizeof(IOFROMAVR)
#define   NOS_OF_AVR_OUTPUTS      4         // Number of motor outputs
#define   NOS_OF_AVR_INPUTS       4         // Number of a/d inputs


typedef   struct                            // From AVR to ARM
{
  UWORD   AdValue[NOS_OF_AVR_INPUTS];       // Raw a/d converter values [0..1023]
  UWORD   Buttons;                          // Raw a/d converter value  [0..1023] (Enter -> +0x07FF)
  UWORD   Battery;                          // Raw a/d converter value  [0..1023] (rechargeable -> +0x8000)
}IOFROMAVR;


typedef   struct                            // From ARM to AVR
{
  UBYTE   Power;                            // Command descriped below
  UBYTE   PwmFreq;                          // Common pwm freq [Khz]    [1..32]
  SBYTE   PwmValue[NOS_OF_AVR_OUTPUTS];     // Pwm value [%]            [-100..100]
  UBYTE   OutputMode;                       // Bitwise Bit 0 = Motor A  [0 = float, 1 = brake]
  UBYTE   InputPower;                       // Bitwise Bit 0 and 4 = input 1 [00 = inactive,01 = pulsed, 11 = constant]
}IOTOAVR;

/*
  Powerdown request:    Power = 0x5A
  Samba boot request:   Power = 0xA5 and PwmFreq = 0x5A
  Copyright string:     Power = 0xCC
*/


#ifdef    INCLUDE_OS

#include  "..\mega48\include\atmega48.c"

IOFROMAVR IoFromAvr =
{
  { 0,0,0,0 },
    0,
    0
};

IOTOAVR   IoToAvr =
{
    0,
    4,
  { 0,0,0,0 },
    0x0F,0x0F
};

#endif

extern    IOTOAVR   IoToAvr;
extern    IOFROMAVR IoFromAvr;
extern    UBYTE     Run;






