//
// Date init       14.12.2004
//
// Revision date   $Date:: 24-06-09 8:53                                     $
//
// Filename        $Workfile:: d_loader.c                                    $
//
// Version         $Revision:: 18                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_load $
//
// Platform        C
//

#include  "stdconst.h"
#include  "modules.h"
#include  "m_sched.h"
#include  "d_loader.h"
#include  "d_loader.r"
#include  <string.h>
#include  <ctype.h>

#define   FILEVERSION                   (0x0000010DL) //(0x0000010CL)

#define   MAX_FILES                     ((FILETABLE_SIZE) - 1)  /* Last file entry is used for file version*/
#define   FILEVERSIONINDEX              ((FILETABLE_SIZE) - 1)  /* Last file entry is used for file version*/
#define   MAX_WRITE_BUFFERS             4
#define   FLASHOFFSET                   (0x100000L)

#define   IS_LOADER_ERR(LStatus)        (((LStatus) & 0xFF00) != SUCCESS)
#define   SECTORINDEXUSERFLASH          ((STARTOFUSERFLASH & ~FLASHOFFSET)/256/32)
#define   SECTORPOINTERUSERFLASH        (((STARTOFUSERFLASH & ~FLASHOFFSET) - ((SECTORINDEXUSERFLASH * 32) * 256))/256)

typedef   struct
{
  const   UBYTE  *pFlash;
  const   UWORD  *pSectorNo;
  ULONG           ReadLength;
  ULONG           DataLength;
  ULONG           FileDlPtr;
  UBYTE           SearchStr[FILENAME_SIZE];
  UWORD           FileIndex;
  UWORD           CheckSum;
  UBYTE           SearchType;
  UBYTE           Status;
  UBYTE           FileType;
  UBYTE           WriteBufNo;
}HANDLE;

typedef   struct
{
  ULONG   Buf[SECTORSIZE/4];
  UBYTE   BufIndex;
  UBYTE   Status;
}WRITEBUF;

static    HANDLE      HandleTable[MAX_HANDLES];
static    WRITEBUF    WriteBuffer[MAX_WRITE_BUFFERS];
static    ULONG       SectorTable[NOOFSECTORS>>5];
static    FILEHEADER  Header;
static    ULONG       FreeUserFlash;
static    UWORD       FreeSectors;

void      dLoaderUpdateSectorTable(void);
UWORD     dLoaderGetFreeHandle(void);
const     UBYTE * dLoaderGetNextSectorPtr(UBYTE Handle);
ULONG     dLoaderReturnFreeFlash(void);
UWORD     dLoaderAllocateHeader(UWORD Handle, ULONG *FileStartAdr, FILEHEADER *pHeader, UWORD HeaderByteSize, UWORD CompleteFileSectorSize);
UWORD     dLoaderFlashFileHeader(UWORD Handle, ULONG FileStartAdr, FILEHEADER *pHeader, UWORD HeaderByteSize);
UWORD     dLoaderFindFirstSector(UBYTE Type, UWORD SectorCount, UWORD *pSector);
UWORD     dLoaderAllocateWriteBuffer(UWORD  Handle);
UWORD     dLoaderSetFilePointer(UWORD Handle, ULONG BytePtr, const UBYTE **pData);
UWORD     dLoaderGetSectorNumber(ULONG Adr);
void      dLoaderCheckVersion(void);
UWORD     dLoaderCheckHandleForReadWrite(UWORD Handle);
UWORD     dLoaderCheckHandle(UWORD Handle, UBYTE Operation);
ULONG     dLoaderCalcFreeFileSpace(UWORD NosOfFreeSectors);
UWORD     dLoaderCheckDownload(UBYTE *pName);
UWORD     dLoaderAvailFileNo(void);


void      dLoaderInit(void)
{
  UWORD   Tmp;

  LOADERInit;

  /* Clear handle table */
  for (Tmp = 0; Tmp < MAX_HANDLES; Tmp++)
  {
    HandleTable[Tmp].Status     = FREE;
    HandleTable[Tmp].WriteBufNo = FREEBUFNO;
  }

  /* Clear write buffers */
  for (Tmp = 0; Tmp < MAX_WRITE_BUFFERS; Tmp++)
  {
    WriteBuffer[Tmp].Status = FREE;
  }

  dLoaderCheckVersion();
  dLoaderUpdateSectorTable();
}


UWORD     dLoaderAvailFileNo(void)
{
  UBYTE   Tmp, Tmp2;
  UWORD   ReturnVal;

  ReturnVal = NOMOREFILES;
  Tmp2      = 0;
  for(Tmp = 0; Tmp < MAX_HANDLES; Tmp++)
  {

    /* Check for files allready downloading except datafiles as the have entered their */
    /* filepointer in the filepointer table at begin of download                       */
    if ((DOWNLOADING == HandleTable[Tmp].Status) && (DATAFILE != HandleTable[Tmp].FileType))
    {
      Tmp2++;
    }
  }
  if ((0xFFFFFFFF == FILEPTRTABLE[(MAX_FILES - 1) - Tmp2]) || (0 == FILEPTRTABLE[(MAX_FILES - 1) - Tmp2]))
  {
    ReturnVal = SUCCESS;
  }
  return(ReturnVal);
}


void      dLoaderWriteFilePtrTable(ULONG *RamFilePtrTable)
{
  UWORD   TmpTableSize;

  /* FILETABLE_SIZE is in LONG */
  TmpTableSize = (FILETABLE_SIZE * 4);
  while(TmpTableSize)
  {
    TmpTableSize -= SECTORSIZE;
    dLoaderWritePage((ULONG)FILEPTRTABLE + TmpTableSize, SECTORSIZE, RamFilePtrTable + (TmpTableSize/4));
  }
}


UWORD     dLoaderInsertPtrTable(const UBYTE *pAdr, UWORD Handle)
{
  UWORD   TmpCnt;
  UWORD   Status;
  ULONG   PtrTable[FILETABLE_SIZE];

  /* It is possible to add the file as checking for number of files */
  /* is done when initiating the file download                      */
  memset(PtrTable, 0, sizeof(PtrTable));
  TmpCnt = MAX_FILES - 1;
  while(TmpCnt)
  {

    /* TmpCnt-- first because you want to copy from index 0 */
    TmpCnt--;
    PtrTable[TmpCnt + 1] = FILEPTRTABLE[TmpCnt];
  }

  /* Copy the new file in position 0 */
  PtrTable[0]         = (ULONG)pAdr;

  /* Add the File version to the top of the file list */
  PtrTable[FILEVERSIONINDEX] = FILEPTRTABLE[FILEVERSIONINDEX];

  /* Write the file pointer table to flash */
  dLoaderWriteFilePtrTable(PtrTable);

  /* FileIndex in HandleTable should be incremented by one - new file index is 0 */
  for (TmpCnt = 0; TmpCnt < MAX_HANDLES; TmpCnt++)
  {
    if (HandleTable[TmpCnt].Status != FREE)
    {
      (HandleTable[TmpCnt].FileIndex)++;
    }
  }
  HandleTable[Handle].FileIndex = 0;
  Status = SUCCESS | Handle;

  return(Status);
}


