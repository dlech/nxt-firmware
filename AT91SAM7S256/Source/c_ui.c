//
// Programmer
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dkandlun                                        $
//
// Revision date   $Date:: 10-06-08 9:26                                     $
//
// Filename        $Workfile:: c_ui.c                                        $
//
// Version         $Revision:: 7                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/c_ui.c $
//
// Platform        C
//

#include  "stdio.h"
#include  "string.h"
#include  "ctype.h"
#include  "stdconst.h"
#include  "modules.h"
#include  "c_ui.iom"
#include  "c_ui.h"
#include  "m_sched.h"
#include  "c_display.iom"
#include  "c_loader.iom"
#include  "c_button.iom"
#include  "c_sound.iom"
#include  "c_input.iom"
#include  "c_output.iom"
#include  "c_ioctrl.iom"
#include  "c_cmd.iom"
#include  "c_comm.iom"
#include  "c_lowspeed.iom"

static    IOMAPUI   IOMapUi;
static    VARSUI    VarsUi;
static    HEADER    **pHeaders;

const     HEADER  cUi =
{
  0x000C0001L,
  "Ui",
  cUiInit,
  cUiCtrl,
  cUiExit,
  (void *)&IOMapUi,
  (void *)&VarsUi,
  (UWORD)sizeof(IOMapUi),
  (UWORD)sizeof(VarsUi),
  0x0000                      // Code size - not used so far
};


// ****** GENERAL GRAPHIC RESOURCES ******************************************

#include  "Display.txt"       // Bitmap for frame used in view and datalog
#include  "LowBattery.txt"    // Bitmap showed when low battery occures
#include  "Font.txt"          // Font used for all text
#ifndef STRIPPED
#include  "Step.txt"          // Bitmap used in On Brick Programming
#endif
#include  "Cursor.txt"        // Bitmap for cursor
#include  "Running.txt"       // Icon collection used for "running" symbol
#include  "Port.txt"          // Font used for naming sensor ports in datalog/bluetooth
#include  "Ok.txt"            // Bitmap for OK buttom in get user string
#include  "Wait.txt"          // Bitmap for feedback
#include  "Fail.txt"          // Bitmap for feedback
#include  "Info.txt"          // Bitmap for feedback
#include  "Icons.txt"         // Icon collection used for menues

#ifndef STRIPPED
// ****** INTRO ANIMATION RESOURCES ******************************************

#include  "RCXintro_1.txt"    // Bitmap for picture 1  in the intro animation
#include  "RCXintro_2.txt"    // Bitmap for picture 2  in the intro animation
#include  "RCXintro_3.txt"    // Bitmap for picture 3  in the intro animation
#include  "RCXintro_4.txt"    // Bitmap for picture 4  in the intro animation
#include  "RCXintro_5.txt"    // Bitmap for picture 5  in the intro animation
#include  "RCXintro_6.txt"    // Bitmap for picture 6  in the intro animation
#include  "RCXintro_7.txt"    // Bitmap for picture 7  in the intro animation
#include  "RCXintro_8.txt"    // Bitmap for picture 8  in the intro animation
#include  "RCXintro_9.txt"    // Bitmap for picture 9  in the intro animation
#include  "RCXintro_10.txt"   // Bitmap for picture 10 in the intro animation
#include  "RCXintro_11.txt"   // Bitmap for picture 11 in the intro animation
#include  "RCXintro_12.txt"   // Bitmap for picture 12 in the intro animation
#include  "RCXintro_13.txt"   // Bitmap for picture 13 in the intro animation
#include  "RCXintro_14.txt"   // Bitmap for picture 14 in the intro animation
#include  "RCXintro_15.txt"   // Bitmap for picture 15 in the intro animation
#include  "RCXintro_16.txt"   // Bitmap for picture 16 in the intro animation

const     BMPMAP *Intro[NO_OF_INTROBITMAPS] = // Picture sequence for the intro animation
{
  (BMPMAP*)RCXintro_1,
  (BMPMAP*)RCXintro_2,
  (BMPMAP*)RCXintro_3,
  (BMPMAP*)RCXintro_4,
  (BMPMAP*)RCXintro_5,
  (BMPMAP*)RCXintro_6,
  (BMPMAP*)RCXintro_7,
  (BMPMAP*)RCXintro_8,
  (BMPMAP*)RCXintro_9,
  (BMPMAP*)RCXintro_10,
  (BMPMAP*)RCXintro_11,
  (BMPMAP*)RCXintro_12,
  (BMPMAP*)RCXintro_13,
  (BMPMAP*)RCXintro_14,
  (BMPMAP*)RCXintro_15,
  (BMPMAP*)RCXintro_16
};
#endif

// ****** STATUS LINE GRAPHIC RESOURCES **************************************

#include  "Status.txt"                  // Status icon collection file

enum STATUS_NO                          // Index in status icon collection file
{
  STATUS_NO_NOT_USED,
  STATUS_NO_RUNNING_0,
  STATUS_NO_RUNNING_1,
  STATUS_NO_RUNNING_2,
  STATUS_NO_RUNNING_3,
  STATUS_NO_RUNNING_4,
  STATUS_NO_RUNNING_5,
  STATUS_NO_RUNNING_6,
  STATUS_NO_RUNNING_7,
  STATUS_NO_RUNNING_8,
  STATUS_NO_RUNNING_9,
  STATUS_NO_RUNNING_10,
  STATUS_NO_RUNNING_11,
  STATUS_NO_BATTERY_0,
  STATUS_NO_BATTERY_1,
  STATUS_NO_BATTERY_2,
  STATUS_NO_BATTERY_3,
  STATUS_NO_BATTERY_4,
  STATUS_NO_BATTERY_5,
  STATUS_NO_RECHARGEABLE_0,
  STATUS_NO_RECHARGEABLE_1,
  STATUS_NO_RECHARGEABLE_2,
  STATUS_NO_RECHARGEABLE_3,
  STATUS_NO_RECHARGEABLE_4,
  STATUS_NO_RECHARGEABLE_5,
  STATUS_NO_BLUETOOTH_0,
  STATUS_NO_BLUETOOTH_1,
  STATUS_NO_BLUETOOTH_2,
  STATUS_NO_BLUETOOTH_3,
  STATUS_NO_BLUETOOTH_4,
  STATUS_NO_BLUETOOTH_5,
  STATUS_NO_USB_0,
  STATUS_NO_USB_1,
  STATUS_NO_USB_2,
  STATUS_NO_USB_3,
  STATUS_NO_USB_4,
  STATUS_NO_USB_5
};

// ****** BT DEVICE GRAPHIC RESOURCES ****************************************

#include  "Devices.txt"       // Icon collection used for Blue tooth devices

// ****** BT CONNECTIONS GRAPHIC RESOURCES ***********************************

#include  "Connections.txt"   // Icon collection used for Blue tooth connections

// ****** FREE TEXT GRAPHIC RESOURCES ****************************************

#include  "Ui.txt"            // Text strings that is'nt defined in menu files

enum                          // String index in text string file
{
  TXT_GENERAL_EMPTY,
  TXT_FB_GENERIC_FAIL,                      // "Failed!"
  
                                            // BlueTooth connect
  TXT_FB_BT_CONNECTING_WAIT,                // "Connecting"
  TXT_FB_BT_CONNECT_BUSY_FAIL,              // "Line is busy"

                                            // BlueTooth send file
  TXT_FB_BT_SENDING_NO_CONN_FAIL,           // "Connection?"
  TXT_FB_BT_SENDING_WAIT,                   // "Sending file"

                                            // BlueTooth on/off
  TXT_FB_BT_TURNING_ON_WAIT,                // "Turning on"
  TXT_FB_BT_TURNING_OFF_WAIT,               // "Turning off"

                                            // BlueTooth seach
  TXT_FB_BT_SEARCHING_WAIT,                 // "Searching"
  TXT_FB_BT_SEARCH_ABORTED_INFO,            // "Aborted!"

                                            // On Brick Programming
  TXT_FB_OBP_MEMORY_FULL_FAIL,              // "Memory full!"
  TXT_FB_FILE_SAVED_INFO,                   // "File saved"
  TXT_FB_FILE_EXIST_FAIL,                   // "File exist"
  TXT_FB_OVERWRITE_FAIL,                    // "overwrite!"

                                            // File delete
  TXT_FB_FD_FILE_DELETED_INFO,              // "File deleted"

                                            // Files delete
  TXT_FB_FD_FILES_INFO,                     // "Files"
  TXT_FB_FD_DELETED_INFO,                   // "deleted"

