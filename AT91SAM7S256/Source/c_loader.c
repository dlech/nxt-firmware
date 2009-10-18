//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 8:27                                     $
//
// Filename        $Workfile:: c_loader.c                                    $
//
// Version         $Revision:: 79                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/c_loader.c $
//
// Platform        C
//

#include  "stdconst.h"
#include  "modules.h"
#include  "c_loader.iom"
#include  "c_ioctrl.iom"
#include  "d_loader.h"
#include  "c_loader.h"

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

void      cLoaderInit(void* pHeader)
{

  IOMapLoader.pFunc       = &cLoaderFileRq;
  VarsLoader.IoMapHandle  = FALSE;
  pHeaders = pHeader;
  dLoaderInit();
  IOMapLoader.FreeUserFlash = dLoaderReturnFreeUserFlash();
}

void      cLoaderCtrl(void)
{
}



UWORD     cLoaderFileRq(UBYTE Cmd, UBYTE *pFileName, UBYTE *pBuffer, ULONG *pLength)
{
  UWORD   ReturnState;

  ReturnState = SUCCESS;

  switch(Cmd)
  {
    case OPENREAD:
    {
      ReturnState = dLoaderOpenRead(pFileName, pLength);
      if (0x8000 <= ReturnState)
      {
        dLoaderCloseHandle(ReturnState);
      }
    }
    break;
    case OPENREADLINEAR:
    {
      ReturnState = dLoaderGetFilePtr(pFileName, pBuffer, pLength);
      if (0x8000 <= ReturnState)
      {
        dLoaderCloseHandle(ReturnState);
      }

    }
    break;
    case OPENWRITE:
    {

      /* This is to create a new file */
      ReturnState = dLoaderCreateFileHeader(*pLength, pFileName, (UBYTE) NONLINEAR, SYSTEMFILE);
      if (0x8000 <= ReturnState)
      {
        dLoaderCloseHandle(ReturnState);
      }
      else
      {
        IOMapLoader.FreeUserFlash = dLoaderReturnFreeUserFlash();
      }
    }
    break;
    case OPENWRITELINEAR:
    {
      ReturnState = dLoaderCreateFileHeader(*pLength, pFileName, (UBYTE) LINEAR, SYSTEMFILE);
      if (0x8000 <= ReturnState)
      {
        dLoaderCloseHandle(ReturnState);
      }
      else
      {
        IOMapLoader.FreeUserFlash = dLoaderReturnFreeUserFlash();
      }
    }
    break;
    case OPENWRITEDATA:
    {

      ReturnState = dLoaderCreateFileHeader(*pLength, pFileName, (UBYTE) LINEAR, DATAFILE);
      if (0x8000 <= ReturnState)
      {
        dLoaderCloseHandle(ReturnState);
      }
      else
      {
        IOMapLoader.FreeUserFlash = dLoaderReturnFreeUserFlash();
      }
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
      ReturnState = dLoaderDelete(pFileName);
      IOMapLoader.FreeUserFlash = dLoaderReturnFreeUserFlash();

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