UWORD     dLoaderDeleteFilePtr(UWORD Handle)
{
  UWORD   ErrorCode;
  UWORD   LongCnt;
  ULONG   PtrTable[FILETABLE_SIZE];

  ErrorCode = SUCCESS;
  if (0xFFFFFFFF != FILEPTRTABLE[HandleTable[Handle].FileIndex])
  {
    ErrorCode = dLoaderCheckFiles(Handle);
    if (0x8000 > ErrorCode)
    {
      for (LongCnt = 0; LongCnt < (HandleTable[Handle].FileIndex); LongCnt++)
      {
        PtrTable[LongCnt] = FILEPTRTABLE[LongCnt];
      }

      /* Skip the file that has to be deleted "LongCnt + 1" */
      for ( ; LongCnt < (MAX_FILES - 1); LongCnt++)
      {
        PtrTable[LongCnt] = FILEPTRTABLE[LongCnt+1];
      }

      /* The top file entry is now free */
      PtrTable[MAX_FILES - 1] = 0xFFFFFFFF;

      /* Insert the file version         */
      PtrTable[MAX_FILES]     = FILEPTRTABLE[MAX_FILES];

      /* Write the file pointer table back into flash */
      dLoaderWriteFilePtrTable(PtrTable);
      dLoaderUpdateSectorTable();

      /* Update the HandleTable[].FileIndex */
      for (LongCnt = 0; LongCnt < MAX_HANDLES; LongCnt++)
      {
        
        /* FileIndex must not be decremented for to the file to be deleted (when Handle = LongCnt)*/
        if ((HandleTable[Handle].FileIndex < HandleTable[LongCnt].FileIndex) && (FREE != HandleTable[LongCnt].Status))
        {
          (HandleTable[LongCnt].FileIndex)--;
        }
      }
    }
  }
  else
  {
    ErrorCode = FILENOTFOUND;
  }
  return(ErrorCode | Handle);
}


void      dLoaderDeleteAllFiles(void)
{
  ULONG   Tmp;
  ULONG   PtrTable[FILETABLE_SIZE];

  /* Close all handles - all files is to be wiped out */
  for (Tmp = 0; Tmp < MAX_HANDLES; Tmp++)
  {
    dLoaderCloseHandle(Tmp);
  }

  for (Tmp = ((STARTOFUSERFLASH-FLASHOFFSET)/SECTORSIZE); Tmp < (SIZEOFFLASH/SECTORSIZE); Tmp++)
  {
    dLoaderErasePage(Tmp<<SECTORSIZESHIFT);
  }

  /* Insert the file version */
  memset(PtrTable, 0xFF, sizeof(PtrTable));
  PtrTable[FILEVERSIONINDEX] = FILEVERSION;

  /* Write an empty file pointer table to flash */
  dLoaderWriteFilePtrTable(PtrTable);

  /* Update all other parameters */
  dLoaderUpdateSectorTable();
  FreeUserFlash = dLoaderReturnFreeFlash();
}


void      dLoaderUpdateSectorTable(void)
{
  UWORD   Tmp;
  UWORD   SectorNo;
  const   FILEHEADER *pFile;
  ULONG   FileSize;
  const UWORD   *pSectorTable;

  Tmp = 0;

  memset(SectorTable, 0, sizeof(SectorTable));

  /* All file pointer are occupied as default */
  while (Tmp < MAX_FILES)
  {
    SectorNo = dLoaderGetSectorNumber((ULONG)&FILEPTRTABLE[Tmp]);
    SectorTable[SectorNo>>5] |= (0x1 << (SectorNo & 0x001F));
    Tmp += (SECTORSIZE >> 2);
  }

  for (Tmp = 0; Tmp < MAX_FILES; Tmp++)
  {
    if ((0xFFFFFFFF != FILEPTRTABLE[Tmp]) && (0x00000000 != FILEPTRTABLE[Tmp]))
    {
      pFile = (const FILEHEADER *) FILEPTRTABLE[Tmp];

      /* This is necessary if the start address is at the first address in an sector */
      SectorNo    = dLoaderGetSectorNumber((ULONG)pFile->FileStartAdr);
      SectorTable[SectorNo>>5] |= (0x1 << (SectorNo & 0x001F));

      /* This is necessary as the first sector (where the fileheader is) is not */
      /* included in the sector table                                           */
      SectorNo                  = dLoaderGetSectorNumber((ULONG)FILEPTRTABLE[Tmp]);
      SectorTable[SectorNo>>5] |= (0x1 << (SectorNo & 0x001F));

      /* First Sector with data has been allocated add this as the initial */
      /* file size                                                         */
      FileSize     = SECTORSIZE - ((pFile->FileStartAdr) & (SECTORSIZE-1)) ;
      pSectorTable = pFile->FileSectorTable;
      while((FileSize < (pFile->FileSize)) && (NOOFSECTORS > (*pSectorTable)))
      {
        SectorTable[(*pSectorTable)>>5] |= (0x1 << ((*pSectorTable) & 0x1F));
        if (0 == ((ULONG)(pSectorTable + 1) & (SECTORSIZE-1)))
        {
          pSectorTable = (UWORD*)(((ULONG)(*pSectorTable) << SECTORSIZESHIFT) | FLASHOFFSET);
        }
        else
        {
          *pSectorTable++;
          FileSize += SECTORSIZE;
        }
      }
    }
  }
  FreeUserFlash = dLoaderReturnFreeFlash();
}


