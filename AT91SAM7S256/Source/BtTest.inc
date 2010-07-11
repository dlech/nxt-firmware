//******* TestPrg ************************************************************

#include  "Test1.txt"
#include  "Test2.txt"
//#define TESTPRG                     // If defined the test program will be included

extern    void BtIo(void);

const     char  BUILD_DATE[] = __DATE__;
const     char  BUILD_TIME[] = __TIME__;

const     char  MONTH[]  = "JanFebMarAprMayJunJulAugSepOctNovDec";

void      GetProtocolVersion(UBYTE *String)
{
  UWORD   Tmp;
  
  Tmp = FIRMWAREVERSION & 0x00FF;

  if (Tmp < 100)
  {
    sprintf((char*)String,"FW        %3u.%02u",(FIRMWAREVERSION >> 8) & 0x00FF,Tmp & 0x00FF);
  }
  else
  {
    sprintf((char*)String,"FW     Hex %2X.%02X",(FIRMWAREVERSION >> 8) & 0x00FF,Tmp & 0x00FF);
  }
}


void      GetARMBuild(UBYTE *String)
{
  UWORD   Tmp;
  UWORD   Lng;
  char    String1[4];
  char    String2[4];

  String1[0] = BUILD_DATE[4];
  String1[1] = BUILD_DATE[5];
  String1[2] = 0;

  Tmp = (UWORD)atoi(String1);
  Lng  = 0;
  Lng += sprintf((char*)&String[Lng],"BUILD ");
  Lng += sprintf((char*)&String[Lng],"%02u",Tmp);

  String1[0] = BUILD_DATE[0];
  String1[1] = BUILD_DATE[1];
  String1[2] = BUILD_DATE[2];
  String1[3] = 0;
  String2[3] = 0;

  Tmp = 0;
  do
  {
    String2[0] = MONTH[0 + 3 * Tmp];
    String2[1] = MONTH[1 + 3 * Tmp];
    String2[2] = MONTH[2 + 3 * Tmp];
    Tmp++;
  }
  while ((Tmp < 12) && (strcmp(String1,String2) != 0));

  Lng += sprintf((char*)&String[Lng],"%02u",Tmp);
  Lng += sprintf((char*)&String[Lng],"%c%c",BUILD_DATE[9],BUILD_DATE[10]);
  Lng += sprintf((char*)&String[Lng],"%c%c",BUILD_TIME[0],BUILD_TIME[1]);
  Lng += sprintf((char*)&String[Lng],"%c%c",BUILD_TIME[3],BUILD_TIME[4]);
}


void      GetBC4Build(UBYTE *String)
{
  sprintf((char*)String,"BC4        %2X.%02X",pMapComm->BrickData.BluecoreVersion[1],pMapComm->BrickData.BluecoreVersion[0]);
}


void      GetAVRBuild(UBYTE *String)
{
  sprintf((char*)String,"AVR         %1u.%02u",((IoFromAvr.Battery >> 13) & 3),((IoFromAvr.Battery >> 10) & 7));
}


void      GetBC4Address(UBYTE *String)
{
  UWORD   Count;
  UBYTE   Tmp;

  Count = (UWORD)sprintf((char*)String,"ID  ");
  for (Tmp = 0;(Tmp < (SIZE_OF_BDADDR - 1)) && (Count <= (DISPLAYLINE_LENGTH - 2));Tmp++)
  {
    Count += (UWORD)sprintf((char*)&String[Count],"%02X",(UWORD)(pMapComm->BrickData.BdAddr[Tmp]) & 0xFF);
  }
}


enum      TSTPRG
{
  SYSTEM_INIT = 1,
    SYSTEM_UNLOCK_INIT,
    SYSTEM_UNLOCK,
  SYSTEM_PAGE,
    TIMER_INIT,
      TIMER_SHOW,
      TIMER_HOLD,
    BT_PAGE,
      BT_RESET,
        BT_RESETTING,
      BT_LIST_INIT,
        BT_LIST,
        BT_CONN_INIT,
          BT_CONN,
      BT_UPDATE_FW,
  TSTPRG_INIT,
    TSTPRG_SELECT_SUBTEST,

    TSTPRG_SENSOR_INIT,
      TSTPRG_SELECT_SENSOR,
      TSTPRG_TOUCH_SENSOR_INIT,
        TSTPRG_TOUCH_SENSOR,
      TSTPRG_SOUND_SENSOR_SELECT,
        TSTPRG_SOUND_SENSOR_INIT,
          TSTPRG_SOUND_SENSOR,
      TSTPRG_LIGHT_SENSOR_SELECT,
        TSTPRG_LIGHT_SENSOR_INIT,
          TSTPRG_LIGHT_SENSOR,
    TSTPRG_SKIP_SENSOR,

    TSTPRG_RCX_INIT,
      TSTPRG_RCX_SELECT,
      TSTPRG_RCX_DISPLAY_INIT,
        TSTPRG_RCX_DISPLAY,
      TSTPRG_RCX_INPUT_SELECT,
        TSTPRG_RCX_INPUT_INIT,
          TSTPRG_RCX_INPUT,
        TSTPRG_RCX_DIGITAL_INIT,
          TSTPRG_RCX_DIGITAL_OK,
          TSTPRG_RCX_DIGITAL_FAIL,
          TSTPRG_RCX_DIGITAL,
      TSTPRG_RCX_MOTOR_INIT,
        TSTPRG_RCX_MOTOR,
      TSTPRG_SKIP_RCX_MOTOR,
    TSTPRG_SKIP_RCX,

    TSTPRG_MOTOR_INIT,
      TSTPRG_MOTOR,
    TSTPRG_SKIP_MOTOR,

  TSTPRG_SKIP,
  TSTPRG_WAIT
};

const     UBYTE TXT_EMPTY[]                 = "                ";
const     UBYTE TXT_LINE[]                  = "----------------";

#ifdef  TESTPRG

const     UBYTE TXT_TEST[]                  = "Timer Test   Bt ";

const     UBYTE TXT_TIMER[]                 = "Reset Hold      ";
const     UBYTE TXT_TIMER_HOLD[]            = "    Continue    ";

const     UBYTE TXT_LAST[]                  = "Last UI->BT Cmd.";
const     UBYTE TXT_BT_PAGE[]               = "Reset List  BtIo";

const     UBYTE TXT_RESETTING[]             = "   Resetting!   ";

const     UBYTE TXT_BT_LIST[]               = "Down ConTab  Up ";
const     UBYTE TXT_BT_CONN[]               = "Down Update  Up ";

const     UBYTE TXT_BTUPDATE[]              = "BT update mode !";
const     UBYTE TXT_DONE[]                  = "   When done    ";
const     UBYTE TXT_RESET[]                 = " activate reset ";
const     UBYTE TXT_REBOOT[]                = "button to reboot";

const     UBYTE TXT_TESTPRG[]               = " TestPrg  V0.08 ";
const     UBYTE TXT_SELECT[]                = "Select sub test ";
const     UBYTE TXT_SUBTEST[]               = "Sens. RCX  Motor";

