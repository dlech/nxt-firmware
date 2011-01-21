//
// Date init       14.12.2004
//
// Revision date   $Date:: 12-03-08 15:28                                    $
//
// Filename        $Workfile:: c_loader.c                                    $
//
// Version         $Revision:: 5                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/c_load $
//
// Platform        C
//

#include  "stdconst.h"
#include  "modules.h"
#include  "c_loader.iom"
#include  "c_ioctrl.iom"
#include  "d_loader.h"
#include  "c_loader.h"
#include <string.h>

static    IOMAPLOADER   IOMapLoader;
static    VARSLOADER    VarsLoader;
static    HEADER        **pHeaders;

const     HEADER        cLoader =
{
  0x00090001L,
  "Loader",
  cLoaderInit,
  cLoaderCtrl,
  cLoaderExit,
  (void *)&IOMapLoader,
  (void *)&VarsLoader,
  (UWORD)sizeof(IOMapLoader),
  (UWORD)sizeof(VarsLoader),
  0x0000                      //Code size - not used so far
};

UWORD     cLoaderFileRq(UBYTE Cmd, UBYTE *pFileName, UBYTE *pBuffer, ULONG *pLength);
UWORD     cLoaderGetIoMapInfo(ULONG ModuleId, UBYTE *pIoMap, UWORD *pIoMapSize);
UWORD     cLoaderFindModule(UBYTE *pBuffer);
void      cLoaderGetModuleName(UBYTE *pDst, UBYTE *pModule);
UWORD     cLoaderCreateFile(UBYTE *pFileName, ULONG *pLength, UBYTE bLinear, UBYTE fType);
UWORD     cLoaderRenameFile(UBYTE *pFileName, UBYTE *pBuffer, ULONG *pLength);
UWORD     cLoaderOpenRead(UBYTE *pFileName, UBYTE *pBuffer, ULONG *pLength, UBYTE bLinear);
UWORD     cLoaderDeleteFile(UBYTE *pFileName);
UWORD     cLoaderResizeFile(UBYTE *pFileName, ULONG pLength);

void      cLoaderInit(void* pHeader)
{

  IOMapLoader.pFunc       = &cLoaderFileRq;
  VarsLoader.IoMapHandle  = FALSE;
  VarsLoader.Resizing     = FALSE;
  pHeaders = pHeader;
  dLoaderInit();
  IOMapLoader.FreeUserFlash = dLoaderReturnFreeUserFlash();
}

void      cLoaderCtrl(void)
{
  if (VarsLoader.Resizing) 
  {
    // keep resizing the file currently in the file resize operation
    // copy 1024 bytes from old file handle to new file handle
    // if no more bytes to copy then set Resizing to FALSE,
    // close both files, and delete the old file.
  }
}

UWORD cLoaderCreateFile(UBYTE *pFileName, ULONG *pLength, UBYTE bLinear, UBYTE fType)
{
  UWORD ReturnState;
  /* This is to create a new file */
  ReturnState = dLoaderCreateFileHeader(*pLength, pFileName, bLinear, fType);
  if (0x8000 <= ReturnState)
  {
    dLoaderCloseHandle(ReturnState);
  }
  else
  {
    IOMapLoader.FreeUserFlash = dLoaderReturnFreeUserFlash();
  }
  return ReturnState;
}

UWORD cLoaderRenameFile(UBYTE *pFileName, UBYTE *pBuffer, ULONG *pLength)
{
  UWORD ReturnState;
  UBYTE FoundName[FILENAME_LENGTH + 1];
  
  /* Check for file exists*/
  ReturnState = dLoaderFind(pBuffer, FoundName, pLength, pLength, (UBYTE) SEARCHING);
  dLoaderCloseHandle(LOADER_HANDLE(ReturnState));
  if (FILENOTFOUND == LOADER_ERR(ReturnState))
  {
    ReturnState = dLoaderFind(pFileName, FoundName, pLength, pLength, (UBYTE) SEARCHING);
    if (ReturnState < 0x8000)
    {
      ReturnState = dLoaderCheckFiles((UBYTE) ReturnState);
      if (ReturnState < 0x8000)
      {
        dLoaderRenameFile((UBYTE) ReturnState, pBuffer);
      }
    }
    dLoaderCloseHandle(LOADER_HANDLE(ReturnState));
  }
  else
  {
    if (SUCCESS == LOADER_ERR(ReturnState))
    {
      ReturnState |= FILEEXISTS;
    }
  }
  return ReturnState;
}

