//
// Programmer
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dkandlun                                        $
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: d_display.r                                   $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_disp $
//
// Platform        C
//

#ifdef    SAM7S256

// Display 128 x 64
// 1/65 duty, 1/9 bias
// VLCD 12.0V

// SPI interface
//
// PCB        LCD       ARM       PIO
// ------     -----     ----      -----
// CS_DIS     -CS1      PA10      NPCS2 (PB)
// DIS_A0     A0        PA12      PA12
// DIS_SCL    SCL       PA14      SPCK  (PA)
// DIS_SDA    SI        PA13      MOSI  (PA)


// CPOL = 0, NCPHA=0,

#define   BT_RESET_OUT      AT91C_PIO_PA11
#define   BT_RESET_IN       AT91C_PIO_PA29
#define   BT_MOSI_OUT       AT91C_PIO_PA13
#define   BT_MOSI_IN        AT91C_PIO_PA20
#define   BT_CLK_OUT        AT91C_PIO_PA14
#define   BT_CLK_IN         AT91C_PIO_PA28
#define   BT_CE_OUT         AT91C_PIO_PA31
#define   BT_CE_IN          AT91C_PIO_PA19
#define   BT_REA_OUT        AT91C_PIO_PA7
#define   BT_MISO_OUT       AT91C_PIO_PA6
#define   BT_MISO_IN        AT91C_PIO_PA12

#pragma optimize=s 9

__ramfunc void SpiBtIo(void)
{
  register ULONG Port;
  
  *AT91C_AIC_IDCR    = 0xFFFFFFFF;        /* Disable all interrupts   */

  *AT91C_PIOA_PER    = BT_RESET_OUT;      /* Enable pin RESET out     */
  *AT91C_PIOA_OER    = BT_RESET_OUT;      /* Set output               */ 
  *AT91C_PIOA_SODR   = BT_RESET_OUT;      /* Set high                 */

  *AT91C_PIOA_PER    = BT_MOSI_OUT;       /* Enable pin MOSI out      */
  *AT91C_PIOA_OER    = BT_MOSI_OUT;       /* Set output               */

  *AT91C_PIOA_PER    = BT_CLK_OUT;        /* Enable pin CLK out       */
  *AT91C_PIOA_OER    = BT_CLK_OUT;        /* Set output               */

  *AT91C_PIOA_PER    = BT_CE_OUT;         /* Enable pin CE out        */
  *AT91C_PIOA_OER    = BT_CE_OUT;         /* Set output               */

  *AT91C_PIOA_PER    = BT_REA_OUT;        /* Enable pin REA out       */
  *AT91C_PIOA_OER    = BT_REA_OUT;        /* Set output               */
  *AT91C_PIOA_SODR   = BT_REA_OUT;        /* Set high                 */

  *AT91C_PIOA_PER    = BT_MISO_OUT;       /* Enable pin MISO out      */
  *AT91C_PIOA_OER    = BT_MISO_OUT;       /* Set output               */

  *AT91C_PIOA_PER    = BT_RESET_IN;       /* Enable pin RESET in      */
  *AT91C_PIOA_ODR    = BT_RESET_IN;       /* Set input                */
  *AT91C_PIOA_IFDR   = BT_RESET_IN;       /* Disable filter           */
  *AT91C_PIOA_IDR    = BT_RESET_IN;       /* Disable interrupt        */
  *AT91C_PIOA_MDDR   = BT_RESET_IN;       /* Disable multidriver      */
  *AT91C_PIOA_PPUDR  = BT_RESET_IN;       /* Disable pullup           */

  *AT91C_PIOA_PER    = BT_MOSI_IN;        /* Enable pin MOSI in       */
  *AT91C_PIOA_ODR    = BT_MOSI_IN;        /* Set input                */
  *AT91C_PIOA_IFDR   = BT_MOSI_IN;        /* Disable filter           */
  *AT91C_PIOA_IDR    = BT_MOSI_IN;        /* Disable interrupt        */
  *AT91C_PIOA_MDDR   = BT_MOSI_IN;        /* Disable multidriver      */
  *AT91C_PIOA_PPUDR  = BT_MOSI_IN;        /* Disable pullup           */

  *AT91C_PIOA_PER    = BT_CLK_IN;         /* Enable pin CLK in        */
  *AT91C_PIOA_ODR    = BT_CLK_IN;         /* Set input                */
  *AT91C_PIOA_IFDR   = BT_CLK_IN;         /* Disable filter           */
  *AT91C_PIOA_IDR    = BT_CLK_IN;         /* Disable interrupt        */
  *AT91C_PIOA_MDDR   = BT_CLK_IN;         /* Disable multidriver      */
  *AT91C_PIOA_PPUDR  = BT_CLK_IN;         /* Disable pullup           */

  *AT91C_PIOA_PER    = BT_CE_IN;          /* Enable pin CE in         */
  *AT91C_PIOA_ODR    = BT_CE_IN;          /* Set input                */
  *AT91C_PIOA_IFDR   = BT_CE_IN;          /* Disable filter           */
  *AT91C_PIOA_IDR    = BT_CE_IN;          /* Disable interrupt        */
  *AT91C_PIOA_MDDR   = BT_CE_IN;          /* Disable multidriver      */
  *AT91C_PIOA_PPUDR  = BT_CE_IN;          /* Disable pullup           */

  *AT91C_PIOA_PER    = BT_MISO_IN;        /* Enable pin MISO in       */
  *AT91C_PIOA_ODR    = BT_MISO_IN;        /* Set input                */
  *AT91C_PIOA_IFDR   = BT_MISO_IN;        /* Disable filter           */
  *AT91C_PIOA_IDR    = BT_MISO_IN;        /* Disable interrupt        */
  *AT91C_PIOA_MDDR   = BT_MISO_IN;        /* Disable multidriver      */
  *AT91C_PIOA_PPUDR  = BT_MISO_IN;        /* Disable pullup           */

  while (1)
  {
    Port = *AT91C_PIOA_PDSR;
    if ((Port & BT_MISO_IN))
    {
      *AT91C_PIOA_SODR = BT_MISO_OUT;
    }
    else
    {
      *AT91C_PIOA_CODR = BT_MISO_OUT;
    }
    if ((Port & BT_MOSI_IN))
    {
      *AT91C_PIOA_SODR = BT_MOSI_OUT;
    }
    else
    {
      *AT91C_PIOA_CODR = BT_MOSI_OUT;
    }
    if ((Port & BT_CLK_IN))
    {
      *AT91C_PIOA_SODR = BT_CLK_OUT;
    }
    else
    {
      *AT91C_PIOA_CODR = BT_CLK_OUT;
    }
    if ((Port & BT_CE_IN))
    {
      *AT91C_PIOA_SODR = BT_CE_OUT;
    }
    else
    {
      *AT91C_PIOA_CODR = BT_CE_OUT;
    }
  }
  
}