                                            // File run
  TXT_FILERUN_RUNNING,                      // "Running"
  TXT_FILERUN_ABORTED,                      // "Aborted!"
  TXT_FILERUN_ENDED,                        // "Ended"
  TXT_FILERUN_FILE_ERROR,                   // "File error! %d"

                                            // Files delete
  TXT_FILESDELETE_DELETING_ALL,             // "Deleting all"
  TXT_FILESDELETE_S_FILES,                  // "%s files!"

                                            // Datalogging
  TXT_DATALOGGING_PRESS_EXIT_TO,            // "Press exit to"
  TXT_DATALOGGING_STOP_DATALOGGING,         // "stop datalogging"
  TXT_DATALOGGING_PORT_OCCUPIED,            // "Port occupied!"
  TXT_DATALOGGING_RATE,                     // "H:MM:SS:00
  TXT_DATALOGGING_TIME,                     // "HH:MM:SS"

                                            // File types
  TXT_FILETYPE_SOUND,                       // "Sound"
  TXT_FILETYPE_LMS,                         // "Software"
  TXT_FILETYPE_NXT,                         // "NXT"
  TXT_FILETYPE_TRY_ME,                      // "Try me"
  TXT_FILETYPE_DATA,                        // "Datalog"

                                            // Get user string
  TXT_GETUSERSTRING_PIN,                    // "Pin:"
  TXT_GETUSERSTRING_FILENAME,               // "Filename:"

                                            // On Brick Programming
  TXT_ONBRICKPROGRAMMING_PLEASE_USE_PORT,   // "Please use port:"
  TXT_ONBRICKPROGRAMMING_1_TOUCH_SENSOR,    // "1 - Touch sensor"
  TXT_ONBRICKPROGRAMMING_2_SOUND_SENSOR,    // "2 - Sound sensor"
  TXT_ONBRICKPROGRAMMING_3_LIGHT_SENSOR,    // "3 - Light sensor"
  TXT_ONBRICKPROGRAMMING_4_ULTRA_SONIC,     // "4 - Ultra sonic"
  TXT_ONBRICKPROGRAMMING_BC_LR_MOTORS,      // "B/C - L/R motors"

                                            // View
  TXT_GENERIC_SELECT,                       // "Select"

                                            // Bluetooth list errors
  TXT_FB_BT_ERROR_LR_COULD_NOT_SAVE_1,      // BT save data error!
  TXT_FB_BT_ERROR_LR_COULD_NOT_SAVE_2,      //
  TXT_FB_BT_ERROR_LR_STORE_IS_FULL_1,       // BT store is full error!
  TXT_FB_BT_ERROR_LR_STORE_IS_FULL_2,       //
  TXT_FB_BT_ERROR_LR_UNKOWN_ADDR_1,         // BT unknown addr. error!
  TXT_FB_BT_ERROR_LR_UNKOWN_ADDR_2,         //

                                            // Datalog errors
  TXT_FB_DL_ERROR_MEMORY_FULL_1,            // Memory is full! 
  TXT_FB_DL_ERROR_MEMORY_FULL_2,            //

                                            // Power of time
  TXT_POWEROFFTIME_NEVER                    // "Never"

};

// ****** FILE TYPE GRAPHIC RESOURCES ****************************************

#define   ALLFILES    0x1A    // Icon collection offset

enum                  // File type id's
{
  FILETYPE_ALL,       // 0 = All
  FILETYPE_SOUND,     // 1 = Sound
  FILETYPE_LMS,       // 2 = LMS
  FILETYPE_NXT,       // 3 = NXT
  FILETYPE_TRYME,     // 4 = Try me
  FILETYPE_DATALOG,   // 5 = Datalog
  FILETYPES
};

const     UBYTE TXT_FILE_EXT[FILETYPES][4] =
{
  "*",                // 0 = All
  TXT_SOUND_EXT,      // 1 = Sound
  TXT_LMS_EXT,        // 2 = LMS
  TXT_NXT_EXT,        // 3 = NXT
  TXT_TRYME_EXT,      // 4 = Try me
  TXT_DATA_EXT        // 5 = Datalog
};

const     UBYTE TXT_FILETYPE[FILETYPES] =
{
  0,                  // NA
  TXT_FILETYPE_SOUND, // 1 = Sound
  TXT_FILETYPE_LMS,   // 2 = LMS
  TXT_FILETYPE_NXT,   // 3 = NXT
  TXT_FILETYPE_TRY_ME,// 4 = Try me
  TXT_FILETYPE_DATA   // 5 = Datalog
};

// ****** POWER OFF DEFINITIONS **********************************************

#define   POWER_OFF_TIME_STEPS    6
#define   POWER_OFF_TIME_DEFAULT  3

const     UBYTE PowerOffTimeSteps[POWER_OFF_TIME_STEPS] = { 0,2,5,10,30,60 }; // [min]

// ****** BATTERY DEFINITIONS ************************************************

#define   BATTERYLIMITS           4   // [Cnt]
#define   BATTERYLIMITHYST        100 // [mV]
#define   RECHARGEABLELIMITHYST   50  // [mV]

const     UWORD BatteryLimits[BATTERYLIMITS] =
{
  6100,6500,7000,7500 // [mV]
};

const     UWORD RechargeableLimits[BATTERYLIMITS] =
{
  7100,7200,7300,7500 // [mV]
};

//******* UI MENU FILE HANDLER *************************************************************************

#include  "Mainmenu.rms"
#include  "Submenu01.rms"
#include  "Submenu02.rms"
#include  "Submenu03.rms"
#include  "Submenu04.rms"
#include  "Submenu05.rms"
#include  "Submenu06.rms"
#include  "Submenu07.rms"

const     UBYTE *MenuPointers[] =
{
  (UBYTE*)MAINMENU,
  (UBYTE*)SUBMENU01,
  (UBYTE*)SUBMENU02,
  (UBYTE*)SUBMENU03,
  (UBYTE*)SUBMENU04,
  (UBYTE*)SUBMENU05,
  (UBYTE*)SUBMENU06,
  (UBYTE*)SUBMENU07
};


UBYTE*    cUiGetMenuPointer(UBYTE FileNo)
{
  return ((UBYTE*)MenuPointers[FileNo]);
}


//******************************************************************************************************

UBYTE*    cUiGetString(UBYTE No)        // Get string in text string file
{
  UBYTE   *Result = NULL;
  TXT     *pUi;
  UWORD   Tmp;

  pUi = (TXT*)Ui;
  if (No)
  {
    if (No <= pUi->ItemsY)
    {
      Tmp  = No - 1;
      Tmp *= pUi->ItemCharsX;
      Result = &(pUi->Data[Tmp]);
    }
  }

  return (Result);
}


UBYTE     cUiReadButtons(void)             // Read buttons
{
  UBYTE   Result = BUTTON_NONE;

  if (!(IOMapUi.Flags & UI_DISABLE_LEFT_RIGHT_ENTER))
  {
    if ((pMapButton->State[BTN3] & PRESSED_STATE))
    {
      Result = BUTTON_LEFT;
    }
    if ((pMapButton->State[BTN2] & PRESSED_STATE))
    {
      Result = BUTTON_RIGHT;
    }
    if ((pMapButton->State[BTN4] & PRESSED_STATE))
    {
      Result = BUTTON_ENTER;
    }
  }
  if (!(IOMapUi.Flags & UI_DISABLE_EXIT))
  {
    if ((pMapButton->State[BTN1] & PRESSED_STATE))
    {
      Result = BUTTON_EXIT;
    }
  }
  if (Result == BUTTON_NONE)
  {
    // All buttons released
    VarsUi.ButtonOld  = BUTTON_NONE;
    VarsUi.ButtonTime = BUTTON_DELAY_TIME;
  }
  else
  {
    // Some button pressed
    if (VarsUi.ButtonOld == BUTTON_NONE)
    {
      // Just pressed
      VarsUi.ButtonOld  = Result;
      VarsUi.ButtonTimer = 0;
    }
    else
    {
      // Still pressed
      Result = BUTTON_NONE;

      if (VarsUi.ButtonTimer >= VarsUi.ButtonTime)
      {
        VarsUi.ButtonTimer = 0;
        VarsUi.ButtonTime  = BUTTON_REPEAT_TIME;
        if ((VarsUi.ButtonOld == BUTTON_LEFT) || (VarsUi.ButtonOld == BUTTON_RIGHT))
        {
          // If arrow repeat
          Result = VarsUi.ButtonOld;
        }
      }
    }
  }
  if (VarsUi.ButtonOld == BUTTON_NONE)
  {
    // If no key - check interface
    Result            = IOMapUi.Button;
    IOMapUi.Button    = BUTTON_NONE;
  }
  if (Result != BUTTON_NONE)
  {
    // If key - play key sound file
    sprintf((char*)pMapSound->SoundFilename,"%s.%s",(char*)UI_KEYCLICK_SOUND,(char*)TXT_FILE_EXT[FILETYPE_SOUND]);
    pMapSound->Volume =  IOMapUi.Volume;
    pMapSound->Mode   =  SOUND_ONCE;
    pMapSound->Flags |=  SOUND_UPDATE;

    // Reset power down timer
    IOMapUi.Flags    |= UI_RESET_SLEEP_TIMER;
  }

  return (Result);
}


