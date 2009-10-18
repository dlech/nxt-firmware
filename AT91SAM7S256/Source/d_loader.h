//
// Date init       14.12.2004
//
// Revision date   $Date:: 28-03-07 14:54                                    $
//
// Filename        $Workfile:: d_loader.h                                    $
//
// Version         $Revision:: 40                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_loader.h $
//
// Platform        C
//

#ifndef   D_LOADER
#define   D_LOADER

#define   STARTOFFILETABLE              (0x13FF00L)
#define   STARTOFUSERFLASH              (0x121400L)//(0x11F000L)
#define   SIZEOFUSERFLASH               (STARTOFFILETABLE - STARTOFUSERFLASH)

#define   SIZEOFFLASH                   262144L
#define   SECTORSIZE                    256L
#define   SECTORSIZESHIFT               8
#define   NOOFSECTORS                   (SIZEOFFLASH/SECTORSIZE)
#define   HEADERFIXEDSIZE               (FILENAME_SIZE + 4 + 4 + 4 + 2 + 2)
#define   FILENAME_SIZE                 (FILENAME_LENGTH + 1)

#define   FULLNAME                      1
#define   NAME                          2
#define   EXTENTION                     3
#define   WILDCARD                      4

/* Enum related to HandleTable Status */
enum
{
  FREE,
  BUSY,
  DOWNLOADING,
  SEARCHING,
  DLERROR
};

/* Enum related to HandleTable WriteBufNo */
enum
{
  FREEBUFNO = 0xFF
};


/* Constants related to filetype */
enum
{
  SYSTEMFILE = 0x01,
  DATAFILE   = 0x02,
  LINEAR     = 0x04,
  NONLINEAR  = 0x08
};

typedef   struct
{
  UBYTE   FileName[FILENAME_SIZE];
  ULONG   FileStartAdr;
  ULONG   FileSize;
  ULONG   DataSize;
  UWORD   CheckSum;
  UWORD   FileType;
  UWORD   FileSectorTable[SIZEOFUSERFLASH/SECTORSIZE];
}FILEHEADER;

void      dLoaderInit(void);
__ramfunc UWORD dLoaderWritePage(ULONG Flash_Address, UWORD Size, ULONG *pBuf);
UWORD     dLoaderInsertPtrTable(const UBYTE *pAdr, UWORD Handle);
UWORD     dLoaderCreateFileHeader(ULONG FileSize, UBYTE *pName, UBYTE LinearState, UBYTE FileType);
UWORD     dLoaderWriteData(UWORD Handle, UBYTE *pBuf, UWORD *pLen);
UWORD     dLoaderCloseHandle(UWORD Handle);
UWORD     dLoaderOpenRead(UBYTE *pFileName, ULONG *pLength);
UWORD     dLoaderRead(UBYTE Handle, UBYTE *pBuf, ULONG *pLength);
UWORD     dLoaderDelete(UBYTE *pFile);
UWORD     dLoaderFind(UBYTE *pFind, UBYTE *pFound, ULONG *pFileLength, ULONG *pDataLength, UBYTE Session);
UWORD     dLoaderFindNext(UWORD Handle, UBYTE *pFound, ULONG *pFileLength, ULONG *pDataLength);
UWORD     dLoaderDeleteFilePtr(UWORD Handle);
void      dLoaderDeleteAllFiles(void);
UWORD     dLoaderGetFilePtr(UBYTE *pFileName, UBYTE *pPtrToFile, ULONG *pFileLength);
void      dLoaderCopyFileName(UBYTE *pDst, UBYTE *pSrc);
UWORD     dLoaderOpenAppend(UBYTE *pFileName, ULONG *pAvailSize);
void      dLoaderCpyToLower(UBYTE *pDst, UBYTE *pSrc, UBYTE Length);
UWORD     dLoaderCheckName(UBYTE *pName, UBYTE *pSearchStr, UBYTE SearchType);
void      dLoaderInsertSearchStr(UBYTE *pDst, UBYTE *pSrc, UBYTE *pSearchType);
ULONG     dLoaderReturnFreeUserFlash(void);
UWORD     dLoaderRenameFile(UBYTE Handle, UBYTE *pNewName);
UWORD     dLoaderCheckFiles(UBYTE Handle);




void      dLoaderExit(void);

#endif