const     UBYTE TXT_SELECT_SENSOR[]         = " Select sensor  ";
const     UBYTE TXT_SENSORS[]               = "Touch Snd. Light";

const     UBYTE TXT_SELECT_TYPE[]           = "  Select type   ";
const     UBYTE TXT_SOUND_SENSORS[]         = " DB         DBA ";
const     UBYTE TXT_LIGHT_SENSORS[]         = "Pasive    Active";

const     UBYTE TXT_SENSOR_TOUCH[]          = "Touch Sensor Tst";
const     UBYTE TXT_SENSOR_SOUND_DB[]       = "DB   Sound Sens.";
const     UBYTE TXT_SENSOR_SOUND_DBA[]      = "DBA  Sound Sens.";
const     UBYTE TXT_SOUND_STOP[]            = "440Hz Stop  4KHz";
const     UBYTE TXT_SENSOR_LIGHT_PASIVE[]   = "Pas. Light Sens.";
const     UBYTE TXT_SENSOR_LIGHT_ACTIVE[]   = "Act. Light Sens.";

const     UBYTE TXT_SUBTEST_STOP[]          = "      Stop      ";

const     UBYTE TXT_MOTOR[]                 = "   Motor test   ";
const     UBYTE TXT_MOTOR_HEADER[]          = "Outp  Set    Cnt";
const     UBYTE TXT_MOTOR_STOP[]            = "Bwd   Stop   Fwd";

const     UBYTE TXT_RCX[]                   = "    RCX test    ";
const     UBYTE TXT_RCX_STOP[]              = "Inp   Disp  Outp";
const     UBYTE TXT_RCX_INPUT_PASIVE[]      = "Input pasive Tst";
const     UBYTE TXT_RCX_INPUT_ACTIVE[]      = "Input active Tst";
const     UBYTE TXT_RCX_INPUT_SELECT[]      = "Pas.  Act.  Dig.";
const     UBYTE TXT_RCX_INPUT_DIGITAL[]     = "Digital I/O  Tst";
const     UBYTE TXT_RCX_DIGITAL_OK[]        = "       OK       ";
const     UBYTE TXT_RCX_DIGITAL_FAIL[]      = "      FAIL      ";
const     UBYTE TXT_MOTOR_NEXT[]            = "Bwd   Next   Fwd";


void      TestPrgRunMotor(UBYTE No,SBYTE Speed)
{
  pMapOutPut->Outputs[No].Mode       = MOTORON | BRAKE;
  pMapOutPut->Outputs[No].Speed      = Speed;
  pMapOutPut->Outputs[No].TachoLimit = 0;
  pMapOutPut->Outputs[No].RunState   = MOTOR_RUN_STATE_RUNNING;
  pMapOutPut->Outputs[No].Flags      = UPDATE_MODE | UPDATE_SPEED | UPDATE_TACHO_LIMIT;
}

void      TestPrgFloatMotor(UBYTE No)
{
  pMapOutPut->Outputs[No].Mode       = 0;
  pMapOutPut->Outputs[No].Speed      = 0;
  pMapOutPut->Outputs[No].TachoLimit = 0;
  pMapOutPut->Outputs[No].RunState   = MOTOR_RUN_STATE_RUNNING;
  pMapOutPut->Outputs[No].Flags      = UPDATE_MODE | UPDATE_SPEED | UPDATE_TACHO_LIMIT;
}

SBYTE     TestPrgReadMotor(UBYTE No)
{
  return ((SBYTE)(pMapOutPut->Outputs[No].TachoCnt / 360));
}

#endif


