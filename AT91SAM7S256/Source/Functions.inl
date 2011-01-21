//
// Programmer
//
// Date init       26.04.2005
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date::                                                   $
//
// Filename        $Workfile::                                               $
//
// Version         $Revision::                                               $
//
// Archive         $Archive::                                                $
//
// Platform        C
//

//******* cUiBtTest **********************************************************

const     UBYTE NONVOLATILE_NAME[]      = UI_NONVOLATILE;     // Non volatile filename without extention
#ifndef STRIPPED
const     UBYTE DEFAULT_PROGRAM_NAME[]  = UI_PROGRAM_DEFAULT; // On brick programming filename without extention
const     UBYTE TEMP_PROGRAM_FILENAME[] = UI_PROGRAM_TEMP;    // On brick programming tmp filename without extention
const     UBYTE VM_PROGRAM_READER[]     = UI_PROGRAM_READER;  // On brick programming script reader filename without extention
#endif
#ifndef STRIPPED
const     UBYTE TEMP_DATALOG_FILENAME[] = UI_DATALOG_TEMP;    // On brick datalog tmp filename without extention
const     UBYTE DEFAULT_DATALOG_NAME[]  = UI_DATALOG_DEFAULT; // On brick datalog filename without extention
#endif
const     UBYTE DEFAULT_PIN_CODE[]      = UI_PINCODE_DEFAULT; // Default blue tooth pin code
const     UBYTE TXT_INVALID_SENSOR[]    = "??????????????";   // Display invalid sensor data


#define   SENSORS   (MENU_SENSOR_INVALID - MENU_SENSOR_EMPTY)

const     UBYTE SENSORTYPE[SENSORS] =  // for view and datalog
{
  0,                        //  MENU_SENSOR_EMPTY
  SOUND_DB,                 //  MENU_SENSOR_SOUND_DB
  SOUND_DBA,                //  MENU_SENSOR_SOUND_DBA
  LIGHT_ACTIVE,             //  MENU_SENSOR_LIGHT
  LIGHT_INACTIVE,           //  MENU_SENSOR_LIGHT_AMB
  SWITCH,                   //  MENU_SENSOR_TOUCH
  0,                        //  MENU_SENSOR_MOTOR_DEG
  0,                        //  MENU_SENSOR_MOTOR_ROT
  LOWSPEED_9V,              //  MENU_SENSOR_ULTRASONIC_IN
  LOWSPEED_9V,              //  MENU_SENSOR_ULTRASONIC_CM
  LOWSPEED_9V,              //  MENU_SENSOR_IIC_TEMP_C
  LOWSPEED_9V,              //  MENU_SENSOR_IIC_TEMP_F
  COLORFULL                 //  MENU_SENSOR_COLOR
};

const     UBYTE SENSORMODE[SENSORS] =  // for view and datalog
{
  0,                        //  MENU_SENSOR_EMPTY
  PCTFULLSCALEMODE,         //  MENU_SENSOR_SOUND_DB
  PCTFULLSCALEMODE,         //  MENU_SENSOR_SOUND_DBA
  PCTFULLSCALEMODE,         //  MENU_SENSOR_LIGHT
  PCTFULLSCALEMODE,         //  MENU_SENSOR_LIGHT_AMB
  BOOLEANMODE,              //  MENU_SENSOR_TOUCH
  0,                        //  MENU_SENSOR_MOTOR_DEG
  0,                        //  MENU_SENSOR_MOTOR_ROT
  0,                        //  MENU_SENSOR_ULTRASONIC_IN
  0,                        //  MENU_SENSOR_ULTRASONIC_CM
  0,                        //  MENU_SENSOR_IIC_TEMP_C
  0,                        //  MENU_SENSOR_IIC_TEMP_F
  0                         //  MENU_SENSOR_COLOR
};

const     UBYTE SENSORFORMAT[SENSORS][9] =
{
  "",                       //  MENU_SENSOR_EMPTY
  "%3.0f %%",               //  MENU_SENSOR_SOUND_DB
  "%3.0f %%",               //  MENU_SENSOR_SOUND_DBA
  "%3.0f %%",               //  MENU_SENSOR_LIGHT
  "%3.0f %%",               //  MENU_SENSOR_LIGHT_AMB
  "%1.0f",                  //  MENU_SENSOR_TOUCH
  "%8.0f `",                //  MENU_SENSOR_MOTOR_DEG
  "%8.0f R",                //  MENU_SENSOR_MOTOR_ROT
  "%3.0f In",               //  MENU_SENSOR_ULTRASONIC_IN
  "%3.0f cm",               //  MENU_SENSOR_ULTRASONIC_CM
  "%5.1f `C",               //  MENU_SENSOR_IIC_TEMP_C
  "%5.1f `F",               //  MENU_SENSOR_IIC_TEMP_F
  "%9.0f"                   //  MENU_SENSOR_COLOR (no of characters)
};

const     float SENSORDIVIDER[SENSORS] =
{
  1.0,                      //  MENU_SENSOR_EMPTY
  1.0,                      //  MENU_SENSOR_SOUND_DB
  1.0,                      //  MENU_SENSOR_SOUND_DBA
  1.0,                      //  MENU_SENSOR_LIGHT
  1.0,                      //  MENU_SENSOR_LIGHT_AMB
  1.0,                      //  MENU_SENSOR_TOUCH
  1.0,                      //  MENU_SENSOR_MOTOR_DEG
  360.0,                    //  MENU_SENSOR_MOTOR_ROT
  2.54,                     //  MENU_SENSOR_ULTRASONIC_IN
  1.0,                      //  MENU_SENSOR_ULTRASONIC_CM
  10.0,                     //  MENU_SENSOR_IIC_TEMP_C
  10.0,                     //  MENU_SENSOR_IIC_TEMP_F
  1.0                       //  MENU_SENSOR_COLOR
};


#define   SENSORSYNCDATA    "Sync data"
#define   SENSORSDATA       "Sdata"
#define   SENSORTIME        "Time"


const     UBYTE SENSORDIRNAME[SENSORS - 1][19] =
{
  "Sound Sensor",           //  MENU_SENSOR_SOUND_DB
  "Sound Sensor",           //  MENU_SENSOR_SOUND_DBA
  "Light Sensor",           //  MENU_SENSOR_LIGHT
  "Light Sensor",           //  MENU_SENSOR_LIGHT_AMB
  "Bumper",                 //  MENU_SENSOR_TOUCH
  "FP Rotation Sensor",     //  MENU_SENSOR_MOTOR_DEG
  "FP Rotation Sensor",     //  MENU_SENSOR_MOTOR_ROT
  "Distance Sensor",        //  MENU_SENSOR_ULTRASONIC_IN
  "Distance Sensor",        //  MENU_SENSOR_ULTRASONIC_CM
  "NXT Temp Sensor",        //  MENU_SENSOR_IIC_TEMP_C
  "NXT Temp Sensor",        //  MENU_SENSOR_IIC_TEMP_F
  "Color Detector"          //  MENU_SENSOR_COLOR
};

const     UBYTE SENSORUNITNAME[SENSORS - 1][5] =
{
  "_dB",                    //  MENU_SENSOR_SOUND_DB
  "_dBa",                   //  MENU_SENSOR_SOUND_DBA
  "_on",                    //  MENU_SENSOR_LIGHT
  "_off",                   //  MENU_SENSOR_LIGHT_AMB
  "",                       //  MENU_SENSOR_TOUCH
  "_deg",                   //  MENU_SENSOR_MOTOR_DEG
  "_rot",                   //  MENU_SENSOR_MOTOR_ROT
  "_in",                    //  MENU_SENSOR_ULTRASONIC_IN
  "_cm",                    //  MENU_SENSOR_ULTRASONIC_CM
  "_C",                     //  MENU_SENSOR_IIC_TEMP_C
  "_F",                     //  MENU_SENSOR_IIC_TEMP_F
  "_0",                     //  MENU_SENSOR_COLOR
};

const     UBYTE SENSORFORMAT2[SENSORS - 1][6] =
{
  "\t%.0f",                 //  MENU_SENSOR_SOUND_DB
  "\t%.0f",                 //  MENU_SENSOR_SOUND_DBA
  "\t%.0f",                 //  MENU_SENSOR_LIGHT
  "\t%.0f",                 //  MENU_SENSOR_LIGHT_AMB
  "\t%.0f",                 //  MENU_SENSOR_TOUCH
  "\t%.0f",                 //  MENU_SENSOR_MOTOR_DEG
  "\t%.0f",                 //  MENU_SENSOR_MOTOR_ROT
  "\t%.0f",                 //  MENU_SENSOR_ULTRASONIC_IN
  "\t%.0f",                 //  MENU_SENSOR_ULTRASONIC_CM
  "\t%.1f",                 //  MENU_SENSOR_IIC_TEMP_C
  "\t%.1f",                 //  MENU_SENSOR_IIC_TEMP_F
  "\t%.0f"                  //  MENU_SENSOR_COLOR
};


//******* cUiWriteLowspeed ***************************************************

void      cUiWriteLowspeed(UBYTE Port,UBYTE TxBytes,UBYTE *TxBuf,UBYTE RxBytes)
{
  Port -= MENU_PORT_1;
  pMapLowSpeed->OutBuf[Port].InPtr    = 0;
  pMapLowSpeed->OutBuf[Port].OutPtr   = 0;

  while (TxBytes)
  {
    pMapLowSpeed->OutBuf[Port].Buf[pMapLowSpeed->OutBuf[Port].InPtr] = *TxBuf;
    pMapLowSpeed->OutBuf[Port].InPtr++;
    TxBuf++;
    TxBytes--;
  }
  pMapLowSpeed->InBuf[Port].BytesToRx = RxBytes;
  pMapLowSpeed->ChannelState[Port]    = LOWSPEED_INIT;
  pMapLowSpeed->State                |= (COM_CHANNEL_ONE_ACTIVE << Port);
}


//******* cUiReadLowspeed ****************************************************

#define   IIC_READY       0
#define   IIC_BUSY        1
#define   IIC_ERROR       2

UBYTE     cUiReadLowspeed(UBYTE Port,UBYTE RxBytes,UWORD *Value)
{
  UBYTE   Result;

  *Value = 0;
  Port -= MENU_PORT_1;
  if ((pMapLowSpeed->ChannelState[Port] == LOWSPEED_IDLE) || (pMapLowSpeed->ChannelState[Port] == LOWSPEED_DONE))
  {
    while (RxBytes)
    {
      (*Value) <<= 8;
      (*Value) |= (UWORD)(pMapLowSpeed->InBuf[Port].Buf[pMapLowSpeed->InBuf[Port].OutPtr]);
      pMapLowSpeed->InBuf[Port].OutPtr++;
      if (pMapLowSpeed->InBuf[Port].OutPtr >= SIZE_OF_LSBUF)
      {
        pMapLowSpeed->InBuf[Port].OutPtr = 0;
      }
      RxBytes--;
    }
    Result = IIC_READY;
  }
  else
  {
    if (pMapLowSpeed->ErrorType[Port] == LOWSPEED_CH_NOT_READY)
    {
      Result = IIC_ERROR;
    }
    else
    {
      Result = IIC_BUSY;
    }
  }

  return (Result);
}


//******* cUiUpdateSensor ****************************************************

#define   SENSOR_SETUP          0
#define   SENSOR_ACQUIRE        3
#define   SENSOR_READ           8
#define   SENSOR_STATES         10

void      cUiUpdateSensor(SWORD Time)
{
#ifndef STRIPPED

  UBYTE   Port;
  UBYTE   Sensor;
  UBYTE   Result;
  SWORD   Tmp;

  if (VarsUi.SensorReset == TRUE)
  {
    for (Port = MENU_PORT_1;Port < MENU_PORT_INVALID;Port++)
    {
      VarsUi.DatalogSampleValid[Port - MENU_PORT_1] = FALSE;
    }
    VarsUi.SensorTimer = (MIN_SENSOR_READ_TIME / SENSOR_STATES);
    VarsUi.SensorState = SENSOR_SETUP;
  }

  VarsUi.SensorTimer += Time;
  if (VarsUi.SensorTimer >= (MIN_SENSOR_READ_TIME / SENSOR_STATES))
  {
    VarsUi.SensorTimer -= (MIN_SENSOR_READ_TIME / SENSOR_STATES);

    for (Port = MENU_PORT_1;Port < MENU_PORT_INVALID;Port++)
    {
      Sensor = VarsUi.DatalogPort[Port - MENU_PORT_1];

      if (Sensor != MENU_SENSOR_EMPTY)
      {
        if ((Sensor == MENU_SENSOR_MOTOR_DEG) || (Sensor == MENU_SENSOR_MOTOR_ROT))
        {
          if (VarsUi.SensorReset == TRUE)
          {
            pMapOutPut->Outputs[Port - MENU_PORT_A].Mode      &= ~(BRAKE | MOTORON);
            pMapOutPut->Outputs[Port - MENU_PORT_A].Flags     |= UPDATE_MODE | UPDATE_SPEED | UPDATE_RESET_COUNT;
            pMapOutPut->Outputs[Port - MENU_PORT_A].TachoCnt   = 0;
          }
          if (VarsUi.SensorState == SENSOR_READ)
          {
            VarsUi.DatalogSampleValue[Port - MENU_PORT_1] = pMapOutPut->Outputs[Port - MENU_PORT_A].TachoCnt;
            VarsUi.DatalogSampleValid[Port - MENU_PORT_1] = TRUE;
          }
        }
        else
        {
          pMapInput->Inputs[Port - MENU_PORT_1].SensorType   = SENSORTYPE[Sensor - MENU_SENSOR_EMPTY];
          pMapInput->Inputs[Port - MENU_PORT_1].SensorMode   = SENSORMODE[Sensor - MENU_SENSOR_EMPTY];
          if ((Sensor == MENU_SENSOR_ULTRASONIC_IN) || (Sensor == MENU_SENSOR_ULTRASONIC_CM))
          {
            if (VarsUi.SensorReset == TRUE)
            {
              cUiWriteLowspeed(Port,3,"\x02\x41\x02",0);
            }
            if (VarsUi.SensorState == SENSOR_ACQUIRE)
            {
              cUiWriteLowspeed(Port,2,"\x02\x42",1);
            }
            if (VarsUi.SensorState == SENSOR_READ)
            {
              Result = cUiReadLowspeed(Port,1,(UWORD*)&Tmp);
              if (Result == IIC_READY)
              {
                if ((UBYTE)Tmp != 0xFF)
                {
                  VarsUi.DatalogSampleValue[Port - MENU_PORT_1] = (SLONG)Tmp;
                  VarsUi.DatalogSampleValid[Port - MENU_PORT_1] = TRUE;
                }
                else
                {
                  VarsUi.DatalogSampleValid[Port - MENU_PORT_1] = FALSE;
                }
              }
              else
              {
                VarsUi.DatalogSampleValid[Port - MENU_PORT_1] = FALSE;
              }
            }
          }
          else
          {
            if ((Sensor == MENU_SENSOR_IIC_TEMP_C) || (Sensor == MENU_SENSOR_IIC_TEMP_F))
            {
              if (VarsUi.SensorState == SENSOR_SETUP)
              {
                cUiWriteLowspeed(Port,3,"\x98\x01\x60",0);
              }
              if (VarsUi.SensorState == SENSOR_ACQUIRE)
              {
                cUiWriteLowspeed(Port,2,"\x98\x00",2);
              }
              if (VarsUi.SensorState == SENSOR_READ)
              {
                Result = cUiReadLowspeed(Port,2,(UWORD*)&Tmp);
                if (Result == IIC_READY)
                {
//                  if (Tmp >= -14080)
                  {
                    if (Sensor == MENU_SENSOR_IIC_TEMP_F)
                    {
                      VarsUi.DatalogSampleValue[Port - MENU_PORT_1] = (SLONG)((float)(Tmp + 4544) / (float)14.2);
                    }
                    else
                    {
                      VarsUi.DatalogSampleValue[Port - MENU_PORT_1] = (SLONG)((float)Tmp / (float)25.6);
                    }
                    VarsUi.DatalogSampleValid[Port - MENU_PORT_1] = TRUE;
                  }
                }
                else
                {
                  if (Result == IIC_ERROR)
                  {
                    VarsUi.DatalogSampleValid[Port - MENU_PORT_1] = FALSE;
                  }
                }
              }
            }
            else
            {
              if (VarsUi.SensorState == SENSOR_READ)
              {
                if (pMapInput->Inputs[Port - MENU_PORT_1].InvalidData != INVALID_DATA)
                {
                  VarsUi.DatalogSampleValue[Port - MENU_PORT_1] = pMapInput->Inputs[Port - MENU_PORT_1].SensorValue;
                  VarsUi.DatalogSampleValid[Port - MENU_PORT_1] = TRUE;
                }
                else
                {
                  VarsUi.DatalogSampleValid[Port - MENU_PORT_1] = FALSE;
                }
              }
            }
          }
        }
      }
    }

    VarsUi.SensorState++;
    if (VarsUi.SensorState >= SENSOR_STATES)
    {
      VarsUi.SensorState = SENSOR_SETUP;
    }

    VarsUi.SensorReset = FALSE;
  }
#endif
}


//******* cUiGetCustomPctFullScale *******************************************


UBYTE     cUiGetCustomPctFullScale(UBYTE Port,UBYTE Sensor)
{
  UBYTE   Result = 0;

  if ((Sensor != MENU_SENSOR_MOTOR_DEG) && (Sensor != MENU_SENSOR_MOTOR_ROT))
  {
    Result = (UBYTE)pMapInput->Inputs[Port - MENU_PORT_1].CustomPctFullScale;
  }

  return (Result);
}



//******* cUiGetCustomActiveStatus *******************************************


UBYTE     cUiGetCustomActiveStatus(UBYTE Port,UBYTE Sensor)
{
  UBYTE   Result = 0;

  if ((Sensor != MENU_SENSOR_MOTOR_DEG) && (Sensor != MENU_SENSOR_MOTOR_ROT))
  {
    Result = (UBYTE)pMapInput->Inputs[Port - MENU_PORT_1].CustomActiveStatus;
  }

  return (Result);
}



//******* cUiPrintSensorInDisplayBuffer **************************************

#define   COLORNAMES      6

const     UBYTE COLORNAME[COLORNAMES][10] =
{
  "1. Black ",
  "2. Blue  ",
  "3. Green ",
  "4. Yellow",
  "5. Red   ",
  "6. White "
};


void      cUiPrintSensorInDisplayBuffer(UBYTE Port)
{
#ifndef STRIPPED
  UBYTE   Sensor;
  float   Value;
  SWORD   Size;
  SWORD   Index;

  Port   -= MENU_PORT_1;
  Sensor  = VarsUi.DatalogPort[Port] - MENU_SENSOR_EMPTY;
  Value   = (float)VarsUi.DatalogSampleValue[Port]  / (float)SENSORDIVIDER[Sensor];
  Size    = sprintf((char*)VarsUi.DisplayBuffer,(char*)SENSORFORMAT[Sensor],(float)0);
  sprintf((char*)VarsUi.DisplayBuffer,"%*.*s",Size,Size,(char*)TXT_INVALID_SENSOR);

  if (VarsUi.DatalogSampleValid[Port] == TRUE)
  {
    if (Sensor == (MENU_SENSOR_COLOR - MENU_SENSOR_EMPTY))
    {
      Index = (SWORD)Value - 1;
      if ((Index >= 0) && (Index < COLORNAMES))
      {
        sprintf((char*)VarsUi.DisplayBuffer,(char*)COLORNAME[Index]);
      }
    }
    else
    {
      if (Size < sprintf((char*)VarsUi.DisplayBuffer,(char*)SENSORFORMAT[Sensor],Value))
      {
        sprintf((char*)VarsUi.DisplayBuffer,"%*.*s",Size,Size,(char*)TXT_INVALID_SENSOR);
      }
    }
  }
#endif
}


//******* cUiReleaseSensors **************************************************

void      cUiReleaseSensors(void)
{
  UBYTE   Tmp;

  for (Tmp = 0;Tmp < NO_OF_INPUTS;Tmp++)
  {
    pMapInput->Inputs[Tmp].SensorType   = NO_SENSOR;
  }
}



//******* cUiBtCommand *******************************************************

enum
{
  UI_BT_CTRL,

