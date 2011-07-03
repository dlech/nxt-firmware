
//
// Date init       14.12.2004
//
// Revision date   $Date:: 19-03-10 12:36                                    $
//
// Filename        $Workfile:: c_input.c                                     $
//
// Version         $Revision:: 40                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/c_inpu $
//
// Platform        C
//

#include  "stdconst.h"
#include  "modules.h"
#include  "c_input.h"
#include  "d_input.h"
#include  "c_output.iom"
#include  "c_loader.iom"
#include  <string.h>


#define   INVALID_RELOAD_NORMAL         20
#define   INVALID_RELOAD_SOUND          300
#define   INVALID_RELOAD_COLOR          400

#define   ROT_SLOW_SPEED                30
#define   ROT_OV_SAMPLING               7

#define   VCC_SENSOR                    5000L
#define   VCC_SENSOR_DIODE              4300L
#define   AD_MAX                        1023L

#define   REFLECTIONSENSORMIN           (1906L/(VCC_SENSOR/AD_MAX))
#define   REFLECTIONSENSORMAX           ((AD_MAX * 4398L)/VCC_SENSOR)
#define   REFLECTIONSENSORPCTDYN        (UBYTE)(((REFLECTIONSENSORMAX - REFLECTIONSENSORMIN) * 100L)/AD_MAX)

#define   NEWLIGHTSENSORMIN             (800L/(VCC_SENSOR/AD_MAX))
#define   NEWLIGHTSENSORMAX             ((AD_MAX * 4400L)/VCC_SENSOR)
#define   NEWLIGHTSENSORPCTDYN          (UBYTE)(((NEWLIGHTSENSORMAX - NEWLIGHTSENSORMIN) * 100L)/AD_MAX)

#define   NEWSOUNDSENSORMIN             (650L/(VCC_SENSOR/AD_MAX))
#define   NEWSOUNDSENSORMAX             ((AD_MAX * 4980L)/VCC_SENSOR)
#define   NEWSOUNDSENSORPCTDYN          (UBYTE)(((NEWSOUNDSENSORMAX - NEWSOUNDSENSORMIN) * 100L)/AD_MAX)

/* Remember this is ARM AD converter  - 3,3 VDC as max voltage      */
/* When in color mode background value is substracted => min = 0!!! */
#define   COLORSENSORBGMIN              (214/(3300/AD_MAX))
#define   COLORSENSORMIN                (1L/(3300/AD_MAX)) /* 1 inserted else div 0 (1L/(120/AD_MAX)) */
#define   COLORSENSORMAX                ((AD_MAX * 3300L)/3300)
#define   COLORSENSORPCTDYN             (UBYTE)(((COLORSENSORMAX - COLORSENSORMIN) * 100L)/AD_MAX)
#define   COLORSENSORBGPCTDYN           (UBYTE)(((COLORSENSORMAX - COLORSENSORBGMIN) * 100L)/AD_MAX)

enum
{
  POWER         = 0x00,
  NO_POWER      = 0x01,
  ACTIVE        = 0x02,
  ALWAYS_ACTIVE = 0x04,
  DIGI_0_HIGH   = 0x08,
  DIGI_1_HIGH   = 0x10,
  DIGI_0_IN     = 0x20,
  DIGI_1_IN     = 0x40,
  CUSTOM_SETUP  = 0x80
};

static    IOMAPINPUT   IOMapInput;
static    VARSINPUT    VarsInput;

const     HEADER       cInput =
{
  0x00030001L,
  "Input",
  cInputInit,
  cInputCtrl,
  cInputExit,
  (void *)&IOMapInput,
  (void *)&VarsInput,
  (UWORD)sizeof(IOMapInput),
  (UWORD)sizeof(VarsInput),
  0x0000                      //Code size - not used so far
};

void      cInputCalcFullScale(UWORD *pRawVal, UWORD ZeroPointOffset, UBYTE PctFullScale, UBYTE InvState);
void      cInputCalcSensorValue(UWORD NewSensorRaw, UWORD *pOldSensorRaw, SWORD *pSensorValue,
                                UBYTE *pBoolean,    UBYTE *pDebounce,     UBYTE *pSampleCnt,
                                UBYTE *LastAngle,   UBYTE *pEdgeCnt,      UBYTE Slope,
                                UBYTE Mode);
void      cInputSetupType(UBYTE Port, UBYTE newType, UBYTE OldType);
void      cInputSetupCustomSensor(UBYTE Port);
void      cInputCalcSensorValues(UBYTE No);
UBYTE     cInputInitColorSensor(UBYTE Port, UBYTE *pInitStatus);
void      cInputCalibrateColor(COLORSTRUCT *pC, UWORD *pNewVals);
SWORD     cInputTempConv(UWORD InputVal);