void      BtIo(void)
{
  SpiBtIo();
}



#define   SPI_BITRATE                   2000000

#define   SPIA0High                     {\
                                          *AT91C_PIOA_SODR = AT91C_PIO_PA12;\
                                        }


#define   SPIA0Low                      {\
                                          *AT91C_PIOA_CODR = AT91C_PIO_PA12;\
                                        }


#define   SPIInit                       {\
                                          *AT91C_PMC_PCER             = (1L << AT91C_ID_SPI);       /* Enable MCK clock     */\
                                          *AT91C_PIOA_PER             = AT91C_PIO_PA12;             /* Enable A0 on PA12    */\
                                          *AT91C_PIOA_OER             = AT91C_PIO_PA12;\
                                          *AT91C_PIOA_CODR            = AT91C_PIO_PA12;\
                                          *AT91C_PIOA_PDR             = AT91C_PA14_SPCK;            /* Enable SPCK on PA14  */\
                                          *AT91C_PIOA_ASR             = AT91C_PA14_SPCK;\
                                          *AT91C_PIOA_ODR             = AT91C_PA14_SPCK;\
                                          *AT91C_PIOA_OWER            = AT91C_PA14_SPCK;\
                                          *AT91C_PIOA_MDDR            = AT91C_PA14_SPCK;\
                                          *AT91C_PIOA_PPUDR           = AT91C_PA14_SPCK;\
                                          *AT91C_PIOA_IFDR            = AT91C_PA14_SPCK;\
                                          *AT91C_PIOA_CODR            = AT91C_PA14_SPCK;\
                                          *AT91C_PIOA_IDR             = AT91C_PA14_SPCK;\
                                          *AT91C_PIOA_PDR             = AT91C_PA13_MOSI;            /* Enable mosi on PA13  */\
                                          *AT91C_PIOA_ASR             = AT91C_PA13_MOSI;\
                                          *AT91C_PIOA_ODR             = AT91C_PA13_MOSI;\
                                          *AT91C_PIOA_OWER            = AT91C_PA13_MOSI;\
                                          *AT91C_PIOA_MDDR            = AT91C_PA13_MOSI;\
                                          *AT91C_PIOA_PPUDR           = AT91C_PA13_MOSI;\
                                          *AT91C_PIOA_IFDR            = AT91C_PA13_MOSI;\
                                          *AT91C_PIOA_CODR            = AT91C_PA13_MOSI;\
                                          *AT91C_PIOA_IDR             = AT91C_PA13_MOSI;\
                                          *AT91C_PIOA_PDR             = AT91C_PA10_NPCS2;           /* Enable npcs0 on PA11  */\
                                          *AT91C_PIOA_BSR             = AT91C_PA10_NPCS2;\
                                          *AT91C_PIOA_ODR             = AT91C_PA10_NPCS2;\
                                          *AT91C_PIOA_OWER            = AT91C_PA10_NPCS2;\
                                          *AT91C_PIOA_MDDR            = AT91C_PA10_NPCS2;\
                                          *AT91C_PIOA_PPUDR           = AT91C_PA10_NPCS2;\
                                          *AT91C_PIOA_IFDR            = AT91C_PA10_NPCS2;\
                                          *AT91C_PIOA_CODR            = AT91C_PA10_NPCS2;\
                                          *AT91C_PIOA_IDR             = AT91C_PA10_NPCS2;\
                                          *AT91C_SPI_CR               = AT91C_SPI_SWRST;            /* Soft reset           */\
                                          *AT91C_SPI_CR               = AT91C_SPI_SPIEN;            /* Enable spi           */\
                                          *AT91C_SPI_MR               = AT91C_SPI_MSTR  | AT91C_SPI_MODFDIS | (0xB << 16);\
                                          AT91C_SPI_CSR[2]              = ((OSC / SPI_BITRATE) << 8) | AT91C_SPI_CPOL;\
                                        }