  UI_BT_GET_DEVICES,          // (UI_BT_GET_DEVICES,Known,*pDevices,NULL)             [Known = 0,1]
  UI_BT_GET_DEVICE_NAME,      // (UI_BT_GET_DEVICE_NAME,Known,*pIndex,*pDeviceName)   [Known = 0,1]
  UI_BT_GET_DEVICE_TYPE,      // (UI_BT_GET_DEVICE_TYPE,Known,*pIndex,*pDeviceType)   [Known = 0,1]

  UI_BT_GET_CONNECTION_NAME,  // (UI_BT_GET_CONNECTION_NAME,NULL,*pConnection,*pConnectionName)
  UI_BT_GET_CONNECTION_TYPE,  // (UI_BT_GET_CONNECTION_TYPE,NULL,*pConnection,*pConnectionType)
  UI_BT_GET_CONNECTION_VALID, // (UI_BT_GET_CONNECTION_NAME,NULL,*pConnection,NULL)

  UI_BT_DUMMY
};


#define   UI_BT_FAILED          0x8200  // General command failed
#define   UI_BT_SUCCES          0x0000  // Command executed succesfully


UBYTE     cUiBTGetDeviceType(UBYTE *pCOD)
{
  ULONG   COD;
  UBYTE   Result;
  UBYTE   Tmp;

  COD      = 0;
  for (Tmp = 0;Tmp < SIZE_OF_CLASS_OF_DEVICE;Tmp++)
  {
    COD  <<= 8;
    COD   |= (ULONG)*pCOD;
    pCOD++;
  }

  Result   = DEVICETYPE_UNKNOWN;
  if ((COD & 0x00001FFF) == 0x00000804)
  {
    Result = DEVICETYPE_NXT;
  }
  if ((COD & 0x00001F00) == 0x00000200)
  {
    Result = DEVICETYPE_PHONE;
  }
  if ((COD & 0x00001F00) == 0x00000100)
  {
    Result = DEVICETYPE_PC;
  }

  return (Result);
}


UBYTE     cUiBTGetDeviceIndex(UBYTE Known,UBYTE No,UBYTE *pIndex)
{
  UBYTE   Result = 0;
  UBYTE   Tmp;

  *pIndex  = 0;
  if (Known)
  {
    for (Tmp = 0;(Tmp < SIZE_OF_BT_DEVICE_TABLE) && (Result == 0);Tmp++)
    {
      if ((pMapComm->BtDeviceTable[Tmp].DeviceStatus & BT_DEVICE_KNOWN))
      {
        if (No == *pIndex)
        {
          *pIndex = Tmp;
          Result  = ~0;
        }
        else
        {
          (*pIndex)++;
        }
      }
    }
  }
  else
  {
    for (Tmp = 0;(Tmp < SIZE_OF_BT_DEVICE_TABLE) && (Result == 0);Tmp++)
    {
      if ((pMapComm->BtDeviceTable[Tmp].DeviceStatus & BT_DEVICE_UNKNOWN) || (pMapComm->BtDeviceTable[Tmp].DeviceStatus & BT_DEVICE_KNOWN))
      {
        if (No == *pIndex)
        {
          *pIndex = Tmp;
          Result  = ~0;
        }
        else
        {
          (*pIndex)++;
        }
      }
    }
  }

  return (Result);
}


UWORD     cUiBTCommand(UBYTE Cmd,UBYTE Flag,UBYTE *pParam1,UBYTE *pParam2)
{
  UWORD   Result = UI_BT_FAILED;

  switch(Cmd)
  {
    case UI_BT_GET_DEVICES :
    {
      cUiBTGetDeviceIndex(Flag,SIZE_OF_BT_DEVICE_TABLE,pParam1);
      Result                = UI_BT_SUCCES;
    }
    break;

    case UI_BT_GET_DEVICE_NAME :
    {
      if ((*pParam1 < SIZE_OF_BT_DEVICE_TABLE) && (pParam2 != NULL))
      {
        pParam2[0]        = 0;
        if (cUiBTGetDeviceIndex(Flag,*pParam1,&VarsUi.BTTmpIndex))
        {
          sprintf((char*)pParam2,"%.*s",DISPLAYLINE_LENGTH,(char*)pMapComm->BtDeviceTable[VarsUi.BTTmpIndex].Name);
          Result          = UI_BT_SUCCES;
        }
      }
    }
    break;

    case UI_BT_GET_DEVICE_TYPE :
    {
      if ((*pParam1 < SIZE_OF_BT_DEVICE_TABLE) && (pParam2 != NULL))
      {
        pParam2[0]        = 0;
        if (cUiBTGetDeviceIndex(Flag,*pParam1,&VarsUi.BTTmpIndex))
        {
          pParam2[0]      = cUiBTGetDeviceType(pMapComm->BtDeviceTable[VarsUi.BTTmpIndex].ClassOfDevice);
          Result          = UI_BT_SUCCES;
        }
      }
    }
    break;

    case UI_BT_GET_CONNECTION_NAME :
    {
      if (*pParam1 < SIZE_OF_BT_CONNECT_TABLE)
      {
        if (pMapComm->BtConnectTable[*pParam1].Name[0])
        {
          if (pParam2 != NULL)
          {
            sprintf((char*)pParam2,"%.*s",DISPLAYLINE_LENGTH,(char*)pMapComm->BtConnectTable[*pParam1].Name);
          }
          Result          = UI_BT_SUCCES;
        }
        else
        {
          if (pParam2 != NULL)
          {
            pParam2[0]    = 0;
          }
        }
      }
    }
    break;

    case UI_BT_GET_CONNECTION_TYPE :
    {
      if ((*pParam1 < SIZE_OF_BT_CONNECT_TABLE) && (pParam2 != NULL))
      {
        pParam2[0]        = 0;
        if (pMapComm->BtConnectTable[*pParam1].Name[0])
        {
          pParam2[0]      = cUiBTGetDeviceType(pMapComm->BtConnectTable[*pParam1].ClassOfDevice);
          Result          = UI_BT_SUCCES;
        }
      }
    }
    break;

    case UI_BT_GET_CONNECTION_VALID :
    {
      if (*pParam1 < SIZE_OF_BT_CONNECT_TABLE)
      {
        if (pMapComm->BtConnectTable[*pParam1].Name[0])
        {
          Result            = UI_BT_SUCCES;
        }
      }
    }
    break;

  }

  return (Result);
}



#include  "BtTest.inc"

//******* cUiNVxxxxx *********************************************************

void      cUiNVWrite(void)
{
  sprintf((char*)VarsUi.NVFilename,"%s.%s",(char*)NONVOLATILE_NAME,(char*)TXT_SYS_EXT);
  VarsUi.NVTmpHandle = pMapLoader->pFunc(FINDFIRST,VarsUi.NVFilename,VarsUi.SearchFilenameBuffer,&VarsUi.NVTmpLength);
  if (!(VarsUi.NVTmpHandle & 0x8000))
  {
    pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.NVTmpHandle,NULL,NULL);
    pMapLoader->pFunc(DELETE,VarsUi.NVFilename,NULL,NULL);
  }
  VarsUi.NVTmpLength = sizeof(NVDATA);
  VarsUi.NVTmpHandle = pMapLoader->pFunc(OPENWRITE,VarsUi.NVFilename,NULL,&VarsUi.NVTmpLength);
  pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.NVTmpHandle,(UBYTE*)&VarsUi.NVData,&VarsUi.NVTmpLength);
  pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.NVTmpHandle,NULL,NULL);
}


void      cUiNVRead(void)
{
  VarsUi.NVData.CheckByte = 0;
  sprintf((char*)VarsUi.NVFilename,"%s.%s",(char*)NONVOLATILE_NAME,(char*)TXT_SYS_EXT);
  VarsUi.NVTmpHandle = pMapLoader->pFunc(OPENREAD,VarsUi.NVFilename,NULL,&VarsUi.NVTmpLength);
  if (!(VarsUi.NVTmpHandle & 0x8000))
  {
    VarsUi.NVTmpLength = sizeof(NVDATA);
    pMapLoader->pFunc(READ,(UBYTE*)&VarsUi.NVTmpHandle,(UBYTE*)&VarsUi.NVData,&VarsUi.NVTmpLength);
    pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.NVTmpHandle,NULL,NULL);
  }
  if (VarsUi.NVData.CheckByte != CHECKBYTE)
  {
    VarsUi.NVData.DatalogEnabled  = DATALOGENABLED;
    VarsUi.NVData.VolumeStep      = MAX_VOLUME;
    VarsUi.NVData.PowerdownCode   = POWER_OFF_TIME_DEFAULT;
    VarsUi.NVData.DatalogNumber   = 0;
    VarsUi.NVData.CheckByte       = CHECKBYTE;
    cUiNVWrite();
  }
}


//******* cUiFeedback ********************************************************

UBYTE     cUiFeedback(BMPMAP *Bitmap,UBYTE TextNo1,UBYTE TextNo2,UWORD Time) // Show bimap and text
{
//  if ((VarsUi.FBState == 0) || ((pMapDisplay->Flags & DISPLAY_POPUP) == 0))
  {
    switch (VarsUi.FBState)
    {
      case 0 : // Set busy
      {
        VarsUi.FBState++;
      }
      break;

      case 1 : // Clear line 2,3,4
      {
        if (DISPLAY_IDLE)
        {
          pMapDisplay->EraseMask  |= (TEXTLINE_BIT(TEXTLINE_2) | TEXTLINE_BIT(TEXTLINE_3) | TEXTLINE_BIT(TEXTLINE_4));
          VarsUi.FBState++;
        }
      }
      break;

      case 2 : // Show bitmap if pressent
      {
        if (DISPLAY_IDLE)
        {
          if (Bitmap != NULL)
          {
            pMapDisplay->pBitmaps[BITMAP_1] = Bitmap;
            pMapDisplay->UpdateMask |= BITMAP_BIT(BITMAP_1);
          }
          VarsUi.FBState++;
        }
      }
      break;

      case 3 : // Get text string
      {
        if (DISPLAY_IDLE)
        {
          pMapDisplay->UpdateMask |= SPECIAL_BIT(TOPLINE);
          VarsUi.FBText            = cUiGetString(TextNo1);
          VarsUi.FBPointer         = 0;
          if (TextNo2)
          {
            VarsUi.FBState = 5;
          }
          else
          {
            VarsUi.FBState++;
          }
        }
      }
      break;

      case 4 : // Show text string
      {
        if ((VarsUi.FBText[VarsUi.FBPointer]) && (VarsUi.FBPointer < NO_OF_FEEDBACK_CHARS))
        {
          pMapDisplay->pFunc(DISPLAY_CHAR,TRUE,24 + VarsUi.FBPointer * 6,16,VarsUi.FBText[VarsUi.FBPointer],0);
          VarsUi.FBPointer++;
        }
        else
        {
          VarsUi.FBTimer = 0;
          VarsUi.FBState = 7;
        }
      }
      break;

      case 5 : // Show text string
      {
        if ((VarsUi.FBText[VarsUi.FBPointer]) && (VarsUi.FBPointer < NO_OF_FEEDBACK_CHARS))
        {
          pMapDisplay->pFunc(DISPLAY_CHAR,TRUE,24 + VarsUi.FBPointer * 6,12,VarsUi.FBText[VarsUi.FBPointer],0);
          VarsUi.FBPointer++;
        }
        else
        {
          if (TextNo2 == 0xFF)
          {
            VarsUi.FBText          = VarsUi.SelectedFilename;
          }
          else
          {
            VarsUi.FBText          = cUiGetString(TextNo2);
          }
          VarsUi.FBPointer         = 0;
          VarsUi.FBState++;
        }
      }
      break;

      case 6 : // Show text string
      {
        if ((VarsUi.FBText[VarsUi.FBPointer]) && (VarsUi.FBPointer < NO_OF_FEEDBACK_CHARS))
        {
          pMapDisplay->pFunc(DISPLAY_CHAR,TRUE,24 + VarsUi.FBPointer * 6,20,VarsUi.FBText[VarsUi.FBPointer],0);
          VarsUi.FBPointer++;
        }
        else
        {
          VarsUi.FBTimer = 0;
          VarsUi.FBState++;
        }
      }
      break;

      case 7 : // Wait if time provided
      {
        if (++VarsUi.FBTimer >= (Time + 100))
        {
          VarsUi.FBState++;
        }
      }
      break;

      default : // Exit
      {
        VarsUi.FBState = 0;
      }
      break;

    }
  }

  return (VarsUi.FBState);
}



//******* cUiFileList ********************************************************

UBYTE     cUiFindNoOfFiles(UBYTE FileType,UBYTE *NoOfFiles)
{
  switch (VarsUi.FNOFState)
  {
    case 0 :
    {
      *NoOfFiles = 0;

      if (FileType >= FILETYPES)
      {
        FileType = FILETYPE_ALL;
      }
      sprintf((char*)VarsUi.FNOFSearchBuffer,"*.%s",TXT_FILE_EXT[FileType]);

      VarsUi.FNOFHandle = pMapLoader->pFunc(FINDFIRST,VarsUi.FNOFSearchBuffer,VarsUi.FNOFNameBuffer,&VarsUi.FNOFLength);
      if (!(VarsUi.FNOFHandle & 0x8000))
      {
        *NoOfFiles = 1;
        VarsUi.FNOFState++;
      }
    }
    break;

    case 1 :
    {
      VarsUi.FNOFHandle = pMapLoader->pFunc(FINDNEXT,(UBYTE*)&VarsUi.FNOFHandle,VarsUi.FNOFNameBuffer,&VarsUi.FNOFLength);
      if (!(VarsUi.FNOFHandle & 0x8000))
      {
        *NoOfFiles += 1;
      }
      else
      {
        pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.FNOFHandle,NULL,NULL);
        VarsUi.FNOFState = 0;
      }
    }
    break;

  }

  return (VarsUi.FNOFState);
}


UBYTE     cUiFindNameForFileNo(UBYTE FileType,UBYTE FileNo,UBYTE *Name)
{
  switch (VarsUi.FNOFState)
  {
    case 0 :
    {
      Name[0] = 0;

      if (FileNo)
      {
        if (FileType >= FILETYPES)
        {
          FileType = FILETYPE_ALL;
        }
        sprintf((char*)VarsUi.FNOFSearchBuffer,"*.%s",TXT_FILE_EXT[FileType]);

        VarsUi.FNOFHandle = pMapLoader->pFunc(FINDFIRST,VarsUi.FNOFSearchBuffer,Name,&VarsUi.FNOFLength);
        if (!(VarsUi.FNOFHandle & 0x8000))
        {
          if (FileNo != 1)
          {
            VarsUi.FNOFFileNo = 1;
            VarsUi.FNOFState++;
          }
          else
          {
            pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.FNOFHandle,NULL,NULL);
          }
        }
      }
    }
    break;

    case 1 :
    {
      VarsUi.FNOFHandle = pMapLoader->pFunc(FINDNEXT,(UBYTE*)&VarsUi.FNOFHandle,Name,&VarsUi.FNOFLength);
      if (!(VarsUi.FNOFHandle & 0x8000))
      {
        VarsUi.FNOFFileNo++;
        if (FileNo == VarsUi.FNOFFileNo)
        {
          pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.FNOFHandle,NULL,NULL);
          VarsUi.FNOFState = 0;
        }
      }
      else
      {
        pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.FNOFHandle,NULL,NULL);
        VarsUi.FNOFState = 0;
      }
    }
    break;

  }

  return (VarsUi.FNOFState);
}


UBYTE     cUiFileList(UBYTE Action)       // Show files and select
{
  switch (Action)
  {
    case MENU_INIT :
    {
      if (!VarsUi.State)
      {
        VarsUi.FileCenter   = 1;
        VarsUi.NextState    = IOMapUi.State;
      }
      Action                = MENU_DRAW;
    }
    break;

    case MENU_LEFT :
    {
      if (!VarsUi.State)
      {
        cUiListLeft(VarsUi.NoOfFiles,&VarsUi.FileCenter);
        VarsUi.NextState    = TEST_BUTTONS;
      }
      Action                = MENU_DRAW;
    }
    break;

    case MENU_RIGHT :
    {
      if (!VarsUi.State)
      {
        cUiListRight(VarsUi.NoOfFiles,&VarsUi.FileCenter);
        VarsUi.NextState    = TEST_BUTTONS;
      }
      Action                = MENU_DRAW;
    }
    break;

    case MENU_SELECT :
    {
    }
    break;

    default :
    {
      if (Action < FILETYPES)
      {
        if (!VarsUi.State)
        {
          VarsUi.FileType   = Action;
          VarsUi.FileCenter = 1;
          VarsUi.NextState  = IOMapUi.State;
        }
        Action              = MENU_DRAW;
      }
      else
      {
        IOMapUi.State       = EXIT_PRESSED;
        VarsUi.State        = 0;
      }
    }
    break;

  }

  if (Action == MENU_DRAW)
  {
    switch (VarsUi.State)
    {
      case 0 :
      {
        VarsUi.FNOFState = 0;
        VarsUi.State++;
      }
      break;

      case 1 :
      {
        if (cUiFindNoOfFiles(VarsUi.FileType,&VarsUi.NoOfFiles) == 0)
        {
          if (VarsUi.NoOfFiles)
          {
            cUiListCalc(VarsUi.NoOfFiles,&VarsUi.FileCenter,&VarsUi.FileLeft,&VarsUi.FileRight);
            VarsUi.State++;
          }
          else
          {
            VarsUi.State  = 0;
            IOMapUi.State = EXIT_PRESSED;
          }
        }
      }
      break;

      case 2 :
      {
        if (cUiFindNameForFileNo(VarsUi.FileType,VarsUi.FileCenter,VarsUi.SelectedFilename) == 0)
        {
          VarsUi.State++;
        }
      }
      break;

      default :
      {
        pMapDisplay->pMenuIcons[MENUICON_LEFT]   = NULL;
        pMapDisplay->pMenuIcons[MENUICON_CENTER] = NULL;
        pMapDisplay->pMenuIcons[MENUICON_RIGHT]  = NULL;

        if (VarsUi.FileLeft)
        {
          pMapDisplay->pMenuIcons[MENUICON_LEFT]    = (UBYTE*)&Icons->Data[(VarsUi.FileType + ALLFILES) * Icons->ItemPixelsX * (Icons->ItemPixelsY / 8)];
          pMapDisplay->UpdateMask                  |= MENUICON_BIT(MENUICON_LEFT);
        }
        if (VarsUi.FileCenter)
        {
          pMapDisplay->pMenuIcons[MENUICON_CENTER]  = (UBYTE*)&Icons->Data[(VarsUi.FileType + ALLFILES) * Icons->ItemPixelsX * (Icons->ItemPixelsY / 8)];
          pMapDisplay->UpdateMask                  |= MENUICON_BIT(MENUICON_CENTER);
        }
        if (VarsUi.FileRight)
        {
          pMapDisplay->pMenuIcons[MENUICON_RIGHT]   = (UBYTE*)&Icons->Data[(VarsUi.FileType + ALLFILES) * Icons->ItemPixelsX * (Icons->ItemPixelsY / 8)];
          pMapDisplay->UpdateMask                  |= MENUICON_BIT(MENUICON_RIGHT);
        }

        pMapDisplay->EraseMask                   |=  TEXTLINE_BIT(TEXTLINE_5);

        // Search forward for termination
        VarsUi.Tmp   = 0;
        while ((VarsUi.SelectedFilename[VarsUi.Tmp]) && (VarsUi.Tmp < FILENAME_LENGTH))
        {
          VarsUi.Tmp++;
        }

        // Search backward for "."
        while ((VarsUi.Tmp) && (VarsUi.SelectedFilename[VarsUi.Tmp] != '.'))
        {
          VarsUi.Tmp--;
        }

        if (VarsUi.Tmp > DISPLAYLINE_LENGTH)
        {
          VarsUi.Tmp = DISPLAYLINE_LENGTH;
        }

        VarsUi.DisplayBuffer[VarsUi.Tmp] = 0;

        // Copy only name not ext
        while (VarsUi.Tmp)
        {
          VarsUi.Tmp--;
          VarsUi.DisplayBuffer[VarsUi.Tmp] = VarsUi.SelectedFilename[VarsUi.Tmp];
        }

        pMapDisplay->pMenuText                    = VarsUi.DisplayBuffer;
        pMapDisplay->EraseMask                   |= MENUICON_BITS;
        pMapDisplay->UpdateMask                  |= (SPECIAL_BIT(FRAME_SELECT) | SPECIAL_BIT(MENUTEXT));

        IOMapUi.State = VarsUi.NextState;
        VarsUi.State  = 0;
      }
      break;

    }
  }

  return (VarsUi.State);
}



//******* cUiVolume **********************************************************