void      cInputInit(void* pHeader)
{
  UBYTE   Tmp;

  memset(IOMapInput.Colors, 0, sizeof(IOMapInput.Colors));
  memset(VarsInput.VarsColor, 0, sizeof(VarsInput.VarsColor));

  /* Init IO map */
  for (Tmp = 0; Tmp < NO_OF_INPUTS; Tmp++)
  {
    IOMapInput.Inputs[Tmp].SensorType         = NO_SENSOR;
    IOMapInput.Inputs[Tmp].SensorMode         = RAWMODE;
    IOMapInput.Inputs[Tmp].SensorRaw          = 0;
    IOMapInput.Inputs[Tmp].SensorValue        = 0;
    IOMapInput.Inputs[Tmp].SensorBoolean      = 0;
    IOMapInput.Inputs[Tmp].InvalidData        = INVALID_DATA;
    IOMapInput.Inputs[Tmp].DigiPinsDir        = 0;
    IOMapInput.Inputs[Tmp].DigiPinsOut        = 0;
    IOMapInput.Inputs[Tmp].CustomActiveStatus = CUSTOMINACTIVE;
    IOMapInput.Inputs[Tmp].CustomZeroOffset   = 0;
    IOMapInput.Inputs[Tmp].CustomPctFullScale = 0;
    dInputRead0(Tmp, &(IOMapInput.Inputs[Tmp].DigiPinsIn));
    dInputRead1(Tmp, &(IOMapInput.Inputs[Tmp].DigiPinsIn));

    VarsInput.EdgeCnt[Tmp]       = 0;
    VarsInput.InputDebounce[Tmp] = 0;
    VarsInput.LastAngle[Tmp]     = 0;
    VarsInput.SampleCnt[Tmp]     = 0;
    VarsInput.InvalidTimer[Tmp]  = INVALID_RELOAD_NORMAL;
    VarsInput.OldSensorType[Tmp] = NO_SENSOR;
  }

  VarsInput.ColorStatus = 0;
  VarsInput.ColorCnt    = 0;

  dInputInit();
}

void      cInputCtrl(void)
{
  UBYTE   Tmp;


  if (VarsInput.ColorStatus)
  {
    switch(VarsInput.ColorCnt)
    {
      case 0:
      {
        VarsInput.ColorCnt = 1;
        dInputSetColorClkInput();

      }
      break;
      case 1:
      {
        VarsInput.ColorCnt = 2;
      }
      break;
      case 2:
      {
        VarsInput.ColorCnt = 0;
        dInputGetAllColors(IOMapInput.Colors, VarsInput.ColorStatus);
      }
      break;
      default:
      {
        VarsInput.ColorCnt = 0;
      }
      break;
    }
  }

  for (Tmp = 0; Tmp < NO_OF_INPUTS; Tmp++)
  {
    UBYTE sType = IOMapInput.Inputs[Tmp].SensorType;
    UBYTE *pType = &IOMapInput.Inputs[Tmp].SensorType;
    UBYTE oldType = VarsInput.OldSensorType[Tmp];

    if (sType != oldType)
    {

      /* Clear all variables for this sensor */
      VarsInput.EdgeCnt[Tmp]       = 0;
      VarsInput.InputDebounce[Tmp] = 0;
      VarsInput.LastAngle[Tmp]     = 0;
      VarsInput.SampleCnt[Tmp]     = 0;
      VarsInput.ColorStatus       &= ~(0x01<<Tmp);
      memset(&(VarsInput.VarsColor[Tmp]),0 ,sizeof(VarsInput.VarsColor[Tmp]));

      VarsInput.InvalidTimer[Tmp] = INVALID_RELOAD_NORMAL;
      /* If old type is color sensor in color lamp mode then turn off leds */
      if ((sType == NO_SENSOR) && 
          (oldType == COLORRED  || oldType == COLORGREEN || 
           oldType == COLORBLUE || oldType == COLORFULL || 
           oldType == COLOREXIT))
      {
        VarsInput.InvalidTimer[Tmp] = INVALID_RELOAD_COLOR;
        IOMapInput.Inputs[Tmp].SensorType = COLOREXIT;
        sType = COLOREXIT;
      }
      /* Setup the pins for the new sensortype */
      cInputSetupType(Tmp, pType, oldType);
      sType = *pType;
      IOMapInput.Inputs[Tmp].InvalidData = INVALID_DATA;
      VarsInput.OldSensorType[Tmp]       = sType;
    }
    else
    {
      if (VarsInput.InvalidTimer[Tmp])
      {

        /* A type change has been carried out earlier - waiting for valid data   */
        /* The color sensor requires special startup sequence with communication */
        if ((sType == COLORFULL) || (sType == COLORRED)  ||
            (sType == COLORGREEN)|| (sType == COLORBLUE) ||
            (sType == COLOREXIT) || (sType == COLORNONE))
        {
          cInputCalcSensorValues(Tmp);
        }

        (VarsInput.InvalidTimer[Tmp])--;
        if (0 == VarsInput.InvalidTimer[Tmp])
        {

          /* Time elapsed - data are now valid */
          IOMapInput.Inputs[Tmp].InvalidData &= ~INVALID_DATA;
        }
      }
      else
      {

        /* The invalid bit could have been set by the VM due to Mode change    */
        /* but input module needs to be called once to update the values       */
        IOMapInput.Inputs[Tmp].InvalidData &= ~INVALID_DATA;
      }
    }

    if (!(INVALID_DATA & (IOMapInput.Inputs[Tmp].InvalidData)))
    {
      cInputCalcSensorValues(Tmp);
    }
  }
}


