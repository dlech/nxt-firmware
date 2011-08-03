
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
#include  "c_cmd.iom"
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


const     SWORD TempConvTable[] =
{
/* 1500,  1460,  1430,  1400,  1380,  1360,  1330,  1310,  1290,  1270,  1250,  1230,  1220,  1200,  1190,  1170,
   1160,  1150,  1140,  1130,  1110,  1100,  1090,  1080,  1070,  1060,  1050,  1040,  1030,  1020,  1010,  1000,
    994,   988,   982,   974,   968,   960,   954,   946,   940,   932,   926,   918,   912,   906,   900,   894,
    890,   884,   878,   874,   868,   864,   858,   854,   848,   844,   838,   832,   828,   822,   816,   812,
    808,   802,   798,   794,   790,   786,   782,   780,   776,   772,   768,   764,   762,   758,   754,   750,
    748,   744,   740,   736,   732,   730,   726,   722,   718,   716,   712,   708,   704,*/ 700,   696,   694,
    690,   688,   684,   682,   678,   674,   672,   668,   666,   662,   660,   656,   654,   650,   648,   644,
    642,   640,   638,   634,   632,   630,   628,   624,   622,   620,   616,   614,   612,   610,   608,   604,
    602,   600,   598,   596,   592,   590,   588,   586,   584,   582,   580,   578,   576,   574,   572,   570,
    568,   564,   562,   560,   558,   556,   554,   552,   550,   548,   546,   544,   542,   540,   538,   536,
    534,   532,   530,   528,   526,   524,   522,   520,   518,   516,   514,   512,   510,   508,   508,   506,
    504,   502,   500,   498,   496,   494,   494,   492,   490,   488,   486,   486,   484,   482,   480,   478,
    476,   476,   474,   472,   470,   468,   468,   466,   464,   462,   460,   458,   458,   456,   454,   452,
    450,   448,   448,   446,   444,   442,   442,   440,   438,   436,   436,   434,   432,   432,   430,   428,
    426,   426,   424,   422,   420,   420,   418,   416,   416,   414,   412,   410,   408,   408,   406,   404,
    404,   402,   400,   398,   398,   396,   394,   394,   392,   390,   390,   388,   386,   386,   384,   382,
    382,   380,   378,   378,   376,   374,   374,   372,   370,   370,   368,   366,   366,   364,   362,   362,
    360,   358,   358,   356,   354,   354,   352,   350,   350,   348,   348,   346,   344,   344,   342,   340,
    340,   338,   338,   336,   334,   334,   332,   332,   330,   328,   328,   326,   326,   324,   322,   322,
    320,   320,   318,   316,   316,   314,   314,   312,   310,   310,   308,   308,   306,   304,   304,   302,
    300,   300,   298,   298,   296,   296,   294,   292,   292,   290,   290,   288,   286,   286,   284,   284,
    282,   282,   280,   280,   278,   278,   276,   274,   274,   272,   272,   270,   270,   268,   268,   266,
    264,   264,   262,   262,   260,   260,   258,   258,   256,   254,   254,   252,   252,   250,   250,   248,
    248,   246,   244,   244,   242,   240,   240,   240,   238,   238,   236,   236,   234,   234,   232,   230,
    230,   228,   228,   226,   226,   224,   224,   222,   220,   220,   218,   218,   216,   216,   214,   214,
    212,   212,   210,   210,   208,   208,   206,   204,   204,   202,   202,   200,   200,   198,   198,   196,
    196,   194,   194,   192,   190,   190,   188,   188,   186,   186,   184,   184,   182,   182,   180,   180,
    178,   178,   176,   176,   174,   174,   172,   172,   170,   170,   168,   168,   166,   166,   164,   164,
    162,   162,   160,   160,   158,   156,   156,   154,   154,   152,   152,   150,   150,   148,   148,   146,
    146,   144,   144,   142,   142,   140,   140,   138,   136,   136,   136,   134,   134,   132,   130,   130,
    128,   128,   126,   126,   124,   124,   122,   122,   120,   120,   118,   118,   116,   116,   114,   114,
    112,   110,   110,   108,   108,   106,   106,   104,   104,   102,   102,   100,   100,    98,    98,    96,
     94,    94,    92,    92,    90,    90,    88,    88,    86,    86,    84,    82,    82,    80,    80,    78,
     78,    76,    76,    74,    74,    72,    72,    70,    70,    68,    68,    66,    66,    64,    62,    62,
     60,    60,    58,    56,    56,    54,    54,    52,    52,    50,    50,    48,    48,    46,    46,    44,
     44,    42,    40,    40,    38,    38,    36,    34,    34,    32,    32,    30,    30,    28,    28,    26,
     24,    24,    22,    22,    20,    20,    18,    16,    16,    14,    14,    12,    12,    10,    10,     8,
      6,     6,     4,     2,     2,     0,     0,    -2,    -4,    -4,    -6,    -6,    -8,   -10,   -10,   -12,
    -12,   -14,   -16,  - 16,   -18,   -20,   -20,   -22,   -22,   -24,   -26,   -26,   -28,   -30,   -30,   -32,
    -34,   -34,   -36,   -36,   -38,   -40,   -40,   -42,   -42,   -44,   -46,   -46,   -48,   -50,   -50,   -52,
    -54,   -54,   -56,   -58,   -58,   -60,   -60,   -62,   -64,   -66,   -66,   -68,   -70,   -70,   -72,   -74,
    -76,   -76,   -78,   -80,   -80,   -82,   -84,   -86,   -86,   -88,   -90,   -90,   -92,   -94,   -94,   -96,
    -98,   -98,  -100,  -102,  -104,  -106,  -106,  -108,  -110,  -112,  -114,  -114,  -116,  -118,  -120,  -120,
   -122,  -124,  -126,  -128,  -130,  -130,  -132,  -134,  -136,  -138,  -140,  -142,  -144,  -146,  -146,  -148,
   -150,  -152,  -154,  -156,  -158,  -160,  -162,  -164,  -166,  -166,  -168,  -170,  -172,  -174,  -176,  -178,
   -180,  -182,  -184,  -186,  -188,  -190,  -192,  -194,  -196,  -196,  -198,  -200/*,-202,  -204,  -206,  -208,
   -210,  -212,  -214,  -216,  -218,  -220,  -224,  -226,  -228,  -230,  -232,  -234,  -236,  -238,  -242,  -246,
   -248,  -250,  -254,  -256,  -260,  -262,  -264,  -268,  -270,  -274,  -276,  -278,  -282,  -284,  -286,  -290,
   -292,  -296,  -298,  -300,  -306,  -308,  -312,  -316,  -320,  -324,  -326,  -330,  -334,  -338,  -342,  -344,
   -348,  -354,  -358,  -362,  -366,  -370,  -376,  -380,  -384,  -388,  -394,  -398,  -404,  -410,  -416,  -420,
   -428,  -432,  -440,  -446,  -450,  -460,  -468,  -476,  -484,  -492,  -500,  -510,  -524,  -534,  -546,  -560,
   -572,  -588,  -600,  -630,  -656,  -684,  -720,  -770 */
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
UBYTE     cInputPinFunc(UBYTE Cmd, UBYTE Port, UBYTE Pin, UBYTE *pData);

void      cInputInit(void* pHeader)
{
  IOMapInput.pFunc = &cInputPinFunc;
  UBYTE   Tmp;

  memset(IOMapInput.Colors, 0, sizeof(IOMapInput.Colors));
  memset(VarsInput.VarsColor, 0, sizeof(VarsInput.VarsColor));

  /* Init IO map */
  for (Tmp = 0; Tmp < NO_OF_INPUTS; Tmp++)
  {
    INPUTSTRUCT * pIn = &(IOMapInput.Inputs[Tmp]);
    pIn->SensorType         = NO_SENSOR;
    pIn->SensorMode         = RAWMODE;
    pIn->SensorRaw          = 0;
    pIn->SensorValue        = 0;
    pIn->SensorBoolean      = 0;
    pIn->InvalidData        = INVALID_DATA;
    pIn->DigiPinsDir        = 0;
    pIn->DigiPinsOut        = 0;
    pIn->CustomActiveStatus = CUSTOMINACTIVE;
    pIn->CustomZeroOffset   = 0;
    pIn->CustomPctFullScale = 0;
    dInputRead0(Tmp, &(pIn->DigiPinsIn));
    dInputRead1(Tmp, &(pIn->DigiPinsIn));

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
    INPUTSTRUCT * pIn = &(IOMapInput.Inputs[Tmp]);
    UBYTE sType = pIn->SensorType;
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
        pIn->SensorType = COLOREXIT;
        sType = COLOREXIT;
      }
      /* Setup the pins for the new sensortype */
      cInputSetupType(Tmp, sType, oldType);
      pIn->InvalidData = INVALID_DATA;
      VarsInput.OldSensorType[Tmp]       = sType;
    }
    else
    {
      if (VarsInput.InvalidTimer[Tmp])
      {

        /* A type change has been carried out earlier - waiting for valid data   */
        /* The color sensor requires special startup sequence with communication */
        if ((sType == COLORFULL) || 
            (sType == COLORRED)  ||
            (sType == COLORGREEN)|| 
            (sType == COLORBLUE) ||
            (sType == COLOREXIT) || 
            (sType == COLORNONE))
        {
          cInputCalcSensorValues(Tmp);
        }

        (VarsInput.InvalidTimer[Tmp])--;
        if (0 == VarsInput.InvalidTimer[Tmp])
        {

          /* Time elapsed - data are now valid */
          pIn->InvalidData &= ~INVALID_DATA;
        }
      }
      else
      {

        /* The invalid bit could have been set by the VM due to Mode change    */
        /* but input module needs to be called once to update the values       */
        pIn->InvalidData &= ~INVALID_DATA;
      }
    }

    if (!(INVALID_DATA & (pIn->InvalidData)))
    {
      cInputCalcSensorValues(Tmp);
    }
  }
}