UBYTE     cUiVolume(UBYTE Action) // MENU_INIT,MENU_LEFT,MENU_RIGHT,MENU_EXIT
{
  switch (Action)
  {
    case MENU_INIT : // Init time counter and cursor bitmap
    {
      VarsUi.Counter    = VarsUi.NVData.VolumeStep + 1;

#ifndef STRIPPED
      VarsUi.pTmp       = (UBYTE*)Cursor;
      for (VarsUi.Tmp = 0;(VarsUi.Tmp < SIZE_OF_CURSOR) && (VarsUi.Tmp < (UBYTE)sizeof(Cursor));VarsUi.Tmp++)
      {
        VarsUi.CursorTmp[VarsUi.Tmp] = *VarsUi.pTmp;
        VarsUi.pTmp++;
      }
#endif
      Action            = MENU_DRAW;
    }
    break;

    case MENU_LEFT : // Dec
    {
      cUiListLeft(MAX_VOLUME + 1,&VarsUi.Counter);
      IOMapUi.Volume    = VarsUi.Counter - 1;
      Action            = MENU_DRAW;
    }
    break;

    case MENU_RIGHT : // Inc
    {
      cUiListRight(MAX_VOLUME + 1,&VarsUi.Counter);
      IOMapUi.Volume    = VarsUi.Counter - 1;
      Action            = MENU_DRAW;
    }
    break;

    case MENU_ENTER : // Enter
    {
      VarsUi.NVData.VolumeStep = VarsUi.Counter - 1;
      cUiNVWrite();
      IOMapUi.Volume    = VarsUi.NVData.VolumeStep;
      pMapSound->Volume = IOMapUi.Volume;
      Action            = MENU_EXIT;
    }
    break;

    case MENU_EXIT : // Leave
    {
      IOMapUi.Volume    = VarsUi.NVData.VolumeStep;
    }
    break;

  }
  if (Action == MENU_DRAW)
  {
    sprintf((char*)VarsUi.DisplayBuffer,"%u",(UWORD)VarsUi.Counter - 1);
    pMapDisplay->pTextLines[TEXTLINE_3] = VarsUi.DisplayBuffer;

#ifndef STRIPPED
    pMapDisplay->pBitmaps[BITMAP_1]     = (BMPMAP*)VarsUi.CursorTmp;
    VarsUi.CursorTmp[4] = 46;
    VarsUi.CursorTmp[5] = 24;
#endif
    pMapDisplay->EraseMask             |= (TEXTLINE_BIT(TEXTLINE_3) | TEXTLINE_BIT(TEXTLINE_4));
    pMapDisplay->TextLinesCenterFlags  |= TEXTLINE_BIT(TEXTLINE_3);
    pMapDisplay->UpdateMask            |= (TEXTLINE_BIT(TEXTLINE_3) | BITMAP_BIT(BITMAP_1));
  }
  if (Action == MENU_EXIT)
  {
    IOMapUi.State       = EXIT_PRESSED;
  }

  return (0);
}



//******* cUiGetUserString ***************************************************

#define   STRINGTYPES       2

#define   TOPTEXT_LINE      TEXTLINE_3
#define   STRING_LINE       TEXTLINE_5

typedef   struct
{
  const   UBYTE Text;
  const   UBYTE *Figures;
  const   UBYTE NoOfFigures;
  const   UBYTE MaxStringLength;
  const   UBYTE WindowSize;
  const   SBYTE DefaultPointer;
}
STRSETS;

const     UBYTE Figures[] = { "0987654321" "\x7F" "abcdefghijklmnopqrstuvwxyz         " };

const     STRSETS StrSets[STRINGTYPES] =
{
  { TXT_GETUSERSTRING_PIN,      Figures,  37, SIZE_OF_BT_PINCODE - 1, 15, 10 },
  { TXT_GETUSERSTRING_FILENAME, Figures, 37, FILENAME_LENGTH - 4   , 15, 10 }
};


UBYTE     cUiGetUserString(UBYTE Type)  // 0=Pincode, 1=filename
{
  UBYTE   Tmp1;
  SBYTE   Tmp2;

  if (Type < STRINGTYPES)
  {
    switch (VarsUi.GUSState)
    {
      case 0 :  // Init screen
      {
        // Disable update and prepare screen
        pMapDisplay->EraseMask              |=  SCREEN_BIT(SCREEN_LARGE);
        pMapDisplay->pBitmaps[BITMAP_1]      = (BMPMAP*)Ok;

        // Set figure pointer to default
        VarsUi.FigurePointer = (SBYTE)StrSets[Type].DefaultPointer;

        // Calculate cursor from default string
        VarsUi.GUSCursor  = 0;
        while ((VarsUi.GUSCursor < DISPLAYLINE_LENGTH) && VarsUi.UserString[VarsUi.GUSCursor])
        {
          VarsUi.GUSCursor++;
        }
        VarsUi.GUSNoname = TRUE;

        VarsUi.GUSState++;
      }
      break;

      case 1 :  // Update user string
      {
        if (!(pMapDisplay->EraseMask & SCREEN_BIT(SCREEN_LARGE)))
        {
          // Display top text
          pMapDisplay->pTextLines[TOPTEXT_LINE]  = cUiGetString(StrSets[Type].Text);
          pMapDisplay->UpdateMask               |= TEXTLINE_BIT(TOPTEXT_LINE);

          Tmp1 = 0;
          while (VarsUi.UserString[Tmp1] && (Tmp1 < StrSets[Type].MaxStringLength))
          {
            VarsUi.DisplayText[Tmp1] = VarsUi.UserString[Tmp1];
            Tmp1++;
          }
          if (Tmp1 < StrSets[Type].MaxStringLength)
          {
            VarsUi.DisplayText[Tmp1] = '_';
            Tmp1++;
          }
          while (Tmp1 < StrSets[Type].MaxStringLength)
          {
            VarsUi.DisplayText[Tmp1] = ' ';
            Tmp1++;
          }
          VarsUi.DisplayText[Tmp1] = 0;

          pMapDisplay->pTextLines[STRING_LINE] = VarsUi.DisplayText;
          pMapDisplay->UpdateMask             |= (TEXTLINE_BIT(STRING_LINE) | SPECIAL_BIT(TOPLINE));
          pMapDisplay->EraseMask              |= BITMAP_BIT(BITMAP_1);
          VarsUi.GUSState++;
        }
      }
      break;

      case 2 :  // Update figure string
      {
        if (!(pMapDisplay->EraseMask & BITMAP_BIT(BITMAP_1)))
        {
          Tmp2 = VarsUi.FigurePointer;

          for (Tmp1 = 0;Tmp1 < 3;Tmp1++)
          {
            if (Tmp2)
            {
              Tmp2--;
            }
            else
            {
              Tmp2 = StrSets[Type].NoOfFigures - 1;
            }
          }
          for (Tmp1 = 0;Tmp1 < 7;Tmp1++)
          {
            if ((Tmp1 == 3) && (StrSets[Type].Figures[Tmp2] == 0x7F))
            {
              pMapDisplay->UpdateMask |= BITMAP_BIT(BITMAP_1);
            }
            else
            {
              pMapDisplay->pFunc(DISPLAY_CHAR,TRUE,5 + Tmp1 * 14,52,StrSets[Type].Figures[Tmp2],0);
            }
            if (Tmp2 < (StrSets[Type].NoOfFigures - 1))
            {
              Tmp2++;
            }
            else
            {
              Tmp2 = 0;
            }
          }
          pMapDisplay->pFunc(DISPLAY_HORIZONTAL_LINE,TRUE,42,47,57,0);
          pMapDisplay->pFunc(DISPLAY_VERTICAL_LINE,TRUE,42,47,0,63);
          pMapDisplay->pFunc(DISPLAY_VERTICAL_LINE,TRUE,57,47,0,63);

          VarsUi.GUSState++;
        }
      }
      break;

      case 3 :  // Get user input
      {
        if ((pMapButton->State[BTN4] & LONG_PRESSED_EV))
        {
          if (VarsUi.GUSCursor)
          {
            if ((VarsUi.UserString[VarsUi.GUSCursor - 1] >= 'a') && (VarsUi.UserString[VarsUi.GUSCursor - 1] <= 'z'))
            {
              VarsUi.UserString[VarsUi.GUSCursor - 1] -= ' ';
              VarsUi.GUSState -= 2;
            }
          }
        }

        switch (cUiReadButtons())
        {
          case BUTTON_LEFT :
          {
            if (VarsUi.FigurePointer)
            {
              VarsUi.FigurePointer--;
            }
            else
            {
              VarsUi.FigurePointer = StrSets[Type].NoOfFigures - 1;
            }
            pMapDisplay->EraseMask |= BITMAP_BIT(BITMAP_1);
            VarsUi.GUSState -= 2;
          }
          break;

          case BUTTON_ENTER :
          {
            switch (StrSets[Type].Figures[VarsUi.FigurePointer])
            {
              case 0x7F :
              {
                VarsUi.GUSState = 100;
              }
              break;

              default :
              {
                VarsUi.GUSNoname = FALSE;
                if (VarsUi.GUSCursor < StrSets[Type].MaxStringLength)
                {
                  VarsUi.UserString[VarsUi.GUSCursor] = StrSets[Type].Figures[VarsUi.FigurePointer];
                  VarsUi.GUSCursor++;
                  VarsUi.UserString[VarsUi.GUSCursor] = 0;
                  VarsUi.GUSState -= 2;
                }
              }
              break;

            }
          }
          break;

          case BUTTON_RIGHT :
          {
            if (VarsUi.FigurePointer < (StrSets[Type].NoOfFigures - 1))
            {
              VarsUi.FigurePointer++;
            }
            else
            {
              VarsUi.FigurePointer = 0;
            }
            pMapDisplay->EraseMask |= BITMAP_BIT(BITMAP_1);
            VarsUi.GUSState -= 2;
          }
          break;

          case BUTTON_EXIT :
          {
            if (VarsUi.GUSCursor)
            {
              if (VarsUi.GUSNoname == TRUE)
              {
                VarsUi.GUSNoname = FALSE;
                while (VarsUi.GUSCursor)
                {
                  VarsUi.UserString[VarsUi.GUSCursor] = 0;
                  VarsUi.GUSCursor--;
                }
              }
              else
              {
                VarsUi.GUSCursor--;
              }
              VarsUi.UserString[VarsUi.GUSCursor] = 0;
              VarsUi.GUSState -= 2;
            }
            else
            {
              VarsUi.UserString[0] = 0;
              VarsUi.GUSState = 100;
            }
          }
          break;

        }
      }
      break;

      default : // Clean up screen
      {
        pMapDisplay->EraseMask |= SCREEN_BIT(SCREEN_BACKGROUND);
        pMapDisplay->UpdateMask = 0;
        IOMapUi.Flags          |= UI_REDRAW_STATUS;
        VarsUi.GUSState         = 0;
      }
      break;
    }
  }

  return (VarsUi.GUSState);
}



//******* cUiDataLogging *****************************************************

void      cUiDrawPortNo(UBYTE *Bitmap,UBYTE MenuIconNo,UBYTE PortNo)
{
  UBYTE   Tmp;

  Bitmap[0] = (UBYTE)(FILEFORMAT_BITMAP >> 8);
  Bitmap[1] = (UBYTE)(FILEFORMAT_BITMAP);
  Bitmap[2] = (UBYTE)(SIZE_OF_PORTBITMAP >> 8);
  Bitmap[3] = (UBYTE)(SIZE_OF_PORTBITMAP);
  Bitmap[4] = DISPLAY_MENUICONS_X_OFFS + DISPLAY_MENUICONS_X_DIFF * MenuIconNo + 2;
  Bitmap[5] = DISPLAY_MENUICONS_Y;
  Bitmap[6] = Port[0].ItemPixelsX;
  Bitmap[7] = Port[0].ItemPixelsY;

  Tmp = 0;
  while (Tmp < Bitmap[6])
  {
    Bitmap[Tmp + FILEHEADER_LENGTH] = Port[0].Data[Tmp + PortNo * Bitmap[6]];
    Tmp++;
  }

}

UBYTE     cUiDataLogging(UBYTE Action)
{
#ifndef STRIPPED
  SBYTE   TmpBuffer[DATALOGBUFFERSIZE + 1];
  
  switch (Action)
  {
    case MENU_INIT : // Initialize all ports to empty
    {
// Show select
      pMapDisplay->pTextLines[TEXTLINE_3] = cUiGetString(TXT_GENERIC_SELECT);
      pMapDisplay->TextLinesCenterFlags  |= TEXTLINE_BIT(TEXTLINE_3);
      pMapDisplay->UpdateMask            |= TEXTLINE_BIT(TEXTLINE_3);
      pMapDisplay->EraseMask             |= SCREEN_BIT(SCREEN_SMALL);

// Init ports
      for (VarsUi.Tmp = 0;VarsUi.Tmp < DATALOGPORTS;VarsUi.Tmp++)
      {
        VarsUi.DatalogPort[VarsUi.Tmp] = MENU_SENSOR_EMPTY;
      }
    }
    break;

    case MENU_EXIT : // Initialize all ports to empty
    {
// Show select
      pMapDisplay->pTextLines[TEXTLINE_3] = cUiGetString(TXT_GENERIC_SELECT);
      pMapDisplay->TextLinesCenterFlags  |= TEXTLINE_BIT(TEXTLINE_3);
      pMapDisplay->UpdateMask            |= TEXTLINE_BIT(TEXTLINE_3);
      pMapDisplay->EraseMask             |= SCREEN_BIT(SCREEN_SMALL);
    }
    break;

    case MENU_TEXT : // Write text
    {
// Init selected sensor and port to none
      VarsUi.SelectedSensor    = MENU_SENSOR_EMPTY;
      VarsUi.SelectedPort      = MENU_PORT_EMPTY;
// Count ports
      VarsUi.Counter           = 0;
      for (VarsUi.Tmp = 0;VarsUi.Tmp < DATALOGPORTS;VarsUi.Tmp++)
      {
        if (MENU_SENSOR_EMPTY != VarsUi.DatalogPort[VarsUi.Tmp])
        {
// Find default port to view
          if (VarsUi.SelectedPort == MENU_PORT_EMPTY)
          {
            VarsUi.SelectedPort   = VarsUi.Tmp + MENU_PORT_1;
          }
          VarsUi.Counter++;
        }
      }
      if (VarsUi.Counter)
      {
// Display text
        pMapDisplay->pTextLines[TEXTLINE_3] = cUiGetString(TXT_DATALOGGING_PRESS_EXIT_TO);
        pMapDisplay->pTextLines[TEXTLINE_4] = cUiGetString(TXT_DATALOGGING_STOP_DATALOGGING);

        pMapDisplay->TextLinesCenterFlags  |= (TEXTLINE_BIT(TEXTLINE_3) | TEXTLINE_BIT(TEXTLINE_4));
        pMapDisplay->UpdateMask            |= (TEXTLINE_BIT(TEXTLINE_3) | TEXTLINE_BIT(TEXTLINE_4));
      }
      else
      {
        cUiMenuPrevFile();
        IOMapUi.State = NEXT_MENU;
        VarsUi.State  = 0;
      }
    }
    break;

    case MENU_RUN : // Run data logging
    {
      switch (VarsUi.State)
      {
        case 0 : // Init log
        {
// Save menu text
          VarsUi.MenuIconTextSave  = pMapDisplay->pMenuText;

// Delete file if exist
          sprintf((char*)VarsUi.FilenameBuffer,"%s.%s",(char*)TEMP_DATALOG_FILENAME,(char*)TXT_FILE_EXT[FILETYPE_DATALOG]);
          VarsUi.TmpHandle = pMapLoader->pFunc(FINDFIRST,VarsUi.FilenameBuffer,VarsUi.SearchFilenameBuffer,&VarsUi.TmpLength);
          if (!(VarsUi.TmpHandle & 0x8000))
          {
            pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.TmpHandle,NULL,NULL);
            pMapLoader->pFunc(DELETE,VarsUi.FilenameBuffer,NULL,NULL);
          }

// Open file
          VarsUi.TmpLength = pMapLoader->FreeUserFlash;
          sprintf((char*)VarsUi.FilenameBuffer,"%s.%s",(char*)TEMP_DATALOG_FILENAME,(char*)TXT_FILE_EXT[FILETYPE_DATALOG]);
          VarsUi.TmpHandle    = pMapLoader->pFunc(OPENWRITEDATA,VarsUi.FilenameBuffer,NULL,&VarsUi.TmpLength);
          VarsUi.DatalogError = VarsUi.TmpHandle;
          if (!(VarsUi.DatalogError & 0x8000))
          {
            VarsUi.TmpLength      = (ULONG)sprintf((char*)TmpBuffer,"%s\t%lu",SENSORSYNCDATA,pMapCmd->SyncTime); 
            VarsUi.DatalogError  |= pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)TmpBuffer,&VarsUi.TmpLength);

            VarsUi.TmpLength      = (ULONG)sprintf((char*)TmpBuffer,"\t%lu",pMapCmd->SyncTick); 
            VarsUi.DatalogError  |= pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)TmpBuffer,&VarsUi.TmpLength);

            VarsUi.TmpLength      = (ULONG)sprintf((char*)TmpBuffer,"\t%lu",pMapCmd->Tick); 
            VarsUi.DatalogError  |= pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)TmpBuffer,&VarsUi.TmpLength);

            VarsUi.TmpLength      = (ULONG)sprintf((char*)TmpBuffer,"\t%lu\t-1\r\n",DATALOG_DEFAULT_SAMPLE_TIME); 
            VarsUi.DatalogError  |= pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)TmpBuffer,&VarsUi.TmpLength);

            VarsUi.TmpLength      = (ULONG)sprintf((char*)TmpBuffer,"%s",SENSORSDATA); 
            VarsUi.DatalogError  |= pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)TmpBuffer,&VarsUi.TmpLength);
            for (VarsUi.Tmp = 0;(VarsUi.Tmp < DATALOGPORTS) && (!(VarsUi.DatalogError & 0x8000));VarsUi.Tmp++)
            {
              if (MENU_SENSOR_EMPTY != VarsUi.DatalogPort[VarsUi.Tmp])
              {
                VarsUi.TmpLength      = (ULONG)sprintf((char*)TmpBuffer,"\t%u_%s%s",(UWORD)(VarsUi.Tmp + 1),(char*)SENSORDIRNAME[(VarsUi.DatalogPort[VarsUi.Tmp] - MENU_SENSOR_EMPTY) - 1],(char*)SENSORUNITNAME[(VarsUi.DatalogPort[VarsUi.Tmp] - MENU_SENSOR_EMPTY) - 1]); 
                VarsUi.DatalogError  |= pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)TmpBuffer,&VarsUi.TmpLength);
              }
            }
            VarsUi.TmpLength      = (ULONG)sprintf((char*)TmpBuffer,"\r\n"); 
            VarsUi.DatalogError  |= pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)TmpBuffer,&VarsUi.TmpLength);
            VarsUi.TmpLength      = (ULONG)sprintf((char*)TmpBuffer,"%s",SENSORTIME); 
            VarsUi.DatalogError  |= pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)TmpBuffer,&VarsUi.TmpLength);
            for (VarsUi.Tmp = 0;(VarsUi.Tmp < DATALOGPORTS) && (!(VarsUi.DatalogError & 0x8000));VarsUi.Tmp++)
            {
              if (MENU_SENSOR_EMPTY != VarsUi.DatalogPort[VarsUi.Tmp])
              {
                VarsUi.TmpLength      = (ULONG)sprintf((char*)TmpBuffer,"\t%s",(char*)SENSORDIRNAME[(VarsUi.DatalogPort[VarsUi.Tmp] - MENU_SENSOR_EMPTY) - 1]); 
                VarsUi.DatalogError  |= pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)TmpBuffer,&VarsUi.TmpLength);
              }
            }
            VarsUi.TmpLength      = (ULONG)sprintf((char*)TmpBuffer,"\r\n"); 
            VarsUi.DatalogError  |= pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)TmpBuffer,&VarsUi.TmpLength);
            if (!(VarsUi.DatalogError & 0x8000))
            {
              VarsUi.DatalogTimer       = 0;
              VarsUi.DatalogSampleTime  = DATALOG_DEFAULT_SAMPLE_TIME;
              VarsUi.DatalogSampleTimer = 0;
              VarsUi.Timer              = 0;
              VarsUi.Update             = TRUE;
              IOMapUi.Flags            |= UI_BUSY;
              VarsUi.DatalogOldTick     = pMapCmd->Tick;
              VarsUi.SensorReset        = TRUE;
              VarsUi.State++;
            }
            else
            {
              pMapDisplay->EraseMask  |= SCREEN_BIT(SCREEN_SMALL);
              pMapDisplay->pBitmaps[BITMAP_1]          = NULL;
              VarsUi.State = 4;
            }
          }
          else
          {
// File error
            pMapDisplay->EraseMask  |= SCREEN_BIT(SCREEN_SMALL);
            pMapDisplay->pBitmaps[BITMAP_1]          = NULL;
            VarsUi.State = 3;
          }
        }
        break;

        case 1 :
        {
// Get real time since last
          VarsUi.DatalogRTC          = (pMapCmd->Tick - VarsUi.DatalogOldTick);
          VarsUi.DatalogOldTick      = pMapCmd->Tick;
// Update all timers
          VarsUi.DatalogTimer       += VarsUi.DatalogRTC;
          VarsUi.DatalogSampleTimer += VarsUi.DatalogRTC;
          VarsUi.ReadoutTimer       += VarsUi.DatalogRTC;
// Update sensor values
          cUiUpdateSensor((SWORD)VarsUi.DatalogRTC);
// Check for select change
          if (VarsUi.Update == TRUE)
          {
            VarsUi.Update = FALSE;
            VarsUi.SelectedSensor                    = VarsUi.DatalogPort[VarsUi.SelectedPort - MENU_PORT_1];
            pMapDisplay->pMenuIcons[MENUICON_CENTER] = cUiMenuGetIconImage(cUiMenuSearchSensorIcon(VarsUi.SelectedSensor));
            pMapDisplay->pMenuIcons[MENUICON_LEFT]   = NULL;
            pMapDisplay->pMenuIcons[MENUICON_RIGHT]  = NULL;

            pMapDisplay->EraseMask                   = SCREEN_BIT(SCREEN_LARGE);
            pMapDisplay->pBitmaps[BITMAP_1]          = (BMPMAP*)Display;
            pMapDisplay->UpdateMask                  = (BITMAP_BIT(BITMAP_1) | MENUICON_BITS | SPECIAL_BIT(TOPLINE) | SPECIAL_BIT(FRAME_SELECT));

            pMapDisplay->pBitmaps[BITMAP_2]          = (BMPMAP*)VarsUi.PortBitmapLeft;
            pMapDisplay->pBitmaps[BITMAP_3]          = (BMPMAP*)VarsUi.PortBitmapCenter;
            pMapDisplay->pBitmaps[BITMAP_4]          = (BMPMAP*)VarsUi.PortBitmapRight;

            cUiDrawPortNo(VarsUi.PortBitmapCenter,MENUICON_CENTER,VarsUi.SelectedPort - MENU_PORT_EMPTY);
            pMapDisplay->UpdateMask                 |= BITMAP_BIT(BITMAP_3);



            if (VarsUi.Counter == 2)
            {
              VarsUi.Tmp = VarsUi.SelectedPort;
              do
              {
                VarsUi.Tmp++;
                if (VarsUi.Tmp >= MENU_PORT_INVALID)
                {
                  VarsUi.Tmp = MENU_PORT_1;
                }
              }
              while (VarsUi.DatalogPort[VarsUi.Tmp - MENU_PORT_1] == MENU_SENSOR_EMPTY);
              if (VarsUi.Tmp > VarsUi.SelectedPort)
              {
                pMapDisplay->pMenuIcons[MENUICON_RIGHT] = cUiMenuGetIconImage(cUiMenuSearchSensorIcon(VarsUi.DatalogPort[VarsUi.Tmp - MENU_PORT_1]));
                cUiDrawPortNo(VarsUi.PortBitmapRight,MENUICON_RIGHT,VarsUi.Tmp - MENU_PORT_EMPTY);
                pMapDisplay->UpdateMask                |= BITMAP_BIT(BITMAP_4);
              }
              else
              {
                pMapDisplay->pMenuIcons[MENUICON_LEFT]  = cUiMenuGetIconImage(cUiMenuSearchSensorIcon(VarsUi.DatalogPort[VarsUi.Tmp - MENU_PORT_1]));
                cUiDrawPortNo(VarsUi.PortBitmapLeft,MENUICON_LEFT,VarsUi.Tmp - MENU_PORT_EMPTY);
                pMapDisplay->UpdateMask                |= BITMAP_BIT(BITMAP_2);
              }
            }
            if (VarsUi.Counter > 2)
            {
              VarsUi.Tmp = VarsUi.SelectedPort;
              do
              {
                VarsUi.Tmp++;
                if (VarsUi.Tmp >= MENU_PORT_INVALID)
                {
                  VarsUi.Tmp = MENU_PORT_1;
                }
              }
              while (VarsUi.DatalogPort[VarsUi.Tmp - MENU_PORT_1] == MENU_SENSOR_EMPTY);
              pMapDisplay->pMenuIcons[MENUICON_RIGHT] = cUiMenuGetIconImage(cUiMenuSearchSensorIcon(VarsUi.DatalogPort[VarsUi.Tmp - MENU_PORT_1]));
              cUiDrawPortNo(VarsUi.PortBitmapRight,MENUICON_RIGHT,VarsUi.Tmp - MENU_PORT_EMPTY);
              pMapDisplay->UpdateMask                |= BITMAP_BIT(BITMAP_4);

              VarsUi.Tmp = VarsUi.SelectedPort;
              do
              {
                VarsUi.Tmp--;
                if (VarsUi.Tmp <= MENU_PORT_EMPTY)
                {
                  VarsUi.Tmp = MENU_PORT_INVALID - 1;
                }
              }
              while (VarsUi.DatalogPort[VarsUi.Tmp - MENU_PORT_1] == MENU_SENSOR_EMPTY);
              pMapDisplay->pMenuIcons[MENUICON_LEFT]  = cUiMenuGetIconImage(cUiMenuSearchSensorIcon(VarsUi.DatalogPort[VarsUi.Tmp - MENU_PORT_1]));
              cUiDrawPortNo(VarsUi.PortBitmapLeft,MENUICON_LEFT,VarsUi.Tmp - MENU_PORT_EMPTY);
              pMapDisplay->UpdateMask                |= BITMAP_BIT(BITMAP_2);


            }
            VarsUi.ReadoutTimer                       = DISPLAY_VIEW_UPDATE;
          }
// Write sample if timeout
          if (VarsUi.DatalogSampleTimer >= VarsUi.DatalogSampleTime)
          {
            VarsUi.DatalogSampleTimer -= VarsUi.DatalogSampleTime;

            VarsUi.TmpLength      = (ULONG)sprintf((char*)TmpBuffer,"%lu",VarsUi.DatalogTimer - VarsUi.DatalogSampleTime); 
            VarsUi.DatalogError  |= pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)TmpBuffer,&VarsUi.TmpLength);
            for (VarsUi.Tmp = 0;(VarsUi.Tmp < DATALOGPORTS) && (!(VarsUi.DatalogError & 0x8000));VarsUi.Tmp++)
            {
              if (MENU_SENSOR_EMPTY != VarsUi.DatalogPort[VarsUi.Tmp])
              {
                if (VarsUi.DatalogSampleValid[VarsUi.Tmp] == TRUE)
                {
                  VarsUi.TmpLength      = (ULONG)sprintf((char*)TmpBuffer,(char*)SENSORFORMAT2[(VarsUi.DatalogPort[VarsUi.Tmp] - MENU_SENSOR_EMPTY) - 1],(float)VarsUi.DatalogSampleValue[VarsUi.Tmp] / SENSORDIVIDER[VarsUi.DatalogPort[VarsUi.Tmp] - MENU_SENSOR_EMPTY]); 
                  VarsUi.DatalogError  |= pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)TmpBuffer,&VarsUi.TmpLength);
                }
                else
                {
                  VarsUi.TmpLength      = (ULONG)sprintf((char*)TmpBuffer,"\t-"); 
                  VarsUi.DatalogError  |= pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)TmpBuffer,&VarsUi.TmpLength);
                }
              }
            }
            VarsUi.TmpLength      = (ULONG)sprintf((char*)TmpBuffer,"\r\n"); 
            VarsUi.DatalogError  |= pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)TmpBuffer,&VarsUi.TmpLength);
          }