void      cInputCalcSensorValues(UBYTE No)
{
  UBYTE sType = IOMapInput.Inputs[No].SensorType;

  switch(sType)
  {
    case SWITCH:
    case TEMPERATURE:
    case REFLECTION:
    case ANGLE:
    case LIGHT_ACTIVE:
    case LIGHT_INACTIVE:
    case SOUND_DB:
    case SOUND_DBA:
    case CUSTOM:
    {
      UWORD InputVal;

      if (sType == CUSTOM) {
        /* Setup and read digital IO */
        cInputSetupCustomSensor(No);
        dInputRead0(No, &(IOMapInput.Inputs[No].DigiPinsIn));
        dInputRead1(No, &(IOMapInput.Inputs[No].DigiPinsIn));
      }

      dInputGetRawAd(&InputVal, No);
      IOMapInput.Inputs[No].ADRaw = InputVal;

      if (sType == REFLECTION)
      {
        cInputCalcFullScale(&InputVal, REFLECTIONSENSORMIN, REFLECTIONSENSORPCTDYN, TRUE);
      }
      else if (sType == TEMPERATURE)
      {
        if (InputVal < 290)
          InputVal = 290;
        else if (InputVal > 928)
          InputVal = 928;
        InputVal = cInputTempConv(InputVal - 290);
        InputVal = InputVal + 200;
        InputVal = (UWORD)(((SLONG)InputVal * (SLONG)1023)/(SLONG)900);
      }
      else if (sType == LIGHT_ACTIVE || sType == LIGHT_INACTIVE)
      {
        cInputCalcFullScale(&InputVal, NEWLIGHTSENSORMIN, NEWLIGHTSENSORPCTDYN, TRUE);
      }
      else if (sType == SOUND_DB || sType == SOUND_DBA)
      {
        cInputCalcFullScale(&InputVal, NEWSOUNDSENSORMIN, NEWSOUNDSENSORPCTDYN, TRUE);
      }
      else if (sType == CUSTOM)
      {
        cInputCalcFullScale(&InputVal, IOMapInput.Inputs[No].CustomZeroOffset, IOMapInput.Inputs[No].CustomPctFullScale, FALSE);
      }
      cInputCalcSensorValue(  InputVal,
                            &(IOMapInput.Inputs[No].SensorRaw),
                            &(IOMapInput.Inputs[No].SensorValue),
                            &(IOMapInput.Inputs[No].SensorBoolean),
                            &(VarsInput.InputDebounce[No]),
                            &(VarsInput.SampleCnt[No]),
                            &(VarsInput.LastAngle[No]),
                            &(VarsInput.EdgeCnt[No]),
                            ((IOMapInput.Inputs[No].SensorMode) & SLOPEMASK),
                            ((IOMapInput.Inputs[No].SensorMode) & MODEMASK));
    }
    break;

    /* Tripple case intended */
    case LOWSPEED:
    case LOWSPEED_9V:
    case HIGHSPEED:
    {
    }
    break;

    /* Four cases intended */
    case COLORRED:
    case COLORGREEN:
    case COLORBLUE:
    case COLORNONE:
    {

      UWORD InputVal;
      switch (IOMapInput.Colors[No].CalibrationState)
      {
        case SENSOROFF:
        {
          /* Check if sensor has been attached */
          if (dInputCheckColorStatus(No))
          {

            /* Sensor has been attached now get cal data */
            VarsInput.VarsColor[No].ColorInitState = 0;
            (IOMapInput.Colors[No].CalibrationState) = SENSORCAL;
          }
        }
        break;
        case SENSORCAL:
        {

          UBYTE Status;
          if (FALSE == cInputInitColorSensor(No, &Status))
          {

            /* Color sensor has been removed during calibration */
            (IOMapInput.Colors[No].CalibrationState) = SENSOROFF;
          }

          if (TRUE == Status)
          {

            /* Use clock to detect errors */
            dInputSetDirInDigi0(No);
            (IOMapInput.Colors[No].CalibrationState) = 0;
          }
        }
        break;
        default:
        {
          if (dInputGetColor(No, &(IOMapInput.Inputs[No].ADRaw)))
          {
            InputVal = IOMapInput.Inputs[No].ADRaw;
            cInputCalcFullScale(&InputVal, COLORSENSORBGMIN, COLORSENSORBGPCTDYN, FALSE);
            cInputCalcSensorValue(InputVal,
                                  &(IOMapInput.Inputs[No].SensorRaw),
                                  &(IOMapInput.Inputs[No].SensorValue),
                                  &(IOMapInput.Inputs[No].SensorBoolean),
                                  &(VarsInput.InputDebounce[No]),
                                  &(VarsInput.SampleCnt[No]),
                                  &(VarsInput.LastAngle[No]),
                                  &(VarsInput.EdgeCnt[No]),
                                  ((IOMapInput.Inputs[No].SensorMode) & SLOPEMASK),
                                  ((IOMapInput.Inputs[No].SensorMode) & MODEMASK));
          }
          else
          {
            IOMapInput.Colors[No].CalibrationState = SENSOROFF;
          }
        }
        break;
      }
    }
    break;
    case COLORFULL:
    {
      switch (IOMapInput.Colors[No].CalibrationState)
      {
        case SENSOROFF:
        {
          /* Check if sensor has been attached */
          if (dInputCheckColorStatus(No))
          {

            /* Sensor has been attached now get cal data */
            VarsInput.VarsColor[No].ColorInitState = 0;
            (IOMapInput.Colors[No].CalibrationState) = SENSORCAL;
          }
        }
        break;
        case SENSORCAL:
        {
          UBYTE Status;

          if (FALSE == cInputInitColorSensor(No, &Status))
          {

            /* Color sensor has been removed during calibration */
            (IOMapInput.Colors[No].CalibrationState) = SENSOROFF;
            VarsInput.ColorStatus &= ~(0x01<<No);
          }

          if (TRUE == Status)
          {

            /* Initialization finished with success recalc the values*/
            (IOMapInput.Colors[No].CalibrationState) = 0;

            /* Calculate Calibration factor */
            VarsInput.ColorStatus |= (0x01<<No);

          }
        }
        break;
        default:
        {

          /* calculate only when new ad values are ready */
          if (0 == VarsInput.ColorCnt)
          {

            UWORD NewSensorVals[NO_OF_COLORS];
            UBYTE ColorCount;

            COLORSTRUCT *pC;

            pC = &(IOMapInput.Colors[No]);

            /* Check if sensor is deteched */
            if (dInputCheckColorStatus(No))
            {

              /* Calibrate the raw ad values returns the SensorRaw */
              cInputCalibrateColor(pC, NewSensorVals);

              for(ColorCount = 0; ColorCount < BLANK; ColorCount++)
              {

                /* Calculate color sensor values */
                cInputCalcSensorValue(NewSensorVals[ColorCount],
                                      &(IOMapInput.Colors[No].SensorRaw[ColorCount]),
                                      &(IOMapInput.Colors[No].SensorValue[ColorCount]),
                                      &(IOMapInput.Colors[No].Boolean[ColorCount]),
                                      &(VarsInput.VarsColor[No].ColorInputDebounce[ColorCount]),
                                      &(VarsInput.VarsColor[No].ColorSampleCnt[ColorCount]),
                                      &(VarsInput.VarsColor[No].ColorLastAngle[ColorCount]),
                                      &(VarsInput.VarsColor[No].ColorEdgeCnt[ColorCount]),
                                      ((IOMapInput.Inputs[No].SensorMode) & SLOPEMASK),
                                      ((IOMapInput.Inputs[No].SensorMode) & MODEMASK));
              }

              /* Calculate background sensor values */
              cInputCalcSensorValue(NewSensorVals[BLANK],
                                    &(IOMapInput.Colors[No].SensorRaw[BLANK]),
                                    &(IOMapInput.Colors[No].SensorValue[BLANK]),
                                    &(IOMapInput.Colors[No].Boolean[BLANK]),
                                    &(VarsInput.VarsColor[No].ColorInputDebounce[BLANK]),
                                    &(VarsInput.VarsColor[No].ColorSampleCnt[BLANK]),
                                    &(VarsInput.VarsColor[No].ColorLastAngle[BLANK]),
                                    &(VarsInput.VarsColor[No].ColorEdgeCnt[BLANK]),
                                    ((IOMapInput.Inputs[No].SensorMode) & SLOPEMASK),
                                    ((IOMapInput.Inputs[No].SensorMode) & MODEMASK));

              /* Color Sensor values has been calculated -                */
              /* now calculate the color and put it in Sensor value       */
              if (((pC->SensorRaw[RED]) > (pC->SensorRaw[BLUE] )) &&
                  ((pC->SensorRaw[RED]) > (pC->SensorRaw[GREEN])))
              {

                /* If all 3 colors are less than 65 OR (Less that 110 and bg less than 40)*/
                if (((pC->SensorRaw[RED])   < 65) ||
                    (((pC->SensorRaw[BLANK]) < 40) && ((pC->SensorRaw[RED])  < 110)))
                {
                  IOMapInput.Inputs[No].SensorValue = BLACKCOLOR;
                }
                else
                {
                  if (((((pC->SensorRaw[BLUE]) >> 2)  + ((pC->SensorRaw[BLUE]) >> 3) + (pC->SensorRaw[BLUE])) < (pC->SensorRaw[GREEN])) &&
                      ((((pC->SensorRaw[GREEN]) << 1)) > (pC->SensorRaw[RED])))
                  {
                    IOMapInput.Inputs[No].SensorValue = YELLOWCOLOR;
                  }
                  else
                  {

                    if ((((pC->SensorRaw[GREEN]) << 1) - ((pC->SensorRaw[GREEN]) >> 2)) < (pC->SensorRaw[RED]))
                    {

                      IOMapInput.Inputs[No].SensorValue = REDCOLOR;
                    }
                    else
                    {

                      if ((((pC->SensorRaw[BLUE]) < 70) ||
                          ((pC->SensorRaw[GREEN]) < 70)) ||
                         (((pC->SensorRaw[BLANK]) < 140) && ((pC->SensorRaw[RED]) < 140)))
                      {
                        IOMapInput.Inputs[No].SensorValue = BLACKCOLOR;
                      }
                      else
                      {
                        IOMapInput.Inputs[No].SensorValue = WHITECOLOR;
                      }
                    }
                  }
                }
              }
              else
              {

                /* Red is not the dominant color */
                if ((pC->SensorRaw[GREEN]) > (pC->SensorRaw[BLUE]))
                {

                  /* Green is the dominant color */
                  /* If all 3 colors are less than 40 OR (Less that 70 and bg less than 20)*/
                  if (((pC->SensorRaw[GREEN])  < 40) ||
                      (((pC->SensorRaw[BLANK]) < 30) && ((pC->SensorRaw[GREEN])  < 70)))
                  {
                    IOMapInput.Inputs[No].SensorValue = BLACKCOLOR;
                  }
                  else
                  {
                    if ((((pC->SensorRaw[BLUE]) << 1)) < (pC->SensorRaw[RED]))
                    {
                      IOMapInput.Inputs[No].SensorValue = YELLOWCOLOR;
                    }
                    else
                    {
                      if ((((pC->SensorRaw[RED]) + ((pC->SensorRaw[RED])>>2)) < (pC->SensorRaw[GREEN])) ||
                          (((pC->SensorRaw[BLUE]) + ((pC->SensorRaw[BLUE])>>2)) < (pC->SensorRaw[GREEN])))
                      {
                        IOMapInput.Inputs[No].SensorValue = GREENCOLOR;
                      }
                      else
                      {
                        if ((((pC->SensorRaw[RED]) < 70) ||
                            ((pC->SensorRaw[BLUE]) < 70)) ||
                            (((pC->SensorRaw[BLANK]) < 140) && ((pC->SensorRaw[GREEN]) < 140)))
                        {
                          IOMapInput.Inputs[No].SensorValue = BLACKCOLOR;
                        }
                        else
                        {
                          IOMapInput.Inputs[No].SensorValue = WHITECOLOR;
                        }
                      }
                    }
                  }
                }
                else
                {

                  /* Blue is the most dominant color        */
                  /* Colors can be blue, white or black     */
                  /* If all 3 colors are less than 48 OR (Less that 85 and bg less than 25)*/
                  if (((pC->SensorRaw[BLUE])   < 48) ||
                      (((pC->SensorRaw[BLANK]) < 25) && ((pC->SensorRaw[BLUE])  < 85)))
                  {
                    IOMapInput.Inputs[No].SensorValue = BLACKCOLOR;
                  }
                  else
                  {
                    if ((((((pC->SensorRaw[RED]) * 48) >> 5) < (pC->SensorRaw[BLUE])) &&
                        ((((pC->SensorRaw[GREEN]) * 48) >> 5) < (pC->SensorRaw[BLUE])))
                        ||
                        (((((pC->SensorRaw[RED])   * 58) >> 5) < (pC->SensorRaw[BLUE])) ||
                         ((((pC->SensorRaw[GREEN]) * 58) >> 5) < (pC->SensorRaw[BLUE]))))
                    {
                      IOMapInput.Inputs[No].SensorValue = BLUECOLOR;
                    }
                    else
                    {

                      /* Color is white or Black */
                      if ((((pC->SensorRaw[RED])  < 60) ||
                          ((pC->SensorRaw[GREEN]) < 60)) ||
                         (((pC->SensorRaw[BLANK]) < 110) && ((pC->SensorRaw[BLUE]) < 120)))
                      {
                        IOMapInput.Inputs[No].SensorValue = BLACKCOLOR;
                      }
                      else
                      {
                        if ((((pC->SensorRaw[RED])  + ((pC->SensorRaw[RED])   >> 3)) < (pC->SensorRaw[BLUE])) ||
                            (((pC->SensorRaw[GREEN]) + ((pC->SensorRaw[GREEN]) >> 3)) < (pC->SensorRaw[BLUE])))
                        {
                          IOMapInput.Inputs[No].SensorValue = BLUECOLOR;
                        }
                        else
                        {
                          IOMapInput.Inputs[No].SensorValue = WHITECOLOR;
                        }
                      }
                    }
                  }
                }
              }
            }
            else
            {
              IOMapInput.Colors[No].CalibrationState = SENSOROFF;
              VarsInput.ColorStatus &= ~(0x01<<No);
            }
          }
          break;
        }
      }
    }
    break;
    case COLOREXIT:
    {
      UBYTE Status;

      VarsInput.ColorStatus &= ~(0x01<<No);
      if (FALSE == cInputInitColorSensor(No, &Status))
      {
        IOMapInput.Inputs[No].SensorType = NO_SENSOR;
      }

      if (TRUE == Status)
      {

        /* Initialization finished with success recalc the values*/
        (IOMapInput.Colors[No].CalibrationState) = 0;
        IOMapInput.Inputs[No].SensorType = NO_SENSOR;
        VarsInput.OldSensorType[No]      = NO_SENSOR;
      }
    }
    break;
    default:
    {
    }
    break;
  }
}