UBYTE     TestPrg(UBYTE Dummy)
{
  static  UWORD Count;
  static  UBYTE TxtBuffer[TEXTLINES][DISPLAYLINE_LENGTH + 1];
  static  UBYTE State = SYSTEM_INIT;
#ifdef TESTPRG
  static  UWORD Pointer;
  static  UWORD InputValues[NO_OF_INPUTS];
  static  SWORD OutputValues[NO_OF_OUTPUTS];
  static  UBYTE VolumeSave;
  static  UBYTE Timer;
  static  UBYTE SubState = 0;
  UBYTE   Tmp;
#endif

  Dummy = Dummy;
  switch (State)
  {
    case SYSTEM_INIT :
    {
      GetProtocolVersion(TxtBuffer[0]);
      GetAVRBuild(TxtBuffer[1]);
      GetBC4Build(TxtBuffer[2]);
      GetARMBuild(TxtBuffer[3]);
      GetBC4Address(TxtBuffer[4]);

      pMapDisplay->pTextLines[TEXTLINE_3] = (UBYTE*)TxtBuffer[0];
      pMapDisplay->pTextLines[TEXTLINE_4] = (UBYTE*)TxtBuffer[1];
      pMapDisplay->pTextLines[TEXTLINE_5] = (UBYTE*)TxtBuffer[2];
      pMapDisplay->pTextLines[TEXTLINE_6] = (UBYTE*)TxtBuffer[3];
      pMapDisplay->pTextLines[TEXTLINE_7] = (UBYTE*)TxtBuffer[4];
      pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->UpdateMask            |= (TEXTLINE_BIT(TEXTLINE_3) | TEXTLINE_BIT(TEXTLINE_4) | TEXTLINE_BIT(TEXTLINE_5) | TEXTLINE_BIT(TEXTLINE_6) | TEXTLINE_BIT(TEXTLINE_7) | TEXTLINE_BIT(TEXTLINE_8));

#ifdef TESTPRG
      SubState  = 0;
#endif
      State     = SYSTEM_UNLOCK_INIT;
    }
    break;

#ifndef TESTPRG

    case SYSTEM_UNLOCK_INIT : // ENTER * 1 + LEFT * 3 + RIGHT * 2 + ENTER * 1 = TEST MENU
    {
      if (cUiReadButtons() != BUTTON_NONE)
      {
        State = TSTPRG_SKIP;
      }
    }
    break;

#else

    case SYSTEM_UNLOCK_INIT : // ENTER * 1 + LEFT * 3 + RIGHT * 2 + ENTER * 1 = TEST MENU
    {
      Tmp = cUiReadButtons();
      switch (Tmp)
      {
        case BUTTON_ENTER :
        {
          switch (SubState)
          {
            case 0 :
            {
              SubState = 1;
              Pointer  = 0;
              Count    = 0;
            }
            break;

            case 3 :
            {
              State = SYSTEM_UNLOCK;
            }
            break;

            default :
            {
              Tmp = BUTTON_EXIT;
            }
            break;

          }
        }
        break;

        case BUTTON_NONE :
        {
        }
        break;

        default :
        {
          switch (SubState)
          {
            case 0 :
            {
              Tmp = BUTTON_EXIT;
            }
            break;

            case 1 :
            {
              if (Tmp == BUTTON_LEFT)
              {
                if (++Count >= 3)
                {
                  Count    = 0;
                  SubState = 2;
                }
                Pointer = 0;
              }
              else
              {
                Tmp = BUTTON_EXIT;
              }
            }
            break;

            case 2 :
            {
              if (Tmp == BUTTON_RIGHT)
              {
                if (++Count >= 2)
                {
                  SubState = 3;
                }
                Pointer = 0;
              }
              else
              {
                Tmp = BUTTON_EXIT;
              }
            }
            break;

          }
        }
        break;

      }
      Pointer++;
      if (((SubState) && (Pointer > 500)) || (Tmp == BUTTON_EXIT))
      {
        State = TSTPRG_SKIP;
      }
    }
    break;

    case SYSTEM_UNLOCK :
    {
      pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_TEST;
      pMapDisplay->UpdateMask            |= TEXTLINE_BIT(TEXTLINE_8);
      State = SYSTEM_PAGE;
    }
    break;

    case SYSTEM_PAGE :
    {
      switch (cUiReadButtons())
      {
        case BUTTON_ENTER :
        {
          IOMapUi.Flags                      &= ~UI_ENABLE_STATUS_UPDATE;
          pMapDisplay->EraseMask             |=  SCREEN_BIT(SCREEN_BACKGROUND);
          State = TSTPRG_INIT;
        }
        break;

        case BUTTON_EXIT :
        {
          Count = 0;
          State = TSTPRG_SKIP;
        }
        break;

        case BUTTON_LEFT :
        {
          IOMapUi.Flags                      &= ~UI_ENABLE_STATUS_UPDATE;
          pMapDisplay->EraseMask             |=  SCREEN_BIT(SCREEN_BACKGROUND);
          pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_2] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_3] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_4] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_5] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_6] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_7] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_TIMER;
          pMapDisplay->EraseMask             |= TEXTLINE_BITS;
          pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
          State = TIMER_INIT;
        }
        break;

        case BUTTON_RIGHT :
        {
          IOMapUi.Flags                      &= ~UI_ENABLE_STATUS_UPDATE;
          pMapDisplay->EraseMask             |=  SCREEN_BIT(SCREEN_BACKGROUND);
          pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_2] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_3] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_4] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_5] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_6] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_7] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_BT_PAGE;
          if (DISPLAYLINE_LENGTH >= 16)
          {
            sprintf((char*)TxtBuffer[2],"Command       %02X",(UWORD)VarsUi.BTCommand & 0xFF);
            sprintf((char*)TxtBuffer[3],"Parameter 1   %02X",(UWORD)VarsUi.BTPar1 & 0xFF);
            sprintf((char*)TxtBuffer[4],"Parameter 2   %02X",(UWORD)VarsUi.BTPar2 & 0xFF);
            sprintf((char*)TxtBuffer[5],"Result      %04X",(UWORD)VarsUi.BTResult);
            pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_LAST;
            pMapDisplay->pTextLines[TEXTLINE_2] = (UBYTE*)TXT_LINE;
            pMapDisplay->pTextLines[TEXTLINE_3] = TxtBuffer[2];
            pMapDisplay->pTextLines[TEXTLINE_4] = TxtBuffer[3];
            pMapDisplay->pTextLines[TEXTLINE_5] = TxtBuffer[4];
            pMapDisplay->pTextLines[TEXTLINE_6] = TxtBuffer[5];
          }
          pMapDisplay->EraseMask             |= TEXTLINE_BITS;
          pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
          State = BT_PAGE;
        }
        break;

      }
    }
    break;

    case TIMER_INIT :
    {
      State = TIMER_SHOW;
    }
    break;

    case TIMER_SHOW :
    {
      sprintf((char*)TxtBuffer[2]," %10lu mS  ",VarsUi.CRPasskey);
      pMapDisplay->pTextLines[TEXTLINE_3] = TxtBuffer[2];
      pMapDisplay->UpdateMask            |= TEXTLINE_BIT(TEXTLINE_3);

      switch (cUiReadButtons())
      {
        case BUTTON_ENTER :
        {
          pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_TIMER_HOLD;
          pMapDisplay->UpdateMask            |= TEXTLINE_BIT(TEXTLINE_8);
          State = TIMER_HOLD;
        }
        break;

        case BUTTON_LEFT :
        {
          pMapDisplay->EraseMask             |= TEXTLINE_BIT(TEXTLINE_3);
          VarsUi.CRPasskey = 0L;
        }
        break;

        case BUTTON_EXIT :
        {
          State = TSTPRG_SKIP;
        }
        break;

      }
    }
    break;

    case TIMER_HOLD :
    {
      switch (cUiReadButtons())
      {
        case BUTTON_ENTER :
        {
          pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_TIMER;
          pMapDisplay->UpdateMask            |= TEXTLINE_BIT(TEXTLINE_8);
          State = TIMER_SHOW;
        }
        break;

        case BUTTON_EXIT :
        {
          State = TSTPRG_SKIP;
        }
        break;

      }
    }
    break;

    case BT_PAGE :
    {
      switch (cUiReadButtons())
      {
        case BUTTON_ENTER :
        {
          for (Count = 0;Count < TEXTLINES;Count++)
          {
            strcpy((char*)TxtBuffer[Count],(char*)TXT_EMPTY);
            pMapDisplay->pTextLines[TEXTLINE_1 + Count] = TxtBuffer[Count];
          }
          pMapDisplay->pTextLines[TEXTLINE_2] = (UBYTE*)TXT_LINE;
          pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_BT_LIST;
          pMapDisplay->EraseMask             |= TEXTLINE_BITS;
          pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
          Pointer = 0;
          State   = BT_LIST_INIT;
        }
        break;

        case BUTTON_EXIT :
        {
          Count = 0;
          State = TSTPRG_SKIP;
        }
        break;

        case BUTTON_LEFT :
        {
          State = BT_RESET;
        }
        break;

        case BUTTON_RIGHT :
        {
          pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_BTUPDATE;
          pMapDisplay->pTextLines[TEXTLINE_2] = (UBYTE*)TXT_LINE;
          pMapDisplay->pTextLines[TEXTLINE_3] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_4] = (UBYTE*)TXT_DONE;
          pMapDisplay->pTextLines[TEXTLINE_5] = (UBYTE*)TXT_RESET;
          pMapDisplay->pTextLines[TEXTLINE_6] = (UBYTE*)TXT_REBOOT;
          pMapDisplay->pTextLines[TEXTLINE_7] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->EraseMask             |= TEXTLINE_BITS;
          pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
          Timer = 0;
          State = BT_UPDATE_FW;
        }
        break;

      }
    }
    break;

    case BT_RESET :
    {
      VarsUi.BTCommand  = (UBYTE)FACTORYRESET;
      VarsUi.BTPar1     = (UBYTE)0;
      VarsUi.BTPar2     = (UBYTE)0;
      if (pMapComm->pFunc(VarsUi.BTCommand,VarsUi.BTPar1,VarsUi.BTPar2,0,NULL,&(VarsUi.BTResult)) == SUCCESS)
      {
        pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_EMPTY;
        pMapDisplay->pTextLines[TEXTLINE_2] = (UBYTE*)TXT_EMPTY;
        pMapDisplay->pTextLines[TEXTLINE_3] = (UBYTE*)TXT_EMPTY;
        pMapDisplay->pTextLines[TEXTLINE_4] = (UBYTE*)TXT_RESETTING;
        pMapDisplay->pTextLines[TEXTLINE_5] = (UBYTE*)TXT_EMPTY;
        pMapDisplay->pTextLines[TEXTLINE_6] = (UBYTE*)TXT_EMPTY;
        pMapDisplay->pTextLines[TEXTLINE_7] = (UBYTE*)TXT_EMPTY;
        pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_EMPTY;
        pMapDisplay->EraseMask             |= TEXTLINE_BITS;
        pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
        State = BT_RESETTING;
      }
      else
      {
        State = TSTPRG_SKIP;
      }
    }
    break;

    case BT_RESETTING :
    {
      if (VarsUi.BTResult != INPROGRESS)
      {
        State = TSTPRG_SKIP;
      }
    }
    break;

    case BT_UPDATE_FW :
    {
      if (++Timer >= 100)
      {
        BtIo();
      }
    }
    break;

    case BT_LIST_INIT :
    {
      sprintf((char*)TxtBuffer[0],"DeviceTable No%2u",Pointer);
      sprintf((char*)TxtBuffer[2],"%-*.*s",DISPLAYLINE_LENGTH,DISPLAYLINE_LENGTH,(char*)pMapComm->BtDeviceTable[Pointer].Name);
      Count = (UWORD)sprintf((char*)TxtBuffer[3],"COD=");
      for (Tmp = 0;(Tmp < SIZE_OF_CLASS_OF_DEVICE) && (Count < (DISPLAYLINE_LENGTH - 2));Tmp++)
      {
        Count += (UWORD)sprintf((char*)&TxtBuffer[3][Count],"%02X",(UWORD)(pMapComm->BtDeviceTable[Pointer].ClassOfDevice[Tmp]) & 0xFF);
      }
      Count = (UWORD)sprintf((char*)TxtBuffer[4],"A=");
      for (Tmp = 0;(Tmp < SIZE_OF_BDADDR) && (Count <= (DISPLAYLINE_LENGTH - 2));Tmp++)
      {
        Count += (UWORD)sprintf((char*)&TxtBuffer[4][Count],"%02X",(UWORD)(pMapComm->BtDeviceTable[Pointer].BdAddr[Tmp]) & 0xFF);
      }
      sprintf((char*)TxtBuffer[5],"Status=%02X",(UWORD)(pMapComm->BtDeviceTable[Pointer].DeviceStatus) & 0xFF);
      pMapDisplay->EraseMask             |= TEXTLINE_BITS;
      pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
      State = BT_LIST;
    }
    break;

    case BT_LIST :
    {
      switch (cUiReadButtons())
      {
        case BUTTON_ENTER :
        {
          for (Count = 0;Count < TEXTLINES;Count++)
          {
            strcpy((char*)TxtBuffer[Count],(char*)TXT_EMPTY);
            pMapDisplay->pTextLines[TEXTLINE_1 + Count] = TxtBuffer[Count];
          }
          pMapDisplay->pTextLines[TEXTLINE_2] = (UBYTE*)TXT_LINE;
          pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_BT_CONN;
          pMapDisplay->EraseMask             |= TEXTLINE_BITS;
          pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
          Pointer = 0;
          State   = BT_CONN_INIT;
        }
        break;

        case BUTTON_EXIT :
        {
          State = SYSTEM_INIT;
        }
        break;

        case BUTTON_LEFT :
        {
          if (Pointer)
          {
            Pointer--;
          }
          else
          {
            Pointer = (SIZE_OF_BT_DEVICE_TABLE - 1);
          }
          State = BT_LIST_INIT;
        }
        break;

        case BUTTON_RIGHT :
        {
          if (Pointer < (SIZE_OF_BT_DEVICE_TABLE - 1))
          {
            Pointer++;
          }
          else
          {
            Pointer = 0;
          }
          State = BT_LIST_INIT;
        }
        break;

      }
    }
    break;

    case BT_CONN_INIT :
    {
      sprintf((char*)TxtBuffer[0],"Conn. Table No%2u",Pointer);
      sprintf((char*)TxtBuffer[2],"%-*.*s",DISPLAYLINE_LENGTH,DISPLAYLINE_LENGTH,(char*)pMapComm->BtConnectTable[Pointer].Name);
      Count = (UWORD)sprintf((char*)TxtBuffer[3],"COD=");
      for (Tmp = 0;(Tmp < SIZE_OF_CLASS_OF_DEVICE) && (Count < (DISPLAYLINE_LENGTH - 2));Tmp++)
      {
        Count += (UWORD)sprintf((char*)&TxtBuffer[3][Count],"%02X",(UWORD)(pMapComm->BtConnectTable[Pointer].ClassOfDevice[Tmp]) & 0xFF);
      }
      Count = (UWORD)sprintf((char*)TxtBuffer[4],"A=");
      for (Tmp = 0;(Tmp < SIZE_OF_BDADDR) && (Count <= (DISPLAYLINE_LENGTH - 2));Tmp++)
      {
        Count += (UWORD)sprintf((char*)&TxtBuffer[4][Count],"%02X",(UWORD)(pMapComm->BtConnectTable[Pointer].BdAddr[Tmp]) & 0xFF);
      }
      sprintf((char*)TxtBuffer[5],"%-*.*s",DISPLAYLINE_LENGTH,DISPLAYLINE_LENGTH,(char*)pMapComm->BtConnectTable[Pointer].PinCode);
      if (DISPLAYLINE_LENGTH >= 16)
      {
        sprintf((char*)TxtBuffer[6],"H=%02X  S=%02X  Q=%02X",(UWORD)(pMapComm->BtConnectTable[Pointer].HandleNr) & 0xFF,(UWORD)(pMapComm->BtConnectTable[Pointer].StreamStatus) & 0xFF,(UWORD)(pMapComm->BtConnectTable[Pointer].LinkQuality) & 0xFF);
      }
      pMapDisplay->EraseMask             |= TEXTLINE_BITS;
      pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
      State = BT_CONN;
    }
    break;

    case BT_CONN :
    {
      switch (cUiReadButtons())
      {
        case BUTTON_ENTER :
        {
          State = BT_CONN_INIT;
        }
        break;

        case BUTTON_EXIT :
        {
          State = SYSTEM_INIT;
        }
        break;

        case BUTTON_LEFT :
        {
          if (Pointer)
          {
            Pointer--;
          }
          else
          {
            Pointer = (SIZE_OF_BT_CONNECT_TABLE - 1);
          }
          State = BT_CONN_INIT;
        }
        break;

        case BUTTON_RIGHT :
        {
          if (Pointer < (SIZE_OF_BT_CONNECT_TABLE - 1))
          {
            Pointer++;
          }
          else
          {
            Pointer = 0;
          }
          State = BT_CONN_INIT;
        }
        break;

      }
    }
    break;

    case TSTPRG_INIT :
    {
      IOMapUi.Flags                      &= ~UI_ENABLE_STATUS_UPDATE;

      pMapDisplay->EraseMask             |=  SCREEN_BIT(SCREEN_BACKGROUND);

      pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_TESTPRG;
      pMapDisplay->pTextLines[TEXTLINE_2] = (UBYTE*)TXT_LINE;
      pMapDisplay->pTextLines[TEXTLINE_3] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_4] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_5] = (UBYTE*)TXT_SELECT;
      pMapDisplay->pTextLines[TEXTLINE_6] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_7] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_SUBTEST;
      pMapDisplay->UpdateMask            |= TEXTLINE_BITS;

      State = TSTPRG_SELECT_SUBTEST;
    }
    break;

    case TSTPRG_SELECT_SUBTEST :
    {
      switch (cUiReadButtons())
      {
        case BUTTON_LEFT :
        {
          State = TSTPRG_SENSOR_INIT;
        }
        break;

        case BUTTON_RIGHT :
        {
          State = TSTPRG_MOTOR_INIT;
        }
        break;

        case BUTTON_ENTER :
        {
          State = TSTPRG_RCX_INIT;
        }
        break;

        case BUTTON_EXIT :
        {
          Count = 0;
          State = TSTPRG_SKIP;
        }
        break;

      }
    }
    break;

    case TSTPRG_SENSOR_INIT :
    {
      pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_TESTPRG;
      pMapDisplay->pTextLines[TEXTLINE_2] = (UBYTE*)TXT_LINE;
      pMapDisplay->pTextLines[TEXTLINE_3] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_4] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_5] = (UBYTE*)TXT_SELECT_SENSOR;
      pMapDisplay->pTextLines[TEXTLINE_6] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_7] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_SENSORS;
      pMapDisplay->UpdateMask            |= TEXTLINE_BITS;

      for (Count = 0;Count < NO_OF_INPUTS;Count++)
      {
        InputValues[Count] = 0x7FFF;
        strcpy((char*)TxtBuffer[Count],"                ");
      }

      State = TSTPRG_SELECT_SENSOR;
    }
    break;

    case TSTPRG_SELECT_SENSOR :
    {
      switch (cUiReadButtons())
      {
        case BUTTON_LEFT :
        {
          State = TSTPRG_TOUCH_SENSOR_INIT;
        }
        break;

        case BUTTON_ENTER :
        {
          pMapDisplay->pTextLines[TEXTLINE_5] = (UBYTE*)TXT_SELECT_TYPE;
          pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_SOUND_SENSORS;
          pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
          State = TSTPRG_SOUND_SENSOR_SELECT;
        }
        break;

        case BUTTON_RIGHT :
        {
          pMapDisplay->pTextLines[TEXTLINE_5] = (UBYTE*)TXT_SELECT_TYPE;
          pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_LIGHT_SENSORS;
          pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
          State = TSTPRG_LIGHT_SENSOR_SELECT;
        }
        break;

        case BUTTON_EXIT :
        {
          State = TSTPRG_INIT;
        }
        break;

      }
    }
    break;

    case TSTPRG_TOUCH_SENSOR_INIT :
    {
      for (Count = 0;Count < NO_OF_INPUTS;Count++)
      {
        pMapDisplay->pTextLines[TEXTLINE_3 + Count] = TxtBuffer[Count];
      }
      pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_SENSOR_TOUCH;
      pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_SUBTEST_STOP;
      pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
      for (Count = 0;Count < NO_OF_INPUTS;Count++)
      {
        pMapInput->Inputs[Count].SensorType     = SWITCH;
      }
      State = TSTPRG_TOUCH_SENSOR;
    }
    break;

    case TSTPRG_TOUCH_SENSOR :
    {
      for (Count = 0;Count < NO_OF_INPUTS;Count++)
      {
        if (InputValues[Count] != pMapInput->Inputs[Count].ADRaw)
        {
          InputValues[Count]    = pMapInput->Inputs[Count].ADRaw;
          sprintf((char*)TxtBuffer[Count]," Input %u = %4u ",(UWORD)Count + 1,(UWORD)InputValues[Count]);
          pMapDisplay->UpdateMask |= TEXTLINE_BIT(TEXTLINE_3 + Count);
        }
      }
      if (cUiReadButtons() != BUTTON_NONE)
      {
        State = TSTPRG_SKIP_SENSOR;
      }
    }
    break;

    case TSTPRG_SOUND_SENSOR_SELECT :
    {
      switch (cUiReadButtons())
      {
        case BUTTON_LEFT :
        {
          pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_SENSOR_SOUND_DB;
          for (Count = 0;Count < NO_OF_INPUTS;Count++)
          {
            pMapInput->Inputs[Count].SensorType     = SOUND_DB;
          }
          State = TSTPRG_SOUND_SENSOR_INIT;
        }
        break;

        case BUTTON_ENTER :
        {
          State = TSTPRG_SKIP_SENSOR;
        }
        break;

        case BUTTON_EXIT :
        {
          State = TSTPRG_SKIP_SENSOR;
        }
        break;

        case BUTTON_RIGHT :
        {
          pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_SENSOR_SOUND_DBA;
          for (Count = 0;Count < NO_OF_INPUTS;Count++)
          {
            pMapInput->Inputs[Count].SensorType     = SOUND_DBA;
          }
          State = TSTPRG_SOUND_SENSOR_INIT;
        }
        break;

      }
    }
    break;

    case TSTPRG_SOUND_SENSOR_INIT :
    {
      VolumeSave        = pMapSound->Volume;
      pMapSound->Volume = MAX_VOLUME;
      for (Count = 0;Count < NO_OF_INPUTS;Count++)
      {
        pMapDisplay->pTextLines[TEXTLINE_3 + Count] = TxtBuffer[Count];
      }
      pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_SOUND_STOP;
      pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
      State = TSTPRG_SOUND_SENSOR;
    }
    break;

    case TSTPRG_SOUND_SENSOR :
    {
      for (Count = 0;Count < NO_OF_INPUTS;Count++)
      {
        if (InputValues[Count] != pMapInput->Inputs[Count].ADRaw)
        {
          InputValues[Count]    = pMapInput->Inputs[Count].ADRaw;
          sprintf((char*)TxtBuffer[Count]," Input %u = %4u ",(UWORD)Count + 1,(UWORD)InputValues[Count]);
          pMapDisplay->UpdateMask |= TEXTLINE_BIT(TEXTLINE_3 + Count);
        }
      }
      switch (cUiReadButtons())
      {
        case BUTTON_LEFT :
        {
          pMapSound->Freq     =  440;
          pMapSound->Duration =  2000;
          pMapSound->Mode     =  SOUND_TONE;
          pMapSound->Flags   |=  SOUND_UPDATE;
        }
        break;

        case BUTTON_ENTER :
        {
          pMapSound->State    =  SOUND_STOP;
          pMapSound->Volume   =  VolumeSave;
          State               = TSTPRG_SKIP_SENSOR;
        }
        break;

        case BUTTON_EXIT :
        {
          pMapSound->State    =  SOUND_STOP;
          pMapSound->Volume   =  VolumeSave;
          State               = TSTPRG_SKIP_SENSOR;
        }
        break;

        case BUTTON_RIGHT :
        {
          pMapSound->Freq     =  4000;
          pMapSound->Duration =  2000;
          pMapSound->Mode     =  SOUND_TONE;
          pMapSound->Flags   |=  SOUND_UPDATE;
        }
        break;

      }
    }
    break;

    case TSTPRG_LIGHT_SENSOR_SELECT :
    {
      switch (cUiReadButtons())
      {
        case BUTTON_LEFT :
        {
          pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_SENSOR_LIGHT_PASIVE;
          for (Count = 0;Count < NO_OF_INPUTS;Count++)
          {
            pMapInput->Inputs[Count].SensorType     = LIGHT_INACTIVE;
          }
          State = TSTPRG_LIGHT_SENSOR_INIT;
        }
        break;

        case BUTTON_ENTER :
        {
          State = TSTPRG_SKIP_SENSOR;
        }
        break;

        case BUTTON_EXIT :
        {
          State = TSTPRG_SKIP_SENSOR;
        }
        break;

        case BUTTON_RIGHT :
        {
          pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_SENSOR_LIGHT_ACTIVE;
          for (Count = 0;Count < NO_OF_INPUTS;Count++)
          {
            pMapInput->Inputs[Count].SensorType     = LIGHT_ACTIVE;
          }
          State = TSTPRG_LIGHT_SENSOR_INIT;
        }
        break;

      }
    }
    break;

    case TSTPRG_LIGHT_SENSOR_INIT :
    {
      for (Count = 0;Count < NO_OF_INPUTS;Count++)
      {
        pMapDisplay->pTextLines[TEXTLINE_3 + Count] = TxtBuffer[Count];
      }
      pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_SUBTEST_STOP;
      pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
      State = TSTPRG_LIGHT_SENSOR;
    }
    break;

    case TSTPRG_LIGHT_SENSOR :
    {
      for (Count = 0;Count < NO_OF_INPUTS;Count++)
      {
        if (InputValues[Count] != pMapInput->Inputs[Count].ADRaw)
        {
          InputValues[Count]    = pMapInput->Inputs[Count].ADRaw;
          sprintf((char*)TxtBuffer[Count]," Input %u = %4u ",(UWORD)Count + 1,(UWORD)InputValues[Count]);
          pMapDisplay->UpdateMask |= TEXTLINE_BIT(TEXTLINE_3 + Count);
        }
      }
      if (cUiReadButtons() != BUTTON_NONE)
      {
        State = TSTPRG_SKIP_SENSOR;
      }
    }
    break;

    case TSTPRG_SKIP_SENSOR :
    {
      for (Count = 0;Count < NO_OF_INPUTS;Count++)
      {
        pMapInput->Inputs[Count].SensorType     = NO_SENSOR;
      }
      State = TSTPRG_SENSOR_INIT;
    }
    break;

    case TSTPRG_MOTOR_INIT :
    {
      pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_MOTOR;
      pMapDisplay->pTextLines[TEXTLINE_2] = (UBYTE*)TXT_LINE;
      pMapDisplay->pTextLines[TEXTLINE_3] = (UBYTE*)TXT_MOTOR_HEADER;
      pMapDisplay->pTextLines[TEXTLINE_4] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_5] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_6] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_7] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_MOTOR_STOP;

      for (Count = 0;Count < NO_OF_OUTPUTS;Count++)
      {
        OutputValues[Count] = 0x7FFF;
        TestPrgRunMotor(Count,0);
        strcpy((char*)TxtBuffer[Count],"                ");
        pMapDisplay->pTextLines[TEXTLINE_4 + Count] = TxtBuffer[Count];
      }

      pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
      State = TSTPRG_MOTOR;
    }
    break;

    case TSTPRG_MOTOR :
    {
      for (Count = 0;Count < NO_OF_OUTPUTS;Count++)
      {
        if (OutputValues[Count] != (SWORD)TestPrgReadMotor(Count))
        {
          OutputValues[Count] = (SWORD)TestPrgReadMotor(Count);
          sprintf((char*)TxtBuffer[Count],"  %c   %-4d  %4d",(char)Count + 'A',(SWORD)pMapOutPut->Outputs[Count].Speed,OutputValues[Count]);
          pMapDisplay->UpdateMask |= TEXTLINE_BIT(TEXTLINE_4 + Count);
        }
      }
      switch (cUiReadButtons())
      {
        case BUTTON_LEFT :
        {
          for (Count = 0;Count < NO_OF_OUTPUTS;Count++)
          {
            OutputValues[Count] = 0x7FFF;
            TestPrgRunMotor(Count,-50);
          }
        }
        break;

        case BUTTON_ENTER :
        {
          for (Count = 0;Count < NO_OF_OUTPUTS;Count++)
          {
            OutputValues[Count] = 0x7FFF;
            TestPrgRunMotor(Count,0);
          }
        }
        break;

        case BUTTON_EXIT :
        {
          State = TSTPRG_SKIP_MOTOR;
        }
        break;

        case BUTTON_RIGHT :
        {
          for (Count = 0;Count < NO_OF_OUTPUTS;Count++)
          {
            OutputValues[Count] = 0x7FFF;
            TestPrgRunMotor(Count,50);
          }
        }
        break;

      }
    }
    break;

    case TSTPRG_SKIP_MOTOR :
    {
      for (Count = 0;Count < NO_OF_OUTPUTS;Count++)
      {
        TestPrgFloatMotor(Count);
      }
      State = TSTPRG_INIT;
    }
    break;

    case TSTPRG_RCX_INIT :
    {
      pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_RCX;
      pMapDisplay->pTextLines[TEXTLINE_2] = (UBYTE*)TXT_LINE;
      pMapDisplay->pTextLines[TEXTLINE_3] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_4] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_5] = (UBYTE*)TXT_SELECT;
      pMapDisplay->pTextLines[TEXTLINE_6] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_7] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_RCX_STOP;

      pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
      State = TSTPRG_RCX_SELECT;
    }
    break;

    case TSTPRG_RCX_SELECT :
    {
      switch (cUiReadButtons())
      {
        case BUTTON_LEFT :
        {
          pMapDisplay->pTextLines[TEXTLINE_5] = (UBYTE*)TXT_SELECT_TYPE;
          pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_RCX_INPUT_SELECT;
          pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
          for (Count = 0;Count < NO_OF_INPUTS;Count++)
          {
            InputValues[Count] = 0x7FFF;
            strcpy((char*)TxtBuffer[Count],"                ");
          }
          State = TSTPRG_RCX_INPUT_SELECT;
        }
        break;

        case BUTTON_ENTER :
        {
          State = TSTPRG_RCX_DISPLAY_INIT;
        }
        break;

        case BUTTON_RIGHT :
        {
          State = TSTPRG_RCX_MOTOR_INIT;
        }
        break;

        case BUTTON_EXIT :
        {
          State = TSTPRG_INIT;
        }
        break;

      }
    }
    break;

    case TSTPRG_RCX_DISPLAY_INIT :
    {
      Count = 0;
      pMapDisplay->EraseMask             |=  SCREEN_BIT(SCREEN_BACKGROUND);
      State = TSTPRG_RCX_DISPLAY;
    }
    break;

    case TSTPRG_RCX_DISPLAY :
    {
      if ((Count & 0x7FF) == 0x000)
      {
        pMapDisplay->pScreens[SCREEN_BACKGROUND]       =  (BMPMAP*)Test1;
        pMapDisplay->UpdateMask                       |=  SCREEN_BIT(SCREEN_BACKGROUND);
      }
      if ((Count & 0x7FF) == 0x3FF)
      {
        pMapDisplay->pScreens[SCREEN_BACKGROUND]       =  (BMPMAP*)Test2;
        pMapDisplay->UpdateMask                       |=  SCREEN_BIT(SCREEN_BACKGROUND);
      }
      Count++;
      if (cUiReadButtons() != BUTTON_NONE)
      {
        State = TSTPRG_SKIP_RCX;
      }
    }
    break;

    case TSTPRG_RCX_INPUT_SELECT :
    {
      switch (cUiReadButtons())
      {
        case BUTTON_LEFT :
        {
          pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_RCX_INPUT_PASIVE;
          for (Count = 0;Count < NO_OF_INPUTS;Count++)
          {
            pMapInput->Inputs[Count].SensorType     = SWITCH;
          }
          State = TSTPRG_RCX_INPUT_INIT;
        }
        break;

        case BUTTON_ENTER :
        {
          pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_RCX_INPUT_ACTIVE;
          for (Count = 0;Count < NO_OF_INPUTS;Count++)
          {
            pMapInput->Inputs[Count].SensorType     = REFLECTION;
          }
          State = TSTPRG_RCX_INPUT_INIT;
        }
        break;

        case BUTTON_EXIT :
        {
          State = TSTPRG_SKIP_RCX;
        }
        break;

        case BUTTON_RIGHT :
        {
          pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_RCX_INPUT_DIGITAL;
          pMapDisplay->pTextLines[TEXTLINE_2] = (UBYTE*)TXT_LINE;
          pMapDisplay->pTextLines[TEXTLINE_3] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_4] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_5] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_6] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_7] = (UBYTE*)TXT_EMPTY;
          pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_SUBTEST_STOP;
          pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
          for (Count = 0;Count < NO_OF_INPUTS;Count++)
          {
            pMapInput->Inputs[Count].SensorType     = CUSTOM;
          }
          SubState = 0;
          Timer    = 0;
          State    = TSTPRG_RCX_DIGITAL_INIT;
        }
        break;

      }
    }
    break;

    case TSTPRG_RCX_INPUT_INIT :
    {
      for (Count = 0;Count < NO_OF_INPUTS;Count++)
      {
        pMapDisplay->pTextLines[TEXTLINE_3 + Count] = TxtBuffer[Count];
      }
      pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_SUBTEST_STOP;
      pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
      State = TSTPRG_RCX_INPUT;
      Timer = 0;
    }
    break;

    case TSTPRG_RCX_INPUT :
    {
      if (++Timer >= 250)
      {
        Timer = 0;
        for (Count = 0;Count < NO_OF_INPUTS;Count++)
        {
          if (InputValues[Count] != pMapInput->Inputs[Count].ADRaw)
          {
            InputValues[Count]    = pMapInput->Inputs[Count].ADRaw;
            sprintf((char*)TxtBuffer[Count]," Input %u = %4u ",(UWORD)Count + 1,(UWORD)InputValues[Count]);
            pMapDisplay->UpdateMask |= TEXTLINE_BIT(TEXTLINE_3 + Count);
          }
        }
      }
      if (cUiReadButtons() != BUTTON_NONE)
      {
        State = TSTPRG_SKIP_RCX;
      }
    }
    break;

    case TSTPRG_RCX_DIGITAL_INIT :
    {
      if (++Timer >= 20)
      {
        Timer = 0;
        switch (SubState)
        {
          case 0 :
          {
            pMapInput->Inputs[0].DigiPinsDir |=  DIGI0;         // Digi 1-0 output        -,
            pMapInput->Inputs[0].DigiPinsDir &= ~DIGI1;         // Digi 1-1 input     -,   |
            pMapInput->Inputs[2].DigiPinsDir &= ~DIGI0;         // Digi 3-0 input      |  -'
            pMapInput->Inputs[2].DigiPinsDir |=  DIGI1;         // Digi 3-1 output    -'

            pMapInput->Inputs[0].DigiPinsOut |=  DIGI0;         // Digi 1-0 output   high
            pMapInput->Inputs[2].DigiPinsOut &= ~DIGI1;         // Digi 3-1 output   low

            SubState++;
          }
          break;

          case 1 :
          {
            if ((pMapInput->Inputs[2].DigiPinsIn & DIGI0) && !(pMapInput->Inputs[0].DigiPinsIn & DIGI1))
            {
              pMapInput->Inputs[0].DigiPinsOut &= ~DIGI0;         // Digi 1-0 output   low
              pMapInput->Inputs[2].DigiPinsOut |=  DIGI1;         // Digi 3-1 output   high

              SubState++;
            }
            else
            {
              State = TSTPRG_RCX_DIGITAL_FAIL;
            }
          }
          break;

          case 2 :
          {
            if (!(pMapInput->Inputs[2].DigiPinsIn & DIGI0) && (pMapInput->Inputs[0].DigiPinsIn & DIGI1))
            {
              pMapInput->Inputs[0].DigiPinsDir &= ~DIGI0;         // Digi 1-0 input
              pMapInput->Inputs[0].DigiPinsDir &= ~DIGI1;         // Digi 1-1 input
              pMapInput->Inputs[2].DigiPinsDir &= ~DIGI0;         // Digi 3-0 input
              pMapInput->Inputs[2].DigiPinsDir &= ~DIGI1;         // Digi 3-1 input

              pMapInput->Inputs[1].DigiPinsDir |=  DIGI0;         // Digi 2-0 output        -,
              pMapInput->Inputs[1].DigiPinsDir &= ~DIGI1;         // Digi 2-1 input     -,   |
              pMapInput->Inputs[3].DigiPinsDir &= ~DIGI0;         // Digi 4-0 input      |  -'
              pMapInput->Inputs[3].DigiPinsDir |=  DIGI1;         // Digi 4-1 output    -'

              pMapInput->Inputs[1].DigiPinsOut |=  DIGI0;         // Digi 2-0 output   high
              pMapInput->Inputs[3].DigiPinsOut &= ~DIGI1;         // Digi 4-1 output   low

              SubState++;
            }
            else
            {
              State = TSTPRG_RCX_DIGITAL_FAIL;
            }
          }
          break;

          case 3 :
          {
            if ((pMapInput->Inputs[3].DigiPinsIn & DIGI0) && !(pMapInput->Inputs[1].DigiPinsIn & DIGI1))
            {
              pMapInput->Inputs[1].DigiPinsOut &= ~DIGI0;         // Digi 2-0 output   low
              pMapInput->Inputs[3].DigiPinsOut |=  DIGI1;         // Digi 4-1 output   high

              SubState++;
            }
            else
            {
              State = TSTPRG_RCX_DIGITAL_FAIL;
            }
          }
          break;

          case 4 :
          {
            if (!(pMapInput->Inputs[3].DigiPinsIn & DIGI0) && (pMapInput->Inputs[1].DigiPinsIn & DIGI1))
            {
              pMapInput->Inputs[1].DigiPinsDir &= ~DIGI0;         // Digi 2-0 input
              pMapInput->Inputs[1].DigiPinsDir &= ~DIGI1;         // Digi 2-1 input
              pMapInput->Inputs[3].DigiPinsDir &= ~DIGI0;         // Digi 4-0 input
              pMapInput->Inputs[3].DigiPinsDir &= ~DIGI1;         // Digi 4-1 input

              State = TSTPRG_RCX_DIGITAL_OK;
            }
            else
            {
              State = TSTPRG_RCX_DIGITAL_FAIL;
            }
          }
          break;

        }
      }
    }
    break;

    case TSTPRG_RCX_DIGITAL_OK :
    {
      pMapDisplay->pTextLines[TEXTLINE_5] = (UBYTE*)TXT_RCX_DIGITAL_OK;
      pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
      State = TSTPRG_RCX_DIGITAL;
    }
    break;

    case TSTPRG_RCX_DIGITAL_FAIL :
    {
      pMapDisplay->pTextLines[TEXTLINE_5] = (UBYTE*)TXT_RCX_DIGITAL_FAIL;
      pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
      State = TSTPRG_RCX_DIGITAL;
    }
    break;

    case TSTPRG_RCX_DIGITAL :
    {
      if (cUiReadButtons() != BUTTON_NONE)
      {
        State = TSTPRG_SKIP_RCX;
      }
    }
    break;

    case TSTPRG_RCX_MOTOR_INIT :
    {
      pMapDisplay->pTextLines[TEXTLINE_1] = (UBYTE*)TXT_MOTOR;
      pMapDisplay->pTextLines[TEXTLINE_2] = (UBYTE*)TXT_LINE;
      pMapDisplay->pTextLines[TEXTLINE_3] = (UBYTE*)TXT_MOTOR_HEADER;
      pMapDisplay->pTextLines[TEXTLINE_4] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_5] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_6] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_7] = (UBYTE*)TXT_EMPTY;
      pMapDisplay->pTextLines[TEXTLINE_8] = (UBYTE*)TXT_MOTOR_NEXT;

      for (Count = 0;Count < NO_OF_OUTPUTS;Count++)
      {
        OutputValues[Count] = 0x7FFF;
        TestPrgRunMotor(Count,0);
        strcpy((char*)TxtBuffer[Count],"                ");
        pMapDisplay->pTextLines[TEXTLINE_4 + Count] = TxtBuffer[Count];
      }

      Pointer = 0;
      pMapDisplay->UpdateMask            |= TEXTLINE_BITS;
      State = TSTPRG_RCX_MOTOR;
    }
    break;

    case TSTPRG_RCX_MOTOR :
    {
      for (Count = 0;Count < NO_OF_OUTPUTS;Count++)
      {
        if (OutputValues[Count] != (SWORD)TestPrgReadMotor(Count))
        {
          OutputValues[Count] = (SWORD)TestPrgReadMotor(Count);
          if (Pointer == Count)
          {
            sprintf((char*)TxtBuffer[Count],"> %c   %-4d  %4d",(char)Count + 'A',(SWORD)pMapOutPut->Outputs[Count].Speed,OutputValues[Count]);
          }
          else
          {
            sprintf((char*)TxtBuffer[Count],"  %c   %-4d  %4d",(char)Count + 'A',(SWORD)pMapOutPut->Outputs[Count].Speed,OutputValues[Count]);
          }
          pMapDisplay->UpdateMask |= TEXTLINE_BIT(TEXTLINE_4 + Count);
        }
      }
      switch (cUiReadButtons())
      {
        case BUTTON_LEFT :
        {
          for (Count = 0;Count < NO_OF_OUTPUTS;Count++)
          {
            OutputValues[Count]               = 0x7FFF;
            if (Pointer == Count)
            {
              TestPrgRunMotor(Count,-50);
            }
            else
            {
              TestPrgRunMotor(Count,0);
            }
          }
        }
        break;

        case BUTTON_ENTER :
        {
          for (Count = 0;Count < NO_OF_OUTPUTS;Count++)
          {
            OutputValues[Count] = 0x7FFF;
            TestPrgRunMotor(Count,0);
          }
          if (++Pointer >= NO_OF_OUTPUTS)
          {
            State = TSTPRG_SKIP_RCX_MOTOR;
          }
        }
        break;

        case BUTTON_EXIT :
        {
          State = TSTPRG_SKIP_RCX_MOTOR;
        }
        break;

        case BUTTON_RIGHT :
        {
          for (Count = 0;Count < NO_OF_OUTPUTS;Count++)
          {
            OutputValues[Count] = 0x7FFF;
            if (Pointer == Count)
            {
              TestPrgRunMotor(Count,50);
            }
            else
            {
              TestPrgRunMotor(Count,0);
            }
          }
        }
        break;

      }
    }
    break;

    case TSTPRG_SKIP_RCX_MOTOR :
    {
      for (Count = 0;Count < NO_OF_OUTPUTS;Count++)
      {
        TestPrgFloatMotor(Count);
      }
      State = TSTPRG_RCX_INIT;
    }
    break;

    case TSTPRG_SKIP_RCX :
    {
      for (Count = 0;Count < NO_OF_INPUTS;Count++)
      {
        pMapInput->Inputs[Count].SensorType     = NO_SENSOR;
      }
      pMapDisplay->EraseMask   |=  SCREEN_BIT(SCREEN_BACKGROUND);
      State = TSTPRG_RCX_INIT;
    }
    break;

#endif

    case TSTPRG_SKIP :
    {
      Count++;
      if (Count == 100)
      {
        pMapDisplay->EraseMask |=  SCREEN_BIT(SCREEN_BACKGROUND);
      }
      if (Count >= 200)
      {
        IOMapUi.Flags          |= UI_REDRAW_STATUS;
        State = 0;
      }
    }
    break;

    default :
    {
      State = SYSTEM_INIT;
    }
    break;

  }

  return (State);
}