UWORD     dLoaderCreateFileHeader(ULONG FileSize, UBYTE *pName, UBYTE LinearState, UBYTE FileType)
{
  UWORD       HeaderByteSize;
  ULONG       FileStartAdr;
  ULONG       CompleteFileByteSize;
  UWORD       Handle;
  UBYTE       Name[FILENAME_SIZE];
  ULONG       FileLength;
  ULONG       DataLength;
  UWORD       ErrorCode;
  UWORD       CompleteSectorNo;
  UWORD       Tmp;

  memset(&(Header.FileName), 0, sizeof(Header.FileName));
  memset(&(Header.FileSectorTable), 0xFF, sizeof(Header.FileSectorTable));

  ErrorCode = dLoaderFind(pName, Name, &FileLength, &DataLength, (UBYTE)BUSY);
  Handle =  ErrorCode & 0x00FF;
  if (SUCCESS == (ErrorCode & 0xFF00))
  {
    ErrorCode |= FILEEXISTS;
  }
  if (FILENOTFOUND == (ErrorCode & 0xFF00))
  {

    /* Here check for the download buffers for a matching download */
    /* in progress                                                 */
    ErrorCode &= 0x00FF;
    ErrorCode = dLoaderCheckDownload(pName);

    if (0x8000 > ErrorCode)
    {

      /* Check for file overflow */
      ErrorCode = dLoaderAvailFileNo();
      if (0x8000 > ErrorCode)
      {

        ErrorCode = dLoaderAllocateWriteBuffer(Handle);
        if (0x8000 > ErrorCode)
        {

          dLoaderCopyFileName((Header.FileName), pName);
          HandleTable[Handle].pSectorNo  = 0;
          HandleTable[Handle].DataLength = FileSize; /* used for end of file detection            */
          Header.FileSize                = FileSize; /* used to program into flash                */
          if (DATAFILE == FileType)
          {
            Header.DataSize = 0;
          }
          else
          {
            Header.DataSize = FileSize;
          }
          HandleTable[Handle].ReadLength = 0;
          HandleTable[Handle].FileType   = FileType | LinearState; /* if it is a datafile it can be stopped     */
          Header.FileType                = FileType | LinearState; /* FileType included for future appending    */

          /* File body size calculation*/
          CompleteFileByteSize = FileSize;

          /* Add the the fixed header to the fixed size    */
          CompleteFileByteSize  += HEADERFIXEDSIZE;

          /* Find the number of sectors used by the fixed file size  */
          CompleteSectorNo       = (CompleteFileByteSize - 1) >> SECTORSIZESHIFT;

          /* Add the size taken by the sectortable               */
          CompleteFileByteSize  += (CompleteSectorNo << 1);

          /* Recalc the number of sectors - to see wether the sectortable has caused */
          /* the sectornumber to encrease                                            */
          Tmp = ((CompleteFileByteSize - 1) >> SECTORSIZESHIFT);

          /* Substract from the original sectors used - Tmp holds the encreased number*/
          Tmp -= CompleteSectorNo;
          if (Tmp)
          {
            /* The sectortable takes up more than one sector */
            CompleteFileByteSize += Tmp << 1;
            CompleteSectorNo     += Tmp;
          }

          HeaderByteSize         = CompleteFileByteSize - FileSize;

          if (HeaderByteSize & 0x0003)
          {
            /* Header size is not a multiplum of 4 - now make it a mul 4 */
            HeaderByteSize  += (0x0004 - (HeaderByteSize & 0x0003));
          }

          if (FileSize <= FreeUserFlash)
          {

            /* Allocate file header   */
            Tmp = (((CompleteFileByteSize - 1) >> SECTORSIZESHIFT) + 1);
            Handle = dLoaderAllocateHeader(Handle, &FileStartAdr, &Header, HeaderByteSize, Tmp);
            if (Handle < 0x8000)
            {
              dLoaderFlashFileHeader(Handle, FileStartAdr, &Header, HeaderByteSize);

              /* If this is a datafile then add the filepointer to the filepointer table */
              /* now as the datafile do not need to have a special size to be valid      */
              if (DATAFILE & HandleTable[Handle].FileType)
              {
                ErrorCode = dLoaderInsertPtrTable((const UBYTE *)HandleTable[Handle].FileDlPtr, Handle);
              }
              FreeSectors -= Tmp;
              FreeUserFlash = dLoaderCalcFreeFileSpace(FreeSectors);
              HandleTable[Handle].Status = DOWNLOADING;
            }
          }
          else
          {
            ErrorCode = NOSPACE;
          }
        }
      }
    }
  }

  return(ErrorCode | Handle);
}


UWORD     dLoaderWriteData(UWORD Handle, UBYTE *pBuf, UWORD *pLen)
{
  UWORD   Tmp;
  UBYTE   *pSectorBuf;

  Handle =  dLoaderCheckHandle(Handle, DOWNLOADING);
  if (0x8000 > Handle)
  {

    if (*pLen > HandleTable[Handle].DataLength)
    {

      /* Write request exceeds filesize - only flash up to filesize*/
      *pLen   = HandleTable[Handle].DataLength;
      WriteBuffer[HandleTable[Handle].WriteBufNo].Status = DLERROR;  /* save error untill close handle */
    }

    pSectorBuf = (UBYTE *)WriteBuffer[HandleTable[Handle].WriteBufNo].Buf;
    for(Tmp = 0; Tmp < *pLen; Tmp++)
    {
      pSectorBuf[WriteBuffer[HandleTable[Handle].WriteBufNo].BufIndex] = pBuf[Tmp];
      if ((WriteBuffer[HandleTable[Handle].WriteBufNo].BufIndex) >= (SECTORSIZE-1))
      {
        dLoaderWritePage(((ULONG)HandleTable[Handle].pFlash & ~(SECTORSIZE - 1)), SECTORSIZE, WriteBuffer[HandleTable[Handle].WriteBufNo].Buf);
        WriteBuffer[HandleTable[Handle].WriteBufNo].BufIndex = 0;
        HandleTable[Handle].pFlash = dLoaderGetNextSectorPtr(Handle);
        memset(WriteBuffer[HandleTable[Handle].WriteBufNo].Buf, 0xFF, sizeof(WriteBuffer[0].Buf));
      }
      else
      {
        (WriteBuffer[HandleTable[Handle].WriteBufNo].BufIndex)++;
      }
    }
    HandleTable[Handle].DataLength -= *pLen;

    /* Check for correct end of file */
    if (0 == HandleTable[Handle].DataLength)
    {
      if ((WriteBuffer[HandleTable[Handle].WriteBufNo].BufIndex) != 0)
      {

        /* write the last data into the file */
        dLoaderWritePage(((ULONG)HandleTable[Handle].pFlash & ~(SECTORSIZE - 1)), WriteBuffer[HandleTable[Handle].WriteBufNo].BufIndex, WriteBuffer[HandleTable[Handle].WriteBufNo].Buf);
        WriteBuffer[HandleTable[Handle].WriteBufNo].BufIndex = 0;
      }
    }
  }
  else
  {
    *pLen   = 0;
  }

  if (DLERROR == WriteBuffer[HandleTable[Handle].WriteBufNo].Status)
  {

    /* DLERROR set due to over flow a file - EOFEXSPECTED should be set */
    /* for repeated overflow requests                                   */
    Handle |= EOFEXSPECTED;
  }
  return(Handle);
}


UWORD     dLoaderGetFreeHandle(void)
{
  UBYTE   Tmp;
  UWORD   Handle;

  Handle = NOMOREHANDLES;
  for(Tmp = 0; Tmp < MAX_HANDLES; Tmp++)
  {
    if (FREE == HandleTable[Tmp].Status)
    {
      HandleTable[Tmp].Status  = BUSY;
      Handle                   = 0;         /* Clear NOMOREHANDLES */
      Handle                   = Tmp;
      Tmp                      = MAX_HANDLES;
    }
  }
  return(Handle);
}

const UBYTE * dLoaderGetNextSectorPtr(UBYTE Handle)
{
  const UBYTE  *pAdr;

  /* Check for the last entry in a sector - if so,           */
  /* then this is the sector number on the next sector table */
  if (!((ULONG)(HandleTable[Handle].pSectorNo + 1) & (SECTORSIZE-1)))
  {
    HandleTable[Handle].pSectorNo = (const UWORD *)(((ULONG)(*(HandleTable[Handle].pSectorNo)) << SECTORSIZESHIFT) | FLASHOFFSET);
  }

  /* If pointing at an illegal adr then set it to NULL */
  if (SIZEOFFLASH < (ULONG)((ULONG)(HandleTable[Handle].pSectorNo) & ~FLASHOFFSET))
  {
    pAdr = NULL;
  }
  else
  {
    pAdr = (const UBYTE *)(((ULONG)(*(HandleTable[Handle].pSectorNo)) << SECTORSIZESHIFT) | FLASHOFFSET);
  }

  (HandleTable[Handle].pSectorNo)++;
  return(pAdr);
}