void      cInputCalcSensorValues(UBYTE No)
{
  INPUTSTRUCT * pIn = &(IOMapInput.Inputs[No]);
  UBYTE sType = pIn->SensorType;

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
        dInputRead0(No, &(pIn->DigiPinsIn));
        dInputRead1(No, &(pIn->DigiPinsIn));
      }
      
      dInputGetRawAd(&InputVal, No);
      pIn->ADRaw = InputVal;
      
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
        InputVal = TempConvTable[(InputVal) - /*197*/ 290];
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
        cInputCalcFullScale(&InputVal, pIn->CustomZeroOffset, pIn->CustomPctFullScale, FALSE);
      }
      cInputCalcSensorValue(  InputVal,
                            &(pIn->SensorRaw),
                            &(pIn->SensorValue),
                            &(pIn->SensorBoolean),
                            &(VarsInput.InputDebounce[No]),
                            &(VarsInput.SampleCnt[No]),
                            &(VarsInput.LastAngle[No]),
                            &(VarsInput.EdgeCnt[No]),
                            ((pIn->SensorMode) & SLOPEMASK),
                            ((pIn->SensorMode) & MODEMASK));

    }
    break;

    /* Triple case intended */
    case LOWSPEED:
    case LOWSPEED_9V:
    case HIGHSPEED:
    {
      UWORD InputVal;
      dInputGetRawAd(&InputVal, No);
      pIn->ADRaw = InputVal;
    }
    break;

    /* Four cases intended */
    case COLORRED:
    case COLORGREEN:
    case COLORBLUE:
    case COLORNONE:
    {
      COLORSTRUCT * pC = &(IOMapInput.Colors[No]);

      UWORD InputVal;
      switch (pC->CalibrationState)
      {
        case SENSOROFF:
        {
          /* Check if sensor has been attached */
          if (dInputCheckColorStatus(No))
          {

            /* Sensor has been attached now get cal data */
            VarsInput.VarsColor[No].ColorInitState = 0;
            (pC->CalibrationState) = SENSORCAL;
          }
        }
        break;
        case SENSORCAL:
        {

          UBYTE Status;
          if (FALSE == cInputInitColorSensor(No, &Status))
          {

            /* Color sensor has been removed during calibration */
            (pC->CalibrationState) = SENSOROFF;
          }

          if (TRUE == Status)
          {

            /* Use clock to detect errors */
            dInputSetDirInDigi0(No);
            (pC->CalibrationState) = 0;
          }
        }
        break;
        default:
        {
          if (dInputGetColor(No, &(pIn->ADRaw)))
          {
            InputVal = pIn->ADRaw;
            cInputCalcFullScale(&InputVal, COLORSENSORBGMIN, COLORSENSORBGPCTDYN, FALSE);
            cInputCalcSensorValue(InputVal,
                                  &(pIn->SensorRaw),
                                  &(pIn->SensorValue),
                                  &(pIn->SensorBoolean),
                                  &(VarsInput.InputDebounce[No]),
                                  &(VarsInput.SampleCnt[No]),
                                  &(VarsInput.LastAngle[No]),
                                  &(VarsInput.EdgeCnt[No]),
                                  ((pIn->SensorMode) & SLOPEMASK),
                                  ((pIn->SensorMode) & MODEMASK));
          }
          else
          {
            pC->CalibrationState = SENSOROFF;
          }
        }
        break;
      }
    }
    break;
    case COLORFULL:
    {
      COLORSTRUCT * pC = &(IOMapInput.Colors[No]);
      switch (pC->CalibrationState)
      {
        case SENSOROFF:
        {
          /* Check if sensor has been attached */
          if (dInputCheckColorStatus(No))
          {

            /* Sensor has been attached now get cal data */
            VarsInput.VarsColor[No].ColorInitState = 0;
            (pC->CalibrationState) = SENSORCAL;
          }
        }
        break;
        case SENSORCAL:
        {
          UBYTE Status;

          if (FALSE == cInputInitColorSensor(No, &Status))
          {

            /* Color sensor has been removed during calibration */
            (pC->CalibrationState) = SENSOROFF;
            VarsInput.ColorStatus &= ~(0x01<<No);
          }

          if (TRUE == Status)
          {

            /* Initialization finished with success recalc the values*/
            (pC->CalibrationState) = 0;

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

            /* Check if sensor is deteched */
            if (dInputCheckColorStatus(No))
            {

              /* Calibrate the raw ad values returns the SensorRaw */
              cInputCalibrateColor(pC, NewSensorVals);

              for(ColorCount = 0; ColorCount < BLANK; ColorCount++)
              {

                /* Calculate color sensor values */
                cInputCalcSensorValue(NewSensorVals[ColorCount],
                                      &(pC->SensorRaw[ColorCount]),
                                      &(pC->SensorValue[ColorCount]),
                                      &(pC->Boolean[ColorCount]),
                                      &(VarsInput.VarsColor[No].ColorInputDebounce[ColorCount]),
                                      &(VarsInput.VarsColor[No].ColorSampleCnt[ColorCount]),
                                      &(VarsInput.VarsColor[No].ColorLastAngle[ColorCount]),
                                      &(VarsInput.VarsColor[No].ColorEdgeCnt[ColorCount]),
                                      ((pIn->SensorMode) & SLOPEMASK),
                                      ((pIn->SensorMode) & MODEMASK));
              }

              /* Calculate background sensor values */
              cInputCalcSensorValue(NewSensorVals[BLANK],
                                    &(pC->SensorRaw[BLANK]),
                                    &(pC->SensorValue[BLANK]),
                                    &(pC->Boolean[BLANK]),
                                    &(VarsInput.VarsColor[No].ColorInputDebounce[BLANK]),
                                    &(VarsInput.VarsColor[No].ColorSampleCnt[BLANK]),
                                    &(VarsInput.VarsColor[No].ColorLastAngle[BLANK]),
                                    &(VarsInput.VarsColor[No].ColorEdgeCnt[BLANK]),
                                    ((pIn->SensorMode) & SLOPEMASK),
                                    ((pIn->SensorMode) & MODEMASK));

              /* Color Sensor values has been calculated -                */
              /* now calculate the color and put it in Sensor value       */
              if (((pC->SensorRaw[RED]) > (pC->SensorRaw[BLUE] )) &&
                  ((pC->SensorRaw[RED]) > (pC->SensorRaw[GREEN])))
              {

                /* If all 3 colors are less than 65 OR (Less that 110 and bg less than 40)*/
                if (((pC->SensorRaw[RED])   < 65) ||
                    (((pC->SensorRaw[BLANK]) < 40) && ((pC->SensorRaw[RED])  < 110)))
                {
                  pIn->SensorValue = BLACKCOLOR;
                }
                else
                {
                  if (((((pC->SensorRaw[BLUE]) >> 2)  + ((pC->SensorRaw[BLUE]) >> 3) + (pC->SensorRaw[BLUE])) < (pC->SensorRaw[GREEN])) &&
                      ((((pC->SensorRaw[GREEN]) << 1)) > (pC->SensorRaw[RED])))
                  {
                    pIn->SensorValue = YELLOWCOLOR;
                  }
                  else
                  {

                    if ((((pC->SensorRaw[GREEN]) << 1) - ((pC->SensorRaw[GREEN]) >> 2)) < (pC->SensorRaw[RED]))
                    {

                      pIn->SensorValue = REDCOLOR;
                    }
                    else
                    {

                      if ((((pC->SensorRaw[BLUE]) < 70) ||
                          ((pC->SensorRaw[GREEN]) < 70)) ||
                         (((pC->SensorRaw[BLANK]) < 140) && ((pC->SensorRaw[RED]) < 140)))
                      {
                        pIn->SensorValue = BLACKCOLOR;
                      }
                      else
                      {
                        pIn->SensorValue = WHITECOLOR;
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
                    pIn->SensorValue = BLACKCOLOR;
                  }
                  else
                  {
                    if ((((pC->SensorRaw[BLUE]) << 1)) < (pC->SensorRaw[RED]))
                    {
                      pIn->SensorValue = YELLOWCOLOR;
                    }
                    else
                    {
                      if ((((pC->SensorRaw[RED]) + ((pC->SensorRaw[RED])>>2)) < (pC->SensorRaw[GREEN])) ||
                          (((pC->SensorRaw[BLUE]) + ((pC->SensorRaw[BLUE])>>2)) < (pC->SensorRaw[GREEN])))
                      {
                        pIn->SensorValue = GREENCOLOR;
                      }
                      else
                      {
                        if ((((pC->SensorRaw[RED]) < 70) ||
                            ((pC->SensorRaw[BLUE]) < 70)) ||
                            (((pC->SensorRaw[BLANK]) < 140) && ((pC->SensorRaw[GREEN]) < 140)))
                        {
                          pIn->SensorValue = BLACKCOLOR;
                        }
                        else
                        {
                          pIn->SensorValue = WHITECOLOR;
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
                    pIn->SensorValue = BLACKCOLOR;
                  }
                  else
                  {
                    if ((((((pC->SensorRaw[RED]) * 48) >> 5) < (pC->SensorRaw[BLUE])) &&
                        ((((pC->SensorRaw[GREEN]) * 48) >> 5) < (pC->SensorRaw[BLUE])))
                        ||
                        (((((pC->SensorRaw[RED])   * 58) >> 5) < (pC->SensorRaw[BLUE])) ||
                         ((((pC->SensorRaw[GREEN]) * 58) >> 5) < (pC->SensorRaw[BLUE]))))
                    {
                      pIn->SensorValue = BLUECOLOR;
                    }
                    else
                    {

                      /* Color is white or Black */
                      if ((((pC->SensorRaw[RED])  < 60) ||
                          ((pC->SensorRaw[GREEN]) < 60)) ||
                         (((pC->SensorRaw[BLANK]) < 110) && ((pC->SensorRaw[BLUE]) < 120)))
                      {
                        pIn->SensorValue = BLACKCOLOR;
                      }
                      else
                      {
                        if ((((pC->SensorRaw[RED])  + ((pC->SensorRaw[RED])   >> 3)) < (pC->SensorRaw[BLUE])) ||
                            (((pC->SensorRaw[GREEN]) + ((pC->SensorRaw[GREEN]) >> 3)) < (pC->SensorRaw[BLUE])))
                        {
                          pIn->SensorValue = BLUECOLOR;
                        }
                        else
                        {
                          pIn->SensorValue = WHITECOLOR;
                        }
                      }
                    }
                  }
                }
              }
            }
            else
            {
              pC->CalibrationState = SENSOROFF;
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
        pIn->SensorType = NO_SENSOR;
      }

      if (TRUE == Status)
      {

        /* Initialization finished with success recalc the values*/
        (IOMapInput.Colors[No].CalibrationState) = 0;
        pIn->SensorType = NO_SENSOR;
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

UBYTE cInputPinFunc(UBYTE Cmd, UBYTE Port, UBYTE Pin, UBYTE *pData)
{
  UBYTE ReturnState = NO_ERR;
  if (IOMapInput.Inputs[Port].SensorType != CUSTOM)
    return (UBYTE)ERR_INVALID_PORT;
  
  UBYTE WaitUSEC = (Cmd&0xFC)>>2;
  UBYTE Dir = (Pin&0xFC)>>2;
  Pin &= 0x03;

  switch(Cmd&0x03)
  {
    case PINDIR:
    {
      if (Pin & DIGI0)
      {
        if (Dir)
          dInputSetDirInDigi0(Port);
        else
          dInputSetDirOutDigi0(Port);
      }
      if (Pin & DIGI1)
      {
        if (Dir)
          dInputSetDirInDigi1(Port);
        else
          dInputSetDirOutDigi1(Port);
      }
    }
    break;
    case SETPIN:
    {
      if (Pin & DIGI0)
        dInputSetDigi0(Port);
      if (Pin & DIGI1)
        dInputSetDigi1(Port);
    }
    break;
    case CLEARPIN:
    {
      if (Pin & DIGI0)
        dInputClearDigi0(Port);
      if (Pin & DIGI1)
        dInputClearDigi1(Port);
    }
    break;
    case READPIN:
    {
      if (Pin & DIGI0)
        dInputRead0(Port, pData);
      if (Pin & DIGI1)
        dInputRead1(Port, pData);
    }
    break;
  }
  if (WaitUSEC)
    dInputWaitUS(WaitUSEC);
  
  return (ReturnState);
}