// Refresh display
          if (++VarsUi.ReadoutTimer >= DISPLAY_VIEW_UPDATE)
          {
            VarsUi.ReadoutTimer = 0;

// Display sensor value
            cUiPrintSensorInDisplayBuffer(VarsUi.SelectedPort);
            pMapDisplay->pTextLines[TEXTLINE_4]      = VarsUi.DisplayBuffer;
            pMapDisplay->TextLinesCenterFlags       |= TEXTLINE_BIT(TEXTLINE_4);
            pMapDisplay->UpdateMask                 |= TEXTLINE_BIT(TEXTLINE_4);
          }

// Test for file error
          if ((VarsUi.DatalogError & 0x8000))
          {
            pMapDisplay->EraseMask  |= SCREEN_BIT(SCREEN_SMALL);
            pMapDisplay->pBitmaps[BITMAP_1]          = NULL;
            VarsUi.State = 4;
          }

// Test for break;
          switch (cUiReadButtons())
          {
            case BUTTON_EXIT :
            {
              VarsUi.State++;
            }
            break;

            case BUTTON_LEFT :
            {
              VarsUi.Tmp = VarsUi.SelectedPort;
              do
              {
                VarsUi.Tmp--;
                if (VarsUi.Tmp <= MENU_PORT_EMPTY)
                {
                  VarsUi.Tmp = MENU_PORT_INVALID - 1;
                }
              }
              while (VarsUi.DatalogPort[VarsUi.Tmp - MENU_PORT_1] == MENU_SENSOR_EMPTY);
              if ((VarsUi.Counter > 2) || (VarsUi.Tmp < VarsUi.SelectedPort))
              {
                VarsUi.SelectedPort = VarsUi.Tmp;
              }
              VarsUi.Update = TRUE;
            }
            break;

            case BUTTON_RIGHT :
            {
              VarsUi.Tmp = VarsUi.SelectedPort;
              do
              {
                VarsUi.Tmp++;
                if (VarsUi.Tmp >= MENU_PORT_INVALID)
                {
                  VarsUi.Tmp = MENU_PORT_1;
                }
              }
              while (VarsUi.DatalogPort[VarsUi.Tmp - MENU_PORT_1] == MENU_SENSOR_EMPTY);
              if ((VarsUi.Counter > 2) || (VarsUi.Tmp > VarsUi.SelectedPort))
              {
                VarsUi.SelectedPort = VarsUi.Tmp;
              }
              VarsUi.Update = TRUE;
            }
            break;

          }
          IOMapUi.Flags    |= UI_RESET_SLEEP_TIMER;
        }
        break;

        case 2 :
        {
// Close file
          pMapLoader->pFunc(CROPDATAFILE,(UBYTE*)&VarsUi.TmpHandle,NULL,NULL);

// Clean up
          pMapDisplay->pMenuText                    = VarsUi.MenuIconTextSave;
          cUiReleaseSensors();

          IOMapUi.Flags &= ~UI_BUSY;
          IOMapUi.State  =  RIGHT_PRESSED;
          VarsUi.State   = 0;
        }
        break;

        case 3 : // Display memory full text
        {
          if (!cUiFeedback((BMPMAP*)Fail,TXT_FB_DL_ERROR_MEMORY_FULL_1,TXT_FB_DL_ERROR_MEMORY_FULL_2,DISPLAY_SHOW_ERROR_TIME))
          {
            cUiMenuPrevFile();
            IOMapUi.State = NEXT_MENU;
            VarsUi.State  = 0;
          }
        }
        break;

        case 4 : // Display memory full text
        {
          if (!cUiFeedback((BMPMAP*)Fail,TXT_FB_DL_ERROR_MEMORY_FULL_1,TXT_FB_DL_ERROR_MEMORY_FULL_2,DISPLAY_SHOW_ERROR_TIME))
          {
            VarsUi.State  = 2;
          }
        }
        break;

      }
    }
    break;

    case MENU_SAVE :                    // Save datalog file
    {
      switch (VarsUi.State)
      {
        case 0 :
        {
          VarsUi.NVData.DatalogNumber++;
          if (VarsUi.NVData.DatalogNumber > MAX_DATALOGS)
          {
            VarsUi.NVData.DatalogNumber = 1;
          }
          cUiNVWrite();
          sprintf((char*)VarsUi.SelectedFilename,"%s%u.%s",(char*)UI_DATALOG_FILENAME,VarsUi.NVData.DatalogNumber,TXT_FILE_EXT[FILETYPE_DATALOG]);
          VarsUi.State++;
        }
        break;

        case 1 :
        {
// Rename TEMP_DATALOG_FILENAME to VarsUi.SelectedFilename(user filename)
          sprintf((char*)VarsUi.FilenameBuffer,"%s.%s",(char*)TEMP_DATALOG_FILENAME,(char*)TXT_FILE_EXT[FILETYPE_DATALOG]);
          VarsUi.TmpHandle = pMapLoader->pFunc(RENAMEFILE,VarsUi.FilenameBuffer,VarsUi.SelectedFilename,&VarsUi.TmpLength);
          VarsUi.State++;
        }
        break;

        case 2 : // Display saved text
        {
          if (!cUiFeedback((BMPMAP*)Info,TXT_FB_FILE_SAVED_INFO,0xFF,DISPLAY_SHOW_FILENAME_TIME))
          {
            VarsUi.State++;
          }
        }
        break;

        default :
        {
          cUiMenuPrevFile();
          IOMapUi.State = NEXT_MENU;
          VarsUi.State  = 0;
        }
        break;

      }
    }
    break;

    case MENU_DELETE : // Delete datalog file
    {
      switch (VarsUi.State)
      {
        case 0 :
        {
// Delete file if exist
          sprintf((char*)VarsUi.FilenameBuffer,"%s.%s",(char*)TEMP_DATALOG_FILENAME,(char*)TXT_FILE_EXT[FILETYPE_DATALOG]);
          pMapLoader->pFunc(DELETE,VarsUi.FilenameBuffer,NULL,NULL);
          VarsUi.State++;
        }
        break;

        case 1 :
        {
          pMapDisplay->EraseMask             |= (TEXTLINE_BIT(TEXTLINE_3) | TEXTLINE_BIT(TEXTLINE_4) | TEXTLINE_BIT(TEXTLINE_5) | MENUICON_BITS | SPECIAL_BIT(MENUTEXT));
          VarsUi.Timer                        = DISPLAY_SHOW_TIME;
          VarsUi.State++;
        }
        break;

        case 2 :
        {
          if (++VarsUi.Timer >= DISPLAY_SHOW_TIME)
          {
            pMapDisplay->EraseMask           |= TEXTLINE_BIT(TEXTLINE_3);
            VarsUi.State++;
          }
        }
        break;

        default :
        {
          VarsUi.State  = 0;
        }
        break;

      }
    }
    break;

    case MENU_SELECT : // Save sensor
    {
      pMapDisplay->pTextLines[TEXTLINE_3] = cUiGetString(TXT_GENERIC_SELECT);
      pMapDisplay->TextLinesCenterFlags  |= TEXTLINE_BIT(TEXTLINE_3);
      pMapDisplay->UpdateMask            |= TEXTLINE_BIT(TEXTLINE_3);

      VarsUi.DatalogPort[VarsUi.SelectedPort - MENU_PORT_1]        = VarsUi.SelectedSensor;
      IOMapUi.State = EXIT_PRESSED;
    }
    break;

    default :
    {
      switch (VarsUi.State)
      {
        case 0 :
        {
          if ((Action > MENU_SENSOR_EMPTY) && (Action < MENU_SENSOR_INVALID))
          {
            VarsUi.SelectedSensor = Action;
          }
          if ((Action > MENU_PORT_EMPTY) && (Action < MENU_PORT_INVALID))
          {
            VarsUi.SelectedPort = Action;
            if (VarsUi.DatalogPort[VarsUi.SelectedPort - MENU_PORT_1] != MENU_SENSOR_EMPTY)
            {

    // Port occupied
              pMapDisplay->pTextLines[TEXTLINE_4] = cUiGetString(TXT_DATALOGGING_PORT_OCCUPIED);
              pMapDisplay->TextLinesCenterFlags  |= TEXTLINE_BIT(TEXTLINE_4);
              pMapDisplay->UpdateMask            |= TEXTLINE_BIT(TEXTLINE_4);
              VarsUi.Timer = 0;
              VarsUi.State++;
            }
          }
        }
        break;

        default :
        {
          if ((++VarsUi.Timer >= DISPLAY_SHOW_TIME) || (BUTTON_NONE != cUiReadButtons()))
          {
            pMapDisplay->EraseMask |= TEXTLINE_BIT(TEXTLINE_4);
            cUiMenuPrev();
            IOMapUi.State           = NEXT_MENU;
            VarsUi.State            = 0;
          }
        }
        break;

      }
    }
    break;

  }
#endif
  return (VarsUi.State);
}


//******* cUiRunning **********************************************************

void      cUiRunning(UBYTE Action)
{
  switch (Action)
  {
    case MENU_INIT :
    {
      VarsUi.RunIconSave                       = pMapDisplay->pMenuIcons[MENUICON_CENTER];
      VarsUi.RunBitmapPointer                  = 0;
      VarsUi.RunTimer                          = 0;
      pMapDisplay->EraseMask                  |= SCREEN_BIT(SCREEN_LARGE);
      pMapDisplay->UpdateMask                 |= SPECIAL_BIT(TOPLINE);
    }
    break;

    case MENU_RUN :
    {
      if ((IOMapUi.Flags & UI_ENABLE_STATUS_UPDATE))
      {
        if (++VarsUi.RunTimer >= RUN_BITMAP_CHANGE_TIME)
        {
          VarsUi.RunTimer    = 0;
          if (++VarsUi.RunBitmapPointer >= Running->ItemsY )
          {
            VarsUi.RunBitmapPointer      = 0;
          }
          pMapDisplay->pMenuIcons[MENUICON_CENTER]  = (UBYTE*)&Running->Data[VarsUi.RunBitmapPointer * Running->ItemPixelsX * (Running->ItemPixelsY / 8)];
          pMapDisplay->EraseMask                   |= MENUICON_BIT(MENUICON_CENTER);
          pMapDisplay->UpdateMask                  |= MENUICON_BIT(MENUICON_CENTER);
        }
      }
    }
    break;

    case MENU_UPDATE :
    {
      pMapDisplay->pMenuIcons[MENUICON_CENTER]  = (UBYTE*)&Running->Data[VarsUi.RunBitmapPointer * Running->ItemPixelsX * (Running->ItemPixelsY / 8)];
      pMapDisplay->UpdateMask                  |= MENUICON_BIT(MENUICON_CENTER);
    }
    break;

    case MENU_EXIT :
    {
      pMapDisplay->pMenuIcons[MENUICON_CENTER] = VarsUi.RunIconSave;
      pMapDisplay->UpdateMask                  = MENUICON_BITS | SPECIAL_BIT(MENUTEXT);
    }
    break;

  }
}

//******* cUiOnBrickProgramming **********************************************