void      cInputCalcSensorValue(UWORD NewSensorRaw, UWORD *pOldSensorRaw, SWORD *pSensorValue,
                                UBYTE *pBoolean,    UBYTE *pDebounce,     UBYTE *pSampleCnt,
                                UBYTE *LastAngle,   UBYTE *pEdgeCnt,      UBYTE Slope,
                                UBYTE Mode)
{
  SWORD   Delta;
  UBYTE   PresentBoolean;
  UBYTE   Sample;

  if (0 == Slope)
  {

    /* This is absolute measure method */
    if (NewSensorRaw > THRESHOLD_FALSE)
    {
      PresentBoolean = FALSE;
    }
    else
    {
      if (NewSensorRaw < THRESHOLD_TRUE)
      {
        PresentBoolean = TRUE;
      }
    }
  }
  else
  {

    /* This is dynamic measure method */
    if (NewSensorRaw > (ACTUAL_AD_RES - Slope))
    {
      PresentBoolean = FALSE;
    }
    else
    {
      if (NewSensorRaw < Slope)
      {
        PresentBoolean = TRUE;
      }
      else
      {
        Delta = *pOldSensorRaw - NewSensorRaw;
        if (Delta < 0)
        {
          if (-Delta > Slope)
          {
            PresentBoolean = FALSE;
          }
        }
        else
        {
          if (Delta > Slope)
          {
            PresentBoolean = TRUE;
          }
        }
      }
    }
  }
  *pOldSensorRaw = NewSensorRaw;

  switch(Mode)
  {

    case RAWMODE:
    {
      *pSensorValue = NewSensorRaw;
    }
    break;

    case BOOLEANMODE:
    {
      *pSensorValue = PresentBoolean;
    }
    break;

    case TRANSITIONCNTMODE:
    {
      if ((*pDebounce) > 0)
      {
        (*pDebounce)--;
      }
      else
      {
        if (*pBoolean != PresentBoolean)
        {
          (*pDebounce) = DEBOUNCERELOAD;
          (*pSensorValue)++;
        }
      }
    }
    break;

    case PERIODCOUNTERMODE:
    {
      if ((*pDebounce) > 0)
      {
        (*pDebounce)--;
      }
      else
      {
        if (*pBoolean != PresentBoolean)
        {
          (*pDebounce) = DEBOUNCERELOAD;
          *pBoolean = PresentBoolean;
          if (++(*pEdgeCnt) > 1)
          {
            if (PresentBoolean == 0)
            {
              (*pEdgeCnt) = 0;
              (*pSensorValue)++;
            }
          }
        }
      }
    }
    break;

    case PCTFULLSCALEMODE:
    {

      /* Output is 0-100 pct */
     *pSensorValue   = ((NewSensorRaw) * 100)/SENSOR_RESOLUTION;
    }
    break;

    case FAHRENHEITMODE:
    {

      /* Fahrenheit mode goes from -40 to 158 degrees */
      *pSensorValue = (((ULONG)(NewSensorRaw) * 900L)/SENSOR_RESOLUTION) - 200;
      *pSensorValue =  ((180L * (ULONG)(*pSensorValue))/100L) + 320;
    }
    break;

    case CELSIUSMODE:
    {

      /* Celsius mode goes from -20 to 70 degrees */
      *pSensorValue   = (((ULONG)(NewSensorRaw * 900L)/SENSOR_RESOLUTION) - 200);
    }
    break;

    case ANGLESTEPSMODE:
    {
      *pBoolean = PresentBoolean;

      if (NewSensorRaw < ANGLELIMITA)
      {
        Sample = 0;
      }
      else
      {
        if (NewSensorRaw < ANGLELIMITB)
        {
          Sample = 1;
        }
        else
        {
          if (NewSensorRaw < ANGLELIMITC)
          {
            Sample = 2;
          }
          else
          {
            Sample = 3;
          }
        }
      }

      switch (*LastAngle)
      {
        case 0 :
        {
          if (Sample == 1)
          {
            if ((*pSampleCnt) >= ROT_SLOW_SPEED )
            {

              if (++(*pSampleCnt) >= (ROT_SLOW_SPEED + ROT_OV_SAMPLING))
              {
                (*pSensorValue)++;
                (*LastAngle) = Sample;
              }
            }
            else
            {
              (*pSensorValue)++;
              (*LastAngle) = Sample;
            }
          }
          if (Sample == 2)
          {
            (*pSensorValue)--;
            (*LastAngle) = Sample;
          }
          if (Sample == 0)
          {
            if ((*pSampleCnt) < ROT_SLOW_SPEED)
            {
              (*pSampleCnt)++;
            }
          }
        }
        break;
        case 1 :
        {
          if (Sample == 3)
          {
            (*pSensorValue)++;
            (*LastAngle) = Sample;
          }
          if (Sample == 0)
          {
            (*pSensorValue)--;
            (*LastAngle) = Sample;
          }
          (*pSampleCnt) = 0;
        }
        break;
        case 2 :
        {
          if (Sample == 0)
          {
            (*pSensorValue)++;
            (*LastAngle) = Sample;
          }
          if (Sample == 3)
          {
            (*pSensorValue)--;
            (*LastAngle) = Sample;
          }
          (*pSampleCnt) = 0;
        }
        break;
        case 3 :
        {
          if (Sample == 2)
          {
            if ((*pSampleCnt) >= ROT_SLOW_SPEED)
            {

              if (++(*pSampleCnt) >= (ROT_SLOW_SPEED + ROT_OV_SAMPLING))
              {
                (*pSensorValue)++;
                (*LastAngle) = Sample;
              }
            }
            else
            {
              (*pSensorValue)++;
              (*LastAngle) = Sample;
            }
          }
          if (Sample == 1)
          {
            (*pSensorValue)--;
             (*LastAngle) = Sample;
          }
          if (Sample == 3)
          {
            if ((*pSampleCnt) < ROT_SLOW_SPEED)
            {
              (*pSampleCnt)++;
            }
          }
        }
        break;
      }
    }
  }

  *pBoolean  = PresentBoolean;
}