UWORD     dLoaderCloseHandle(UWORD Handle)
{
  UWORD       RtnStatus;
  FILEHEADER  *TmpFileHeader;

  RtnStatus = Handle;

  /* if it is a normal handle or handle closed due to an error then error must be different */
  /* from the no more handles available error (else you would delete a used handle)         */
  if (((0x8000 > Handle) || (NOMOREHANDLES != (Handle & 0xFF00))) && ((UBYTE)Handle < MAX_HANDLES))
  {
    Handle &= 0x00FF;
    if (FREE == HandleTable[Handle].Status)
    {
      RtnStatus |= HANDLEALREADYCLOSED;
    }
    else
    {

      /* Handle was NOT free - now close it */
      if (DOWNLOADING == HandleTable[Handle].Status)
      {
        if (DATAFILE & HandleTable[Handle].FileType)
        {

          /* This is a Datafile that should be closed and this is a legal action */
          /* 1. Write the data from the writebuffer into flash                   */
          /* 2. Update the Datalength in the file header                         */
          /* This takes minimum 8 mS (2 page writes into flash)                  */

          if (WriteBuffer[HandleTable[Handle].WriteBufNo].BufIndex)
          {

            /* There are databytes in the writebuffer write them into flash      */
            dLoaderWritePage(((ULONG)HandleTable[Handle].pFlash & ~(SECTORSIZE - 1)), WriteBuffer[HandleTable[Handle].WriteBufNo].BufIndex, WriteBuffer[HandleTable[Handle].WriteBufNo].Buf);
          }

          /* Now the databuffer is free now use if for a buffer for the fileheader*/
          memcpy(WriteBuffer[HandleTable[Handle].WriteBufNo].Buf, (void const*)HandleTable[Handle].FileDlPtr, SECTORSIZE);
          TmpFileHeader = (FILEHEADER *) WriteBuffer[HandleTable[Handle].WriteBufNo].Buf;
          TmpFileHeader->DataSize = TmpFileHeader->FileSize - HandleTable[Handle].DataLength;
          dLoaderWritePage(((ULONG)HandleTable[Handle].FileDlPtr & ~(SECTORSIZE - 1)), SECTORSIZE, WriteBuffer[HandleTable[Handle].WriteBufNo].Buf);
        }
        else
        {

          /* This is a system file being closed now update the file pointer table if no error and complete file written */
          if ((DLERROR != WriteBuffer[HandleTable[Handle].WriteBufNo].Status) && (0 == HandleTable[Handle].DataLength))
          {

            /* no error durig download - add the file pointer to the file pointer table */
            Handle = dLoaderInsertPtrTable((const UBYTE *) HandleTable[Handle].FileDlPtr, Handle);
          }
          else
          {

            /* an error has occured during download - now clean up the mess... */
            dLoaderUpdateSectorTable();
          }
        }
      }
      if (HandleTable[Handle].WriteBufNo != FREEBUFNO)
      {
        WriteBuffer[HandleTable[Handle].WriteBufNo].Status = FREE;
        HandleTable[Handle].WriteBufNo = FREEBUFNO;
      }
      HandleTable[Handle].Status = FREE;
    }
  }
  return(RtnStatus);
}


UWORD     dLoaderOpenRead(UBYTE *pFileName, ULONG *pLength)
{
  UWORD   Handle;
  UBYTE   Name[FILENAME_SIZE];
  const   FILEHEADER *TmpHeader;
  ULONG   FileLength;
  ULONG   DataLength;

  Handle = dLoaderFind(pFileName, Name, &FileLength, &DataLength, (UBYTE)BUSY);
  if (0x8000 > Handle)
  {
    if (FileLength)
    {
      TmpHeader = (FILEHEADER const *)(FILEPTRTABLE[HandleTable[Handle].FileIndex]);
      HandleTable[Handle].pFlash = (const UBYTE *)TmpHeader->FileStartAdr;
      HandleTable[Handle].pSectorNo    = TmpHeader->FileSectorTable;
      HandleTable[Handle].DataLength = TmpHeader->DataSize;
      HandleTable[Handle].ReadLength = 0;
      *pLength = TmpHeader->DataSize;
    }
    else
    {
      Handle = FILENOTFOUND;
    }
  }
  return(Handle);
}

UWORD     dLoaderSeek(UBYTE Handle, SLONG offset, UBYTE from)
{
  // move the ReadLength file pointer for this handle to the new offset
  // and update pFlash appropriately
  UWORD   Status;
  SLONG   distFromStart;
  const   FILEHEADER *TmpHeader;

  Status = dLoaderCheckHandleForReadWrite(Handle);
  if (0x8000 > Status)
  {
    Status = Handle;
    // calculate distance from start regardless of "from"
    // and start from there going forward unless distance > current
    // in which case start from current going forward
    switch (from) {
    case SEEK_FROMSTART:
      distFromStart = offset;
      break;
    case SEEK_FROMCURRENT:
      distFromStart = (SLONG)HandleTable[Handle].ReadLength + offset;
      break;
    case SEEK_FROMEND:
      distFromStart = (SLONG)HandleTable[Handle].DataLength + offset;
      break;
    }
    if (distFromStart != HandleTable[Handle].ReadLength) {
      if ((distFromStart < 0) || (distFromStart > HandleTable[Handle].DataLength))
        return (Status | INVALIDSEEK);
      if (distFromStart < HandleTable[Handle].ReadLength) {
        // start from the beginning in this case
        TmpHeader = (FILEHEADER const *)(FILEPTRTABLE[HandleTable[Handle].FileIndex]);
        HandleTable[Handle].pFlash       = (const UBYTE *)TmpHeader->FileStartAdr;
        HandleTable[Handle].pSectorNo    = TmpHeader->FileSectorTable;
        HandleTable[Handle].ReadLength = 0;
      }
      else
        distFromStart -= HandleTable[Handle].ReadLength; // dist from current
      // now move forward from the current location
      while (distFromStart > 0) {
        distFromStart--;
        // move to next byte in the flash
        HandleTable[Handle].pFlash++;
        // update our file pointer
        HandleTable[Handle].ReadLength++;
        // if we reach a flash sector boundary then find the next sector pointer
        if (!((ULONG)(HandleTable[Handle].pFlash) & (SECTORSIZE-1)))
        {
          HandleTable[Handle].pFlash = dLoaderGetNextSectorPtr(Handle);
        }
      }
      // if we are open for writing then we need to do a little more work
      if (HandleTable[Handle].Status == DOWNLOADING)
      {
        // open for writing
        WriteBuffer[HandleTable[Handle].WriteBufNo].BufIndex = (ULONG)(HandleTable[Handle].pFlash) & (SECTORSIZE - 1);
        memcpy(WriteBuffer[HandleTable[Handle].WriteBufNo].Buf, (const UBYTE *)((ULONG)(HandleTable[Handle].pFlash) & ~(SECTORSIZE - 1)), WriteBuffer[HandleTable[Handle].WriteBufNo].BufIndex );
      }
    }
  }
  return(Status);
}