void      cUiListLeft(UBYTE Limit,UBYTE *Center)
{
  UBYTE   Tmp;

  Tmp = *Center;
  if (Tmp > 1)
  {
    Tmp--;
  }
  else
  {
    if (Limit > 2)
    {
      Tmp = Limit;
    }
  }
  *Center = Tmp;
}


void      cUiListRight(UBYTE Limit,UBYTE *Center)
{
  UBYTE   Tmp;

  Tmp = *Center;
  if (Tmp < Limit)
  {
    Tmp++;
  }
  else
  {
    if (Limit > 2)
    {
      Tmp = 1;
    }
  }
  *Center = Tmp;
}


void      cUiListCalc(UBYTE Limit,UBYTE *Center,UBYTE *Left,UBYTE *Right)
{
  switch (Limit)
  {
    case 1 :
    {
      *Left   = 0;
      *Right  = 0;
    }
    break;

    case 2 :
    {
      if ((*Center) == 1)
      {
        *Left  = 0;
        *Right = 2;
      }
      else
      {
        *Left  = 1;
        *Right = 0;
      }
    }
    break;

    default :
    {
      *Left = *Center - 1;
      if ((*Left) < 1)
      {
        *Left = Limit;
      }
      *Right = *Center + 1;
      if ((*Right) > Limit)
      {
        *Right = 1;
      }
    }
    break;

  }
}


UBYTE     cUiMenuSearchSensorIcon(UBYTE Sensor)
{
  UBYTE    Result = 0;
  MENUITEM *MenuItem;
  UBYTE    Index;

  for (Index = 0;(Index < IOMapUi.pMenu->Items) && (Result == NULL);Index++)
  {
    MenuItem = &IOMapUi.pMenu->Data[Index];
    if (MenuItem->FunctionParameter == Sensor)
    {
      Result = MenuItem->IconImageNo;
    }
  }

  return (Result);
}


ULONG     cUiMenuGetId(MENUITEM *pMenuItem)
{
  ULONG   MenuId;

  MenuId  = (ULONG)pMenuItem->ItemId01;
  MenuId |= (ULONG)pMenuItem->ItemId23 << 8;
  MenuId |= (ULONG)pMenuItem->ItemId45 << 16;
  MenuId |= (ULONG)pMenuItem->ItemId67 << 24;

  return (MenuId);
}


ULONG     cUiMenuGetSpecialMask(MENUITEM *pMenuItem)
{
  ULONG   Mask;

  Mask = 0;
  if (pMenuItem != NULL)
  {
    Mask  = (ULONG)pMenuItem->SpecialMask0;
    Mask |= (ULONG)pMenuItem->SpecialMask1 << 8;
    Mask |= (ULONG)pMenuItem->SpecialMask2 << 16;
    Mask |= (ULONG)pMenuItem->SpecialMask3 << 24;
  }

  return (Mask);
}


UBYTE*    cUiMenuGetIconImage(UBYTE No)
{
  UBYTE   *Image;

  Image = NULL;
  if (No < (Icons->ItemsX * Icons->ItemsY))
  {
    Image = (UBYTE*)&Icons->Data[No * Icons->ItemPixelsX * (Icons->ItemPixelsY / 8)];
  }

  return (Image);
}


ULONG     cUiMenuMotherId(ULONG Id,UBYTE Level)
{
  ULONG   MotherIdMask;

  MotherIdMask  = 0xFFFFFFFFL >> ((8 - Level) * 4);
  MotherIdMask |= 0xFFFFFFFFL << ((Level + 1) * 4);

  return (Id & MotherIdMask);
}


UBYTE     cUiMenuIdValid(MENUFILE *pMenuFile,ULONG Id)
{
  ULONG   SpecialMask;
  ULONG   MotherId;
  UBYTE   Level;
  UBYTE   Result;

  Result = FALSE;
  Level  = pMenuFile->MenuLevel;

  if (Level)
  {
    SpecialMask = pMenuFile->MenuLevels[Level - 1].SpecialFlags;
    MotherId    = pMenuFile->MenuLevels[Level - 1].Id;
    if ((SpecialMask & MENU_SKIP_THIS_MOTHER_ID))
    {
      MotherId &= ~(0x0000000F << ((Level - 1) * 4));
      SpecialMask >>= 28;
      MotherId |=  (SpecialMask << ((Level - 1) * 4));
    }
    if (MotherId == cUiMenuMotherId(Id,Level))
    {
      Id >>= (Level * 4);
      if ((Id & 0x0000000F) && (!(Id & 0xFFFFFFF0)))
      {
        Result = TRUE;
      }
    }
  }
  else
  {
    if ((Id & 0x0000000F) && (!(Id & 0xFFFFFFF0)))
    {
      Result = TRUE;
    }
  }

  return (Result);
}


UBYTE     cUiMenuGetNoOfMenus(MENU *pMenu,MENUFILE *pMenuFile)
{
  ULONG   MenuId;
  UBYTE   NoOfMenus;
  UBYTE   Index;

  NoOfMenus = 0;
  for (Index = 0;Index < pMenu->Items;Index++)
  {
    MenuId  = cUiMenuGetId(&pMenu->Data[Index]);

    if (cUiMenuIdValid(pMenuFile,MenuId) == TRUE)
    {
      if ((cUiMenuGetSpecialMask(&pMenu->Data[Index]) & MENU_ONLY_BT_ON))
      {
        // BT module must be on
        if (!(IOMapUi.BluetoothState & BT_STATE_OFF))
        {
          // Yes
          NoOfMenus++;
        }
      }
      else
      {
        if ((cUiMenuGetSpecialMask(&pMenu->Data[Index]) & MENU_ONLY_DATALOG_ENABLED))
        {
          // Datalog menu must be enabled
          if (VarsUi.NVData.DatalogEnabled)
          {
            // Yes
            NoOfMenus++;
          }
        }
        else
        {
          // No restrictions
          NoOfMenus++;
        }
      }
    }
  }

  return (NoOfMenus);
}


UBYTE     cUiGetMenuItemIndex(MENU *pMenu,MENUFILE *pMenuFile,UBYTE No)
{
  ULONG   MenuId;
  UBYTE   NoOfMenus;
  UBYTE   Index;
  UBYTE   TmpIndex = 0;

  NoOfMenus = 0;
  for (Index = 0;(Index < pMenu->Items) && (No != NoOfMenus);Index++)
  {
    MenuId  = cUiMenuGetId(&pMenu->Data[Index]);

    if (cUiMenuIdValid(pMenuFile,MenuId) == TRUE)
    {
      if ((cUiMenuGetSpecialMask(&pMenu->Data[Index]) & MENU_ONLY_BT_ON))
      {
        // BT module must be on
        if (!(IOMapUi.BluetoothState & BT_STATE_OFF))
        {
          // Yes
          TmpIndex = Index;
          NoOfMenus++;
        }
      }
      else
      {
        if ((cUiMenuGetSpecialMask(&pMenu->Data[Index]) & MENU_ONLY_DATALOG_ENABLED))
        {
          // Datalog menu must be enabled
          if (VarsUi.NVData.DatalogEnabled)
          {
            // Yes
            TmpIndex = Index;
            NoOfMenus++;
          }
        }
        else
        {
          // No restrictions
          TmpIndex = Index;
          NoOfMenus++;
        }
      }
    }
  }
  if (No != NoOfMenus)
  {
    Index = TmpIndex + 1;
  }

  return (Index);
}