void      cInputCalcFullScale(UWORD *pRawVal, UWORD ZeroPointOffset, UBYTE PctFullScale, UBYTE InvStatus)
{
  if (*pRawVal >= ZeroPointOffset)
  {
    *pRawVal -= ZeroPointOffset;
  }
  else
  {
    *pRawVal = 0;
  }

  *pRawVal = (*pRawVal * 100)/PctFullScale;
  if (*pRawVal > SENSOR_RESOLUTION)
  {
    *pRawVal = SENSOR_RESOLUTION;
  }
  if (TRUE == InvStatus)
  {
    *pRawVal = SENSOR_RESOLUTION - *pRawVal;
  }
}


void      cInputSetupType(UBYTE Port, UBYTE newType, UBYTE OldType)
{

  switch(newType)
  {
    case NO_SENSOR:
    case SWITCH:
    case TEMPERATURE:
    {
      dInputSetInactive(Port);
      dInputSetDirInDigi0(Port);
      dInputSetDirInDigi1(Port);
    }
    break;

    case REFLECTION:
    case ANGLE:
    {
      dInputSetActive(Port);
      dInputClearDigi0(Port);
      dInputClearDigi1(Port);
    }
    break;

    case LIGHT_ACTIVE:
    {
      dInputSetInactive(Port);
      dInputSetDigi0(Port);
      dInputClearDigi1(Port);
    }
    break;

    case LIGHT_INACTIVE:
    {
      dInputSetInactive(Port);
      dInputClearDigi0(Port);
      dInputClearDigi1(Port);
    }
    break;

    case SOUND_DB:
    {
      VarsInput.InvalidTimer[Port] = INVALID_RELOAD_SOUND;
      dInputSetInactive(Port);
      dInputSetDigi0(Port);
      dInputClearDigi1(Port);
    }
    break;

    case SOUND_DBA:
    {
      VarsInput.InvalidTimer[Port] = INVALID_RELOAD_SOUND;
      dInputSetInactive(Port);
      dInputClearDigi0(Port);
      dInputSetDigi1(Port);
    }
    break;

    case CUSTOM:
    {
      cInputSetupCustomSensor(Port);
    }
    break;

    case LOWSPEED:
    {
      dInputSetInactive(Port);
      dInputSetDigi0(Port);
      dInputSetDigi1(Port);
    }
    break;

    case LOWSPEED_9V:
    {
      dInputSet9v(Port);
      dInputSetDigi0(Port);
      dInputSetDigi1(Port);
    }
    break;

    case HIGHSPEED:
    {
      dInputSetInactive(Port);
      dInputSetDirInDigi0(Port);
      dInputSetDirInDigi1(Port);
    }
    break;

    case COLORFULL:
    case COLORRED:
    case COLORGREEN:
    case COLORBLUE:
    case COLORNONE:
    {
      VarsInput.InvalidTimer[Port] = INVALID_RELOAD_COLOR;
      dInputSetInactive(Port);
      dInputSetDigi0(Port);
      dInputSetDirInDigi1(Port);
      IOMapInput.Colors[Port].CalibrationState = SENSORCAL;
      VarsInput.VarsColor[Port].ColorInitState = 0;

      IOMapInput.Inputs[Port].SensorValue = BLACKCOLOR;
    }
    break;

    default:
    {
    }
    break;
  }
}