UWORD     dLoaderTell(UBYTE Handle, ULONG* filePos)
{
  UWORD   Status;

  Status = dLoaderCheckHandleForReadWrite(Handle);
  if (0x8000 > Status)
  {
    Status = Handle;
    *filePos = HandleTable[Handle].ReadLength;
  }
  return(Status);
}

UWORD      dLoaderRead(UBYTE Handle, UBYTE *pBuffer, ULONG *pLength)
{
  UWORD   ByteCnt, Status;

  Status = dLoaderCheckHandle(Handle, BUSY);
  if (0x8000 > Status)
  {
    Status  = Handle;
    ByteCnt = 0;
    while (ByteCnt < *pLength)
    {
      if (HandleTable[Handle].DataLength <= HandleTable[Handle].ReadLength)
      {
        // if the file pointer (ReadLength) is >= file size then return EOF
        *pLength = ByteCnt;
        Status  |= ENDOFFILE;
      }
      else
      {
        // copy a byte at a time from pFlash to pBuffer
        *pBuffer = *(HandleTable[Handle].pFlash);
        pBuffer++;
        ByteCnt++;
        // move to next byte in the flash
        HandleTable[Handle].pFlash++;
        // update our file pointer
        HandleTable[Handle].ReadLength++;
        // if we reach a flash sector boundary then find the next sector pointer
        if (!((ULONG)(HandleTable[Handle].pFlash) & (SECTORSIZE-1)))
        {
          HandleTable[Handle].pFlash = dLoaderGetNextSectorPtr(Handle);
        }
      }
    }
  }
  return(Status);
}

UWORD      dLoaderDelete(UBYTE *pFile)
{
  UWORD   LStatus;
  ULONG   FileLength;
  ULONG   DataLength;
  UBYTE   Name[FILENAME_SIZE];

  LStatus = dLoaderFind(pFile, Name, &FileLength, &DataLength, (UBYTE)BUSY);

  if (!IS_LOADER_ERR(LStatus))
  {
    LStatus = dLoaderDeleteFilePtr((UBYTE)LStatus);
  }

  dLoaderCloseHandle(LStatus);

  return(LStatus);
}

UWORD     dLoaderFind(UBYTE *pFind, UBYTE *pFound, ULONG *pFileLength, ULONG *pDataLength, UBYTE Session)
{
  UWORD   Handle;

  Handle = dLoaderGetFreeHandle();
  if (Handle < 0x8000)
  {
    if (FILENAME_LENGTH < strlen((const char*)pFind))
    {
      Handle |= ILLEGALFILENAME;
    }
    else
    {
      HandleTable[Handle].FileIndex = 0xFFFF;
      HandleTable[Handle].Status    = Session;
      dLoaderInsertSearchStr((HandleTable[Handle].SearchStr), pFind, &(HandleTable[Handle].SearchType));
      Handle = dLoaderFindNext(Handle, pFound, pFileLength, pDataLength);
    }
  }

  return(Handle);
}

UWORD     dLoaderFindNext(UWORD Handle, UBYTE *pFound, ULONG *pFileLength, ULONG *pDataLength)
{
  UBYTE   Tmp;
  UWORD   ReturnVal;
  FILEHEADER  *pHeader;

  *pFileLength  = 0;
  ReturnVal     = Handle | FILENOTFOUND;


  for (Tmp = ((HandleTable[Handle].FileIndex) + 1); Tmp < MAX_FILES; Tmp++)
  {
    if (0xFFFFFFFF != FILEPTRTABLE[Tmp])
    {
      if (SUCCESS == dLoaderCheckName((UBYTE*)FILEPTRTABLE[Tmp], HandleTable[Handle].SearchStr, HandleTable[Handle].SearchType))
      {
        HandleTable[Handle].FileIndex = Tmp;
        Tmp = MAX_FILES;
        ReturnVal = Handle;
      }
    }
  }
  if (0x8000 > ReturnVal)
  {
    pHeader = (FILEHEADER *)FILEPTRTABLE[HandleTable[Handle].FileIndex];
    if (NULL != pFileLength)
    {
      *pFileLength = pHeader->FileSize;
    }
    if (NULL != pDataLength)
    {
      *pDataLength = pHeader->DataSize;
    }
    if (NULL != pFound)
    {
      dLoaderCopyFileName(pFound, (UBYTE *)pHeader->FileName);
    }
  }
  return(ReturnVal);
}


ULONG     dLoaderReturnFreeFlash(void)
{
  ULONG   SectorCnt, IndexPtr;
  UWORD   Sectors;


  Sectors  = 0;
  IndexPtr = (ULONG)0x01 << SECTORPOINTERUSERFLASH;  /* Offset in first index can be different from 0 */
  for(SectorCnt = SECTORINDEXUSERFLASH; SectorCnt <= ((NOOFSECTORS>>5)-1); SectorCnt++)
  {
    for( ; IndexPtr > 0; IndexPtr<<=1)
    {
      if (!(SectorTable[SectorCnt] & IndexPtr))
      {
        Sectors++;
      }
    }
    IndexPtr = 0x00000001;
  }

  FreeSectors = Sectors;
  return(dLoaderCalcFreeFileSpace(Sectors));
}

ULONG     dLoaderCalcFreeFileSpace(UWORD NosOfFreeSectors)
{
  UWORD   SectorCnt;
  ULONG   Space;
  ULONG   HeaderSpace;

  /* Calculate only if any sectors available */
  if (NosOfFreeSectors)
  {

    Space = (ULONG)NosOfFreeSectors << SECTORSIZESHIFT;

    /* (FreeSectors - 1) is beacuse the the first sector of a file do not         */
    /* require an entry in the sector table - it is pointed to by the filepointer */
    /* in the file pointer table*/
    SectorCnt = NosOfFreeSectors - 1;

    /* If more that one sector is used for the header the first filebody sector do not */
    /* require an entry in the sectortable - it is pointed to by the file startpointer */
    /* in the file header */
    if ((((SectorCnt<<1) + HEADERFIXEDSIZE) & (SECTORSIZE - 1)) < 4)
    {
      SectorCnt--;
    }

    HeaderSpace = (HEADERFIXEDSIZE + (SectorCnt << 1));
    if (HeaderSpace & 0x0003)
    {
      /* Header size is not a multiplum of 4 - now make it a mul 4 */
      HeaderSpace  += (0x0004 - (HeaderSpace & 0x0003));
    }
    Space -= HeaderSpace;
  }
  return(Space);
}


UWORD     dLoaderGetFilePtr(UBYTE *pFileName, UBYTE *pPtrToFile, ULONG *pFileLength)
{
  UWORD       RtnVal;
  UBYTE       FoundFile[16];
  FILEHEADER  *File;
  ULONG       DataLength;


  RtnVal = dLoaderFind(pFileName, FoundFile, pFileLength, &DataLength, (UBYTE)BUSY);
  if (0x8000 > RtnVal)
  {

    File = (FILEHEADER*) FILEPTRTABLE[HandleTable[RtnVal].FileIndex];
    if (LINEAR & File->FileType)
    {
      *((ULONG*)pPtrToFile) = File->FileStartAdr;
    }
    else
    {
      RtnVal |= NOTLINEARFILE;
    }
  }
  return(RtnVal);
}

