//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: m_sched.c                                     $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/m_sche $
//
// Platform        C
//


#define   INCLUDE_OS

#define   MODULEHEADERS                 32

#include  "stdconst.h"
#include  "modules.h"
#include  "m_sched.h"

#include  "c_comm.h"
#include  "c_input.h"
#include  "c_button.h"
#include  "c_loader.h"
#include  "c_sound.h"
#include  "c_display.h"
#include  "c_lowspeed.h"
#include  "c_output.h"
#include  "c_cmd.h"
#include  "c_cmd.iom"
#include  "c_ioctrl.h"
#include  "c_ui.h"


static    const HEADER*  pModuleHeaders[MODULEHEADERS] = 
{
  &cComm,
  &cInput,
  &cButton,
  &cDisplay,
  &cLoader,
  &cLowSpeed,
  &cOutput,
  &cSound,
  &cIOCtrl,
  &cCmd,
  &cUi,
  0
};


void      mSchedInit(void)
{
  UWORD   Tmp;
 
  Tmp = 0;
  while(pModuleHeaders[Tmp])
  {
    (*pModuleHeaders[Tmp]).cInit((void*) pModuleHeaders);
    Tmp++;
  }
}


UBYTE     mSchedCtrl(void)
{
  UWORD   Tmp;

  Tmp = 0;
  while(pModuleHeaders[Tmp])
  {
    (*pModuleHeaders[Tmp]).cCtrl();
    Tmp++;
  }

  return(((IOMAPCMD*)(pModuleHeaders[ENTRY_CMD]->pIOMap))->Awake);
}


void      mSchedExit(void)
{
  UWORD   Tmp;

  Tmp = 0;
  while(pModuleHeaders[Tmp])
  {
    (*pModuleHeaders[Tmp]).cExit();
    Tmp++;
  }
}

