//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 10-11-05 15:35                                    $
//
// Filename        $Workfile:: c_armcomm.c                                   $
//
// Version         $Revision:: 17                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/c_armcom $
//
// Platform        C
//

/* Event                                        Function                                  State
---------------------------------------         -------------------------------------     -----------------------------------

Voltage > 4,3V, wakeup or reset button          Brake motor drivers                       RESET
                                                                                          POWERUP  
                                                Batt measument off                        POWERUP_CHECK_FOR_RECHARGEABLE
                                                Wait 10 mS
                                                Rechargeable if switch > 0,5V       
                                                Batt measurement on                       POWERUP_CHECK_VOLTAGE_FOR_ARM_ON
                                                Wait 10 mS                               
                                                Check voltage for ARM on <= 11,8V           
                                                Batt measument off                        POWERUP_DISABLE_AMP
                                                Wait 10 mS                               
                                                Turn ARM on                               POWERUP_TURN_ARM_ON
                                                Brake off
                                                Wait 500 mS                               POWERUP_ENABLE_AMP
                                                Batt measurement on                       
                                                Check voltage for ARM on >= 6,5V (rechg)  POWERUP_CHECK_RECHARGEABLE_VOLTAGE   
Samba active                                    Reset copyright timer                     ON
                                                Read all inputs and update buttons        ON_RUNNING
                                                Check ARM communicating
                                                Check for high voltage/samba button
                                                Control led (Batt measurement on/off)
                                                Check for ARM samba request
                                                Check for ARM powerdown request
                                                Check for ARM copyright invalid


High voltage (batt > 12,2V or samba button)     Turn of input current drive               ON_HIGH_VOLTAGE
                                                Turn off ARM
                                                Brake output drivers
                                                Batt measurement off                      ON_CHECK_BUTTON
                                                Check samba button


Power down request or copyright invalid                                                   POWERDOWN
                                                Batt measurement off                      POWERDOWN_DISABLE_AMP
                                                Wait 10 mS
                                                Turn ARM off                              POWERDOWN_TURN_ARM_OFF
                                                Wait 1 sec
Rechargeable < 6,5V                                                                       OFF
                                                                                          SLEEP


Samba button (long press) or samba request                                                SAMBA
                                                Wait 100 mS                               SAMBA_ACTIVATE
                                                Batt measurement forced high              SAMBA_TURN_ARM_OFF_AND_WAIT
                                                Batt measurement off
                                                Turn ARM off
                                                Wait 1 sec
                                                Turn ARM on                               SAMBA_TURN_ARM_ON_AND_WAIT
                                                Wait 10 sec
                                                Turn ARM off                              SAMBA_TURN_ARM_OFF_FOR_RESET
                                                Remove batt measurement force
                                                Wait 1 sec
                                                Turn ARM on                               SAMBA_TURN_ARM_ON
                                                                                          ON


*/

#include  "stdconst.h"
#include  "c_armcomm.h"
#include  "d_power.h"
#include  "d_output.h"
#include  "d_input.h"
#include  "d_button.h"
#include  "d_armcomm.h"
#include  "d_timer.h"


#define   INPUTPOWER_ONTIME             3000    // [uS] time between input A/D samples
#define   COPYRIGHT_TIME                300000L // [mS] time to power down if no copy right string found

#define   POWERUP_ENABLE_MEASURE_TIME   10      // [mS] time to enable voltage divider for measurement
#define   POWERUP_DISABLE_MEASURE_TIME  10      // [mS] time to disable voltage divider for measurement
#define   POWERUP_DISABLE_AMP_TIME      10      // [mS] time after amp is disenabled
#define   POWERUP_ENABLE_AMP_TIME       100     // [mS] time before amp is enabled
#define   POWERUP_RECHARGE_TEST_TIME    1000    // [mS] time testing voltage if rechargeable (to show low batt on display)
#define   ON_ARM_TIMEOUT_TIME           2000    // [mS] time between ARM communication (max)
#define   LED_TOGGLE_TIME               500     // [mS] time between led toggles on and off
#define   CHECK_TEST_BUTTON_TIME        2000    // [mS] time for stable button reading (samba activate)
#define   BUTTON_ACCEPT_TIME            200     // [mS] time from samba accept to actual active
#define   SAMBA_POWEROFF_TIME           1000    // [mS] time for ARM power to drop
#define   SAMBA_BOOT_TIME               10000   // [mS] time for copying samba boot loader
#define   POWEROFF_TIME                 1000    // [mS] time from ARM off to sleep