UWORD     dLoaderAllocateHeader(UWORD Handle, ULONG *FileStartAdr, FILEHEADER *pHeader, UWORD HeaderByteSize, UWORD CompleteFileSectorSize)
{
  UWORD   Tmp;
  UWORD   SectorTableIndex;
  ULONG   SectorIndex;
  UWORD   HeaderSectorSize;
  UBYTE   EvenHeader;
  UWORD   FileBodySectorSize;
  UWORD   ErrorCode;

  HeaderSectorSize        = ((HeaderByteSize - 1) >> SECTORSIZESHIFT) + 1;
  FileBodySectorSize      = (((pHeader->FileSize - (SECTORSIZE - (HeaderByteSize & (SECTORSIZE - 1)))) - 1) >> SECTORSIZESHIFT) + 1;

  /* First allocate the file file header - this means the file name,    */
  /* the file start adress, and the sector table                        */

  /* SectorTableIndex indicates in the last word of a sector in which   */
  /* sector the sectortable continues                                   */
  SectorTableIndex = ((SECTORSIZE - HEADERFIXEDSIZE)>>1) - 1;

  /* Find first free sector - here there is a differende between linear or not*/
  ErrorCode = dLoaderFindFirstSector(pHeader->FileType, CompleteFileSectorSize, &Tmp);

  if (SUCCESS == ErrorCode)
  {
    *FileStartAdr = (ULONG)((ULONG)Tmp << SECTORSIZESHIFT) | FLASHOFFSET;
    HandleTable[Handle].FileDlPtr = *FileStartAdr;

    SectorIndex  = (Tmp >> 5);
    Tmp         &= 0x1F;

    SectorTable[SectorIndex]|= (0x01<<Tmp);

    /* Advance to next sector */
    Tmp++;

    /* if only one sector used for for file header */
    pHeader->FileStartAdr = (ULONG)(*FileStartAdr) + HeaderByteSize;

    /* if there is a sectortable it always starts right after the fixed header (Name + start + size)*/
    HandleTable[Handle].pSectorNo = (const UWORD *)(*FileStartAdr + HEADERFIXEDSIZE);

    /* First header has been allocated by find first function                   */
    HeaderSectorSize--;
    UWORD   TmpHSS = HeaderSectorSize;

    /* Next part is only executed when more than one sector is used */
    if (HeaderSectorSize)
    {
      UBYTE   ExitCode = FALSE;

      while ((FALSE == ExitCode) && (SectorIndex < (NOOFSECTORS/32)))
      {
        for(; ((Tmp < 32) && (ExitCode == FALSE)); Tmp++)
        {
          if (!(SectorTable[SectorIndex] & (0x01<<Tmp)))
          {
            /* Sector is free you can have this one */
            SectorTable[SectorIndex]       |= (0x01<<Tmp);
            pHeader->FileSectorTable[SectorTableIndex] = (SectorIndex << 5) + Tmp;
            SectorTableIndex += (SECTORSIZE/2);
            HeaderSectorSize--;
            if (0 == HeaderSectorSize)
            {
              pHeader->FileStartAdr = (((SectorIndex << 5) + Tmp) << SECTORSIZESHIFT) + (HeaderByteSize - (TmpHSS<<SECTORSIZESHIFT)) | FLASHOFFSET;
              ExitCode = TRUE;
            }
          }
        }
        if (FALSE == ExitCode)
        {
          SectorIndex++;
          Tmp = 0;
        }
      }
    }

    EvenHeader = FALSE;
    if (((HeaderByteSize & (SECTORSIZE - 1)) >= (SECTORSIZE - 2)) || (0 == (HeaderByteSize & (SECTORSIZE - 1))))
    {

      /* The header uses exact one or several sectors */
      /* meaning that the next sector do not go into  */
      /* the sectortable as it is pointed to by the   */
      /* FileStart pointer                            */
      EvenHeader = TRUE;
    }

    /* Now allocated the file body */
    SectorTableIndex =  0;
    while ((FileBodySectorSize > 0) && (SectorIndex < (NOOFSECTORS/32)))
    {
      for(; Tmp < 32; Tmp++)
      {
        if (!(SectorTable[SectorIndex] & (0x01<<Tmp)))
        {
          if (TRUE == EvenHeader)
          {
            /* This sector do not go into the sectortable  */
            /* it is pointed to by the filestart pointer   */
            SectorTable[SectorIndex] |= (0x01<<Tmp);
            pHeader->FileStartAdr = (((SectorIndex << 5) + Tmp) << SECTORSIZESHIFT) | FLASHOFFSET;
            EvenHeader = FALSE;
          }
          else
          {

            /* Sector is free you can have this one */
            SectorTable[SectorIndex] |= (0x01<<Tmp);
            if (((((SECTORSIZE - HEADERFIXEDSIZE)>>1)-1) == SectorTableIndex) || (((SectorTableIndex - ((SECTORSIZE - HEADERFIXEDSIZE)>>1)) & 0x7F) == 127))
            {
              SectorTableIndex++;
            }
            pHeader->FileSectorTable[SectorTableIndex] = (SectorIndex << 5) + Tmp;
            SectorTableIndex++;
            FileBodySectorSize--;
            if (0 == FileBodySectorSize)
            {
              Tmp             = 32;
              SectorIndex     = (NOOFSECTORS/32);
            }
          }
        }
      }
      SectorIndex++;
      Tmp = 0;
    }
  }
  else
  {
    Handle |= ErrorCode;
  }
  return(Handle);
}


UWORD     dLoaderFindFirstSector(UBYTE Type, UWORD SectorCount, UWORD *pSector)
{
  UWORD   CompleteSectorSize;
  UWORD   SectorIndex;
  UBYTE   Tmp;
  UWORD   SectorCnt;
  UWORD   ErrorCode;


  ErrorCode = SUCCESS;

  SectorIndex = SECTORINDEXUSERFLASH;
  Tmp         = SECTORPOINTERUSERFLASH;



  if (LINEAR & Type)
  {
    CompleteSectorSize  = SectorCount;
    ErrorCode           = NOLINEARSPACE;

    /* find linear adress space */
    SectorCnt = CompleteSectorSize;

    while ((SectorCnt  > 0) && (SectorIndex < (NOOFSECTORS>>5)))
    {
      if ((SectorTable[SectorIndex]) & ((ULONG)0x01<<Tmp))
      {
        SectorCnt = CompleteSectorSize;
      }
      else
      {
        SectorCnt--;
        if (0 == SectorCnt)
        {
          *pSector    = ((SectorIndex<<5) + Tmp) - CompleteSectorSize + 1;
          SectorIndex = (NOOFSECTORS>>5);
          ErrorCode   = SUCCESS;
        }
      }

      if (0x1F == Tmp)
      {
        SectorIndex++;
      }
      Tmp = (Tmp + 1) & 0x1F;
    }
  }
  else
  {
    ErrorCode = UNDEFINEDERROR;
    while(SectorIndex < (NOOFSECTORS>>5))
    {
      if (!((SectorTable[SectorIndex]) & ((ULONG)0x01<<Tmp)))
      {
        *pSector    = (SectorIndex<<5) + Tmp;
        SectorIndex = (NOOFSECTORS>>5);
        ErrorCode   = SUCCESS;
      }
      if (0x1F == Tmp)
      {
        SectorIndex++;
      }
      Tmp = (Tmp + 1) & 0x1F;
    }
  }
  return(ErrorCode);
}