UBYTE     cUiOnBrickProgramming(UBYTE Action) // On brick programming
{
#ifndef STRIPPED
  switch (Action)
  {
    case MENU_INIT :                    // Show motor / sensor text
    {
      pMapDisplay->pTextLines[TEXTLINE_3] = cUiGetString(TXT_ONBRICKPROGRAMMING_PLEASE_USE_PORT);
      pMapDisplay->pTextLines[TEXTLINE_4] = cUiGetString(TXT_ONBRICKPROGRAMMING_1_TOUCH_SENSOR);
      pMapDisplay->pTextLines[TEXTLINE_5] = cUiGetString(TXT_ONBRICKPROGRAMMING_2_SOUND_SENSOR);
      pMapDisplay->pTextLines[TEXTLINE_6] = cUiGetString(TXT_ONBRICKPROGRAMMING_3_LIGHT_SENSOR);
      pMapDisplay->pTextLines[TEXTLINE_7] = cUiGetString(TXT_ONBRICKPROGRAMMING_4_ULTRA_SONIC);
      pMapDisplay->pTextLines[TEXTLINE_8] = cUiGetString(TXT_ONBRICKPROGRAMMING_BC_LR_MOTORS);
      pMapDisplay->EraseMask             |= (TEXTLINE_BIT(TEXTLINE_3) | TEXTLINE_BIT(TEXTLINE_4) | TEXTLINE_BIT(TEXTLINE_5) | TEXTLINE_BIT(TEXTLINE_6) | TEXTLINE_BIT(TEXTLINE_7) | TEXTLINE_BIT(TEXTLINE_8));
      pMapDisplay->UpdateMask            &= ~SPECIAL_BIT(FRAME_SELECT);
      pMapDisplay->UpdateMask            |= (TEXTLINE_BIT(TEXTLINE_3) | TEXTLINE_BIT(TEXTLINE_4) | TEXTLINE_BIT(TEXTLINE_5) | TEXTLINE_BIT(TEXTLINE_6) | TEXTLINE_BIT(TEXTLINE_7) | TEXTLINE_BIT(TEXTLINE_8) | SPECIAL_BIT(TOPLINE));
      pMapDisplay->TextLinesCenterFlags  |= (TEXTLINE_BIT(TEXTLINE_3) | TEXTLINE_BIT(TEXTLINE_4) | TEXTLINE_BIT(TEXTLINE_5) | TEXTLINE_BIT(TEXTLINE_6) | TEXTLINE_BIT(TEXTLINE_7) | TEXTLINE_BIT(TEXTLINE_8));
      IOMapUi.State = TEST_BUTTONS;
    }
    break;

    case MENU_TEXT :                    // Show empty program steps
    {
      pMapDisplay->EraseMask             |=  SCREEN_BIT(SCREEN_LARGE);

      VarsUi.pTmp = (UBYTE*)Cursor;
      for (VarsUi.Tmp = 0;(VarsUi.Tmp < SIZE_OF_CURSOR) && (VarsUi.Tmp < (UBYTE)sizeof(Cursor));VarsUi.Tmp++)
      {
        VarsUi.CursorTmp[VarsUi.Tmp] = *VarsUi.pTmp;
        VarsUi.pTmp++;
      }

      for (VarsUi.ProgramStepPointer = 0;VarsUi.ProgramStepPointer < ON_BRICK_PROGRAMSTEPS;VarsUi.ProgramStepPointer++)
      {
        VarsUi.ProgramSteps[VarsUi.ProgramStepPointer] = MENU_ACTION_EMPTY;
      }
      VarsUi.ProgramStepPointer = 0;
      Action = MENU_DRAW;
    }
    break;

    case MENU_EXIT :                    // Delete one steps and exit at the end
    {
      if (VarsUi.ProgramStepPointer)
      {
        if (VarsUi.ProgramStepPointer < ON_BRICK_PROGRAMSTEPS)
        {
          VarsUi.ProgramSteps[VarsUi.ProgramStepPointer] = MENU_ACTION_EMPTY;
        }
        VarsUi.ProgramStepPointer--;
      }
      else
      {
        IOMapUi.State  = NEXT_MENU;
      }
      Action = MENU_DRAW;
    }
    break;

    case MENU_RUN :                     // Run program steps until end or user press exit button
    {
      switch (VarsUi.State)
      {
        case 0 :
        {
          VarsUi.pTmp = (UBYTE*)Cursor;
          for (VarsUi.Tmp = 0;(VarsUi.Tmp < SIZE_OF_CURSOR) && (VarsUi.Tmp < (UBYTE)sizeof(Cursor));VarsUi.Tmp++)
          {
            VarsUi.CursorTmp[VarsUi.Tmp] = *VarsUi.pTmp;
            VarsUi.pTmp++;
          }
          pMapDisplay->pBitmaps[BITMAP_1]     = (BMPMAP*)VarsUi.CursorTmp;
          cUiRunning(MENU_INIT);
          Action = MENU_DRAW;
          VarsUi.State++;
        }
        break;

        case 1 :                        // If sound finished -> Init text and program pointer
        {
          if (SOUND_IDLE == pMapSound->State)
          {
            VarsUi.ProgramStepPointer  = ON_BRICK_PROGRAMSTEPS;
            VarsUi.MenuIconTextSave    = pMapDisplay->pMenuText;
            pMapDisplay->EraseMask    |= SPECIAL_BIT(MENUTEXT);
            VarsUi.State++;
          }
        }
        break;

        case 2 :                        // load file to run
        {
          if (PROG_IDLE == pMapCmd->ProgStatus)
          {
            sprintf((char*)pMapCmd->FileName,"%s.%s",(char*)VM_PROGRAM_READER,(char*)TXT_SYS_EXT);
            pMapCmd->ActivateFlag = TRUE;
            VarsUi.State++;
          }
        }
        break;

        case 3 :                        // Wait for end of file
        {
          if (PROG_RUNNING != pMapCmd->ProgStatus)
          {
            pMapCmd->ProgStatus = PROG_RESET;
            VarsUi.State = 99;
            VarsUi.ProgramStepPointer = ON_BRICK_PROGRAMSTEPS;
          }
          else
          {
            if (VarsUi.OBPTimer >= MIN_DISPLAY_UPDATE_TIME)
            {
              if (IOMapUi.OBPPointer != VarsUi.ProgramStepPointer)
              {
                VarsUi.ProgramStepPointer = IOMapUi.OBPPointer;
                Action = MENU_DRAW;
              }
            }
          }
        }
        break;

        default :                       // Program stopped
        {
          pMapDisplay->pMenuText      = VarsUi.MenuIconTextSave;
          pMapDisplay->UpdateMask    |= SPECIAL_BIT(MENUTEXT);
          Action                      = MENU_DRAW;
          VarsUi.State                = 0;
        }
        break;

      }
      if (VarsUi.State)
      {
        cUiRunning(MENU_RUN);
      }
      else
      {
        cUiRunning(MENU_EXIT);
      }
    }
    break;

    case MENU_LEFT :                    // NA
    {
      IOMapUi.State = TEST_BUTTONS;
    }
    break;

    case MENU_RIGHT :                   // NA
    {
      IOMapUi.State = TEST_BUTTONS;
    }
    break;

    case MENU_UPDATE :                  // NA
    {
      Action = MENU_DRAW;
    }
    break;

    case MENU_SAVE :                    // Save NXT program
    {
      switch (VarsUi.State)
      {
        case 0 :
        {
          // Suggest default filename to user
          strcpy((char*)VarsUi.UserString,(char*)DEFAULT_PROGRAM_NAME);
          VarsUi.State++;
        }
        break;

        case 1 :
        {
          if (!cUiGetUserString(1))
          {
            if (VarsUi.UserString[0])
            {
              sprintf((char*)VarsUi.SelectedFilename,"%s.%s",VarsUi.UserString,TXT_FILE_EXT[FILETYPE_NXT]);

              // If tmp file exist -> ask for overwrite
              VarsUi.TmpHandle = pMapLoader->pFunc(FINDFIRST,(UBYTE*)VarsUi.SelectedFilename,VarsUi.FilenameBuffer,&VarsUi.TmpLength);
              if (!(VarsUi.TmpHandle & 0x8000))
              {
                pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.TmpHandle,NULL,NULL);
                VarsUi.State++;
              }
              else
              {
                VarsUi.State += 2;
              }
            }
            else
            {
              VarsUi.State = 99;
            }
          }
        }
        break;

        case 2 :
        {
          if (!cUiFeedback((BMPMAP*)Fail,TXT_FB_FILE_EXIST_FAIL,TXT_FB_OVERWRITE_FAIL,0))
          {
            VarsUi.State          = 0;
          }
        }
        break;

        case 3 :
        {
          // Rename TEMP_PROGRAM_FILENAME to VarsUi.SelectedFilename(user filename)
          sprintf((char*)VarsUi.FilenameBuffer,"%s.%s",(char*)TEMP_PROGRAM_FILENAME,(char*)TXT_TMP_EXT);
          VarsUi.TmpHandle = pMapLoader->pFunc(RENAMEFILE,VarsUi.FilenameBuffer,VarsUi.SelectedFilename,&VarsUi.TmpLength);
          pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.TmpHandle,NULL,NULL);
          VarsUi.State++;
        }
        break;

        case 4 : // Display saved text
        {
          if (!cUiFeedback((BMPMAP*)Info,TXT_FB_FILE_SAVED_INFO,0,DISPLAY_SHOW_TIME))
          {
            VarsUi.State++;
          }
        }
        break;

        default :
        {
          cUiMenuPrevFile();
          IOMapUi.State = NEXT_MENU;
          VarsUi.State  = 0;
        }
        break;

      }
    }
    break;

    case MENU_OVERWRITE :               // Over write existing file
    {
      switch (VarsUi.State)
      {
        case 0 :
        {
          // Delete VarsUi.SelectedFilename(user filename)
          VarsUi.TmpHandle = pMapLoader->pFunc(FINDFIRST,(UBYTE*)VarsUi.SelectedFilename,VarsUi.FilenameBuffer,&VarsUi.TmpLength);
          if (!(VarsUi.TmpHandle & 0x8000))
          {
            pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.TmpHandle,NULL,NULL);
            pMapLoader->pFunc(DELETE,VarsUi.SelectedFilename,NULL,NULL);
          }

          // Rename TEMP_PROGRAM_FILENAME to VarsUi.SelectedFilename(user filename)
          sprintf((char*)VarsUi.FilenameBuffer,"%s.%s",(char*)TEMP_PROGRAM_FILENAME,(char*)TXT_TMP_EXT);
          VarsUi.TmpHandle = pMapLoader->pFunc(RENAMEFILE,VarsUi.FilenameBuffer,VarsUi.SelectedFilename,&VarsUi.TmpLength);
          pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.TmpHandle,NULL,NULL);
          VarsUi.State++;
        }
        break;

        default : // Display saved text
        {
          if (!cUiFeedback((BMPMAP*)Info,TXT_FB_FILE_SAVED_INFO,0,DISPLAY_SHOW_TIME))
          {
            VarsUi.State  = 0;
          }
        }
        break;

      }

    }
    break;

    default :                           // Insert selected action/waitfor in program and save if finished
    {
      switch (VarsUi.State)
      {
        case 0 :
        {
          VarsUi.ProgramSteps[VarsUi.ProgramStepPointer] = Action;
          if (VarsUi.ProgramStepPointer < ON_BRICK_PROGRAMSTEPS)
          {
            VarsUi.ProgramStepPointer++;
          }
          if (VarsUi.ProgramStepPointer == ON_BRICK_PROGRAMSTEPS)
          {
            // If tmp file exist -> delete it
            sprintf((char*)VarsUi.FilenameBuffer,"%s.%s",(char*)TEMP_PROGRAM_FILENAME,(char*)TXT_TMP_EXT);
            VarsUi.TmpHandle = pMapLoader->pFunc(FINDFIRST,VarsUi.FilenameBuffer,VarsUi.SearchFilenameBuffer,&VarsUi.TmpLength);
            if (!(VarsUi.TmpHandle & 0x8000))
            {
              pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.TmpHandle,NULL,NULL);
              pMapLoader->pFunc(DELETE,VarsUi.FilenameBuffer,NULL,NULL);
            }

            // Save program as tmp file
            VarsUi.TmpLength = FILEHEADER_LENGTH + ON_BRICK_PROGRAMSTEPS;
            VarsUi.TmpHandle = pMapLoader->pFunc(OPENWRITE,VarsUi.FilenameBuffer,NULL,&VarsUi.TmpLength);
            if (!(VarsUi.TmpHandle & 0x8000))
            {
              VarsUi.FileHeader[0] = (UBYTE)(FILEFORMAT_PROGRAM >> 8);
              VarsUi.FileHeader[1] = (UBYTE)(FILEFORMAT_PROGRAM);
              VarsUi.FileHeader[2] = (UBYTE)(ON_BRICK_PROGRAMSTEPS >> 8);
              VarsUi.FileHeader[3] = (UBYTE)(ON_BRICK_PROGRAMSTEPS);
              VarsUi.FileHeader[4] = (UBYTE)(ON_BRICK_PROGRAMSTEPS);
              VarsUi.FileHeader[5] = (UBYTE)0;
              VarsUi.FileHeader[6] = (UBYTE)0;
              VarsUi.FileHeader[7] = (UBYTE)0;
              VarsUi.TmpLength = FILEHEADER_LENGTH;
              pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)VarsUi.FileHeader,&VarsUi.TmpLength);
              VarsUi.TmpLength = ON_BRICK_PROGRAMSTEPS;
              pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)VarsUi.ProgramSteps,&VarsUi.TmpLength);
              pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.TmpHandle,NULL,NULL);
            }
            else
            {
              VarsUi.State++;
            }
          }
          Action = MENU_DRAW;
        }
        break;

        default : // Display memory error text
        {
          if (!cUiFeedback((BMPMAP*)Fail,TXT_FB_OBP_MEMORY_FULL_FAIL,0,DISPLAY_SHOW_ERROR_TIME))
          {
            cUiMenuPrevFile();
            IOMapUi.State = NEXT_MENU;
            VarsUi.State  = 0;
          }
        }
        break;

      }
    }
    break;

  }

  // Update display screen
  if (Action == MENU_DRAW)
  {
    VarsUi.OBPTimer = 0;

    for (VarsUi.Pointer = 0;VarsUi.Pointer < ON_BRICK_PROGRAMSTEPS;VarsUi.Pointer++)
    {
      VarsUi.Tmp = VarsUi.ProgramSteps[VarsUi.Pointer];
      if ((VarsUi.Tmp >= MENU_ACTION_EMPTY) && (VarsUi.Tmp < MENU_ACTION_INVALID))
      {
        VarsUi.Tmp -= MENU_ACTION_EMPTY;
        pMapDisplay->StepIcons[VarsUi.Pointer] = VarsUi.Tmp + 1;
      }
      if ((VarsUi.Tmp >= MENU_WAIT_EMPTY) && (VarsUi.Tmp < MENU_WAIT_INVALID))
      {
        VarsUi.Tmp -= MENU_WAIT_EMPTY;
        pMapDisplay->StepIcons[VarsUi.Pointer] = VarsUi.Tmp + 1 + 16;
      }
      if (VarsUi.Tmp == MENU_LOOP)
      {
        pMapDisplay->StepIcons[VarsUi.Pointer] = 31;
      }
      if (VarsUi.Tmp == MENU_STOP)
      {
        pMapDisplay->StepIcons[VarsUi.Pointer] = 32;
      }
      pMapDisplay->UpdateMask          |= STEPICON_BIT(STEPICON_1 + VarsUi.Pointer);
    }

    // and cursor
    pMapDisplay->pBitmaps[BITMAP_1]     = (BMPMAP*)VarsUi.CursorTmp;
    if (VarsUi.ProgramStepPointer < ON_BRICK_PROGRAMSTEPS)
    {
      VarsUi.CursorTmp[4] = 13 + (VarsUi.ProgramStepPointer * 17);
      VarsUi.CursorTmp[5] = 24;
      pMapDisplay->UpdateMask          |= BITMAP_BIT(BITMAP_1);
    }
    if (PROG_RUNNING != pMapCmd->ProgStatus)
    {
      pMapDisplay->EraseMask           |= (TEXTLINE_BIT(TEXTLINE_3) | TEXTLINE_BIT(TEXTLINE_2));
    }
    pMapDisplay->EraseMask             |= TEXTLINE_BIT(TEXTLINE_4);
    pMapDisplay->UpdateMask            |= (SPECIAL_BIT(STEPLINE) | SPECIAL_BIT(TOPLINE));
  }

#endif
  return (VarsUi.State);
}



//******* cUiFileRun **********************************************************

UBYTE     cUiFindFileType(UBYTE *Filename) // Find file type number
{
  UBYTE   Ext[FILENAME_LENGTH + 1];
  UBYTE   Result;
  UBYTE   Tmp1;
  UBYTE   Tmp2;

  Result = FILETYPE_ALL;

  Tmp1   = 0;
  while ((Filename[Tmp1]) && (Tmp1 < FILENAME_LENGTH))    // Search forward for termination
  {
    Tmp1++;
  }

  while ((Tmp1) && (Filename[Tmp1] != '.'))               // Search backward for "."
  {
    Tmp1--;
  }

  if (Filename[Tmp1] == '.')                              // If "."
  {
    Tmp1++;
    Tmp2 = 0;

    while ((Filename[Tmp1]) && (Tmp1 < FILENAME_LENGTH))  // Convert to upper to Ext
    {
      Ext[Tmp2] = tolower(Filename[Tmp1]);
      Tmp1++;
      Tmp2++;
    }
    Ext[Tmp2] = 0;                                        // Inser termination

    // Calculate type
    for (Tmp1 = FILETYPE_ALL;(Tmp1 < FILETYPES) && (Result == FILETYPE_ALL);Tmp1++)
    {
      if (strcmp((char*)TXT_FILE_EXT[Tmp1],(char*)Ext) == 0)
      {
        Result = Tmp1;
      }
    }
  }

  return (Result);
}


#define   FILERUN_FILENAMELINE   TEXTLINE_4
#define   FILERUN_TEXTLINE       TEXTLINE_5