UWORD cLoaderOpenRead(UBYTE *pFileName, UBYTE *pBuffer, ULONG *pLength, UBYTE bLinear)
{
  UWORD ReturnState;
  if (bLinear)
    ReturnState = dLoaderGetFilePtr(pFileName, pBuffer, pLength);
  else
    ReturnState = dLoaderOpenRead(pFileName, pLength);
  if (0x8000 <= ReturnState)
  {
    dLoaderCloseHandle(ReturnState);
  }
  return ReturnState;
}

UWORD cLoaderDeleteFile(UBYTE *pFileName)
{
  UWORD ReturnState;
  ReturnState = dLoaderDelete(pFileName);
  IOMapLoader.FreeUserFlash = dLoaderReturnFreeUserFlash();
  return ReturnState;
}

UWORD cLoaderResizeFile(UBYTE *pFileName, ULONG pLength)
{
  UWORD ReturnState = SUCCESS;
  /* 
    All that this method can do is start the process of
    resizing a file.  To do that we will 
    a) rename the file
    b) open old file for reading
    c) create new file for writing
    d) store both handles in VarsLoader & set resizing flag
    e) if any errors occur in a, b, or c then restore original file
    f) return LOADER_BUSY (maybe?)
  */
/*
  // rename file to _tmpoldname
  strcat __frsFRArgs.NewFilename, '_tmp', __frsOldName
  mov __frsFRArgs.OldFilename, __frsOldName
  syscall FileRename, __frsFRArgs
  mov __frsResult, __frsFRArgs.Result
  brtst NEQ, __frsEnd, __frsResult
  // old file has been renamed successfully
  mov __frsFOReadArgs.Filename, __frsFRArgs.NewFilename
  syscall FileOpenRead, __frsFOReadArgs
  mov __frsResult, __frsFOReadArgs.Result
  brtst NEQ, __frsOpenReadFailed, __frsResult
  // renamed file is open for reading
  mov __frsFOWriteArgs.Filename, __frsOldName
  mov __frsFOWriteArgs.Length, __frsNewSize
  syscall FileOpenWrite, __frsFOWriteArgs
  mov __frsResult, __frsFOWriteArgs.Result
  brtst NEQ, __frsOpenWriteFailed, __frsResult
  // both files are open
  mov __frsFReadArgs.FileHandle, __frsFOReadArgs.FileHandle
  mov __frsFWriteArgs.FileHandle, __frsFOWriteArgs.FileHandle
__frsCopyLoop:
  set __frsFReadArgs.Length, 1024
  syscall FileRead, __frsFReadArgs
  brtst NEQ, __frsEndLoop, __frsFReadArgs.Result
  brtst LTEQ, __frsEndLoop, __frsFReadArgs.Length
  mov __frsFWriteArgs.Buffer, __frsFReadArgs.Buffer
  mov __frsFWriteArgs.Length, __frsFReadArgs.Length
  syscall FileWrite, __frsFWriteArgs
  brtst NEQ, __frsEndLoop, __frsFWriteArgs.Result
  jmp __frsCopyLoop
__frsEndLoop:
  // close read file
  mov __frsFCArgs.FileHandle, __frsFOReadArgs.FileHandle
  syscall FileClose, __frsFCArgs
  // close write file
  mov __frsFCArgs.FileHandle, __frsFOWriteArgs.FileHandle
  syscall FileClose, __frsFCArgs
  // delete read file
  mov __frsFDArgs.Filename, __frsFOReadArgs.Filename
  syscall FileDelete, __frsFDArgs
  jmp __frsEnd
__frsOpenWriteFailed:
  // close read file
  mov __frsFCArgs.FileHandle, __frsFOReadArgs.FileHandle
  syscall FileClose, __frsFCArgs
//  jmp __frsEnd
__frsOpenReadFailed:
  // if the open read failed rename tmp back to original and exit
  mov __frsFRArgs.OldFilename, __frsFRArgs.NewFilename
  mov __frsFRArgs.NewFilename, __frsOldName
  syscall FileRename, __frsFRArgs
__frsEnd:
  return
*/
  return ReturnState;
}