UBYTE     cUiMenuGetNo(MENU *pMenu,ULONG Id,UBYTE Level)
{
  ULONG   MenuId;
  ULONG   MotherId;
  UBYTE   Index;
  UBYTE   No;
  UBYTE   NoOfItems;

  No        = 0;
  NoOfItems = 0;

  MotherId = cUiMenuMotherId(Id,Level);

  for (Index = 0;(Index < pMenu->Items) && (No == 0);Index++)
  {
    MenuId  = cUiMenuGetId(&pMenu->Data[Index]);

    // Scanning all id's until No is found
    if (!(MenuId >> ((Level + 1) * 4)))
    {
      // MenuId is above or on actual level
      if (((MenuId >> (Level * 4)) & 0x0000000F))
      {
        // MenuId is on actual level
        if (MotherId == cUiMenuMotherId(MenuId,Level))
        {
          // Same mother id
          NoOfItems++;
          if (MenuId == Id)
          {
            No = NoOfItems;
          }
        }
      }
    }
  }

  return (No);
}

void      cUiUpdateStatus(void)
{
  UWORD   Tmp;
  UWORD   Hyst;
  UWORD   *pTmp;
  UBYTE   Pointer;

  if (++VarsUi.UpdateCounter >= RUN_STATUS_CHANGE_TIME)
  {
    VarsUi.UpdateCounter = 0;

    // Update running status icon pointer
    if (++VarsUi.Running >= 12)
    {
      VarsUi.Running = 0;
    }

    // Get battery voltage limits
    if ((IoFromAvr.Battery & 0x8000))
    {
      IOMapUi.Rechargeable = 1;
      pTmp = (UWORD*)RechargeableLimits;
      Hyst = RECHARGEABLELIMITHYST;
    }
    else
    {
      IOMapUi.Rechargeable = 0;
      pTmp = (UWORD*)BatteryLimits;
      Hyst = BATTERYLIMITHYST;
    }

    // Calculate battery voltage
    Tmp = IoFromAvr.Battery & 0x03FF;
    Tmp = (UWORD)((float)Tmp * BATTERY_COUNT_TO_MV);

    IOMapUi.BatteryVoltage = Tmp;

    // Find new battery state
    Pointer = 0;
    while ((Tmp > pTmp[Pointer]) && (Pointer < BATTERYLIMITS))
    {
      Pointer++;
    }

    // Change battery state
    if (Pointer != IOMapUi.BatteryState)
    {
      if (Pointer > IOMapUi.BatteryState)
      {
        if (Tmp > (pTmp[IOMapUi.BatteryState] + Hyst))
        {
          IOMapUi.BatteryState   = Pointer;
        }
      }
      else
      {
        IOMapUi.BatteryState   = Pointer;
      }
    }

    // Control toggle and bitmap
    if (IOMapUi.BatteryState)
    {
      VarsUi.BatteryToggle = 0;
      VarsUi.LowBatt = 0;
    }
    else
    {
      if (VarsUi.LowBatt < 255)
      {
        VarsUi.LowBatt++;
      }

      if (VarsUi.BatteryToggle)
      {
        VarsUi.BatteryToggle = 0;
      }
      else
      {
        VarsUi.BatteryToggle = 1;
      }
    }

    // Ensure frequently status updates
    IOMapUi.Flags |= UI_UPDATE;
  }

  if ((IOMapUi.Flags & UI_ENABLE_STATUS_UPDATE))
  {
    if ((IOMapUi.Flags & UI_UPDATE) || (IOMapUi.Flags & UI_REDRAW_STATUS))
    {
      VarsUi.ErrorTimer = 0;
      pMapDisplay->pStatusText   =  (UBYTE*)VarsUi.StatusText;

      // Status line update nessesary
      if (IOMapUi.BatteryState < Status->ItemsX)
      {
        // Update battery status icons
        if (IoFromAvr.Battery & 0x8000)
        {
          VarsUi.NewStatusIcons[STATUSICON_BATTERY] = STATUS_NO_RECHARGEABLE_0 + IOMapUi.BatteryState + VarsUi.BatteryToggle;
        }
        else
        {
          VarsUi.NewStatusIcons[STATUSICON_BATTERY] = STATUS_NO_BATTERY_0 + IOMapUi.BatteryState + VarsUi.BatteryToggle;
        }
      }

      // Update bluetooth status icons
      if ((IOMapUi.BluetoothState & (BT_STATE_VISIBLE | BT_STATE_CONNECTED | BT_STATE_OFF)) < Status->ItemsX)
      {
        VarsUi.NewStatusIcons[STATUSICON_BLUETOOTH] = STATUS_NO_BLUETOOTH_0 + (IOMapUi.BluetoothState & (BT_STATE_VISIBLE | BT_STATE_CONNECTED | BT_STATE_OFF));
      }

      // Update usb status icons
      if (IOMapUi.UsbState < 6)
      {
        VarsUi.NewStatusIcons[STATUSICON_USB] = STATUS_NO_USB_0 + IOMapUi.UsbState;
      }

      // Update running status icons
      if (IOMapUi.RunState == FALSE)
      {
        VarsUi.Running = 0;
      }
      VarsUi.NewStatusIcons[STATUSICON_VM] = STATUS_NO_RUNNING_0 + VarsUi.Running;

      // Update only changed status icons
      for (Pointer = 0;Pointer < STATUSICONS;Pointer++)
      {
        if ((pMapDisplay->StatusIcons[Pointer] != VarsUi.NewStatusIcons[Pointer]))
        {
          pMapDisplay->StatusIcons[Pointer]  = VarsUi.NewStatusIcons[Pointer];
          pMapDisplay->UpdateMask           |= STATUSICON_BIT(Pointer);
        }
      }

      if ((IOMapUi.Flags & UI_REDRAW_STATUS))
      {
        // Entire status line needs to be redrawed
        if (pMapComm->BrickData.Name[0])
        {
          for (Pointer = 0;Pointer < STATUSTEXT_SIZE;Pointer++)
          {
            VarsUi.StatusText[Pointer] = pMapComm->BrickData.Name[Pointer];
          }
          VarsUi.StatusText[Pointer] = 0;
        }
        pMapDisplay->EraseMask  |= SPECIAL_BIT(STATUSTEXT);
        pMapDisplay->UpdateMask |= SPECIAL_BIT(STATUSTEXT);
        pMapDisplay->UpdateMask |= (SPECIAL_BIT(TOPLINE) | STATUSICON_BITS);
      }

      // Clear update flag
      IOMapUi.Flags &= ~UI_REDRAW_STATUS;
      IOMapUi.Flags &= ~UI_UPDATE;
    }
  }
  else
  {
    pMapDisplay->UpdateMask &= ~(STATUSICON_BITS | SPECIAL_BIT(TOPLINE) | SPECIAL_BIT(STATUSTEXT));
  }
}




void      cUiMenuCallFunction(UBYTE Function,UBYTE Parameter)
{
  if (Function)
  {
    VarsUi.Function  = Function;
    VarsUi.Parameter = Parameter;
  }
}


void      cUiMenuNextFile(void)
{
  MENU    *pTmpMenu;

  pTmpMenu      = (MENU*)cUiGetMenuPointer(VarsUi.pMenuLevel->NextFileNo);
  if (pTmpMenu != NULL)
  {
    if (VarsUi.MenuFileLevel < (MENUFILELEVELS - 1))
    {
      VarsUi.MenuFileLevel++;
      VarsUi.MenuFiles[VarsUi.MenuFileLevel].FileId     = VarsUi.pMenuLevel->NextFileNo;
      VarsUi.MenuFiles[VarsUi.MenuFileLevel].MenuLevel  = 0;
      VarsUi.MenuFiles[VarsUi.MenuFileLevel].MenuLevels[VarsUi.MenuFiles[VarsUi.MenuFileLevel].MenuLevel].ItemIndex = VarsUi.pMenuLevel->NextMenuNo;
      IOMapUi.pMenu                                     = pTmpMenu;
    }
  }
}


void      cUiMenuPrevFile(void)
{
  if (VarsUi.MenuFileLevel)
  {
    VarsUi.MenuFileLevel--;
    IOMapUi.pMenu  = (MENU*)cUiGetMenuPointer(VarsUi.MenuFiles[VarsUi.MenuFileLevel].FileId);
  }
}


void      cUiMenuNext(void)
{
  if (VarsUi.pMenuFile->MenuLevel < (MENULEVELS - 1))
  {
    VarsUi.pMenuFile->MenuLevel++;
    VarsUi.pMenuFile->MenuLevels[VarsUi.pMenuFile->MenuLevel].ItemIndex = VarsUi.pMenuLevel->NextMenuNo;
  }
}


void      cUiMenuPrev(void)
{
  if (VarsUi.pMenuFile->MenuLevel)
  {
    VarsUi.pMenuFile->MenuLevel--;
  }
}