UWORD     dLoaderFlashFileHeader(UWORD Handle, ULONG FileStartAdr, FILEHEADER *pHeader, UWORD HeaderByteSize)
{
  ULONG   *pBufPtr;
  ULONG   FlashPtr;
  UWORD   HeaderSectorSize;

  pBufPtr           = (ULONG*)pHeader;
  FlashPtr          = FileStartAdr;
  HeaderSectorSize  = (HeaderByteSize - 1)  >> SECTORSIZESHIFT;

  dLoaderWritePage(FlashPtr, SECTORSIZE, pBufPtr);

  while(HeaderSectorSize)
  {
    pBufPtr  += (SECTORSIZE>>2);
    FlashPtr  = (((*(pBufPtr - 1) & 0xFFFF0000) >> 16) << SECTORSIZESHIFT) | FLASHOFFSET;
    dLoaderWritePage(FlashPtr, SECTORSIZE, pBufPtr);
    HeaderSectorSize--;
  }

  /* Prepare for actual data download */
  memcpy(WriteBuffer[HandleTable[Handle].WriteBufNo].Buf, pBufPtr, SECTORSIZE);
  WriteBuffer[HandleTable[Handle].WriteBufNo].BufIndex = (UWORD)(pHeader->FileStartAdr) & (SECTORSIZE-1);
  HandleTable[Handle].pFlash = (UBYTE *)pHeader->FileStartAdr;

  return(Handle);
}


UWORD     dLoaderGetSectorNumber(ULONG Adr)
{
  UWORD SectorNo;

  SectorNo = (Adr & ~FLASHOFFSET)>>SECTORSIZESHIFT;

  return(SectorNo);
}


UWORD     dLoaderAllocateWriteBuffer(UWORD  Handle)
{
  UBYTE   Tmp;
  UWORD   ErrorCode;

  ErrorCode = NOWRITEBUFFERS;
  for (Tmp = 0; Tmp < MAX_WRITE_BUFFERS; Tmp++)
  {
    if (FREE == WriteBuffer[Tmp].Status)
    {
      WriteBuffer[Tmp].Status = BUSY;
      memset(WriteBuffer[Tmp].Buf, 0xFF, sizeof(WriteBuffer[Tmp].Buf));
      WriteBuffer[Tmp].BufIndex = 0;
      HandleTable[Handle].WriteBufNo = Tmp;
      ErrorCode = SUCCESS;
      Tmp = MAX_WRITE_BUFFERS;
    }
  }
  Handle |= ErrorCode;
  return(Handle);
}

UWORD     dLoaderCheckFiles(UBYTE Handle)
{
  UBYTE   Tmp;
  UBYTE   Index;
  UWORD   ErrorCode;

  ErrorCode = SUCCESS;
  Index = HandleTable[Handle].FileIndex;
  for (Tmp = 0; Tmp < MAX_HANDLES; Tmp++)
  {
    if (((BUSY == HandleTable[Tmp].Status) || (DOWNLOADING == HandleTable[Tmp].Status)) && (Index == HandleTable[Tmp].FileIndex) && (Tmp != Handle))
    {
      ErrorCode = FILEISBUSY;
    }
  }
  return(Handle | ErrorCode);
}


void     dLoaderCopyFileName(UBYTE *pDst, UBYTE *pSrc)
{
  UBYTE  Tmp;

  for(Tmp = 0; Tmp < FILENAME_SIZE; Tmp++, pDst++)
  {
    if ('\0' != *pSrc)
    {
      *pDst = *pSrc;
      pSrc++;
    }
    else
    {
      *pDst = '\0';
    }
  }
}


void      dLoaderCheckVersion(void)
{
  if (FILEPTRTABLE[FILEVERSIONINDEX] != FILEVERSION)
  {
    dLoaderDeleteAllFiles();
  }
}


UWORD     dLoaderOpenAppend(UBYTE *pFileName, ULONG *pAvailSize)
{
  UWORD       Handle;
  ULONG       FileSize, DataSize;
  UBYTE       Name[FILENAME_SIZE];
  FILEHEADER  *pHeader;

  *pAvailSize = 0;

  Handle = dLoaderFind(pFileName, Name, &FileSize, &DataSize, (UBYTE)BUSY);
  if (0x8000 > Handle)
  {

    /* Check for an append in progress for this file */
    if (0x8000 > dLoaderCheckDownload(pFileName))
    {

      /* File has bee found - check for then correct filetype (Datafile) */
      pHeader = (FILEHEADER *)FILEPTRTABLE[HandleTable[Handle].FileIndex];
      if (DATAFILE & pHeader->FileType)
      {
        if (FileSize > DataSize)
        {
          /* Append is possible */
          Handle = dLoaderAllocateWriteBuffer(Handle);
          if (Handle < 0x8000)
          {
            dLoaderSetFilePointer(Handle, DataSize, &(HandleTable[Handle].pFlash));
            WriteBuffer[HandleTable[Handle].WriteBufNo].BufIndex = (ULONG)(HandleTable[Handle].pFlash) & (SECTORSIZE - 1);
            memcpy(WriteBuffer[HandleTable[Handle].WriteBufNo].Buf, (const UBYTE *)((ULONG)(HandleTable[Handle].pFlash) & ~(SECTORSIZE - 1)), WriteBuffer[HandleTable[Handle].WriteBufNo].BufIndex );
            HandleTable[Handle].FileDlPtr  = FILEPTRTABLE[HandleTable[Handle].FileIndex];
            HandleTable[Handle].Status     = (UBYTE)DOWNLOADING;
            *pAvailSize                    = FileSize - DataSize;
            HandleTable[Handle].DataLength = *pAvailSize;
            HandleTable[Handle].ReadLength = DataSize;
            HandleTable[Handle].FileType   = pHeader->FileType;
          }
        }
        else
        {
          Handle |= FILEISFULL;
        }
      }
      else
      {
        Handle |= APPENDNOTPOSSIBLE;
      }
    }
    else
    {
      Handle |= FILEISBUSY;
    }
  }

  return(Handle);
}


UWORD     dLoaderSetFilePointer(UWORD Handle, ULONG BytePtr, const UBYTE **pData)
{
  ULONG       AdrOffset;
  const UBYTE *Adr;
  UWORD       SectorNo;
  UWORD       Tmp;
  FILEHEADER  *pHeader;


  pData = pData;
  pHeader = (FILEHEADER*)FILEPTRTABLE[HandleTable[Handle].FileIndex];
  HandleTable[Handle].pSectorNo = pHeader->FileSectorTable;

  /* Get the sector offset */
  AdrOffset = SECTORSIZE - ((pHeader->FileStartAdr) & (SECTORSIZE - 1));

  if (BytePtr > AdrOffset)
  {
    BytePtr  = BytePtr - AdrOffset;
    SectorNo = ((BytePtr >> SECTORSIZESHIFT) + 1);

    for (Tmp = 0; Tmp < SectorNo; Tmp++)
    {
      Adr = dLoaderGetNextSectorPtr(Handle);
      if (BytePtr > SECTORSIZE)
      {
        BytePtr -= SECTORSIZE;
      }
    }
    *pData = (const UBYTE *)((ULONG)Adr + BytePtr);
  }
  else
  {

    /* Pointer reside in the first sector of the file body */
    *pData = (const UBYTE *)((ULONG)(pHeader->FileStartAdr) + BytePtr);
  }
  return(Handle);
}