void      cInputSetupCustomSensor(UBYTE Port)
{
  if ((IOMapInput.Inputs[Port].DigiPinsDir) & 0x01)
  {
    if ((IOMapInput.Inputs[Port].DigiPinsOut) & 0x01)
    {
      dInputSetDigi0(Port);
    }
    else
    {
      dInputClearDigi0(Port);
    }
  }
  else
  {
    dInputSetDirInDigi0(Port);
  }
  if ((IOMapInput.Inputs[Port].DigiPinsDir) & 0x02)
  {
    if ((IOMapInput.Inputs[Port].DigiPinsOut) & 0x02)
    {
      dInputSetDigi1(Port);
    }
    else
    {
      dInputClearDigi1(Port);
    }
  }
  else
  {
    dInputSetDirInDigi1(Port);
  }

  if (CUSTOMACTIVE == (IOMapInput.Inputs[Port].CustomActiveStatus))
  {
    dInputSetActive(Port);
  }
  else
  {
    if (CUSTOM9V == (IOMapInput.Inputs[Port].CustomActiveStatus))
    {
      dInputSet9v(Port);
    }
    else
    {
      dInputSetInactive(Port);
    }
  }
}


SWORD     cInputTempConv(UWORD InputVal)
{
  static const long long TempCoeff[] = { -5425ll, 9261399ll, -6686663252ll,
    2573629857807ll, -822478326197838ll, 195856762719738784ll };
  const unsigned int TempCoeffShift = 48;
  /* Replace the original table with polynomial. */
  int i;
  long long Input = InputVal;
  long long Output = TempCoeff[0];
  for (i = 1; i < sizeof TempCoeff / sizeof TempCoeff[0]; i++)
    Output = Output * Input + TempCoeff[i];
  /* Round. */
  return Output + (1ll << TempCoeffShift - 1) >> TempCoeffShift;
}


