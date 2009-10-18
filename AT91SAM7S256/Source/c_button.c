//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 9:58                                     $
//
// Filename        $Workfile:: c_button.c                                    $
//
// Version         $Revision:: 16                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/c_button.c $
//
// Platform        C
//

#include  "stdconst.h"
#include  "modules.h"
#include  "c_button.h"
#include  "c_button.iom"
#include  "c_button.h"
#include  "d_button.h"

#define   BTN_PRESCALER      2

enum
{
  LONG_TIME     = (2000/BTN_PRESCALER)
};

static    IOMAPBUTTON   IOMapButton;
static    VARSBUTTON    VarsButton;
static    UBYTE         BtnCnt;

const     HEADER        cButton =
{
  0x00040001L,
  "Button",
  cButtonInit,
  cButtonCtrl,
  cButtonExit,
  (void *)&IOMapButton,
  (void *)&VarsButton,
  (UWORD)sizeof(IOMapButton),
  (UWORD)sizeof(VarsButton),
  0x0000                      //Code size - not used so far
};


void      cButtonInit(void* pHeader)
{
  UBYTE   Tmp;

  for (Tmp = 0; Tmp < NO_OF_BTNS; Tmp++)
  {
    IOMapButton.State[Tmp]                = 0;
    IOMapButton.BtnCnt[Tmp].PressedCnt    = 0;
    IOMapButton.BtnCnt[Tmp].LongPressCnt  = 0;
    IOMapButton.BtnCnt[Tmp].ShortRelCnt   = 0;
    IOMapButton.BtnCnt[Tmp].LongRelCnt    = 0;
    VarsButton.Cnt[Tmp]                   = 0;
  }
  VarsButton.OldState = 0;
  BtnCnt              = 0;
  dButtonInit(BTN_PRESCALER);
}

void      cButtonCtrl(void)
{
  UBYTE ButtonState, Tmp, ButtonNo;

  for (Tmp = 0; Tmp < NO_OF_BTNS; Tmp++)
  {
    IOMapButton.State[Tmp] &= ~PRESSED_EV;
  }
  if (++BtnCnt >= BTN_PRESCALER)
  {
    BtnCnt = 0;
    dButtonRead(&ButtonState);

    ButtonNo = 0x01;
    for (Tmp = 0; Tmp < NO_OF_BTNS; Tmp++)
    {
      if (ButtonState & ButtonNo)
      {
        if (LONG_TIME >= (VarsButton.Cnt[Tmp]))
        {
          (VarsButton.Cnt[Tmp])++;
        }
        IOMapButton.State[Tmp] = PRESSED_STATE;
        if (!((VarsButton.OldState) & ButtonNo))
        {

          /* Button just pressed */
          (IOMapButton.State[Tmp]) |= PRESSED_EV;
          (IOMapButton.BtnCnt[Tmp].PressedCnt)++;
          VarsButton.Cnt[Tmp]     = 0;
        }
        else
        {
          if (LONG_TIME == VarsButton.Cnt[Tmp])
          {
            IOMapButton.State[Tmp] |= LONG_PRESSED_EV;
            (IOMapButton.BtnCnt[Tmp].LongPressCnt)++;
          }
        }
      }
      else
      {
        IOMapButton.State[Tmp] = 0x00;
        if ((VarsButton.OldState) & ButtonNo)
        {
          if (VarsButton.Cnt[Tmp] > LONG_TIME)
          {
            IOMapButton.State[Tmp] = LONG_RELEASED_EV;
            (IOMapButton.BtnCnt[Tmp].LongRelCnt)++;

          }
          else
          {
            IOMapButton.State[Tmp] = SHORT_RELEASED_EV;
            (IOMapButton.BtnCnt[Tmp].ShortRelCnt)++;
          }
        }
      }
      ButtonNo <<= 1;
      IOMapButton.BtnCnt[Tmp].RelCnt = ((IOMapButton.BtnCnt[Tmp].ShortRelCnt) + (IOMapButton.BtnCnt[Tmp].LongRelCnt));
    }
    VarsButton.OldState = ButtonState;    
  }
}

void      cButtonExit(void)
{
  dButtonExit();
}