UBYTE     cUiFileRun(UBYTE Action)      // Run selected file
{
  switch (Action)
  {

    case MENU_INIT :
    {
      VarsUi.Tmp   = 0;
      while ((VarsUi.SelectedFilename[VarsUi.Tmp]) && (VarsUi.Tmp < FILENAME_LENGTH))    // Search forward for termination
      {
        VarsUi.Tmp++;
      }

      while ((VarsUi.Tmp) && (VarsUi.SelectedFilename[VarsUi.Tmp] != '.'))               // Search backward for "."
      {
        VarsUi.Tmp--;
      }

      if (VarsUi.Tmp > DISPLAYLINE_LENGTH)
      {
        VarsUi.Tmp = DISPLAYLINE_LENGTH;
      }

      VarsUi.DisplayBuffer[VarsUi.Tmp] = 0;

      while (VarsUi.Tmp)                                                           // Copy only name not ext
      {
        VarsUi.Tmp--;
        VarsUi.DisplayBuffer[VarsUi.Tmp] = VarsUi.SelectedFilename[VarsUi.Tmp];
      }

      pMapDisplay->pTextLines[FILERUN_FILENAMELINE] = (UBYTE*)VarsUi.DisplayBuffer;
      pMapDisplay->TextLinesCenterFlags             = TEXTLINE_BIT(FILERUN_FILENAMELINE);
      pMapDisplay->UpdateMask                       = TEXTLINE_BIT(FILERUN_FILENAMELINE);
    }
    break;

    case MENU_RUN :
    {
      if (VarsUi.Timer < DISPLAY_SHOW_TIME)
      {
        VarsUi.Timer++;
      }

      switch (VarsUi.State)
      {
        case 0 :
        {
          IOMapUi.Flags |= UI_BUSY;
          VarsUi.State++;
        }
        break;

        case 1 :                          // Set state from extention when sound is ready
        {
          if (SOUND_IDLE == pMapSound->State)
          {
            pMapDisplay->pTextLines[FILERUN_TEXTLINE]  = cUiGetString(TXT_FILERUN_RUNNING);
            pMapDisplay->UpdateMask                    = (TEXTLINE_BIT(FILERUN_TEXTLINE) | TEXTLINE_BIT(FILERUN_FILENAMELINE));
            pMapDisplay->TextLinesCenterFlags          = (TEXTLINE_BIT(FILERUN_TEXTLINE) | TEXTLINE_BIT(FILERUN_FILENAMELINE));
            cUiRunning(MENU_INIT);
            VarsUi.State++;
          }
        }
        break;

        case 2 :
        {
          if ((!pMapDisplay->EraseMask) && (!pMapDisplay->UpdateMask))
          {
            VarsUi.State = 10 * cUiFindFileType(VarsUi.SelectedFilename);
            if (VarsUi.State == (FILETYPE_TRYME * 10))
            {
              VarsUi.State = FILETYPE_LMS * 10;
            }
          }
        }
        break;

        case (FILETYPE_SOUND * 10 + 0) :  // Start sound file (*.snd, *.rso)  Wait for sound idle
        {
          strcpy((char*)pMapSound->SoundFilename,(char*)VarsUi.SelectedFilename);
          pMapSound->Volume =  IOMapUi.Volume;
          pMapSound->Mode   =  SOUND_ONCE;
          pMapSound->Flags |=  SOUND_UPDATE;
          VarsUi.State++;
        }
        break;

        case (FILETYPE_SOUND * 10 + 1) :  // Wait for stop or user break
        {
          cUiRunning(MENU_RUN);

          if (SOUND_IDLE == pMapSound->State)
          {
            pMapDisplay->pTextLines[FILERUN_TEXTLINE]  = cUiGetString(TXT_FILERUN_ENDED);
            VarsUi.State = 99;
          }
          if (BUTTON_EXIT == cUiReadButtons())
          {
            pMapSound->Flags &= ~SOUND_UPDATE;
            pMapSound->State  =  SOUND_STOP;
            pMapDisplay->pTextLines[FILERUN_TEXTLINE]  = cUiGetString(TXT_FILERUN_ABORTED);
            VarsUi.State = 99;
          }
        }
        break;

        case (FILETYPE_LMS * 10 + 0) : // Start LMS file (*.rxe)
        {
          if ((!pMapDisplay->EraseMask) && (pMapCmd->ProgStatus == PROG_IDLE) && (!pMapButton->State[BTN4]))
          {
            strcpy((char*)pMapCmd->FileName,(char*)VarsUi.SelectedFilename);
            pMapCmd->ActivateFlag = TRUE;
            VarsUi.State++;
          }
        }
        break;

        case (FILETYPE_LMS * 10 + 1) : // Wait for program stop or user break
        {
          cUiRunning(MENU_RUN);

          if ((IOMapUi.Flags & UI_REDRAW_STATUS) && (IOMapUi.Flags & UI_ENABLE_STATUS_UPDATE))
          {
            pMapDisplay->pTextLines[FILERUN_FILENAMELINE] = (UBYTE*)VarsUi.DisplayBuffer;
            pMapDisplay->TextLinesCenterFlags             = TEXTLINE_BIT(FILERUN_FILENAMELINE);
            pMapDisplay->UpdateMask                       = TEXTLINE_BIT(FILERUN_FILENAMELINE);
            pMapDisplay->pTextLines[FILERUN_TEXTLINE]     = cUiGetString(TXT_FILERUN_RUNNING);
            pMapDisplay->UpdateMask                       = (TEXTLINE_BIT(FILERUN_TEXTLINE) | TEXTLINE_BIT(FILERUN_FILENAMELINE));
            pMapDisplay->TextLinesCenterFlags             = (TEXTLINE_BIT(FILERUN_TEXTLINE) | TEXTLINE_BIT(FILERUN_FILENAMELINE));
          }

          switch (pMapCmd->ProgStatus)
          {
            case PROG_RUNNING :
            {
            }
            break;

            case PROG_OK :
            {
              pMapDisplay->pTextLines[FILERUN_TEXTLINE]  = cUiGetString(TXT_FILERUN_ENDED);
              VarsUi.State = 99;
            }
            break;

            case PROG_ABORT :
            {
              pMapDisplay->pTextLines[FILERUN_TEXTLINE]  = cUiGetString(TXT_FILERUN_ABORTED);
              VarsUi.State = 99;
            }
            break;

            default :
            {
              sprintf((char*)VarsUi.DisplayText,(char*)cUiGetString(TXT_FILERUN_FILE_ERROR), pMapCmd->ProgStatus);
              pMapDisplay->pTextLines[FILERUN_TEXTLINE]  = VarsUi.DisplayText;
              VarsUi.State = 99;
            }
            break;

          }
        }
        break;
#ifndef STRIPPED
        case (FILETYPE_NXT * 10 + 0) :// Start Program file (*.prg)
        {
          VarsUi.TmpHandle = pMapLoader->pFunc(OPENREAD,VarsUi.SelectedFilename,NULL,&VarsUi.TmpLength);
          if (!(VarsUi.TmpHandle & 0x8000))
          {
            VarsUi.TmpLength = FILEHEADER_LENGTH;
            pMapLoader->pFunc(READ,(UBYTE*)&VarsUi.TmpHandle,VarsUi.FileHeader,&VarsUi.TmpLength);
            VarsUi.TmpLength = ON_BRICK_PROGRAMSTEPS;
            pMapLoader->pFunc(READ,(UBYTE*)&VarsUi.TmpHandle,VarsUi.ProgramSteps,&VarsUi.TmpLength);
            pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.TmpHandle,NULL,NULL);
          }
          if ((ON_BRICK_PROGRAMSTEPS == VarsUi.TmpLength) && (VarsUi.FileHeader[0] == (UBYTE)(FILEFORMAT_PROGRAM >> 8)) && (VarsUi.FileHeader[1] == (UBYTE)(FILEFORMAT_PROGRAM)))
          {
            // If tmp file exist -> delete it
            sprintf((char*)VarsUi.FilenameBuffer,"%s.%s",(char*)TEMP_PROGRAM_FILENAME,(char*)TXT_TMP_EXT);
            VarsUi.TmpHandle = pMapLoader->pFunc(FINDFIRST,VarsUi.FilenameBuffer,VarsUi.SearchFilenameBuffer,&VarsUi.TmpLength);
            if (!(VarsUi.TmpHandle & 0x8000))
            {
              pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.TmpHandle,NULL,NULL);
              pMapLoader->pFunc(DELETE,VarsUi.FilenameBuffer,NULL,NULL);
            }

            // Save program as tmp file
            VarsUi.TmpLength = FILEHEADER_LENGTH + ON_BRICK_PROGRAMSTEPS;
            VarsUi.TmpHandle = pMapLoader->pFunc(OPENWRITE,VarsUi.FilenameBuffer,NULL,&VarsUi.TmpLength);
            if (!(VarsUi.TmpHandle & 0x8000))
            {
              VarsUi.TmpLength = FILEHEADER_LENGTH;
              pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)VarsUi.FileHeader,&VarsUi.TmpLength);
              VarsUi.TmpLength = ON_BRICK_PROGRAMSTEPS;
              pMapLoader->pFunc(WRITE,(UBYTE*)&VarsUi.TmpHandle,(UBYTE*)VarsUi.ProgramSteps,&VarsUi.TmpLength);
              pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.TmpHandle,NULL,NULL);
            }

            pMapDisplay->UpdateMask &= ~TEXTLINE_BIT(FILERUN_FILENAMELINE);
            pMapDisplay->EraseMask  |=  TEXTLINE_BIT(FILERUN_FILENAMELINE);
            VarsUi.State++;
          }
          else
          {
            pMapDisplay->pTextLines[FILERUN_TEXTLINE]  = cUiGetString(TXT_FILERUN_FILE_ERROR);
            VarsUi.State = 99;
          }
          VarsUi.GUSState = 0;
        }
        break;

        case (FILETYPE_NXT * 10 + 1) : // Wait for program stop or user break
        {
          VarsUi.State = VarsUi.GUSState;
          cUiOnBrickProgramming(MENU_RUN);
          VarsUi.GUSState = VarsUi.State;
          if (VarsUi.State)
          {
            VarsUi.State = (FILETYPE_NXT * 10 + 1);
          }
          else
          {
            pMapDisplay->pTextLines[FILERUN_TEXTLINE]  = cUiGetString(TXT_FILERUN_ENDED);
            VarsUi.State = 99;
          }
        }
        break;
#endif
        case 99 :                         // Wait for display show time or user action
        {
          pMapDisplay->EraseMask                     = SCREEN_BIT(SCREEN_LARGE);
          pMapDisplay->UpdateMask                    = (TEXTLINE_BIT(FILERUN_TEXTLINE) | TEXTLINE_BIT(FILERUN_FILENAMELINE));
          pMapDisplay->TextLinesCenterFlags          = (TEXTLINE_BIT(FILERUN_TEXTLINE) | TEXTLINE_BIT(FILERUN_FILENAMELINE));
          IOMapUi.Flags                             |= UI_REDRAW_STATUS | UI_ENABLE_STATUS_UPDATE;
          cUiRunning(MENU_UPDATE);
          VarsUi.Timer = 0;
          VarsUi.State++;
        }
        break;

        default :
        {
          if ((++VarsUi.Timer >= DISPLAY_SHOW_TIME) || (BUTTON_NONE != cUiReadButtons()))
          {
            if (pMapCmd->ProgStatus != PROG_IDLE)
              pMapCmd->ProgStatus              = PROG_RESET;
            pMapDisplay->UpdateMask            = 0;
            pMapDisplay->TextLinesCenterFlags  = 0;
            cUiRunning(MENU_EXIT);
            pMapDisplay->EraseMask             = TEXTLINE_BIT(FILERUN_TEXTLINE);
            pMapDisplay->TextLinesCenterFlags |= TEXTLINE_BIT(FILERUN_FILENAMELINE);
            pMapDisplay->UpdateMask           |= TEXTLINE_BIT(FILERUN_FILENAMELINE);
            IOMapUi.Flags                     &= ~UI_BUSY;
            VarsUi.State                       = 0;
          }
        }
        break;

      }
    }
    break;

  }

  return (VarsUi.State);
}



//******* cUiFileDelete *******************************************************

UBYTE     cUiFileDelete(UBYTE Action)
{
  if (MENU_INIT == Action)
  {
    switch (VarsUi.State)
    {
      case 0 :
      {
        VarsUi.State++;
      }
      break;

      case 1 :
      {
        if (SOUND_IDLE == pMapSound->State)
        {
          VarsUi.State++;
        }
      }
      break;

      case 2 :
      {
        pMapLoader->pFunc(DELETE,VarsUi.SelectedFilename,NULL,NULL);
        VarsUi.State++;
      }
      break;

      default : // Display deleted text
      {
        if (!cUiFeedback((BMPMAP*)Info,TXT_FB_FD_FILE_DELETED_INFO,0,DISPLAY_SHOW_TIME))
        {
          IOMapUi.State = EXIT_PRESSED;
          VarsUi.State  = 0;
        }
      }
      break;

    }
  }

  return (VarsUi.State);
}


//******* cUiView ************************************************************

UBYTE     cUiView(UBYTE Action) // MENU_INIT
{
  switch (VarsUi.State)
  {
    case 0 :
    {
      switch (Action)
      {
        case MENU_INIT : // Init
        {
          pMapDisplay->pTextLines[TEXTLINE_3] = cUiGetString(TXT_GENERIC_SELECT);
          pMapDisplay->TextLinesCenterFlags  |= TEXTLINE_BIT(TEXTLINE_3);
          pMapDisplay->UpdateMask            |= TEXTLINE_BIT(TEXTLINE_3);
          pMapDisplay->EraseMask             |= SCREEN_BIT(SCREEN_SMALL);
#ifndef STRIPPED
// Init ports
          for (VarsUi.Tmp = 0;VarsUi.Tmp < DATALOGPORTS;VarsUi.Tmp++)
          {
            VarsUi.DatalogPort[VarsUi.Tmp] = MENU_SENSOR_EMPTY;
          }
#endif
        }
        break;

        default :
        {
          if ((Action > MENU_SENSOR_EMPTY) && (Action < MENU_SENSOR_INVALID))
          {
            VarsUi.SelectedSensor = Action;
          }
          if ((Action >= MENU_PORT_1) && (Action <= MENU_PORT_C))
          {
            VarsUi.SelectedPort = Action;
#ifndef STRIPPED
            VarsUi.DatalogPort[VarsUi.SelectedPort - MENU_PORT_1] = VarsUi.SelectedSensor;
#endif

            IOMapUi.Flags |= UI_BUSY;
            pMapDisplay->EraseMask             |= SCREEN_BIT(SCREEN_LARGE);
            pMapDisplay->pBitmaps[BITMAP_1]     = (BMPMAP*)Display;
            pMapDisplay->UpdateMask             = BITMAP_BIT(BITMAP_1);
            IOMapUi.Flags                      |=  UI_REDRAW_STATUS;
            VarsUi.ReadoutTimer                 = 0;;
            VarsUi.State++;

            VarsUi.SensorReset = TRUE;
          }
        }
        break;

      }
    }
    break;

    case 1 :
    {
      VarsUi.ReadoutTimer++;
      cUiUpdateSensor(1);
      if (VarsUi.ReadoutTimer >= DISPLAY_VIEW_UPDATE)
      {
        VarsUi.ReadoutTimer = 0;
        cUiPrintSensorInDisplayBuffer(VarsUi.SelectedPort);
        pMapDisplay->pTextLines[TEXTLINE_4] = VarsUi.DisplayBuffer;
        pMapDisplay->TextLinesCenterFlags  |= TEXTLINE_BIT(TEXTLINE_4);
        pMapDisplay->UpdateMask            |= TEXTLINE_BIT(TEXTLINE_4);
      }

      VarsUi.Tmp = cUiReadButtons();
      if (VarsUi.Tmp == BUTTON_EXIT)
      {
        pMapDisplay->pTextLines[TEXTLINE_3] =  cUiGetString(TXT_GENERIC_SELECT);
        pMapDisplay->TextLinesCenterFlags  |=  TEXTLINE_BIT(TEXTLINE_3);
        pMapDisplay->UpdateMask            |=  TEXTLINE_BIT(TEXTLINE_3);
        pMapDisplay->UpdateMask            &=  ~TEXTLINE_BIT(TEXTLINE_4);
        pMapDisplay->EraseMask             |=  SCREEN_BIT(SCREEN_SMALL);
        VarsUi.State++;
      }
      if (VarsUi.Tmp == BUTTON_ENTER)
      {
        VarsUi.SensorReset = TRUE;
      }
    }
    break;

    default :
    {
      cUiReleaseSensors();
      IOMapUi.Flags &= ~UI_BUSY;
      VarsUi.State  = 0;
      IOMapUi.State = EXIT_PRESSED;
    }
    break;

  }

  return (VarsUi.State);
}



//******* cUiBtOn ************************************************************

UBYTE     cUiBtOn(UBYTE Action)
{
  switch (Action)
  {
    case MENU_ON :
    {
      switch (VarsUi.State)
      {
        case 0 : // Turn BT on
        {
          VarsUi.BTCommand  = (UBYTE)BTON;
          VarsUi.BTPar1     = (UBYTE)0;
          VarsUi.BTPar2     = (UBYTE)0;
          if (pMapComm->pFunc(VarsUi.BTCommand,VarsUi.BTPar1,VarsUi.BTPar2,0,NULL,&(VarsUi.BTResult)) == SUCCESS)
          {
            VarsUi.State++;
          }
          else
          {
            VarsUi.State = 99;
          }
        }
        break;

        case 1 : // Display turning on text
        {
          if (!cUiFeedback((BMPMAP*)Wait,TXT_FB_BT_TURNING_ON_WAIT,0,0))
          {
            VarsUi.State++;
          }
        }
        break;

        case 2 : // Check result
        {
          if (VarsUi.BTResult != INPROGRESS)
          {
            if (VarsUi.BTResult == SUCCESS)
            {
              Action = MENU_EXIT;
            }
            else
            {
              VarsUi.State++;
            }
          }
        }
        break;

        default : // Display fail text
        {
          if (!cUiFeedback((BMPMAP*)Fail,TXT_FB_GENERIC_FAIL,0,DISPLAY_SHOW_ERROR_TIME))
          {
            Action = MENU_EXIT;
          }
        }
        break;

      }
    }
    break;

    case MENU_OFF :
    {
      switch (VarsUi.State)
      {
        case 0 : // Turn BT off
        {
          VarsUi.BTCommand  = (UBYTE)BTOFF;
          VarsUi.BTPar1     = (UBYTE)0;
          VarsUi.BTPar2     = (UBYTE)0;
          if (pMapComm->pFunc(VarsUi.BTCommand,VarsUi.BTPar1,VarsUi.BTPar2,0,NULL,&(VarsUi.BTResult)) == SUCCESS)
          {
            VarsUi.State++;
          }
          else
          {
            VarsUi.State = 99;
          }
        }
        break;

        case 1 : // Display turning off text
        {
          if (!cUiFeedback((BMPMAP*)Wait,TXT_FB_BT_TURNING_OFF_WAIT,0,0))
          {
            VarsUi.State++;
          }
        }
        break;

        case 2 : // Check result
        {
          if (VarsUi.BTResult != INPROGRESS)
          {
            if (VarsUi.BTResult == SUCCESS)
            {
              Action = MENU_EXIT;
            }
            else
            {
              VarsUi.State++;
            }
          }
        }
        break;

        default : // Display fail text
        {
          if (!cUiFeedback((BMPMAP*)Fail,TXT_FB_GENERIC_FAIL,0,DISPLAY_SHOW_ERROR_TIME))
          {
            Action = MENU_EXIT;
          }
        }
        break;

      }
    }
    break;

  }
  if (Action == MENU_EXIT)
  {
    VarsUi.State  = 0;
    IOMapUi.State = EXIT_PRESSED;
  }

  return (VarsUi.State);
}



//******* cUiBtVisiability ***************************************************

UBYTE     cUiBtVisiability(UBYTE Action) // Visibility on/off
{
  switch (Action)
  {
    case MENU_ON : // Set visible
    {
      switch (VarsUi.State)
      {
        case 0 :
        {
          VarsUi.BTCommand  = (UBYTE)VISIBILITY;
          VarsUi.BTPar1     = (UBYTE)1;
          VarsUi.BTPar2     = (UBYTE)0;
          if (pMapComm->pFunc(VarsUi.BTCommand,VarsUi.BTPar1,VarsUi.BTPar2,0,NULL,&(VarsUi.BTResult)) == SUCCESS)
          {
            VarsUi.State++;
          }
          else
          {
            Action = MENU_EXIT;
          }
        }
        break;

        default :
        {
          if (VarsUi.BTResult != INPROGRESS)
          {
            Action = MENU_EXIT;
          }
        }
        break;

      }
    }
    break;

    case MENU_OFF : // Set invisible
    {
      switch (VarsUi.State)
      {
        case 0 :
        {
          VarsUi.BTCommand  = (UBYTE)VISIBILITY;
          VarsUi.BTPar1     = (UBYTE)0;
          VarsUi.BTPar2     = (UBYTE)0;
          if (pMapComm->pFunc(VarsUi.BTCommand,VarsUi.BTPar1,VarsUi.BTPar2,0,NULL,&(VarsUi.BTResult)) == SUCCESS)
          {
            VarsUi.State++;
          }
          else
          {
            Action = MENU_EXIT;
          }
        }
        break;

        default :
        {
          if (VarsUi.BTResult != INPROGRESS)
          {
            Action = MENU_EXIT;
          }
        }
        break;

      }
    }
    break;

  }
  if (Action == MENU_EXIT)
  {
    VarsUi.State  = 0;
    IOMapUi.State = EXIT_PRESSED;
  }

  return (VarsUi.State);
}



//******* cUiBtSearch ********************************************************