UBYTE      cInputInitColorSensor(UBYTE Port, UBYTE *pInitStatus)
{

  *pInitStatus = FALSE;
  switch(VarsInput.VarsColor[Port].ColorInitState)
  {
    case 0:
    {
      dInputSetDigi0(Port);
      dInputSetDigi1(Port);
      VarsInput.VarsColor[Port].ColorInitState++;
    }
    break;
    case 1:
    {
      dInputClearDigi0(Port);
      VarsInput.VarsColor[Port].ColorInitState++;
    }
    break;

    case 2:
    {
      dInputSetDigi0(Port);
      VarsInput.VarsColor[Port].ColorInitState++;
    }
    break;
    case 3:
    {

      dInputClearDigi0(Port);

      /* Clear clock for 100mS - use pit timer*/
      dInputClearColor100msTimer(Port);
      VarsInput.VarsColor[Port].ColorInitState++;
    }
    break;
    case 4:
    {

      /* Wait 100mS            */
      if (dInputChkColor100msTimer(Port))
      {
        VarsInput.VarsColor[Port].ColorInitState += 1;
      }
    }
    break;
    case 5:
    {
      UBYTE TmpType;

      if (COLOREXIT == IOMapInput.Inputs[Port].SensorType)
      {
        TmpType = COLORNONE;
      }
      else
      {
        TmpType = IOMapInput.Inputs[Port].SensorType;
      }
      dInputColorTx(Port, TmpType);

      /* Be ready to receive data from sensor */
      dInputSetDirInDigi1(Port);
      VarsInput.VarsColor[Port].ReadCnt = 0;
      VarsInput.VarsColor[Port].ColorInitState++;
    }
    break;
    case 6:
    {
      UBYTE Data;
      UBYTE DataCnt;
      UBYTE *pData;

      DataCnt = (VarsInput.VarsColor[Port].ReadCnt);
      pData   = (UBYTE*)(IOMapInput.Colors[Port].Calibration);

      /* Read first byte of cal data */
      dInputReadCal(Port, &Data);

      pData[DataCnt] = Data;

      /* If all bytes has been read - then continue to next step */
      if (++(VarsInput.VarsColor[Port].ReadCnt) >= ((sizeof(IOMapInput.Colors[Port].Calibration) + sizeof(IOMapInput.Colors[Port].CalLimits))))
      {
        VarsInput.VarsColor[Port].ColorInitState++;
      }
    }
    break;
    case 7:
    {

      /* Check CRC then continue or restart if false */
      UWORD Crc, CrcCheck;
      UBYTE Cnt;
      UBYTE Data;
      UBYTE *pData;

      dInputReadCal(Port, &Data);
      Crc  = (UWORD)(Data) << 8;
      dInputReadCal(Port, &Data);
      Crc += (UWORD)Data;
      CrcCheck = 0x5AA5;
      pData = (UBYTE*)(IOMapInput.Colors[Port].Calibration);
      for (Cnt = 0; Cnt < (sizeof(IOMapInput.Colors[Port].Calibration) + sizeof(IOMapInput.Colors[Port].CalLimits)); Cnt++)
      {
        UWORD i,j;
        UBYTE c;
        c = pData[Cnt];
        for(i = 0; i != 8; c >>= 1, i++)
        {
          j = (c^CrcCheck) & 1;
          CrcCheck >>= 1;

          if(j)
          {
            CrcCheck ^= 0xA001;
          }
        }

      }
      if ((CrcCheck != Crc))
      {

        /* incorrect!!! try again */
        VarsInput.VarsColor[Port].ColorInitState = 0;
        VarsInput.InvalidTimer[Port] = INVALID_RELOAD_COLOR;
      }
      else
      {

        /* Correct crc sum -> calculate the calibration values then exit */
        VarsInput.VarsColor[Port].ColorInitState = 0;

        /* Sensor is almost ready - needs a little time to make first measurements */
        VarsInput.InvalidTimer[Port] = 10;
        *pInitStatus = TRUE;
      }
    }
    break;
    default:
    {
      VarsInput.VarsColor[Port].ColorInitState = 0;
    }
    break;
  }
  return(dInputCheckColorStatus(Port));
}