void      cUiMenuEnter(void)
{
  // Call function with parameter (if pressent)
  if (!(VarsUi.pMenuLevel->SpecialFlags & MENU_INIT_CALLS))
  {
    cUiMenuCallFunction(VarsUi.pMenuLevel->FunctionNo,VarsUi.pMenuLevel->Parameter);
  }

  if (VarsUi.EnterOnlyCalls != TRUE)
  {
    if ((VarsUi.pMenuLevel->SpecialFlags & MENU_ENTER_LEAVES_MENUFILE))
    {
      cUiMenuPrevFile();
    }
    else
    {
      // Load new menu file (if pressent)
      if (VarsUi.pMenuLevel->NextFileNo)
      {
        cUiMenuNextFile();
      }
      else
      {
        // Activate next menu level (if pressent)
        if (VarsUi.pMenuLevel->NextMenuNo)
        {
          cUiMenuNext();
        }
      }
    }
    IOMapUi.State         = NEXT_MENU;
  }
  else
  {
    VarsUi.EnterOnlyCalls = FALSE;
    IOMapUi.State         = DRAW_MENU;
  }
}


void      cUiMenuExit(void)
{

  if ((VarsUi.pMenuLevel->SpecialFlags & MENU_EXIT_CALLS))
  {
    // Call function with parameter (if pressent)
    if (!(VarsUi.pMenuLevel->SpecialFlags & MENU_INIT_CALLS))
    {
      cUiMenuCallFunction(VarsUi.pMenuLevel->FunctionNo,VarsUi.pMenuLevel->Parameter);
    }
  }

  // Call function with 0xFF (if ordered)
  if ((VarsUi.pMenuLevel->SpecialFlags & MENU_EXIT_CALLS_WITH_FF))
  {
    cUiMenuCallFunction(VarsUi.pMenuLevel->FunctionNo,MENU_EXIT);
  }

  if (VarsUi.ExitOnlyCalls != TRUE)
  {
    if ((VarsUi.pMenuFile->MenuLevel) && (!(VarsUi.pMenuLevel->SpecialFlags & MENU_EXIT_LEAVES_MENUFILE)))
    {
      if (!(VarsUi.pMenuLevel->SpecialFlags & MENU_EXIT_LOAD_MENU))
      {
        cUiMenuPrev();
        if ((VarsUi.pMenuLevel->SpecialFlags & MENU_BACK_TWICE))
        {
          if (VarsUi.pMenuFile->MenuLevel)
          {
            cUiMenuPrev();
          }
          VarsUi.SecondTime   = FALSE;
        }
        if ((VarsUi.pMenuLevel->SpecialFlags & MENU_BACK_THREE_TIMES))
        {
          if (VarsUi.pMenuFile->MenuLevel)
          {
            cUiMenuPrev();
          }
          if (VarsUi.pMenuFile->MenuLevel)
          {
            cUiMenuPrev();
          }
          VarsUi.SecondTime   = FALSE;
        }
      }
      else
      {
        VarsUi.EnterOnlyCalls = FALSE;
        VarsUi.ExitOnlyCalls  = FALSE;
        if (VarsUi.pMenuLevel->NextFileNo)
        {
          cUiMenuNextFile();
        }
        else
        {
          // Activate next menu level (if pressent)
          if (VarsUi.pMenuLevel->NextMenuNo)
          {
            cUiMenuNext();
          }
        }
      }
      if ((VarsUi.pMenuLevel->SpecialFlags & MENU_EXIT_LOAD_POINTER))
      {
        VarsUi.pMenuFile->MenuLevels[VarsUi.pMenuFile->MenuLevel].ItemIndex = (UBYTE)((VarsUi.pMenuLevel->SpecialFlags) >> 24) & 0x0F;
      }
    }
    else
    {
      if (!(VarsUi.pMenuLevel->SpecialFlags & MENU_EXIT_LOAD_MENU))
      {
        cUiMenuPrevFile();
      }
      else
      {
        VarsUi.EnterOnlyCalls = FALSE;
        VarsUi.ExitOnlyCalls  = FALSE;
        if (VarsUi.pMenuLevel->NextFileNo)
        {
          cUiMenuNextFile();
        }
        else
        {
          // Activate next menu level (if pressent)
          if (VarsUi.pMenuLevel->NextMenuNo)
          {
            cUiMenuNext();
          }
        }
      }
    }
    IOMapUi.State         = NEXT_MENU;
  }
  else
  {
    VarsUi.ExitOnlyCalls  = FALSE;
    IOMapUi.State         = DRAW_MENU;
  }
}


void      cUiLoadLevel(UBYTE FileLevel,UBYTE MenuLevel,UBYTE MenuIndex)
{
  UBYTE   Tmp;

  VarsUi.MenuFileLevel = FileLevel;
  VarsUi.MenuFiles[FileLevel].MenuLevel = MenuLevel;
  VarsUi.MenuFiles[FileLevel].MenuLevels[MenuLevel].ItemIndex = MenuIndex;
  IOMapUi.pMenu  = (MENU*)cUiGetMenuPointer(VarsUi.MenuFiles[VarsUi.MenuFileLevel].FileId);

  VarsUi.pMenuFile  = &VarsUi.MenuFiles[VarsUi.MenuFileLevel];
  VarsUi.pMenuLevel = &VarsUi.pMenuFile->MenuLevels[VarsUi.pMenuFile->MenuLevel];

  // Count no of menues on current level
  VarsUi.pMenuLevel->Items = cUiMenuGetNoOfMenus(IOMapUi.pMenu,VarsUi.pMenuFile);

  if (VarsUi.pMenuLevel->Items)
  {
    // if items > 0  -> prepare allways center icon
    Tmp = cUiGetMenuItemIndex(IOMapUi.pMenu,VarsUi.pMenuFile,VarsUi.pMenuLevel->ItemIndex);

    if (VarsUi.pMenuItem != &IOMapUi.pMenu->Data[Tmp - 1])
    {
      VarsUi.pMenuItem    = &IOMapUi.pMenu->Data[Tmp - 1];
      VarsUi.SecondTime   = FALSE;
    }

    // Save center menu item parameters
    VarsUi.pMenuLevel->Id           = cUiMenuGetId(VarsUi.pMenuItem);
    VarsUi.pMenuLevel->IconImageNo  = VarsUi.pMenuItem->IconImageNo;
    VarsUi.pMenuLevel->IconText     = VarsUi.pMenuItem->IconText;
    VarsUi.pMenuLevel->SpecialFlags = cUiMenuGetSpecialMask(VarsUi.pMenuItem);
    VarsUi.pMenuLevel->FunctionNo   = VarsUi.pMenuItem->FunctionIndex;
    VarsUi.pMenuLevel->Parameter    = VarsUi.pMenuItem->FunctionParameter;
    VarsUi.pMenuLevel->NextFileNo   = VarsUi.pMenuItem->FileLoadNo;
    VarsUi.pMenuLevel->NextMenuNo   = VarsUi.pMenuItem->NextMenu;
  }
}

#include  "Functions.inl"


void      cUiInit(void* pHeader)
{
  pHeaders                  = pHeader;
  VarsUi.Initialized        = FALSE;
  IOMapUi.BluetoothState    = BT_STATE_OFF;
  IOMapUi.UsbState          = 0;
  IOMapUi.State             = INIT_DISPLAY;
}


