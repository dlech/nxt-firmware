//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dkandlun                                        $
//
// Revision date   $Date:: 28-12-04 14:19                                    $
//
// Filename        $Workfile:: c_armcomm.h                                   $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Peripheral/Firmware/Source/c_ar $
//
// Platform        C
//


#ifndef   C_ARMCOMM
#define   C_ARMCOMM

#define   NOS_OF_MOTORS                 4
#define   NOS_OF_SENSORS                4
#define   NOS_OF_BTNS                   5

typedef   struct
{
  UBYTE TimerTik;
  UBYTE MotorStatus[NOS_OF_MOTORS];
  UBYTE MotorSpeed[NOS_OF_MOTORS];
}InputMap;

typedef   struct
{
  SWORD SensorValue[NOS_OF_SENSORS];
  UBYTE ButtonState[NOS_OF_BTNS];
}OutputMap;

void      cArmCommInit(void);
UBYTE     cArmCommCtrl(void);
void      cArmCommExit(void);

#endif
