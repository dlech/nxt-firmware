//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: d_loader.r                                    $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_load $
//
// Platform        C
//

#ifdef    SAM7S256

#define   AT91C_MC_CORRECT_KEY          0x5A000000L

static    ULONG   SectorImage[SECTORSIZE>>2];

#define   LOADERInit


__ramfunc UWORD AT91F_Flash_Ready (void)
{
  UWORD status;
  status = 0;

  //* Wait the end of command
  while ((status & AT91C_MC_FRDY) != AT91C_MC_FRDY )
  {
    status = AT91C_BASE_MC->MC_FSR;
  }
  return status;
}

__ramfunc UWORD      dLoaderWritePage(ULONG Flash_Address, UWORD Size, ULONG *pBuf)
{
  //* set the Flash controller base address
  AT91PS_MC ptMC = AT91C_BASE_MC;
  unsigned int i, page, status;
  unsigned int * Flash;

  //* init flash pointer
  Flash = (unsigned int *) (Flash_Address | (unsigned int)AT91C_IFLASH);

  //* Get the Flash page number
  page = ((Flash_Address & ~(unsigned int)AT91C_IFLASH) >> SECTORSIZESHIFT);

  //* copy the new value
  if (Size & 0x0003)
  {
    Size = Size + (0x0004 - (Size & 0x0003));
  }
	for (i=0; (i < SECTORSIZE) & (Size > 0) ;i++, Flash++,pBuf++,Size-=4 )
  {
	  //* copy the flash to the write buffer ensuring code generation
	  *Flash=*pBuf;
	}

  //* Write the write page command
  ptMC->MC_FCR = AT91C_MC_CORRECT_KEY | AT91C_MC_FCMD_START_PROG | (AT91C_MC_PAGEN & (page <<8));

  //* Wait the end of command
  status = AT91F_Flash_Ready();

  //* Check the result
  if ( (status & ( AT91C_MC_PROGE | AT91C_MC_LOCKE ))!=0)
  {
    return FALSE;
  }
  return TRUE;

}

__ramfunc UWORD      dLoaderErasePage(ULONG Flash_Address)
{
  //* set the Flash controller base address
  AT91PS_MC ptMC = AT91C_BASE_MC;
  unsigned int i, page, status, Size;
  unsigned int * Flash;

  Size = SECTORSIZE;

  //* init flash pointer
  Flash = (unsigned int *) (Flash_Address | (unsigned int)AT91C_IFLASH);

  //* Get the Flash page number
  page = ((Flash_Address & ~(unsigned int)AT91C_IFLASH) >> SECTORSIZESHIFT);

  //* copy the new value
	for (i=0; (i < SECTORSIZE) & (Size > 0) ;i++, Flash++,Size-=4 )
  {
	  //* copy the flash to the write buffer ensuring code generation
	  *Flash=0xFFFFFFFF;
	}

  //* Write the write page command
  ptMC->MC_FCR = AT91C_MC_CORRECT_KEY | AT91C_MC_FCMD_START_PROG | (AT91C_MC_PAGEN & (page <<8));

  //* Wait the end of command
  status = AT91F_Flash_Ready();

  //* Check the result
  if ( (status & ( AT91C_MC_PROGE | AT91C_MC_LOCKE ))!=0)
  {
    return FALSE;
  }
  return TRUE;

}





#endif