void      dLoaderCpyToLower(UBYTE *pDst, UBYTE *pSrc, UBYTE Length)
{
  UBYTE   Tmp;

  for(Tmp = 0; Tmp < Length; Tmp++)
  {
    pDst[Tmp] =(UBYTE)toupper((UWORD)pSrc[Tmp]);
  }

  /* The requried length has been copied - now fill with zeros */
  for(Tmp = Length; Tmp < FILENAME_SIZE; Tmp++)
  {
    pDst[Tmp] = '\0';
  }
}

UWORD     dLoaderCheckName(UBYTE *pName, UBYTE *pSearchStr, UBYTE SearchType)
{
  UBYTE   TmpName[FILENAME_SIZE];
  UWORD   RtnVal;

  RtnVal  = UNDEFINEDERROR;

  dLoaderCpyToLower(TmpName, pName, (UBYTE)FILENAME_SIZE);

  RtnVal = SUCCESS;
  switch (SearchType)
  {
    case FULLNAME:
    {
      if (0 != strcmp((const char*)TmpName, (const char *)pSearchStr))
      {
        RtnVal = UNDEFINEDERROR;
      }
    }
    break;
    case NAME:
    {
      if (0 != memcmp(TmpName, pSearchStr, strlen((const char *)pSearchStr)))
      {
        RtnVal = UNDEFINEDERROR;
      }
    }
    break;
    case EXTENTION:
    {
      if (0 == strstr((const char *)TmpName, (const char*)pSearchStr))
      {
        RtnVal = UNDEFINEDERROR;
      }
    }
    break;
    case WILDCARD:
    {
      RtnVal = SUCCESS;
    }
    break;
    default:
    {
    }
    break;
  }
  return(RtnVal);
}

void      dLoaderInsertSearchStr(UBYTE *pDst, UBYTE *pSrc, UBYTE *pSearchType)
{
  UBYTE  Tmp;

  *pSearchType = WILDCARD;
  if (0 != strstr((char const *)pSrc, "*.*"))
  {

    /* find all */
    strcpy ((PSZ)pDst, (PSZ)pSrc);
    *pSearchType = WILDCARD;
  }
  else
  {

    /* Using other wild cards? */
    Tmp =  strlen((char const *)pSrc);
    if (0 != strstr((PSZ)(pSrc), ".*"))
    {

      /* Extention wildcard */
      dLoaderCpyToLower(pDst, pSrc, (Tmp-1));
      *pSearchType = NAME;
    }
    else
    {
      if (0 != strstr((PSZ)(pSrc), "*."))
      {

        /* Filename wildcard */
        dLoaderCpyToLower(pDst, &pSrc[1], (UBYTE)4);
        *pSearchType = EXTENTION;
      }
      else
      {

        /* no wildcards used */
        dLoaderCpyToLower(pDst, pSrc, Tmp);
        *pSearchType = FULLNAME;
      }
    }
  }
}

UWORD     dLoaderCheckHandleForReadWrite(UWORD Handle)
{
  if (MAX_HANDLES > Handle)
  {
    if ((DOWNLOADING != HandleTable[(UBYTE)Handle].Status) && 
        (BUSY != HandleTable[(UBYTE)Handle].Status))
    {
      Handle |= ILLEGALHANDLE;
    }
  }
  else
  {
    Handle |= ILLEGALHANDLE;
  }
  return(Handle);
}

UWORD     dLoaderCheckHandle(UWORD Handle, UBYTE Operation)
{

  if (MAX_HANDLES > Handle)
  {
    if (Operation != HandleTable[(UBYTE)Handle].Status)
    {
      Handle |= ILLEGALHANDLE;
    }
  }
  else
  {
    Handle |= ILLEGALHANDLE;
  }
  return(Handle);
}

ULONG     dLoaderReturnFreeUserFlash(void)
{
  return(FreeUserFlash);
}

UWORD     dLoaderRenameFile(UBYTE Handle, UBYTE *pNewName)
{
  ULONG       SectorBuf[SECTORSIZE/4];
  ULONG       *pFile;
  UBYTE       Tmp;
  FILEHEADER  *pHeader;

  pFile = (ULONG *)FILEPTRTABLE[HandleTable[Handle].FileIndex];
  for (Tmp = 0; Tmp < (SECTORSIZE/4); Tmp++)
  {
    SectorBuf[Tmp] = pFile[Tmp];
  }

  pHeader = (FILEHEADER *) SectorBuf;

  dLoaderCopyFileName((pHeader->FileName), pNewName);
  dLoaderWritePage((ULONG)pFile, SECTORSIZE, SectorBuf);
  return(SUCCESS);
}


UWORD     dLoaderCheckDownload(UBYTE *pName)
{
  UBYTE   Tmp;
  UWORD   ErrorCode;

  ErrorCode = SUCCESS;
  for(Tmp = 0; Tmp < MAX_HANDLES; Tmp ++)
  {
    if (DOWNLOADING == HandleTable[Tmp].Status)
    {
      if (SUCCESS == dLoaderCheckName(pName, HandleTable[Tmp].SearchStr, FULLNAME))
      {
        Tmp = MAX_HANDLES;
        ErrorCode = FILEEXISTS;
      }
    }
  }
  return(ErrorCode);
}




UWORD     dLoaderCropDatafile(UBYTE Handle)
{
  UWORD             ReturnVal;
  ULONG             SectorBuffer[SECTORSIZE];
  UBYTE             FileIndex;

  /* Save the fileindex for use after the handle has been closed */
  FileIndex = HandleTable[Handle].FileIndex;

  ReturnVal = dLoaderCloseHandle(Handle);
  if (0x8000 > ReturnVal)
  {

    /* Successful close handle now try to crop the file if filesize and datasize differs */
    /* and File exists                                                                   */
    if (((FILEPTRTABLE[FileIndex]) != 0x00000000) && ((FILEPTRTABLE[FileIndex]) != 0xFFFFFFFF))
    {
      if (((FILEHEADER const *)(FILEPTRTABLE[FileIndex]))->FileSize != ((FILEHEADER const *)(FILEPTRTABLE[FileIndex]))->DataSize)
      {
        memcpy(SectorBuffer, (void const*)(FILEPTRTABLE[FileIndex]), SECTORSIZE);
        ((FILEHEADER*)SectorBuffer)->FileSize = ((FILEHEADER const *)(FILEPTRTABLE[FileIndex]))->DataSize;
        dLoaderWritePage((ULONG)(FILEPTRTABLE[HandleTable[Handle].FileIndex]), SECTORSIZE, SectorBuffer);

        /* Update sectortable and available flash size */
        dLoaderUpdateSectorTable();
      }
    }
  }
  return(ReturnVal);
}

void      dLoaderExit(void)
{
}