UBYTE     cUiBtSearch(UBYTE Action) // Search for devices
{
  if (Action == MENU_INIT) // Init
  {
    switch (VarsUi.State)
    {
      case 0 : // Show three menu icons
      {
        pMapDisplay->pMenuIcons[MENUICON_LEFT]    = pMapDisplay->pMenuIcons[MENUICON_CENTER];
        pMapDisplay->pMenuIcons[MENUICON_RIGHT]   = pMapDisplay->pMenuIcons[MENUICON_CENTER];
        pMapDisplay->UpdateMask                  |= MENUICON_BITS;
        VarsUi.State++;
      }
      break;

      case 1 : // Display wait text and start search
      {
        if (!cUiFeedback((BMPMAP*)Wait,TXT_FB_BT_SEARCHING_WAIT,0,0))
        {
          VarsUi.BTCommand  = (UBYTE)SEARCH;
          VarsUi.BTPar1     = (UBYTE)1;
          VarsUi.BTPar2     = (UBYTE)0;
          if (pMapComm->pFunc(VarsUi.BTCommand,VarsUi.BTPar1,VarsUi.BTPar2,0,NULL,&(VarsUi.BTResult)) == SUCCESS)
          {
            VarsUi.DisplayBuffer[0]  = 0;
            pMapDisplay->pMenuText   = VarsUi.DisplayBuffer;
            pMapDisplay->UpdateMask |= SPECIAL_BIT(MENUTEXT);
            VarsUi.NoOfNames         = 0;
            VarsUi.NoOfDevices       = 0;
            VarsUi.State++;
          }
          else
          {
            VarsUi.State = 99;
          }
        }
      }
      break;

      case 2 : // Wait for search finished
      {
        if (VarsUi.NoOfNames != pMapComm->BtDeviceNameCnt)
        {
          VarsUi.NoOfNames = pMapComm->BtDeviceNameCnt;

          if ((VarsUi.NoOfNames) && (VarsUi.NoOfNames <= DISPLAYLINE_LENGTH))
          {
            sprintf((char*)VarsUi.DisplayBuffer,"%.*s",VarsUi.NoOfNames,"****************");
            pMapDisplay->pMenuText = VarsUi.DisplayBuffer;
            pMapDisplay->UpdateMask |= SPECIAL_BIT(MENUTEXT);
          }
        }
        if (VarsUi.NoOfDevices != pMapComm->BtDeviceCnt)
        {
          VarsUi.NoOfDevices = pMapComm->BtDeviceCnt;

          if ((VarsUi.NoOfDevices) && (VarsUi.NoOfDevices <= DISPLAYLINE_LENGTH))
          {
            sprintf((char*)VarsUi.DisplayBuffer,"%.*s",VarsUi.NoOfDevices,"????????????????");
            pMapDisplay->pMenuText = VarsUi.DisplayBuffer;
            pMapDisplay->UpdateMask |= SPECIAL_BIT(MENUTEXT);
          }
        }

        if (VarsUi.BTResult != INPROGRESS)
        {
          cUiBTCommand(UI_BT_GET_DEVICES,0,&VarsUi.Devices,NULL);
          if (VarsUi.Devices)
          {
            VarsUi.State++;
          }
          else
          {
            VarsUi.State = 99;
          }
        }

        if (cUiReadButtons() == BUTTON_EXIT)
        {
          VarsUi.BTCommand  = (UBYTE)STOPSEARCH;
          VarsUi.BTPar1     = (UBYTE)0;
          VarsUi.BTPar2     = (UBYTE)0;
          pMapComm->pFunc(VarsUi.BTCommand,VarsUi.BTPar1,VarsUi.BTPar2,0,NULL,&(VarsUi.BTResult));
          VarsUi.State      = 4;
        }
      }
      break;

      case 3 : // Auto enter to next menu
      {
        IOMapUi.State = ENTER_PRESSED;
        VarsUi.State = 0;
      }
      break;

      case 4 : // Display info text
      {
        if (!cUiFeedback((BMPMAP*)Info,TXT_FB_BT_SEARCH_ABORTED_INFO,0,DISPLAY_SHOW_TIME))
        {
          VarsUi.State++;
        }
      }
      break;

      case 5 : // Wait for abort
      {
        if (VarsUi.BTResult != INPROGRESS)
        {
          cUiBTCommand(UI_BT_GET_DEVICES,0,&VarsUi.Devices,NULL);
          if (VarsUi.Devices)
          {
            VarsUi.State++;
          }
          else
          {
            VarsUi.State = 99;
          }
        }
      }
      break;

      case 6 : // Auto enter to next menu
      {
        IOMapUi.State   = ENTER_PRESSED;
        VarsUi.State    = 0;
      }
      break;

      default : // Display fail text
      {
        if (!cUiFeedback((BMPMAP*)Fail,TXT_FB_GENERIC_FAIL,0,DISPLAY_SHOW_ERROR_TIME))
        {
          VarsUi.State  = 0;
          IOMapUi.State = EXIT_PRESSED;
        }
      }
      break;

    }
  }

  return (VarsUi.State);
}



//******* cUiBtDeviceList ****************************************************

UBYTE     cUiBtDeviceList(UBYTE Action) // Show devices
{
  switch (Action)
  {
    case MENU_INIT : // Init "Search" list
    {
      VarsUi.SelectedDevice                 = 0;
      VarsUi.DevicesKnown                   = 0;
      cUiBTCommand(UI_BT_GET_DEVICES,VarsUi.DevicesKnown,&VarsUi.Devices,NULL);
      if (VarsUi.Devices)
      {
        pMapDisplay->pTextLines[TEXTLINE_3] = cUiGetString(TXT_GENERIC_SELECT);
        pMapDisplay->TextLinesCenterFlags  |= TEXTLINE_BIT(TEXTLINE_3);
        pMapDisplay->UpdateMask            |= TEXTLINE_BIT(TEXTLINE_3);
        VarsUi.MenuIconTextSave             = pMapDisplay->pMenuText;
        VarsUi.DeviceCenter                 = 1;
        Action                              = MENU_DRAW;
        IOMapUi.State                       = TEST_BUTTONS;
      }
      else
      {
        Action                              = MENU_EXIT;
      }
    }
    break;

    case MENU_INIT_ALTERNATIVE : // Init only "My contacts"
    {
      VarsUi.SelectedDevice                 = 0;
      VarsUi.DevicesKnown                   = 1;
      cUiBTCommand(UI_BT_GET_DEVICES,VarsUi.DevicesKnown,&VarsUi.Devices,NULL);
      if (VarsUi.Devices)
      {
        pMapDisplay->pTextLines[TEXTLINE_3] = cUiGetString(TXT_GENERIC_SELECT);
        pMapDisplay->TextLinesCenterFlags  |= TEXTLINE_BIT(TEXTLINE_3);
        pMapDisplay->UpdateMask            |= TEXTLINE_BIT(TEXTLINE_3);
        VarsUi.MenuIconTextSave             = pMapDisplay->pMenuText;
        VarsUi.DeviceCenter                 = 1;
        Action                              = MENU_DRAW;
        IOMapUi.State                       = TEST_BUTTONS;
      }
      else
      {
        Action                              = MENU_EXIT;
      }
    }
    break;

    case MENU_LEFT : // Left button
    {
      cUiListLeft(VarsUi.Devices,&VarsUi.DeviceCenter);
      Action                  = MENU_DRAW;
      IOMapUi.State           = TEST_BUTTONS;
    }
    break;

    case MENU_RIGHT : // Right button
    {
      cUiListRight(VarsUi.Devices,&VarsUi.DeviceCenter);
      Action                  = MENU_DRAW;
      IOMapUi.State           = TEST_BUTTONS;
    }
    break;

    case MENU_SELECT : // Select for connection
    {
      VarsUi.SelectedDevice   = VarsUi.DeviceCenter;
      pMapDisplay->pMenuText  = VarsUi.MenuIconTextSave;
      IOMapUi.State           = NEXT_MENU;
    }
    break;

    case MENU_DELETE : // Remove device from "My contacts"
    {
      switch (VarsUi.State)
      {
        case 0 :
        {
          if (VarsUi.SelectedDevice)
          {
            if (cUiBTGetDeviceIndex(VarsUi.DevicesKnown,VarsUi.SelectedDevice - 1,&VarsUi.BTIndex))
            {
              VarsUi.BTCommand  = (UBYTE)REMOVEDEVICE;
              VarsUi.BTPar1     = (UBYTE)VarsUi.BTIndex;
              VarsUi.BTPar2     = (UBYTE)0;
              if (pMapComm->pFunc(VarsUi.BTCommand,VarsUi.BTPar1,VarsUi.BTPar2,0,NULL,&(VarsUi.BTResult)) == SUCCESS)
              {
                VarsUi.State++;
              }
              else
              {
                VarsUi.State    = 99;
              }
            }
            else
            {
              Action            = MENU_EXIT;
            }
            VarsUi.SelectedDevice = 0;
          }
          else
          {
            Action              = MENU_EXIT;
          }
        }
        break;

        case 1 :
        {
          if (VarsUi.BTResult != INPROGRESS)
          {
            if (VarsUi.BTResult == SUCCESS)
            {
              Action            = MENU_EXIT;
            }
            else
            {
              VarsUi.State      = 99;
            }
          }
        }
        break;

        default : // Display fail text
        {
          if (!cUiFeedback((BMPMAP*)Fail,TXT_FB_GENERIC_FAIL,0,DISPLAY_SHOW_ERROR_TIME))
          {
            Action              = MENU_EXIT;
          }
        }
        break;

      }
    }
    break;

  }

  if (Action == MENU_DRAW)
  {
    cUiListCalc(VarsUi.Devices,&VarsUi.DeviceCenter,&VarsUi.DeviceLeft,&VarsUi.DeviceRight);

    pMapDisplay->pMenuIcons[MENUICON_LEFT]   = NULL;
    pMapDisplay->pMenuIcons[MENUICON_CENTER] = NULL;
    pMapDisplay->pMenuIcons[MENUICON_RIGHT]  = NULL;

    if (VarsUi.DeviceLeft)
    {
      VarsUi.Tmp = VarsUi.DeviceLeft - 1;
      cUiBTCommand(UI_BT_GET_DEVICE_TYPE,VarsUi.DevicesKnown,&VarsUi.Tmp,&VarsUi.DeviceType);
      pMapDisplay->pMenuIcons[MENUICON_LEFT]    = (UBYTE*)&Devices->Data[VarsUi.DeviceType * Devices->ItemPixelsX * (Devices->ItemPixelsY / 8)];
      pMapDisplay->UpdateMask                  |= MENUICON_BIT(MENUICON_LEFT);
    }
    if (VarsUi.DeviceCenter)
    {
      VarsUi.Tmp = VarsUi.DeviceCenter - 1;
      cUiBTCommand(UI_BT_GET_DEVICE_TYPE,VarsUi.DevicesKnown,&VarsUi.Tmp,&VarsUi.DeviceType);
      pMapDisplay->pMenuIcons[MENUICON_CENTER]  = (UBYTE*)&Devices->Data[VarsUi.DeviceType * Devices->ItemPixelsX * (Devices->ItemPixelsY / 8)];
      pMapDisplay->UpdateMask                  |= MENUICON_BIT(MENUICON_CENTER);
    }
    if (VarsUi.DeviceRight)
    {
      VarsUi.Tmp = VarsUi.DeviceRight - 1;
      cUiBTCommand(UI_BT_GET_DEVICE_TYPE,VarsUi.DevicesKnown,&VarsUi.Tmp,&VarsUi.DeviceType);
      pMapDisplay->pMenuIcons[MENUICON_RIGHT]   = (UBYTE*)&Devices->Data[VarsUi.DeviceType * Devices->ItemPixelsX * (Devices->ItemPixelsY / 8)];
      pMapDisplay->UpdateMask                  |= MENUICON_BIT(MENUICON_RIGHT);
    }

    pMapDisplay->EraseMask                   |=  TEXTLINE_BIT(TEXTLINE_5);

    VarsUi.Tmp = VarsUi.DeviceCenter - 1;
    cUiBTCommand(UI_BT_GET_DEVICE_NAME,VarsUi.DevicesKnown,&VarsUi.Tmp,VarsUi.DisplayBuffer);

    pMapDisplay->pMenuText                    = VarsUi.DisplayBuffer;
    pMapDisplay->EraseMask                   |= MENUICON_BITS;
    pMapDisplay->UpdateMask                  |= (SPECIAL_BIT(FRAME_SELECT) | SPECIAL_BIT(MENUTEXT));
  }
  if (Action == MENU_EXIT)
  {
    VarsUi.State  = 0;
    IOMapUi.State = EXIT_PRESSED;
  }

  return (VarsUi.State);
}


//******* cUiBtConnectList ***************************************************

UBYTE     cUiBtConnectList(UBYTE Action) // Show connections and maybe disconnect
{
  switch  (Action)
  {
    case MENU_INIT : // Init
    {
      VarsUi.Slots                = SIZE_OF_BT_CONNECT_TABLE;
      VarsUi.SlotCenter           = 2;
      Action                      = MENU_DRAW;
      IOMapUi.State               = TEST_BUTTONS;
    }
    break;

    case MENU_LEFT : // Left button
    {
      cUiListLeft(VarsUi.Slots,&VarsUi.SlotCenter);
      Action                      = MENU_DRAW;
      IOMapUi.State               = TEST_BUTTONS;
    }
    break;

    case MENU_RIGHT : // Right button
    {
      cUiListRight(VarsUi.Slots,&VarsUi.SlotCenter);
      Action                      = MENU_DRAW;
      IOMapUi.State               = TEST_BUTTONS;
    }
    break;

    case MENU_UPDATE : // Check connection valid
    {
      VarsUi.Tmp = VarsUi.SlotCenter - 1;
      if (cUiBTCommand(UI_BT_GET_CONNECTION_VALID,NULL,&VarsUi.Tmp,NULL) != UI_BT_SUCCES)
      {
        Action = MENU_EXIT;
      }
    }
    break;

    case MENU_DISCONNECT : // Disconnect
    {
      switch (VarsUi.State)
      {
        case 0 :
        {
          VarsUi.SelectedSlot = VarsUi.SlotCenter - 1;
          VarsUi.BTCommand  = (UBYTE)DISCONNECT;
          VarsUi.BTPar1     = (UBYTE)VarsUi.SelectedSlot;
          VarsUi.BTPar2     = (UBYTE)0;
          if (pMapComm->pFunc(VarsUi.BTCommand,VarsUi.BTPar1,VarsUi.BTPar2,0,NULL,&(VarsUi.BTResult)) == SUCCESS)
          {
            VarsUi.State++;
          }
          else
          {
            VarsUi.State    = 99;
          }
        }
        break;

        case 1 :
        {
          if (VarsUi.BTResult != INPROGRESS)
          {
            if (VarsUi.BTResult == SUCCESS)
            {
              Action        = MENU_EXIT;
            }
            else
            {
              VarsUi.State  = 99;
            }
          }
        }
        break;

        default : // Display fail text
        {
          if (!cUiFeedback((BMPMAP*)Fail,TXT_FB_GENERIC_FAIL,0,DISPLAY_SHOW_ERROR_TIME))
          {
            Action          = MENU_EXIT;
          }
        }
        break;

      }
    }
    break;

  }
  if (Action == MENU_DRAW)
  {
    cUiListCalc(VarsUi.Slots,&VarsUi.SlotCenter,&VarsUi.SlotLeft,&VarsUi.SlotRight);

    pMapDisplay->pBitmaps[BITMAP_2]          = (BMPMAP*)VarsUi.PortBitmapLeft;
    pMapDisplay->pBitmaps[BITMAP_3]          = (BMPMAP*)VarsUi.PortBitmapCenter;
    pMapDisplay->pBitmaps[BITMAP_4]          = (BMPMAP*)VarsUi.PortBitmapRight;

    VarsUi.Tmp = VarsUi.SlotLeft - 1;
    if (cUiBTCommand(UI_BT_GET_CONNECTION_VALID,NULL,&VarsUi.Tmp,NULL) == UI_BT_SUCCES)
    {
      cUiBTCommand(UI_BT_GET_CONNECTION_TYPE,NULL,&VarsUi.Tmp,&VarsUi.DeviceType);
      pMapDisplay->pMenuIcons[MENUICON_LEFT]    = (UBYTE*)&Devices->Data[VarsUi.DeviceType * Devices->ItemPixelsX * (Devices->ItemPixelsY / 8)];
      cUiDrawPortNo(VarsUi.PortBitmapLeft,MENUICON_LEFT,VarsUi.Tmp);
      pMapDisplay->UpdateMask                  |= BITMAP_BIT(BITMAP_2);
    }
    else
    {
      pMapDisplay->pMenuIcons[MENUICON_LEFT]    = (UBYTE*)&Connections->Data[VarsUi.Tmp * Connections->ItemPixelsX * (Connections->ItemPixelsY / 8)];
    }

    VarsUi.Tmp = VarsUi.SlotCenter - 1;
    cUiBTCommand(UI_BT_GET_CONNECTION_NAME,NULL,&VarsUi.Tmp,VarsUi.DisplayBuffer);
    pMapDisplay->EraseMask                     |=  TEXTLINE_BIT(TEXTLINE_5);
    pMapDisplay->pMenuText                      = VarsUi.DisplayBuffer;

    if (cUiBTCommand(UI_BT_GET_CONNECTION_VALID,NULL,&VarsUi.Tmp,NULL) == UI_BT_SUCCES)
    {
      cUiBTCommand(UI_BT_GET_CONNECTION_TYPE,NULL,&VarsUi.Tmp,&VarsUi.DeviceType);
      pMapDisplay->pMenuIcons[MENUICON_CENTER]  = (UBYTE*)&Devices->Data[VarsUi.DeviceType * Devices->ItemPixelsX * (Devices->ItemPixelsY / 8)];
      cUiDrawPortNo(VarsUi.PortBitmapCenter,MENUICON_CENTER,VarsUi.Tmp);
      pMapDisplay->UpdateMask                  |= BITMAP_BIT(BITMAP_3);
    }
    else
    {
      pMapDisplay->pMenuIcons[MENUICON_CENTER]  = (UBYTE*)&Connections->Data[VarsUi.Tmp * Connections->ItemPixelsX * (Connections->ItemPixelsY / 8)];
    }
    VarsUi.Tmp = VarsUi.SlotRight - 1;
    if (cUiBTCommand(UI_BT_GET_CONNECTION_VALID,NULL,&VarsUi.Tmp,NULL) == UI_BT_SUCCES)
    {
      cUiBTCommand(UI_BT_GET_CONNECTION_TYPE,NULL,&VarsUi.Tmp,&VarsUi.DeviceType);
      pMapDisplay->pMenuIcons[MENUICON_RIGHT]   = (UBYTE*)&Devices->Data[VarsUi.DeviceType * Devices->ItemPixelsX * (Devices->ItemPixelsY / 8)];
      cUiDrawPortNo(VarsUi.PortBitmapRight,MENUICON_RIGHT,VarsUi.Tmp);
      pMapDisplay->UpdateMask                  |= BITMAP_BIT(BITMAP_4);
    }
    else
    {
      pMapDisplay->pMenuIcons[MENUICON_RIGHT]   = (UBYTE*)&Connections->Data[VarsUi.Tmp * Connections->ItemPixelsX * (Connections->ItemPixelsY / 8)];
    }
    pMapDisplay->EraseMask                     &= ~SCREEN_BIT(SCREEN_LARGE);
    pMapDisplay->EraseMask                     |= MENUICON_BITS;
    pMapDisplay->UpdateMask                    |= (MENUICON_BITS | SPECIAL_BIT(FRAME_SELECT) | SPECIAL_BIT(MENUTEXT));
  }
  if (Action == MENU_EXIT)
  {
    VarsUi.State  = 0;
    IOMapUi.State = EXIT_PRESSED;
  }


  return (VarsUi.State);
}