#define   SPIWrite(pString,Length)      {\
                                          *AT91C_SPI_TPR = (unsigned int)pString;\
                                          *AT91C_SPI_TCR = (unsigned int)Length;\
                                          *AT91C_SPI_PTCR = AT91C_PDC_TXTEN;\
                                        }



#define   CMD             0
#define   DAT             1
#define   DISP_LINES      8

#if       defined (PROTOTYPE_PCB_3) || (PROTOTYPE_PCB_4)

#define   ACTUAL_WIDTH                      100

UBYTE     DisplayInitString[] =
{
  0xEB,   // LCD bias setting = 1/9         0xEB
  0x2F,   // Power control    = internal    0x2F
  0xA4,   // All points not on              0xA4
  0xA6,   // Not inverse                    0xA6
  0x40,   // Start line = 0                 0x40
  0x81,   // Electronic volume              0x81
  0x5A,   //      -"-                       0x5F
  0xC4,   // LCD mapping                    0xC4
  0x27,   // Set temp comp.                 0x27-
  0x29,   // Panel loading                  0x28    0-1
  0xA0,   // Framerate                      0xA0-
  0x88,   // CA++                           0x88-
  0x23,   // Multiplex 1:65                 0x23
  0xAF    // Display on                     0xAF
};

#else

#define   ACTUAL_WIDTH                      128

UBYTE     DisplayInitString[] =
{
  0xA2,   // LCD bias setting = 1/9
  0x2F,   // Power control    = internal
  0xA4,   // All points not on
  0xA6,   // Not inverse
  0x40,   // Start line = 0
  0x81,   // Electronic volume
  0x3F,   //      -"-
  0xA0,   // LCD mapping
  0x27,   // Resistor ratio
  0xC8,   // Common output state selection
  0xF8,   // Booster ratio
  0x00,   //      -"-
  0xE3,   // nop
  0xAF    // Display on
};

#endif

UBYTE     DisplayLineString[DISP_LINES][3] =
{
  { 0xB0,0x10,0x00 },
  { 0xB1,0x10,0x00 },
  { 0xB2,0x10,0x00 },
  { 0xB3,0x10,0x00 },
  { 0xB4,0x10,0x00 },
  { 0xB5,0x10,0x00 },
  { 0xB6,0x10,0x00 },
  { 0xB7,0x10,0x00 }
};

UBYTE     DisplayWrite(UBYTE Type,UBYTE *pData,UWORD Length)
{
  UBYTE   Result = FALSE;

  if ((*AT91C_SPI_SR & AT91C_SPI_TXEMPTY))
  {
    if (Type)
    {
      SPIA0High;
    }
    else
    {
      SPIA0Low;
    }
    SPIWrite(pData,Length);
    Result = TRUE;
  }

  return (Result);
}

UBYTE     DisplayUpdate(UWORD Height,UWORD Width,UBYTE *pImage)
{
  static  UWORD State = 0;
  static  UWORD Line;

  if (State == 0)
  {
    if (DisplayWrite(CMD,(UBYTE*)DisplayInitString,sizeof(DisplayInitString)) == TRUE)
    {
      Line = 0;
      State++;
    }
  }
  else
  {
    if ((State & 1))
    {
      if (DisplayWrite(CMD,(UBYTE*)DisplayLineString[Line],3) == TRUE)
      {
        State++;
      }
    }
    else
    {
      if (DisplayWrite(DAT,(UBYTE*)&pImage[Line * Width],ACTUAL_WIDTH) == TRUE)
      {
        State++;
        if (++Line >= (Height / 8))
        {
          State = 0;
        }
      }
    }
  }

  return (State);
}


#if       defined (PROTOTYPE_PCB_3)

#define   DISPLAYInit                   {\
                                          TSTInit;\
                                          TSTOn;\
                                          SPIInit;\
                                        }
                                        
#else

#define   DISPLAYInit                   {\
                                          SPIInit;\
                                        }
                                        
#endif                                        

#define   DISPLAYOn                     {\
                                          DisplayInitString[6]  = 0x5A;\
                                          DisplayInitString[13] = 0xAF;\
                                        }

#define   DISPLAYOff                    {\
                                          DisplayInitString[6]  = 0x00;\
                                          DisplayInitString[13] = 0xAE;\
                                        }

#define   DISPLAYUpdate(H,W,I)          DisplayUpdate(H,W,I)

#define   DISPLAYExit

#endif

#ifdef    PCWIN

#endif
