//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 8:27                                     $
//
// Filename        $Workfile:: c_loader.h                                    $
//
// Version         $Revision:: 8                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/c_loader.h $
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
}VARSLOADER;

void      cLoaderInit(void* pHeader);
void      cLoaderCtrl(void);
void      cLoaderExit(void);

extern    const HEADER cLoader;

#endif