UBYTE     cUiBtConnect(UBYTE Action) // Select connection no and insert device
{
  switch  (Action)
  {
    case MENU_INIT : // Init
    {
      VarsUi.Slots                = SIZE_OF_BT_CONNECT_TABLE - 1;
      VarsUi.SlotCenter           = 1;
      Action                      = MENU_DRAW;
      IOMapUi.State               = TEST_BUTTONS;
    }
    break;

    case MENU_LEFT : // Left button
    {
      cUiListLeft(VarsUi.Slots,&VarsUi.SlotCenter);
      Action                      = MENU_DRAW;
      IOMapUi.State               = TEST_BUTTONS;
    }
    break;

    case MENU_RIGHT : // Right button
    {
      cUiListRight(VarsUi.Slots,&VarsUi.SlotCenter);
      Action                      = MENU_DRAW;
      IOMapUi.State               = TEST_BUTTONS;
    }
    break;

    case MENU_CONNECT : // Insert device
    {
      switch (VarsUi.State)
      {
        case 0 : // Check selected device
        {
          VarsUi.SelectedSlot = (UBYTE)VarsUi.SlotCenter;
          if (VarsUi.SelectedDevice)
          {
            VarsUi.State++;
          }
          else
          {
            Action = MENU_EXIT;
          }
        }
        break;

        case 1 : // Display wait text
        {
          if (!cUiFeedback((BMPMAP*)Wait,TXT_FB_BT_CONNECTING_WAIT,0,0))
          {
            if (cUiBTGetDeviceIndex(VarsUi.DevicesKnown,VarsUi.SelectedDevice - 1,&VarsUi.BTIndex))
            {
              VarsUi.BTCommand  = (UBYTE)CONNECT;
              VarsUi.BTPar1     = (UBYTE)VarsUi.BTIndex;
              VarsUi.BTPar2     = (UBYTE)VarsUi.SelectedSlot;
              if (pMapComm->pFunc(VarsUi.BTCommand,VarsUi.BTPar1,VarsUi.BTPar2,0,NULL,&(VarsUi.BTResult)) == SUCCESS)
              {
                VarsUi.State++;
              }
              else
              {
                VarsUi.State = 99;
              }
            }
            else
            {
              VarsUi.State = 99;
            }
          }
        }
        break;

        case 2 : // Wait for result
        {
          if (VarsUi.BTResult != INPROGRESS)
          {
            if (VarsUi.BTResult == SUCCESS)
            {
              Action = MENU_EXIT;
            }
            else
            {
              if (VarsUi.BTResult == REQPIN)
              {
                sprintf((char*)pMapSound->SoundFilename,"%s.%s",(char*)UI_ATTENTION_SOUND,(char*)TXT_FILE_EXT[FILETYPE_SOUND]);
                pMapSound->Volume =  IOMapUi.Volume;
                pMapSound->Mode   =  SOUND_ONCE;
                pMapSound->Flags |=  SOUND_UPDATE;
                strcpy((char*)VarsUi.UserString,(char*)DEFAULT_PIN_CODE);
                VarsUi.State++;
              }
              else
              {
                VarsUi.State = 6;
              }
            }
          }
        }
        break;

        case 3 : // Get pincode and send
        {
          if (!cUiGetUserString(0))
          {
            if (VarsUi.UserString[0] == 0)
            {
              sprintf((char*)VarsUi.UserString,"%08lX",VarsUi.CRPasskey);
              Action = MENU_EXIT;
            }
            else
            {
              VarsUi.State++;
            }
            pMapComm->pFunc2(VarsUi.UserString);
          }
        }
        break;

        case 4 : // Display wait text
        {
          if (!cUiFeedback((BMPMAP*)Wait,TXT_FB_BT_CONNECTING_WAIT,0,0))
          {
            VarsUi.State++;
          }
        }
        break;

        case 5 : // Wait for result
        {
          if (VarsUi.BTResult != INPROGRESS)
          {
            if (VarsUi.BTResult == SUCCESS)
            {
              Action = MENU_EXIT;
            }
            else
            {
              VarsUi.State = 6;
            }
          }
        }
        break;

        case 6 : // Display busy text
        {
          if (!cUiFeedback((BMPMAP*)Fail,TXT_FB_BT_CONNECT_BUSY_FAIL,0,DISPLAY_SHOW_ERROR_TIME))
          {
            Action = MENU_EXIT;
          }
        }
        break;

        default : // Display fail text
        {
          if (!cUiFeedback((BMPMAP*)Fail,TXT_FB_GENERIC_FAIL,0,DISPLAY_SHOW_ERROR_TIME))
          {
            Action = MENU_EXIT;
          }
        }
        break;

      }
    }
    break;

    case MENU_SEND :
    {
      switch (VarsUi.State)
      {
        case 0 : // Check connection
        {
          VarsUi.SelectedSlot = (UBYTE)VarsUi.SlotCenter;
          if (VarsUi.SelectedFilename[0] && (cUiBTCommand(UI_BT_GET_CONNECTION_NAME,NULL,&VarsUi.SelectedSlot,NULL) == UI_BT_SUCCES))
          {
            VarsUi.State += 2;
          }
          else
          {
            VarsUi.State++;
          }
        }
        break;

        case 1 : // Display fail text
        {
          if (!cUiFeedback((BMPMAP*)Fail,TXT_FB_BT_SENDING_NO_CONN_FAIL,0,DISPLAY_SHOW_ERROR_TIME))
          {
            Action = MENU_EXIT;
          }
        }
        break;

        case 2 : // Display wait text and send file
        {
          if (!cUiFeedback((BMPMAP*)Wait,TXT_FB_BT_SENDING_WAIT,0,0))
          {
            VarsUi.BTCommand  = (UBYTE)SENDFILE;
            VarsUi.BTPar1     = (UBYTE)VarsUi.SelectedSlot;
            VarsUi.BTPar2     = (UBYTE)0;
            if (pMapComm->pFunc(VarsUi.BTCommand,VarsUi.BTPar1,VarsUi.BTPar2,0,VarsUi.SelectedFilename,&(VarsUi.BTResult)) == SUCCESS)
            {
              VarsUi.Timer = 0;
              VarsUi.State++;
            }
            else
            {
              VarsUi.State = 4;
            }
          }
        }
        break;

        case 3 : // Wait for result
        {
          if (VarsUi.BTResult != INPROGRESS)
          {
            if (VarsUi.BTResult == SUCCESS)
            {
              VarsUi.State += 2;
            }
            else
            {
              VarsUi.State++;
            }
          }
          VarsUi.Timer++;
        }
        break;

        case 4 : // Display fail text
        {
          if (!cUiFeedback((BMPMAP*)Fail,TXT_FB_GENERIC_FAIL,0,DISPLAY_SHOW_ERROR_TIME))
          {
            Action = MENU_EXIT;
          }
        }
        break;

        case 5 : // Wait min. "DISPLAY_SHOW_TIME" to show "TXT_FB_BT_SENDING_WAIT"
        {
          if (++VarsUi.Timer >= DISPLAY_SHOW_TIME)
          {
            Action = MENU_EXIT;
          }
        }
        break;

      }
    }
    break;

  }
  if (Action == MENU_DRAW) // Update display
  {
    cUiListCalc(VarsUi.Slots,&VarsUi.SlotCenter,&VarsUi.SlotLeft,&VarsUi.SlotRight);

    pMapDisplay->pBitmaps[BITMAP_2]          = (BMPMAP*)VarsUi.PortBitmapLeft;
    pMapDisplay->pBitmaps[BITMAP_3]          = (BMPMAP*)VarsUi.PortBitmapCenter;
    pMapDisplay->pBitmaps[BITMAP_4]          = (BMPMAP*)VarsUi.PortBitmapRight;

    VarsUi.Tmp = VarsUi.SlotLeft;
    if (cUiBTCommand(UI_BT_GET_CONNECTION_VALID,NULL,&VarsUi.Tmp,NULL) == UI_BT_SUCCES)
    {
      cUiBTCommand(UI_BT_GET_CONNECTION_TYPE,NULL,&VarsUi.Tmp,&VarsUi.DeviceType);
      pMapDisplay->pMenuIcons[MENUICON_LEFT]    = (UBYTE*)&Devices->Data[VarsUi.DeviceType * Devices->ItemPixelsX * (Devices->ItemPixelsY / 8)];
      cUiDrawPortNo(VarsUi.PortBitmapLeft,MENUICON_LEFT,VarsUi.Tmp);
      pMapDisplay->UpdateMask                  |= BITMAP_BIT(BITMAP_2);
    }
    else
    {
      pMapDisplay->pMenuIcons[MENUICON_LEFT]    = (UBYTE*)&Connections->Data[VarsUi.Tmp * Connections->ItemPixelsX * (Connections->ItemPixelsY / 8)];
    }

    VarsUi.Tmp = VarsUi.SlotCenter;
    cUiBTCommand(UI_BT_GET_CONNECTION_NAME,NULL,&VarsUi.Tmp,VarsUi.DisplayBuffer);
    pMapDisplay->EraseMask                     |=  TEXTLINE_BIT(TEXTLINE_5);
    pMapDisplay->pMenuText                      = VarsUi.DisplayBuffer;

    if (cUiBTCommand(UI_BT_GET_CONNECTION_VALID,NULL,&VarsUi.Tmp,NULL) == UI_BT_SUCCES)
    {
      cUiBTCommand(UI_BT_GET_CONNECTION_TYPE,NULL,&VarsUi.Tmp,&VarsUi.DeviceType);
      pMapDisplay->pMenuIcons[MENUICON_CENTER]  = (UBYTE*)&Devices->Data[VarsUi.DeviceType * Devices->ItemPixelsX * (Devices->ItemPixelsY / 8)];
      cUiDrawPortNo(VarsUi.PortBitmapCenter,MENUICON_CENTER,VarsUi.Tmp);
      pMapDisplay->UpdateMask                  |= BITMAP_BIT(BITMAP_3);
    }
    else
    {
      pMapDisplay->pMenuIcons[MENUICON_CENTER]  = (UBYTE*)&Connections->Data[VarsUi.Tmp * Connections->ItemPixelsX * (Connections->ItemPixelsY / 8)];
    }
    VarsUi.Tmp = VarsUi.SlotRight;
    if (cUiBTCommand(UI_BT_GET_CONNECTION_VALID,NULL,&VarsUi.Tmp,NULL) == UI_BT_SUCCES)
    {
      cUiBTCommand(UI_BT_GET_CONNECTION_TYPE,NULL,&VarsUi.Tmp,&VarsUi.DeviceType);
      pMapDisplay->pMenuIcons[MENUICON_RIGHT]   = (UBYTE*)&Devices->Data[VarsUi.DeviceType * Devices->ItemPixelsX * (Devices->ItemPixelsY / 8)];
      cUiDrawPortNo(VarsUi.PortBitmapRight,MENUICON_RIGHT,VarsUi.Tmp);
      pMapDisplay->UpdateMask                  |= BITMAP_BIT(BITMAP_4);
    }
    else
    {
      pMapDisplay->pMenuIcons[MENUICON_RIGHT]   = (UBYTE*)&Connections->Data[VarsUi.Tmp * Connections->ItemPixelsX * (Connections->ItemPixelsY / 8)];
    }
    pMapDisplay->EraseMask                     &= ~SCREEN_BIT(SCREEN_LARGE);
    pMapDisplay->EraseMask                     |= MENUICON_BITS;
    pMapDisplay->UpdateMask                    |= (MENUICON_BITS | SPECIAL_BIT(FRAME_SELECT) | SPECIAL_BIT(MENUTEXT));
  }
  if (Action == MENU_EXIT)
  {
    IOMapUi.State   = EXIT_PRESSED;
    VarsUi.State    = 0;
  }

  return (VarsUi.State);
}



//******* cUiPowerOffTime ****************************************************

UBYTE     cUiPowerOffTime(UBYTE Action) // MENU_INIT,MENU_LEFT,MENU_RIGHT,MENU_EXIT
{
  switch (Action)
  {
    case MENU_INIT : // Init time counter and cursor bitmap
    {
      VarsUi.Counter        = VarsUi.NVData.PowerdownCode + 1;

#ifndef STRIPPED
      VarsUi.pTmp           = (UBYTE*)Cursor;
      for (VarsUi.Tmp = 0;(VarsUi.Tmp < SIZE_OF_CURSOR) && (VarsUi.Tmp < (UBYTE)sizeof(Cursor));VarsUi.Tmp++)
      {
        VarsUi.CursorTmp[VarsUi.Tmp] = *VarsUi.pTmp;
        VarsUi.pTmp++;
      }
#endif
      Action                = MENU_DRAW;
    }
    break;

    case MENU_LEFT : // Dec
    {
      cUiListLeft(POWER_OFF_TIME_STEPS,&VarsUi.Counter);
      Action                = MENU_DRAW;
    }
    break;

    case MENU_RIGHT : // Inc
    {
      cUiListRight(POWER_OFF_TIME_STEPS,&VarsUi.Counter);
      Action                = MENU_DRAW;
    }
    break;

    case MENU_ENTER : // Enter
    {
      VarsUi.NVData.PowerdownCode = VarsUi.Counter - 1;
      cUiNVWrite();
      IOMapUi.SleepTimeout  = PowerOffTimeSteps[VarsUi.NVData.PowerdownCode];
      Action                = MENU_EXIT;
    }
    break;

  }

  if (Action == MENU_DRAW)
  {
    if (VarsUi.Counter > 1)
    {
      sprintf((char*)VarsUi.DisplayBuffer,"%u",(UWORD)PowerOffTimeSteps[VarsUi.Counter - 1]);
    }
    else
    {
      sprintf((char*)VarsUi.DisplayBuffer,(char*)cUiGetString(TXT_POWEROFFTIME_NEVER));
    }
    pMapDisplay->pTextLines[TEXTLINE_3] = VarsUi.DisplayBuffer;

#ifndef STRIPPED
    pMapDisplay->pBitmaps[BITMAP_1]     = (BMPMAP*)VarsUi.CursorTmp;
    VarsUi.CursorTmp[4] = 46;
    VarsUi.CursorTmp[5] = 24;
#endif
    pMapDisplay->EraseMask             |= (TEXTLINE_BIT(TEXTLINE_3) | TEXTLINE_BIT(TEXTLINE_4));
    pMapDisplay->TextLinesCenterFlags  |= TEXTLINE_BIT(TEXTLINE_3);
    pMapDisplay->UpdateMask            |= (TEXTLINE_BIT(TEXTLINE_3) | BITMAP_BIT(BITMAP_1));
  }
  if (Action == MENU_EXIT)
  {
    IOMapUi.State        = EXIT_PRESSED;
  }

  return (0);
}



//******* cUiBTConnectRequest ************************************************

UBYTE     cUiBTConnectRequest(UBYTE Action)
{
  switch (Action)
  {
    case MENU_INIT :
    {
      switch (VarsUi.CRState)
      {
        case 0 :
        {
          sprintf((char*)pMapSound->SoundFilename,"%s.%s",(char*)UI_ATTENTION_SOUND,(char*)TXT_FILE_EXT[FILETYPE_SOUND]);
          pMapSound->Volume =  IOMapUi.Volume;
          pMapSound->Mode   =  SOUND_ONCE;
          pMapSound->Flags |=  SOUND_UPDATE;
          VarsUi.CRState++;
        }
        break;

        case 1 :
        {
          if (DISPLAY_IDLE)
          {
            pMapDisplay->Flags |= DISPLAY_POPUP;
            VarsUi.CRState++;
          }
        }
        break;

        case 2 :
        {
          strcpy((char*)VarsUi.UserString,(char*)DEFAULT_PIN_CODE);
          IOMapUi.Flags                 |= UI_REDRAW_STATUS;
          VarsUi.CRState++;
        }
        break;

        case 3 : // Get pincode and send
        {
          if (!cUiGetUserString(0))
          {
            if (VarsUi.UserString[0] == 0)
            {
              sprintf((char*)VarsUi.UserString,"%08lX",VarsUi.CRPasskey);
            }
            pMapComm->pFunc2(VarsUi.UserString);
            VarsUi.CRState++;
          }
        }
        break;

        case 4 :
        {
          if (DISPLAY_IDLE)
          {
            pMapDisplay->Flags &= ~DISPLAY_POPUP;
            VarsUi.CRState      = 0;
          }
        }
        break;

      }
    }
    break;

  }

  return (VarsUi.CRState);
}



//******* cUiFilesDelete *****************************************************

UBYTE     cUiFilesDelete(UBYTE Action)
{
  switch (Action)
  {
    case MENU_INIT :
    {
      pMapDisplay->pTextLines[TEXTLINE_3] = cUiGetString(TXT_FILESDELETE_DELETING_ALL);
      pMapDisplay->TextLinesCenterFlags  |= TEXTLINE_BIT(TEXTLINE_3);
      pMapDisplay->UpdateMask            |= TEXTLINE_BIT(TEXTLINE_3);
      sprintf((char*)VarsUi.DisplayBuffer,(char*)cUiGetString(TXT_FILESDELETE_S_FILES),(char*)cUiGetString(TXT_FILETYPE[VarsUi.SelectedType]));
      pMapDisplay->pTextLines[TEXTLINE_4] = VarsUi.DisplayBuffer;
      pMapDisplay->TextLinesCenterFlags  |= TEXTLINE_BIT(TEXTLINE_4);
      pMapDisplay->UpdateMask            |= TEXTLINE_BIT(TEXTLINE_4);
      IOMapUi.State                       = TEST_BUTTONS;
    }
    break;

    case MENU_DELETE :
    {
      switch (VarsUi.State)
      {
        case 0 :
        {
          if (VarsUi.SelectedType < FILETYPES)
          {
            sprintf((char*)VarsUi.FilenameBuffer,"*.%s",TXT_FILE_EXT[VarsUi.SelectedType]);
          }
          else
          {
            sprintf((char*)VarsUi.FilenameBuffer,"*.*");
          }
          VarsUi.State++;
        }
        break;

        case 1 :
        {
          if (SOUND_IDLE == pMapSound->State)
          {
            VarsUi.State++;
          }
        }
        break;

        case 2 : // Delete files
        {
          VarsUi.TmpHandle = pMapLoader->pFunc(FINDFIRST,VarsUi.FilenameBuffer,VarsUi.SelectedFilename,&VarsUi.TmpLength);
          if (!(VarsUi.TmpHandle & 0x8000))
          {
            pMapLoader->pFunc(CLOSE,(UBYTE*)&VarsUi.TmpHandle,NULL,NULL);
            pMapLoader->pFunc(DELETE,VarsUi.SelectedFilename,NULL,NULL);
          }
          else
          {
            pMapDisplay->EraseMask |= MENUICON_BITS;
            pMapDisplay->EraseMask |= SPECIAL_BIT(MENUTEXT);
            VarsUi.State++;
          }
        }
        break;

        default : // Display Files deleted text
        {
          if (!cUiFeedback((BMPMAP*)Info,TXT_FB_FD_FILES_INFO,TXT_FB_FD_DELETED_INFO,DISPLAY_SHOW_TIME))
          {
            IOMapUi.State = EXIT_PRESSED;
            VarsUi.State  = 0;
          }
        }
        break;

      }
    }
    break;

    default :
    {
      if (Action < FILETYPES)
      {
        VarsUi.SelectedType = Action;
      }
      else
      {
        VarsUi.SelectedType = FILETYPE_ALL;
      }
    }
    break;

  }

  return (VarsUi.State);
}


//******* cUiOff *************************************************************

UBYTE     cUiOff(UBYTE Action)          // Tell AVR to turn off ARM
{
  if (Action == MENU_INIT)
  {
    switch (VarsUi.State)
    {
      case 0 :                          // Stop VM if running
      {
        if (pMapCmd->ProgStatus == PROG_RUNNING)
        {
          pMapCmd->DeactivateFlag = TRUE;
        }
        VarsUi.State++;
      }
      break;

      case 1 :                          // When VM is stopped -> Display off and close all connections
      {
        if (pMapCmd->ProgStatus != PROG_RUNNING)
        {
          pMapDisplay->Flags     &= ~DISPLAY_ON;
          VarsUi.BTCommand        = (UBYTE)DISCONNECTALL;
          VarsUi.BTPar1           = (UBYTE)0;
          VarsUi.BTPar2           = (UBYTE)0;
          pMapComm->pFunc(VarsUi.BTCommand,VarsUi.BTPar1,VarsUi.BTPar2,0,NULL,&(VarsUi.BTResult));
          VarsUi.State++;
        }
      }
      break;

      case 2 :                          //  Send off command to AVR
      {
        if (VarsUi.BTResult != INPROGRESS)
        {
          pMapIoCtrl->PowerOn   = POWERDOWN;
          VarsUi.Timer          = 0;
          VarsUi.State++;
        }
      }
      break;

      case 3 :                          // Wait for power off
      {
        if (++VarsUi.Timer >= ARM_WAIT_FOR_POWER_OFF)
        {
          VarsUi.State++;
        }
      }
      break;

      case 4 :                          // Vitual off state (if still power) wait for on button
      {
        pMapIoCtrl->PowerOn     = 0;
        if (BUTTON_ENTER == cUiReadButtons())
        {
          VarsUi.State++;
        }
      }
      break;

      default :                         // Turn on again
      {
        IOMapUi.State           = INIT_DISPLAY;
        VarsUi.State            = 0;
      }
      break;

    }
  }

  return (VarsUi.State);
}



//******* FUNCTIONS **********************************************************

enum      FUNC_NO                       // Must reffer to entry in Functions
{                                       // used in Menus to repressent function
  FUNC_NO_NOT_USED              = 0x00,
  FUNC_NO_TEST_PROGRAM          = 0x01,
  FUNC_NO_OFF                   = 0x02,
  FUNC_NO_BT_ON                 = 0x03,
  FUNC_NO_POWER_OFF_TIME        = 0x04,
  FUNC_NO_FILES_DELETE          = 0x05,
  FUNC_NO_FILE_LIST             = 0x06,
  FUNC_NO_VOLUME                = 0x07,
  FUNC_NO_FILE_RUN              = 0x08,
  FUNC_NO_FILE_DELETE           = 0x09,
  FUNC_NO_FREE1                 = 0x0A,
  FUNC_NO_ON_BRICK_PROGRAMMING  = 0x0B,
  FUNC_NO_FREE2                 = 0x0C,
  FUNC_NO_BT_CONNECT_REQUEST    = 0x0D,
  FUNC_NO_VIEW                  = 0x0E,
  FUNC_NO_GET_USER_STRING       = 0x0F,
  FUNC_NO_BT_CONNECT            = 0x10,
  FUNC_NO_BT_VISIABILITY        = 0x11,
  FUNC_NO_BT_SEARCH             = 0x12,
  FUNC_NO_BT_DEVICE_LIST        = 0x13,
  FUNC_NO_BT_CONNECT_LIST       = 0x14,
  FUNC_NO_MAX
};

FUNCTION  Functions[] =                 // Use same index as FUNC_NO
{
  0,
  TestPrg,
  cUiOff,
  cUiBtOn,
  cUiPowerOffTime,
  cUiFilesDelete,
  cUiFileList,
  cUiVolume,
  cUiFileRun,
  cUiFileDelete,
  cUiDataLogging,
  cUiOnBrickProgramming,
  0,
  cUiBTConnectRequest,
  cUiView,
  cUiGetUserString,
  cUiBtConnect,
  cUiBtVisiability,
  cUiBtSearch,
  cUiBtDeviceList,
  cUiBtConnectList
};