void      cInputCalibrateColor(COLORSTRUCT *pC, UWORD *pNewVals)
{
  UBYTE CalRange;

  if ((pC->ADRaw[BLANK]) < pC->CalLimits[1])
  {
    CalRange = 2;
  }
  else
  {
    if ((pC->ADRaw[BLANK]) < pC->CalLimits[0])
    {
      CalRange = 1;
    }
    else
    {
      CalRange = 0;
    }
  }

  pNewVals[RED] = 0;
  if ((pC->ADRaw[RED]) > (pC->ADRaw[BLANK]))
  {
    pNewVals[RED] = (UWORD)(((ULONG)((pC->ADRaw[RED]) - (pC->ADRaw[BLANK])) * (pC->Calibration[CalRange][RED])) >> 16);
  }

  pNewVals[GREEN] = 0;
  if ((pC->ADRaw[GREEN]) > (pC->ADRaw[BLANK]))
  {
     pNewVals[GREEN] = (UWORD)(((ULONG)((pC->ADRaw[GREEN]) - (pC->ADRaw[BLANK])) * (pC->Calibration[CalRange][GREEN])) >> 16);
  }

  pNewVals[BLUE] = 0;
  if ((pC->ADRaw[BLUE]) > (pC->ADRaw[BLANK]))
  {
    pNewVals[BLUE] = (UWORD)(((ULONG)((pC->ADRaw[BLUE]) -(pC->ADRaw[BLANK])) * (pC->Calibration[CalRange][BLUE])) >> 16);
  }

  pNewVals[BLANK] = (pC->ADRaw[BLANK]);
  cInputCalcFullScale(&(pNewVals[BLANK]), COLORSENSORBGMIN, COLORSENSORBGPCTDYN, FALSE);
  (pNewVals[BLANK]) = (UWORD)(((ULONG)(pNewVals[BLANK]) * (pC->Calibration[CalRange][BLANK])) >> 16);
}


void      cInputExit(void)
{
  dInputExit();
}