#define   RECHARGEABLE_SWITCH_VOLTAGE   500L    // [mV] trigger point for rechageable battery detect switch
#define   ARM_POWERUP_MAX_VOLTAGE       11800L  // [mV] maximum allowable voltage when turning on ARM
#define   ARM_ON_MAX_VOLTAGE            12200L  // [mV] maximum allowable voltage when running ARM
#define   ARM_ON_OK_VOLTAGE             10000L  // [mV] maximum allowable voltage when turning on ARM (after high voltage)      
#define   ARM_ON_MIN_VOLTAGE            6500L   // [mV] minimum allowable voltage when turning on ARM (rechargeable)      





// Use compiler to calculate ticks from time
#define   INPUTPOWER_ONTICK             (UBYTE)(((ULONG)TIMER_RESOLUTION / (1000000L / (ULONG)INPUTPOWER_ONTIME)))
#define   COPYRIGHT_TICK                (ULONG)(((ULONG)COPYRIGHT_TIME * 1000L) / (ULONG)INPUTPOWER_ONTIME)
#define   POWERUP_ENABLE_MEASURE_TICK   (UWORD)(((ULONG)POWERUP_ENABLE_MEASURE_TIME * 1000L) / (ULONG)INPUTPOWER_ONTIME)
#define   POWERUP_DISABLE_MEASURE_TICK  (UWORD)(((ULONG)POWERUP_DISABLE_MEASURE_TIME * 1000L) / (ULONG)INPUTPOWER_ONTIME)
#define   POWERUP_DISABLE_AMP_TICK      (UWORD)(((ULONG)POWERUP_DISABLE_AMP_TIME * 1000L) / (ULONG)INPUTPOWER_ONTIME)
#define   POWERUP_ENABLE_AMP_TICK       (UWORD)(((ULONG)POWERUP_ENABLE_AMP_TIME * 1000L) / (ULONG)INPUTPOWER_ONTIME)
#define   POWERUP_RECHARGE_TEST_TICK    (UWORD)(((ULONG)POWERUP_RECHARGE_TEST_TIME * 1000L) / (ULONG)INPUTPOWER_ONTIME)
#define   ON_ARM_TIMEOUT_TICK           (UWORD)(((ULONG)ON_ARM_TIMEOUT_TIME * 1000L) / (ULONG)INPUTPOWER_ONTIME)
#define   LED_TOGGLE_TICK               (UWORD)(((ULONG)LED_TOGGLE_TIME * 1000L) / (ULONG)INPUTPOWER_ONTIME)
#define   CHECK_TEST_BUTTON_TICK        (UWORD)(((ULONG)CHECK_TEST_BUTTON_TIME * 1000L) / (ULONG)INPUTPOWER_ONTIME)
#define   SAMBA_POWEROFF_TICK           (UWORD)(((ULONG)SAMBA_POWEROFF_TIME * 1000L) / (ULONG)INPUTPOWER_ONTIME)
#define   SAMBA_BOOT_TICK               (UWORD)(((ULONG)SAMBA_BOOT_TIME * 1000L) / (ULONG)INPUTPOWER_ONTIME)
#define   BUTTON_ACCEPT_TICK            (UWORD)(((ULONG)BUTTON_ACCEPT_TIME * 1000L) / (ULONG)INPUTPOWER_ONTIME)
#define   POWEROFF_TICK                 (UWORD)(((ULONG)POWEROFF_TIME * 1000L) / (ULONG)INPUTPOWER_ONTIME)