UWORD     cLoaderFileRq(UBYTE Cmd, UBYTE *pFileName, UBYTE *pBuffer, ULONG *pLength)
{
  UWORD   ReturnState;

  ReturnState = SUCCESS;

  switch(Cmd)
  {
    case OPENREAD:
    {
      ReturnState = cLoaderOpenRead(pFileName, pBuffer, pLength, FALSE);
    }
    break;
    case OPENREADLINEAR:
    {
      ReturnState = cLoaderOpenRead(pFileName, pBuffer, pLength, TRUE);
    }
    break;
    case OPENWRITE:
    {

      /* This is to create a new file */
      ReturnState = cLoaderCreateFile(pFileName, pLength, (UBYTE) NONLINEAR, SYSTEMFILE);
    }
    break;
    case OPENWRITELINEAR:
    {
      ReturnState = cLoaderCreateFile(pFileName, pLength, (UBYTE) LINEAR, SYSTEMFILE);
    }
    break;
    case OPENWRITEDATA:
    {
      ReturnState = cLoaderCreateFile(pFileName, pLength, (UBYTE) NONLINEAR, DATAFILE);
    }
    break;
    case OPENAPPENDDATA:
    {
      ReturnState = dLoaderOpenAppend(pFileName, pLength);
      if (LOADER_ERR(ReturnState) != SUCCESS)
      {
        dLoaderCloseHandle(ReturnState);
      }
    }
    break;
    case CLOSE:
    {
      ReturnState = dLoaderCloseHandle(*pFileName);
    }
    break;
    case CROPDATAFILE:
    {
      ReturnState = dLoaderCropDatafile(*pFileName);
      IOMapLoader.FreeUserFlash = dLoaderReturnFreeUserFlash();
    }
    break;
    case RESIZEDATAFILE:
    {
      ReturnState = cLoaderResizeFile(pFileName, *pLength);
      IOMapLoader.FreeUserFlash = dLoaderReturnFreeUserFlash();
    }
    break;
    case SEEKFROMSTART:
    case SEEKFROMCURRENT:
    case SEEKFROMEND:
    {
      // *pFileName is the handle, *pLength is the offset, Cmd-SEEKFROMSTART is the origin
      ReturnState = dLoaderSeek(*pFileName, *(SLONG*)pLength, Cmd-SEEKFROMSTART);
    }
    break;
    case FILEPOSITION:
    {
      // *pFileName is the handle, pLength is the returned file position
      ReturnState = dLoaderTell(*pFileName, pLength);
    }
    break;
    case READ:
    {
      ReturnState = dLoaderRead(*pFileName, pBuffer, pLength);
    }
    break;
    case WRITE:
    {
      ReturnState = dLoaderWriteData(*pFileName, pBuffer, (UWORD*)pLength);
    }
    break;
    case FINDFIRST:
    {
      ULONG DataLength;

      ReturnState = dLoaderFind(pFileName, pBuffer, pLength, &DataLength, (UBYTE) SEARCHING);
      if (0x8000 <= ReturnState)
      {
        dLoaderCloseHandle(ReturnState);
      }
    }
    break;
    case FINDNEXT:
    {
      UWORD Handle;
      ULONG DataLength;

      Handle = *pFileName;
      ReturnState = dLoaderFindNext(Handle, pBuffer, pLength, &DataLength);
    }
    break;
    case DELETE:
    {
      ReturnState = cLoaderDeleteFile(pFileName);

    }
    break;
    case DELETEUSERFLASH:
    {
      dLoaderDeleteAllFiles();
      IOMapLoader.FreeUserFlash = dLoaderReturnFreeUserFlash();

    }
    break;

    case FINDFIRSTMODULE:
    {
      if (FALSE == VarsLoader.IoMapHandle)
      {
        VarsLoader.IoMapHandle    = TRUE;
        VarsLoader.ModSearchIndex = 0;
        dLoaderInsertSearchStr(VarsLoader.ModSearchStr, pFileName, &(VarsLoader.ModSearchType));
        ReturnState = cLoaderFindModule(pBuffer);
      }
      else
      {
        ReturnState = NOMOREHANDLES;
      }
    }
    break;

    case FINDNEXTMODULE:
    {
      ReturnState = cLoaderFindModule(pBuffer);
    }
    break;

    case CLOSEMODHANDLE:
    {
      VarsLoader.IoMapHandle  = FALSE;
      ReturnState             = SUCCESS;
    }
    break;

    case IOMAPREAD:
    {

      UBYTE *pIoMap;
      ULONG Ptr;
      UWORD IoMapSize;
      UBYTE Tmp;

      pIoMap = NULL;
      ReturnState = cLoaderGetIoMapInfo((*(ULONG*)(pFileName)),(UBYTE*)(&pIoMap), &IoMapSize);

      /* Did we have a valid module ID ?*/
      if (SUCCESS == LOADER_ERR(ReturnState))
      {

        /* This is the offset  */
        Ptr  =   pBuffer[0];
        Ptr |=  (UWORD)pBuffer[1] << 8;

        /* is the offset within the limits of the iomap size? */
        if ((Ptr + *pLength) <= IoMapSize)
        {

          /* Add the offset to the pointer */
          pIoMap += Ptr;

          for (Tmp = 0; Tmp < *pLength; Tmp++)
          {
            pBuffer[Tmp + 2] = *pIoMap;
            pIoMap++;
          }
        }
        else
        {

          /* Error - not within the bounderies */
          ReturnState = OUTOFBOUNDERY;
          *pLength    = 0;
        }
      }
      else
      {

        /* Error - not a valid module id */
        *pLength = 0;
      }
    }
    break;

    case IOMAPWRITE:
    {
      UBYTE *pIoMap;
      ULONG Ptr;
      UWORD IoMapSize;
      UWORD Tmp;


      pIoMap = NULL;
      ReturnState = cLoaderGetIoMapInfo(*((ULONG*)pFileName), (UBYTE*)&pIoMap, &IoMapSize);

      if (LOADER_ERR(ReturnState) == SUCCESS)
      {

        /* This is the offset  */
        Ptr  = *pBuffer;
        pBuffer++;
        Tmp = *pBuffer;
        Ptr |= Tmp << 8;
        pBuffer++;

        if ((Ptr + *pLength) <= IoMapSize)
        {

          pIoMap += Ptr;
          for (Tmp = 0; Tmp < *pLength; Tmp++)
          {
            *pIoMap = pBuffer[Tmp];
            pIoMap++;
          }
        }
        else
        {

          /* Error - not within the bounderies */
          ReturnState  = OUTOFBOUNDERY;
          *pLength = 0;
        }
      }
      else
      {

        /* Error - not a valid module id */
        *pLength = 0;
      }
    }
    break;

    case RENAMEFILE:
    {
      ReturnState = cLoaderRenameFile(pFileName, pBuffer, pLength);
    }
    break;

    default:
    {
    }
    break;
  }
  return (ReturnState);
}

