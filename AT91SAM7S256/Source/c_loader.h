//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: c_loader.h                                    $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/c_load $
//
// Platform        C
//

#ifndef   C_LOADER
#define   C_LOADER

enum
{
  LOADER_BUSY,
  TOO_MANY_FILES,
  NO_MORE_FLASH,
  LOADER_SUCCESS
};

typedef   struct
{
  UBYTE   ModSearchStr[FILENAME_LENGTH + 1];
  UBYTE   ModSearchIndex;
  UBYTE   ModSearchType;
  UBYTE   UsbStatus;
  UBYTE   IoMapHandle;
  UBYTE   Resizing;
  UBYTE   ResizeOldHandle;
  UBYTE   ResizeNewHandle;
}VARSLOADER;

void      cLoaderInit(void* pHeader);
void      cLoaderCtrl(void);
void      cLoaderExit(void);

extern    const HEADER cLoader;

#endif