// Use compiler to calculate counts from voltage
#define   ADC_REFERENCE                 5000L   // [mv]
#define   ADC_RESOLUTION                1023L   // [Count]
#define   RESISTOR_HIGH                 22000L  // [ohm]
#define   RESISTOR_LOW                  12000L  // [ohm]  
#define   RECHARGEABLE_SWITCH_COUNT     (UWORD)(((((RECHARGEABLE_SWITCH_VOLTAGE * RESISTOR_LOW) / (RESISTOR_LOW + RESISTOR_HIGH)) * ADC_RESOLUTION) / ADC_REFERENCE))
#define   ARM_POWERUP_MAX_COUNT         (UWORD)(((((ARM_POWERUP_MAX_VOLTAGE * RESISTOR_LOW) / (RESISTOR_LOW + RESISTOR_HIGH)) * ADC_RESOLUTION) / ADC_REFERENCE))
#define   ARM_ON_MAX_COUNT              (UWORD)(((((ARM_ON_MAX_VOLTAGE * RESISTOR_LOW) / (RESISTOR_LOW + RESISTOR_HIGH)) * ADC_RESOLUTION) / ADC_REFERENCE))
#define   ARM_ON_OK_COUNT               (UWORD)(((((ARM_ON_OK_VOLTAGE * RESISTOR_LOW) / (RESISTOR_LOW + RESISTOR_HIGH)) * ADC_RESOLUTION) / ADC_REFERENCE))
#define   ARM_ON_MIN_COUNT              (UWORD)(((((ARM_ON_MIN_VOLTAGE * RESISTOR_LOW) / (RESISTOR_LOW + RESISTOR_HIGH)) * ADC_RESOLUTION) / ADC_REFERENCE))

#define   TEST_BUTTON_VALUE             (ADC_RESOLUTION - 10)

// State machine states
enum      
{
  RESET,
  POWERUP,
    POWERUP_CHECK_FOR_RECHARGEABLE,
    POWERUP_CHECK_VOLTAGE_FOR_ARM_ON,
    POWERUP_DISABLE_AMP,
    POWERUP_TURN_ARM_ON,
    POWERUP_ENABLE_AMP,
    POWERUP_CHECK_RECHARGEABLE_VOLTAGE,
  ON,
    ON_RUNNING,
    ON_HIGH_VOLTAGE,
    ON_CHECK_BUTTON,
  SAMBA,
    SAMBA_ACTIVATE,
    SAMBA_TURN_ARM_OFF_AND_WAIT,
    SAMBA_TURN_ARM_ON_AND_WAIT,
    SAMBA_TURN_ARM_OFF_FOR_RESET,
    SAMBA_TURN_ARM_ON,
  POWERDOWN,
    POWERDOWN_DISABLE_AMP,
    POWERDOWN_TURN_ARM_OFF,
  OFF,
  SLEEP
};


UBYTE     State;
UBYTE     OldState;
UBYTE     OverwriteFloat;
UWORD     StateTimer;
UBYTE     Rechargeable;
UWORD     ArmTimer;
UBYTE     ArmFucked;
UBYTE     LedState;
UWORD     ButtonTimer;
ULONG     CopyRightTimer;


void      cArmCommInit(void)
{
  dPowerInit();
  dOutputInit();
  dInputInit();
  dButtonInit();
  dArmCommInit();
  dTimerInit();

  State     = RESET;
  OldState  = ~State;
}