UWORD     cLoaderGetIoMapInfo(ULONG ModuleId, UBYTE *pIoMap, UWORD *pIoMapSize)
{
  UBYTE   Tmp;
  UBYTE   Exit;
  UWORD   RtnVal;

  RtnVal = SUCCESS;
  Tmp = 0;
  Exit = FALSE;
  while((Tmp < 32) && (Exit == FALSE))
  {
    if ((*(pHeaders[Tmp])).ModuleID == ModuleId)
    {
      Exit = TRUE;
    }
    else
    {
      Tmp++;
    }
  }

  /* Did we have a valid module ID ?*/
  if (TRUE == Exit)
  {
    /* Get the pointer of the module io map */
   *((ULONG *)pIoMap) = (ULONG)((*(pHeaders[Tmp])).pIOMap);
    *pIoMapSize = (*(pHeaders[Tmp])).IOMapSize;
  }
  else
  {
    RtnVal = MODULENOTFOUND;
  }

  /* To avoid a warning - this is optimized away */
  *pIoMap = *pIoMap;
  return(RtnVal);
}

UWORD     cLoaderFindModule(UBYTE *pBuffer)
{
  UBYTE   Tmp;
  UWORD   RtnVal;
  UBYTE   ModuleName[FILENAME_SIZE];

  RtnVal  = MODULENOTFOUND;

  for (Tmp = VarsLoader.ModSearchIndex; Tmp < 32; Tmp++)
  {
    if (pHeaders[Tmp] != 0)
    {

      cLoaderGetModuleName(ModuleName, ((*(pHeaders[Tmp])).ModuleName));
      if (SUCCESS == dLoaderCheckName(ModuleName, VarsLoader.ModSearchStr, VarsLoader.ModSearchType))
      {

        dLoaderCopyFileName(pBuffer, ModuleName);

        pBuffer[FILENAME_SIZE] = (UBYTE) ((*(pHeaders[Tmp])).ModuleID);
        pBuffer[FILENAME_SIZE + 1] = (UBYTE)(((*(pHeaders[Tmp])).ModuleID) >> 8);
        pBuffer[FILENAME_SIZE + 2] = (UBYTE)(((*(pHeaders[Tmp])).ModuleID) >> 16);
        pBuffer[FILENAME_SIZE + 3] = (UBYTE)(((*(pHeaders[Tmp])).ModuleID) >> 24);

        pBuffer[FILENAME_SIZE + 4] = (UBYTE)(((*(pHeaders[Tmp])).ModuleSize));
        pBuffer[FILENAME_SIZE + 5] = (UBYTE)(((*(pHeaders[Tmp])).ModuleSize) >> 8);
        pBuffer[FILENAME_SIZE + 6] = (UBYTE)(((*(pHeaders[Tmp])).ModuleSize) >> 16);
        pBuffer[FILENAME_SIZE + 7] = (UBYTE)(((*(pHeaders[Tmp])).ModuleSize) >> 24);

        pBuffer[FILENAME_SIZE + 8] = (UBYTE) ((*(pHeaders[Tmp])).IOMapSize);
        pBuffer[FILENAME_SIZE + 9] = (UBYTE)(((*(pHeaders[Tmp])).IOMapSize) >> 8);

        RtnVal     = SUCCESS;
        (VarsLoader.ModSearchIndex)    = Tmp + 1;
        Tmp        = 32;
      }
    }
  }
  return(RtnVal);
}

void      cLoaderGetModuleName(UBYTE *pDst, UBYTE *pModule)
{
  UBYTE   Tmp;

  for(Tmp = 0; Tmp < FILENAME_SIZE; Tmp++)
  {
    if (0 != pModule[Tmp])
    {
      pDst[Tmp] = pModule[Tmp];
    }
    else
    {
      pDst[Tmp++] = '.';
      pDst[Tmp++] = 'm';
      pDst[Tmp++] = 'o';
      pDst[Tmp++] = 'd';
      pDst[Tmp] = '\0';
      Tmp = FILENAME_SIZE;
    }
  }
}

void      cLoaderExit(void)
{
}