void      cUiCtrl(void)
{
  UBYTE   Tmp;

// Testcode for low battery voltage
/*
  if ((pMapInput->Inputs[0].InvalidData != INVALID_DATA) && (pMapInput->Inputs[0].ADRaw < 500))
  {
    if (VarsUi.LowBatt < 255)
    {
      VarsUi.LowBatt++;
    }
  }
  else
  {
    VarsUi.LowBatt = 0;
  }
*/
//

// Testcode for BT connect request
/*
  if ((pMapInput->Inputs[0].InvalidData != INVALID_DATA) && (pMapInput->Inputs[0].ADRaw < 500))
  {
    IOMapUi.BluetoothState |= BT_CONNECT_REQUEST | BT_PIN_REQUEST;
  }
*/
//

// Testcode for BT error attention
/*
  if ((pMapInput->Inputs[0].InvalidData != INVALID_DATA) && (pMapInput->Inputs[0].ADRaw < 500))
  {
    IOMapUi.Error = LR_UNKOWN_ADDR;
    IOMapUi.BluetoothState |= BT_ERROR_ATTENTION;
  }
*/
//

// Testcode for execute program
/*
  if ((pMapInput->Inputs[0].InvalidData != INVALID_DATA) && (pMapInput->Inputs[0].ADRaw < 500))
  {
    if ((!(IOMapUi.Flags & UI_EXECUTE_LMS_FILE)) && (IOMapUi.State > INIT_MENU))
    {
      strcpy((char*)IOMapUi.LMSfilename,"Untitled-1.rxe");
      IOMapUi.Flags |= UI_EXECUTE_LMS_FILE;
    }
  }
*/
//

// Testcode for force off
/*
  if ((pMapInput->Inputs[0].InvalidData != INVALID_DATA) && (pMapInput->Inputs[0].ADRaw < 500) && (VarsUi.Initialized == TRUE))
  {
    IOMapUi.ForceOff  = TRUE;
  }
*/
//

  if ((!(IOMapUi.Flags & UI_EXECUTE_LMS_FILE)) && (IOMapUi.State == INIT_INTRO)/* && ((pMapButton->State[BTN1] & PRESSED_STATE)!=PRESSED_STATE)*/)
  {
    UWORD LStatus;
    if (LOADER_ERR(LStatus = pMapLoader->pFunc(FINDFIRST, UI_STARTUP_PROGRAM, NULL, NULL)) == SUCCESS)
    {
      //Close file handle returned by FINDFIRST
      pMapLoader->pFunc(CLOSE, LOADER_HANDLE_P(LStatus), NULL, NULL);
      strcpy((char*)IOMapUi.LMSfilename, UI_STARTUP_PROGRAM);
      IOMapUi.Flags |= UI_EXECUTE_LMS_FILE;
      IOMapUi.State = INIT_MENU;
    }
  }

  VarsUi.CRPasskey++;
  VarsUi.ButtonTimer++;
#ifndef STRIPPED
  VarsUi.OBPTimer++;
#endif
  switch (IOMapUi.State)
  {
    case INIT_DISPLAY : // Load font and icons
    {
//      pMapLoader->pFunc(DELETEUSERFLASH,NULL,NULL,NULL);

      VarsUi.Initialized                              =  FALSE;

      IOMapUi.Flags                                   =  UI_BUSY;
      IOMapUi.RunState                                =  1;
      IOMapUi.BatteryState                            =  0;
      IOMapUi.Error                                   =  0;
      IOMapUi.ForceOff                                =  FALSE;
      VarsUi.LowBatt                                  =  0;
      VarsUi.LowBattHasOccured                        =  0;

      pMapDisplay->pFont                              =  (FONT*)Font;
      pMapDisplay->pStatusIcons                       =  (ICON*)Status;
      pMapDisplay->pStatusText                        =  (UBYTE*)VarsUi.StatusText;
#ifndef STRIPPED
      pMapDisplay->pStepIcons                         =  (ICON*)Step;
#endif

      VarsUi.State                                    =  0;
      VarsUi.Pointer                                  =  0;
      VarsUi.Timer                                    =  -INTRO_START_TIME;

      VarsUi.FNOFState                                =  0;
      VarsUi.FBState                                  =  0;

      VarsUi.UpdateCounter                            =  0;
      VarsUi.Running                                  =  0;
      VarsUi.BatteryToggle                            =  0;

      VarsUi.GUSState                                 =  0;

      IOMapUi.pMenu                                   =  (MENU*)cUiGetMenuPointer(0);
      IOMapUi.State                                   =  INIT_INTRO;

      pMapDisplay->EraseMask                          =  SCREEN_BIT(SCREEN_BACKGROUND);
#ifndef STRIPPED
      pMapDisplay->pBitmaps[BITMAP_1]                 =  (BMPMAP*)Intro[VarsUi.Pointer];
#endif
      pMapDisplay->UpdateMask                         =  BITMAP_BIT(BITMAP_1);
      pMapDisplay->Flags                             |=  DISPLAY_ON;

      cUiNVRead();
      IOMapUi.Volume                                  = VarsUi.NVData.VolumeStep;
      IOMapUi.SleepTimeout                            = PowerOffTimeSteps[VarsUi.NVData.PowerdownCode];
    }
    break;

    case INIT_LOW_BATTERY :
    {
      if (++VarsUi.Timer >= (INTRO_LOWBATT_TIME))
      {
        VarsUi.LowBattHasOccured        = 2;
        pMapDisplay->EraseMask          =  SCREEN_BIT(SCREEN_BACKGROUND);
#ifndef STRIPPED
        pMapDisplay->pBitmaps[BITMAP_1] =  (BMPMAP*)Intro[VarsUi.Pointer];
#endif
        pMapDisplay->UpdateMask         =  BITMAP_BIT(BITMAP_1);
        IOMapUi.Flags                  &= ~UI_ENABLE_STATUS_UPDATE;
        VarsUi.State                    =  0;
        VarsUi.Pointer                  =  0;
        VarsUi.Timer                    =  -INTRO_START_TIME;
        IOMapUi.State                   =  INIT_INTRO;
      }
    }
    break;

    case INIT_INTRO :
    {
      if (VarsUi.LowBattHasOccured == 1)
      {
        IOMapUi.Flags |=  UI_ENABLE_STATUS_UPDATE;
        IOMapUi.Flags |=  UI_UPDATE;
        IOMapUi.Flags |=  UI_REDRAW_STATUS;
        VarsUi.Timer   =  0;
        IOMapUi.State  =  INIT_LOW_BATTERY;
      }
      else
      {
        if (VarsUi.LowBattHasOccured == 0)
        {
          if (VarsUi.LowBatt)
          {
            pMapDisplay->pBitmaps[BITMAP_1] = (BMPMAP*)LowBattery;
            pMapDisplay->UpdateMask         =  BITMAP_BIT(BITMAP_1);
            VarsUi.LowBattHasOccured = 1;
          }
        }
#ifndef STRIPPED
        if (++VarsUi.Timer >= (INTRO_SHIFT_TIME))
        {
          switch (VarsUi.State)
          {
            case 0 :
            {
              pMapDisplay->Flags &= ~DISPLAY_REFRESH;
              VarsUi.State++;
            }
            break;

            case 1 :
            {
              if ((pMapDisplay->Flags & DISPLAY_REFRESH_DISABLED))
              {
                if (VarsUi.Pointer < NO_OF_INTROBITMAPS)
                {
                  pMapDisplay->EraseMask          |= BITMAP_BIT(BITMAP_1);
                  pMapDisplay->pBitmaps[BITMAP_1]  = (BMPMAP*)Intro[VarsUi.Pointer];
                  pMapDisplay->UpdateMask          = BITMAP_BIT(BITMAP_1);
                  if (VarsUi.Pointer == 11)
                  {
                    sprintf((char*)pMapSound->SoundFilename,"%s.%s",(char*)UI_STARTUP_SOUND,(char*)TXT_FILE_EXT[FILETYPE_SOUND]);
                    pMapSound->Volume       =  IOMapUi.Volume;
                    pMapSound->Mode         =  SOUND_ONCE;
                    pMapSound->Flags       |=  SOUND_UPDATE;
                  }
                  VarsUi.Pointer++;
                }
                else
                {
                  pMapDisplay->Flags              |= DISPLAY_REFRESH;
                  IOMapUi.State                    = INIT_WAIT;
                }
                VarsUi.State++;
              }
            }
            break;

            default :
            {
              if (!(pMapDisplay->UpdateMask & BITMAP_BIT(BITMAP_1)))
              {
                pMapDisplay->Flags |= DISPLAY_REFRESH;
                VarsUi.Timer        = 0;
                VarsUi.State        = 0;
              }
            }
            break;

          }
        }
#else
        pMapDisplay->EraseMask |=  SCREEN_BIT(SCREEN_BACKGROUND);
        IOMapUi.State = INIT_MENU;
#endif
      }
    }
    break;
#ifndef STRIPPED
    case INIT_WAIT :
    {
      if (++VarsUi.Timer >= INTRO_STOP_TIME)
      {
        pMapDisplay->EraseMask |=  SCREEN_BIT(SCREEN_BACKGROUND);
        IOMapUi.State           = INIT_MENU;
      }
    }
    break;
#endif
    case INIT_MENU :
    {
      // Restart menu system
      VarsUi.Function       = 0;
      VarsUi.MenuFileLevel  = 0;

      cUiLoadLevel(0,0,1);
      cUiLoadLevel(0,1,1);

      VarsUi.EnterOnlyCalls = FALSE;
      VarsUi.ExitOnlyCalls  = FALSE;

      IOMapUi.State         = NEXT_MENU;
    }
    break;

    case NEXT_MENU : // prepare icons
    {
      // Init various variables
      VarsUi.State      = 0;

      // Init icon pointers
      pMapDisplay->pMenuIcons[MENUICON_LEFT]    =  NULL;
      pMapDisplay->pMenuIcons[MENUICON_CENTER]  =  NULL;
      pMapDisplay->pMenuIcons[MENUICON_RIGHT]   =  NULL;

      cUiLoadLevel(VarsUi.MenuFileLevel,VarsUi.MenuFiles[VarsUi.MenuFileLevel].MenuLevel,VarsUi.MenuFiles[VarsUi.MenuFileLevel].MenuLevels[VarsUi.MenuFiles[VarsUi.MenuFileLevel].MenuLevel].ItemIndex);

      // Find menu icons
      if (VarsUi.pMenuLevel->Items)
      {
        // Prepare center icon
        pMapDisplay->pMenuIcons[MENUICON_CENTER]  =  cUiMenuGetIconImage(VarsUi.pMenuLevel->IconImageNo);
        pMapDisplay->pMenuText                    =  VarsUi.pMenuLevel->IconText;

        if (VarsUi.pMenuLevel->Items == 2)
        {
          // if 2 menues  -> prepare left or right icon
          if (VarsUi.pMenuLevel->ItemIndex == 1)
          {
            Tmp = cUiGetMenuItemIndex(IOMapUi.pMenu,VarsUi.pMenuFile,2);
            if (Tmp)
            {
              Tmp--;
              pMapDisplay->pMenuIcons[MENUICON_RIGHT]   =  cUiMenuGetIconImage(IOMapUi.pMenu->Data[Tmp].IconImageNo);
            }
          }
          else
          {
            Tmp = cUiGetMenuItemIndex(IOMapUi.pMenu,VarsUi.pMenuFile,1);
            if (Tmp)
            {
              Tmp--;
              pMapDisplay->pMenuIcons[MENUICON_LEFT]    =  cUiMenuGetIconImage(IOMapUi.pMenu->Data[Tmp].IconImageNo);
            }
          }
        }

        if (VarsUi.pMenuLevel->Items > 2)
        {
          // if more menues -> prepare left and right icon
          if (VarsUi.pMenuLevel->ItemIndex > 1)
          {
            Tmp = VarsUi.pMenuLevel->ItemIndex -1;
          }
          else
          {
            Tmp = VarsUi.pMenuLevel->Items;
          }
          Tmp = cUiGetMenuItemIndex(IOMapUi.pMenu,VarsUi.pMenuFile,Tmp);
          if (Tmp)
          {
            Tmp--;
            pMapDisplay->pMenuIcons[MENUICON_LEFT]    =  cUiMenuGetIconImage(IOMapUi.pMenu->Data[Tmp].IconImageNo);

          }
          if (VarsUi.pMenuLevel->ItemIndex < VarsUi.pMenuLevel->Items)
          {
            Tmp = VarsUi.pMenuLevel->ItemIndex + 1;
          }
          else
          {
            Tmp = 1;
          }
          Tmp = cUiGetMenuItemIndex(IOMapUi.pMenu,VarsUi.pMenuFile,Tmp);
          if (Tmp)
          {
            Tmp--;
            pMapDisplay->pMenuIcons[MENUICON_RIGHT]   =  cUiMenuGetIconImage(IOMapUi.pMenu->Data[Tmp].IconImageNo);
          }
        }
      }
      if ((VarsUi.pMenuLevel->SpecialFlags & MENU_ENTER_ONLY_CALLS))
      {
        VarsUi.EnterOnlyCalls = TRUE;
      }
      if ((VarsUi.pMenuLevel->SpecialFlags & MENU_EXIT_ONLY_CALLS))
      {
        VarsUi.ExitOnlyCalls = TRUE;
      }

      IOMapUi.State                             =  DRAW_MENU;
    }
    break;

    case DRAW_MENU : // If no function active -> erase screen, draw statusline and menu icons
    {
      if (VarsUi.Function)
      {
        // Function active
        if (VarsUi.Function < FUNC_NO_MAX)
        {
          if (Functions[VarsUi.Function](VarsUi.Parameter) == 0)
          {
            VarsUi.Function = 0;
          }
        }
        else
        {
          VarsUi.Function = 0;
        }
      }
      else
      {
        // function inactive - erase screen
        if (!(VarsUi.pMenuLevel->SpecialFlags & MENU_LEAVE_BACKGROUND))
        {
          pMapDisplay->EraseMask    |=  SCREEN_BIT(SCREEN_LARGE);

          // Draw only icons, frame and icon text
          pMapDisplay->UpdateMask    = (MENUICON_BITS | SPECIAL_BIT(FRAME_SELECT) | SPECIAL_BIT(MENUTEXT));
          pMapDisplay->TextLinesCenterFlags = 0;
        }
        else
        {
          pMapDisplay->EraseMask    |=  (SPECIAL_BIT(MENUTEXT) | MENUICON_BITS);

          // Draw icons, frame and icon text
          pMapDisplay->UpdateMask   |= (MENUICON_BITS | SPECIAL_BIT(FRAME_SELECT) | SPECIAL_BIT(MENUTEXT));
        }

        // Draw status
        IOMapUi.Flags               |=  (UI_ENABLE_STATUS_UPDATE | UI_UPDATE | UI_REDRAW_STATUS);

        if ((VarsUi.pMenuLevel->SpecialFlags & MENU_INIT_CALLS_WITH_0) && (VarsUi.SecondTime == FALSE))
        {
          VarsUi.SecondTime = TRUE;
          cUiMenuCallFunction(VarsUi.pMenuLevel->FunctionNo,MENU_INIT);
        }
        else
        {
          if ((VarsUi.pMenuLevel->SpecialFlags & MENU_INIT_CALLS_WITH_1) && (VarsUi.SecondTime == FALSE))
          {
            VarsUi.SecondTime = TRUE;
            cUiMenuCallFunction(VarsUi.pMenuLevel->FunctionNo,MENU_INIT_ALTERNATIVE);
          }
          else
          {
            if ((VarsUi.pMenuLevel->SpecialFlags & MENU_INIT_CALLS) && (VarsUi.SecondTime == FALSE))
            {
              VarsUi.SecondTime = TRUE;
              cUiMenuCallFunction(VarsUi.pMenuLevel->FunctionNo,VarsUi.pMenuLevel->Parameter);
            }
            else
            {
              if ((VarsUi.pMenuLevel->SpecialFlags & MENU_AUTO_PRESS_ENTER))
              {
                IOMapUi.State = ENTER_PRESSED;
              }
              else
              {
                IOMapUi.State = TEST_BUTTONS;
              }
            }
          }
        }
      }
    }
    break;

    case TEST_BUTTONS : // Test buttons to execute new functions and new menus
    {
      if (VarsUi.Initialized == FALSE)
      {
        VarsUi.Initialized    =  TRUE;
        IOMapUi.Flags        &= ~UI_BUSY;
      }

      switch (cUiReadButtons())
      {
        case BUTTON_LEFT :
        {
          IOMapUi.State = LEFT_PRESSED;
        }
        break;

        case BUTTON_RIGHT :
        {
          IOMapUi.State = RIGHT_PRESSED;
        }
        break;

        case BUTTON_ENTER :
        {
          IOMapUi.State = ENTER_PRESSED;
        }
        break;

        case BUTTON_EXIT :
        {
          if (!(VarsUi.pMenuLevel->SpecialFlags & MENU_EXIT_DISABLE))
          {
            IOMapUi.State = EXIT_PRESSED;
          }
        }
        break;

      }

    }
    break;

    case LEFT_PRESSED :
    {
      if ((VarsUi.pMenuLevel->SpecialFlags & MENU_LEFT_RIGHT_AS_CALL))
      {
        cUiMenuCallFunction(VarsUi.pMenuLevel->FunctionNo,MENU_LEFT);
      }
      else
      {
        VarsUi.SecondTime     = FALSE;
        VarsUi.EnterOnlyCalls = FALSE;
        VarsUi.ExitOnlyCalls  = FALSE;

        if (VarsUi.pMenuLevel->ItemIndex > 1)
        {
          VarsUi.pMenuLevel->ItemIndex--;
        }
        else
        {
          if (VarsUi.pMenuLevel->Items > 2)
          {
            VarsUi.pMenuLevel->ItemIndex = VarsUi.pMenuLevel->Items;
          }
        }
      }
      IOMapUi.State = NEXT_MENU;
    }
    break;

    case RIGHT_PRESSED :
    {
      if ((VarsUi.pMenuLevel->SpecialFlags & MENU_LEFT_RIGHT_AS_CALL))
      {
        cUiMenuCallFunction(VarsUi.pMenuLevel->FunctionNo,MENU_RIGHT);
      }
      else
      {
        VarsUi.SecondTime     = FALSE;
        VarsUi.EnterOnlyCalls = FALSE;
        VarsUi.ExitOnlyCalls  = FALSE;

        if (VarsUi.pMenuLevel->ItemIndex < VarsUi.pMenuLevel->Items)
        {
          VarsUi.pMenuLevel->ItemIndex++;
        }
        else
        {
          if (VarsUi.pMenuLevel->Items > 2)
          {
            VarsUi.pMenuLevel->ItemIndex = 1;
          }
        }
      }
      IOMapUi.State = NEXT_MENU;
    }
    break;

    case ENTER_PRESSED :
    {
      if ((VarsUi.pMenuLevel->SpecialFlags & MENU_ENTER_ACT_AS_EXIT))
      {
        cUiMenuExit();
      }
      else
      {
        cUiMenuEnter();
      }
    }
    break;

    case EXIT_PRESSED :
    {
      if ((VarsUi.pMenuLevel->SpecialFlags & MENU_EXIT_ACT_AS_ENTER))
      {
        cUiMenuEnter();
      }
      else
      {
        cUiMenuExit();
      }
    }
    break;

    case CONNECT_REQUEST :
    {
      if (cUiBTConnectRequest(MENU_INIT) == 0)
      {
        IOMapUi.BluetoothState &= ~BT_CONNECT_REQUEST;
        cUiLoadLevel(0,1,1);
        IOMapUi.State           = NEXT_MENU;
        IOMapUi.Flags          &= ~UI_BUSY;
      }
    }
    break;

    case EXECUTE_FILE :
    {
      cUiLoadLevel(0,1,1);
      cUiMenuEnter();
      cUiLoadLevel(VarsUi.MenuFileLevel,VarsUi.MenuFiles[VarsUi.MenuFileLevel].MenuLevel,VarsUi.MenuFiles[VarsUi.MenuFileLevel].MenuLevels[VarsUi.MenuFiles[VarsUi.MenuFileLevel].MenuLevel].ItemIndex);
      cUiMenuEnter();
      cUiLoadLevel(VarsUi.MenuFileLevel,VarsUi.MenuFiles[VarsUi.MenuFileLevel].MenuLevel,VarsUi.MenuFiles[VarsUi.MenuFileLevel].MenuLevels[VarsUi.MenuFiles[VarsUi.MenuFileLevel].MenuLevel].ItemIndex);
      cUiMenuEnter();
      cUiLoadLevel(VarsUi.MenuFileLevel,VarsUi.MenuFiles[VarsUi.MenuFileLevel].MenuLevel,VarsUi.MenuFiles[VarsUi.MenuFileLevel].MenuLevels[VarsUi.MenuFiles[VarsUi.MenuFileLevel].MenuLevel].ItemIndex);

      VarsUi.Function = 0;
      VarsUi.State                                    =  0;
      VarsUi.Pointer                                  =  0;
      VarsUi.FNOFState                                =  0;
      VarsUi.FBState                                  =  0;
      VarsUi.GUSState                                 =  0;

      strcpy((char*)VarsUi.SelectedFilename,(char*)IOMapUi.LMSfilename);
      IOMapUi.State   = EXECUTING_FILE;
      VarsUi.FileType = FILETYPE_LMS;
      cUiFileRun(MENU_INIT);
    }
    break;

    case EXECUTING_FILE :
    {
      if (cUiFileRun(MENU_RUN) == 0)
      {
        IOMapUi.Flags &= ~UI_EXECUTE_LMS_FILE;
        IOMapUi.State  = NEXT_MENU;
      }
    }
    break;

    case LOW_BATTERY :
    {
      if (DISPLAY_IDLE)
      {
        if (cUiReadButtons() != BUTTON_NONE)
        {
          pMapDisplay->Flags              &= ~DISPLAY_POPUP;
          pMapDisplay->pBitmaps[BITMAP_1]  = (BMPMAP*)VarsUi.LowBattSavedBitmap;
          IOMapUi.State                    = VarsUi.LowBattSavedState;
          IOMapUi.Flags                   &= ~UI_BUSY;
        }
      }
    }
    break;

    case BT_ERROR :
    {
      switch (IOMapUi.Error)
      {
        case LR_COULD_NOT_SAVE :
        {
          Tmp = TXT_FB_BT_ERROR_LR_COULD_NOT_SAVE_1;
        }
        break;

        case LR_STORE_IS_FULL :
        {
          Tmp = TXT_FB_BT_ERROR_LR_STORE_IS_FULL_1;
        }
        break;

        default :
        {
          Tmp = TXT_FB_BT_ERROR_LR_UNKOWN_ADDR_1;
        }
        break;

      }

      if (!cUiFeedback((BMPMAP*)Fail,Tmp,Tmp + 1,DISPLAY_SHOW_ERROR_TIME))
      {
        IOMapUi.BluetoothState &= ~BT_ERROR_ATTENTION;
        cUiLoadLevel(0,1,1);
        IOMapUi.State           = NEXT_MENU;
        IOMapUi.Flags          &= ~UI_BUSY;
      }
    }
    break;

  }

  // Check for low battery voltage
  if (VarsUi.LowBatt >= LOW_BATT_THRESHOLD)
  {
    if (!VarsUi.LowBattHasOccured)
    {
      if (DISPLAY_IDLE)
      {
        if (!(IOMapUi.Flags & UI_BUSY))
        {
          pMapDisplay->Flags                |= DISPLAY_POPUP;
          VarsUi.LowBattHasOccured           = 1;
          VarsUi.LowBattSavedState           = IOMapUi.State;
          VarsUi.LowBattSavedBitmap          = (UBYTE*)pMapDisplay->pBitmaps[BITMAP_1];
          pMapDisplay->pBitmaps[BITMAP_1]    = (BMPMAP*)LowBattery;
          pMapDisplay->UpdateMask            = BITMAP_BIT(BITMAP_1);
          IOMapUi.Flags                     |= UI_REDRAW_STATUS;
          IOMapUi.Flags                     |= UI_BUSY;
          IOMapUi.State                      = LOW_BATTERY;
        }
      }
    }
  }

  // Check for incomming BT connection requests
  if ((IOMapUi.BluetoothState & BT_CONNECT_REQUEST) && (!(IOMapUi.Flags & UI_BUSY)))
  {
    IOMapUi.Flags |= UI_BUSY;
    IOMapUi.State  = CONNECT_REQUEST;
  }

  // Check for BT errors
  if ((IOMapUi.BluetoothState & BT_ERROR_ATTENTION) && (!(IOMapUi.Flags & UI_BUSY)))
  {
    IOMapUi.Flags |= UI_BUSY;
    IOMapUi.State  = BT_ERROR;
  }

  // Check for incomming execute program
  if ((IOMapUi.Flags & UI_EXECUTE_LMS_FILE) && (!(IOMapUi.Flags & UI_BUSY)))
  {
    // Reset power down timer
    IOMapUi.Flags |= UI_RESET_SLEEP_TIMER;

    // Set state and busy
    IOMapUi.Flags |= UI_BUSY;
    IOMapUi.State  = EXECUTE_FILE;
  }

  // Check for power timeout
  if ((IOMapUi.Flags & UI_RESET_SLEEP_TIMER))
  {
    IOMapUi.Flags        &= ~UI_RESET_SLEEP_TIMER;
    IOMapUi.SleepTimer    = 0;
    VarsUi.SleepTimer     = 0;
  }
  if (IOMapUi.SleepTimeout)
  {
    if (++VarsUi.SleepTimer >= 60000)
    {
      VarsUi.SleepTimer   = 0;
      if (++IOMapUi.SleepTimer >= IOMapUi.SleepTimeout)
      {
        IOMapUi.ForceOff  = TRUE;
      }
    }
  }
  else
  {
    IOMapUi.Flags |= UI_RESET_SLEEP_TIMER;
  }

  // Check for "long prees on exit" power off
  if ((pMapButton->State[BTN1] & LONG_PRESSED_EV) && (pMapCmd->ProgStatus != PROG_RUNNING) && (VarsUi.Initialized == TRUE) && (VarsUi.State == 0))
  {
    IOMapUi.ForceOff = TRUE;
  }

  // Check for "force" off
  if (IOMapUi.ForceOff != FALSE)
  {
    IOMapUi.ForceOff = FALSE;
    VarsUi.Function  = FUNC_NO_OFF;
    VarsUi.Parameter = MENU_INIT;
    VarsUi.State     = 0;
    IOMapUi.State    = DRAW_MENU;
  }

  // Update status line
  cUiUpdateStatus();
}


void      cUiExit(void)
{
  VarsUi.Initialized        = FALSE;
  IOMapUi.State             = INIT_DISPLAY;
}