UBYTE     cArmCommCtrl(void)
{
  UBYTE   Result = TRUE;

  // Update state machine if timeout (or RESET)
  if ((dTimerRead() >= INPUTPOWER_ONTICK) || (State == RESET))
  {
    dTimerClear();

    // Maintain StateTimer (clear if state changes else increament)
    if (State != OldState)
    {
      OldState   = State;
      StateTimer = 0;
    }
    else
    {
      StateTimer++;
    }

    // STATE MACHINE
    switch (State)
    {

      case RESET :
      {
        if (!StateTimer)
        {
          OverwriteFloat = TRUE;
          State          = POWERUP;
        }
      }
      break;

      case POWERUP :
      {
        State = POWERUP_CHECK_FOR_RECHARGEABLE;
      }
      break;

      case POWERUP_CHECK_FOR_RECHARGEABLE :
      {
        if (!StateTimer)
        {
          dPowerDeselect();
        }
        if (StateTimer >= POWERUP_DISABLE_MEASURE_TICK)
        {
          if (dPowerConvert() > RECHARGEABLE_SWITCH_COUNT)
          {
            Rechargeable = TRUE;
          }
          dPowerRechargeable(Rechargeable);
          State = POWERUP_CHECK_VOLTAGE_FOR_ARM_ON;
        }
      }
      break;

      case POWERUP_CHECK_VOLTAGE_FOR_ARM_ON :
      {
        if (!StateTimer)
        {
          dPowerSelect();
        }
        if (StateTimer >= POWERUP_ENABLE_MEASURE_TICK)
        {
          if (dPowerConvert() <= ARM_POWERUP_MAX_COUNT)
          {
            State = POWERUP_DISABLE_AMP;
          }
        }
      }
      break;

      case POWERUP_DISABLE_AMP :
      {
        if (!StateTimer)
        {
          dPowerDeselect();
        }
        if (StateTimer >= POWERUP_DISABLE_AMP_TICK)
        {
          State = POWERUP_TURN_ARM_ON;
        }
      }
      break;

      case POWERUP_TURN_ARM_ON :
      {
        dPowerWriteOn(TRUE);
        OverwriteFloat = FALSE;
        State = POWERUP_ENABLE_AMP;
      }
      break;

      case POWERUP_ENABLE_AMP :
      {
        if (StateTimer >= POWERUP_ENABLE_AMP_TICK)
        {
          dPowerSelect();
          State = POWERUP_CHECK_RECHARGEABLE_VOLTAGE;
        }
      }
      break;

      case POWERUP_CHECK_RECHARGEABLE_VOLTAGE :
      {
        if (Rechargeable == TRUE)
        {
          if (dPowerConvert() < ARM_ON_MIN_COUNT)
          {
            if (StateTimer >= POWERUP_RECHARGE_TEST_TICK)
            {
              State = OFF;
            }
          }
          else
          {
            State = ON;
          }
        }
        else
        {
          State = ON;
        }
      }
      break;

      case ON :
      {
        CopyRightTimer = 0L;
        State          = ON_RUNNING;
      }
      break;

      case ON_RUNNING :
      {

        // Read all inputs
        dInputSelect(0);
        dInputConvert(0);
        dInputConvert(0);
        dInputDeselect(0);

        dInputSelect(1);
        dInputConvert(1);
        dInputConvert(1);
        dInputDeselect(1);

        dInputSelect(2);
        dInputConvert(2);
        dInputConvert(2);
        dInputDeselect(2);

        dInputSelect(3);
        dInputConvert(3);
        dInputConvert(3);
        dInputDeselect(3);

        // Update buttons
        dButtonUpdate();

        // Check for ARM communication
        if (dArmCommCheck() == TRUE)
        {
          ArmTimer = 0;
          ArmFucked = FALSE;
        }

        if (ArmTimer >= ON_ARM_TIMEOUT_TICK)
        {
          ArmFucked = TRUE;
        }
        else
        {
          ArmTimer++;
        }

        // Check for high voltage
        dPowerSelect();
        if (dPowerConvert() > ARM_ON_MAX_COUNT)
        {
          State = ON_HIGH_VOLTAGE;
        }

        // Control led
        if (ArmFucked == TRUE)
        {
          if (StateTimer >= LED_TOGGLE_TICK)
          {
            StateTimer = 0;
            if (LedState == TRUE)
            {
              LedState = FALSE;
            }
            else
            {
              LedState = TRUE;
            }
          }
        }
        else
        {
          LedState = TRUE;
        }

        if (LedState == FALSE)
        {
          dPowerDeselect();
        }

        // Check for SAMBA request
        if (dPowerReadBoot() == TRUE)
        {
          State = SAMBA;
        }

        // Check for POWERDOWN request
        if (dPowerReadOn() == FALSE)
        {
          State = POWERDOWN;        
        }

        // Check for CopyRight valid
        if (dArmCommCopyRight() != TRUE)
        {
          if (++CopyRightTimer >= COPYRIGHT_TICK)
          {
            State = POWERDOWN;
          }
        }
      }
      break;

      case ON_HIGH_VOLTAGE :
      {
        dInputInit();
        dPowerWriteOn(FALSE);             
        OverwriteFloat = TRUE;
        ButtonTimer = CHECK_TEST_BUTTON_TICK;
        State = ON_CHECK_BUTTON;
      }
      break;

      case ON_CHECK_BUTTON :
      {
        dPowerSelect();
        if (ButtonTimer)
        {
          dPowerDeselect();
          if (dPowerConvert() >= TEST_BUTTON_VALUE)
          {
            ButtonTimer++;
            if (ButtonTimer > (CHECK_TEST_BUTTON_TICK * 2))
            {
              dPowerSelect();
              State = SAMBA;
            }
          }
          else
          {
            ButtonTimer--; 
          }
        }
        else
        {
          if (dPowerConvert() <= ARM_ON_OK_COUNT)
          {
            State = RESET;
          }
        }
      }
      break;

      case POWERDOWN :
      {
        State = POWERDOWN_DISABLE_AMP;
      }
      break;

      case POWERDOWN_DISABLE_AMP :
      {
        if (!StateTimer)
        {
          dPowerDeselect();
        }
        if (StateTimer >= POWERUP_DISABLE_AMP_TICK)
        {
          State = POWERDOWN_TURN_ARM_OFF;
        }
      }
      break;

      case POWERDOWN_TURN_ARM_OFF :
      {
        if (!StateTimer)
        {
          dPowerWriteOn(FALSE);
        }
        if (StateTimer >= POWEROFF_TICK)
        {
          State = OFF;
        }
      }
      break;

      case OFF :
      {
        State = SLEEP;
      }
      break;

      case SAMBA :
      {
        State = SAMBA_ACTIVATE;
      }
      break;

      case SAMBA_ACTIVATE :
      {
        if (++StateTimer >= BUTTON_ACCEPT_TICK)
        {
          State = SAMBA_TURN_ARM_OFF_AND_WAIT;
        }
      }
      break;

      case SAMBA_TURN_ARM_OFF_AND_WAIT :
      {
        if (!StateTimer)
        {
          dPowerHigh();
          dPowerDeselect();
          dPowerWriteOn(FALSE);
        }
        if (++StateTimer >= SAMBA_POWEROFF_TICK)
        {
          State = SAMBA_TURN_ARM_ON_AND_WAIT;
        }
      }
      break;

      case SAMBA_TURN_ARM_ON_AND_WAIT :
      {
        if (!StateTimer)
        {
          dPowerWriteOn(TRUE);                
        }
        if (++StateTimer >= SAMBA_BOOT_TICK)
        {
          State = SAMBA_TURN_ARM_OFF_FOR_RESET;
        }
      }
      break;

      case SAMBA_TURN_ARM_OFF_FOR_RESET :
      {
        if (!StateTimer)
        {
          dPowerWriteOn(FALSE);                
          dPowerFloat();
        }
        if (++StateTimer >= SAMBA_POWEROFF_TICK)
        {
          State = SAMBA_TURN_ARM_ON;
        }
      }
      break;

      case SAMBA_TURN_ARM_ON :
      {
        dPowerWriteOn(TRUE);                
        State          = ON;
      }
      break;

      case SLEEP :
      {
        Result = FALSE;
      }
      break;

    }
  }

  // Update allways output
  dOutputUpdate(OverwriteFloat);

  return(Result);
}


void      cArmCommExit(void)
{
  dTimerExit();
  dArmCommExit();
  dButtonExit();
  dInputExit();
  dOutputExit();
  dPowerExit();
}
