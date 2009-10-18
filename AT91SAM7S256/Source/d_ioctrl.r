//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 9:50                                     $
//
// Filename        $Workfile:: d_ioctrl.r                                    $
//
// Version         $Revision:: 21                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_ioctrl.r $
//
// Platform        C
//


#ifdef    SAM7S256

#define   NO_TO_TX                      BYTES_TO_TX + 1
#define   NO_TO_RX                      BYTES_TO_RX + 1
#define   TIMEOUT                       2100



extern    void I2cHandler(void);

static    UBYTE     *pIrq;
static    UBYTE     Cnt;
static    UBYTE     NoToTx;
static    UBYTE     I2cStatus;
static    UBYTE     I2cInBuffer[NO_TO_RX];
static    UBYTE     I2cOutBuffer[NO_TO_TX];
static    UBYTE     RxSum;

#define   I2C_IDLE                      1
#define   I2C_ERROR                     2
#define   I2C_TX                        3
#define   I2C_RX                        4

#define   I2CClk                        400000L
#define   TIME400KHz                    (((OSC/16L)/(I2CClk * 2)) + 1)
#define   CLDIV                         (((OSC/I2CClk)/2)-3)

#define   DEVICE_ADR                    0x01
#define   DISABLEI2cIrqs                *AT91C_TWI_IDR = 0x000001C7
#define   ISSUEStopCond                 *AT91C_TWI_CR  = AT91C_TWI_STOP


#define   WAITClk                       {\
                                          ULONG PitTmr;\
                                          PitTmr = (*AT91C_PITC_PIIR & AT91C_PITC_CPIV) + TIME400KHz;\
                                          if (PitTmr >= (*AT91C_PITC_PIMR & AT91C_PITC_CPIV))\
                                          {\
                                            PitTmr -= (*AT91C_PITC_PIMR & AT91C_PITC_CPIV);\
                                          }\
                                          while ((*AT91C_PITC_PIIR & AT91C_PITC_CPIV) < PitTmr);\
                                        }



#define   RESETI2c                      {\
                                          UBYTE Tmp;\
                                          *AT91C_PMC_PCER  = (1L<<AT91C_ID_TWI);/* Enable TWI Clock        */\
                                          *AT91C_PIOA_MDER = AT91C_PA4_TWCK;    /* enable open drain on SCL*/\
                                          *AT91C_PIOA_SODR = AT91C_PA4_TWCK;    /* SCL is high             */\
                                          *AT91C_PIOA_OER  = AT91C_PA4_TWCK;    /* SCL is output           */\
                                          *AT91C_PIOA_ODR  = AT91C_PA3_TWD;     /* SDA is input            */\
                                          *AT91C_PIOA_PER  = (AT91C_PA4_TWCK | AT91C_PA3_TWD); /* Disable peripheal */\
                                          Tmp = 0;\
                                          /* Clock minimum 9 times and both SCK and SDA should be high */\
                                          while(((!(*AT91C_PIOA_PDSR & AT91C_PA3_TWD)) || (!(*AT91C_PIOA_PDSR & AT91C_PA4_TWCK))) || (Tmp <= 9))\
                                          {\
                                            *AT91C_PIOA_CODR = AT91C_PA4_TWCK; /* SCL is low         */\
                                            WAITClk;\
                                            *AT91C_PIOA_SODR = AT91C_PA4_TWCK; /* SCL is high        */\
                                            WAITClk;\
                                            Tmp++;\
                                          }\
                                          *AT91C_TWI_CR    =  AT91C_TWI_SWRST;\
                                          I2cStatus = I2C_IDLE;\
                                        }

#define   IOCTRLInit                    {\
                                          memset(I2cInBuffer, 0x00, sizeof(I2cInBuffer));\
                                          *AT91C_AIC_IDCR   = (1L<<AT91C_ID_TWI);            /* Disable AIC irq    */\
                                          DISABLEI2cIrqs;                                    /* Disable TWI irq    */\
                                          RESETI2c;\
                                          IoToAvr.Power     = 0;\
                                          *AT91C_AIC_ICCR   = (1L<<AT91C_ID_TWI);            /* Clear AIC irq      */\
                                           AT91C_AIC_SVR[AT91C_ID_TWI] = (unsigned int)I2cHandler;\
                                           AT91C_AIC_SMR[AT91C_ID_TWI] = ((AT91C_AIC_PRIOR_HIGHEST) | (AT91C_AIC_SRCTYPE_INT_EDGE_TRIGGERED));\
                                          *AT91C_AIC_IECR   = (1L<<AT91C_ID_TWI);               /* Enables AIC irq */\
                                          *AT91C_PIOA_ASR   = (AT91C_PA4_TWCK | AT91C_PA3_TWD); /* Sel. per. A     */\
                                          *AT91C_PIOA_PDR   = (AT91C_PA4_TWCK | AT91C_PA3_TWD); /* Sel. per on pins*/\
                                          *AT91C_PIOA_MDER  = (AT91C_PA4_TWCK | AT91C_PA3_TWD); /* Open drain      */\
                                          *AT91C_PIOA_PPUDR = (AT91C_PA4_TWCK | AT91C_PA3_TWD); /* no pull up      */\
                                          *AT91C_TWI_CWGR   = (CLDIV | (CLDIV << 8));           /* 400KHz clock    */\
                                          NoToTx            = 0;\
                                        }


#define   IOCTRLExit                    DISABLEI2cIrqs;\
                                        *AT91C_AIC_IDCR   = (1L<<AT91C_ID_TWI);               /* Disable AIC irq  */\
                                        *AT91C_AIC_ICCR   = (1L<<AT91C_ID_TWI);               /* Clear AIC irq    */\
                                        *AT91C_PMC_PCDR   = (1L<<AT91C_ID_TWI);               /* Disable clock    */\
                                        *AT91C_PIOA_MDER  = (AT91C_PA4_TWCK | AT91C_PA3_TWD); /* Open drain       */\
                                        *AT91C_PIOA_PPUDR = (AT91C_PA4_TWCK | AT91C_PA3_TWD); /* no pull up       */\
                                        *AT91C_PIOA_PER   = (AT91C_PA4_TWCK | AT91C_PA3_TWD); /* Disable peripheal*/

#define   INSERTPower(Power)            IoToAvr.Power     = Power
#define   INSERTPwm(Pwm)                IoToAvr.PwmFreq   = Pwm

const     UBYTE CopyrightStr[] =        {"\xCC"COPYRIGHTSTRING};



#define   UNLOCKTx                      if (I2cStatus == I2C_IDLE)\
                                        {\
                                          I2cStatus           = I2C_TX;\
                                          DISABLEI2cIrqs;\
                                          pIrq                = (UBYTE*)CopyrightStr;\
                                          Cnt                 = 0;\
                                          NoToTx              = COPYRIGHTSTRINGLENGTH + 1; /* +1 is the 0xCC command */\
                                          *AT91C_AIC_ICCR     = (1L<<AT91C_ID_TWI);\
                                          *AT91C_TWI_MMR      = (AT91C_TWI_IADRSZ_NO | (DEVICE_ADR << 16)); /* no internal adr, write dir */\
                                          *AT91C_TWI_CR       = AT91C_TWI_MSEN | AT91C_TWI_START;\
                                          *AT91C_TWI_IER      = 0x000001C4;  /* Enable TX related irq */\
                                        }\
                                        else\
                                        {\
                                          if ((I2cStatus == I2C_ERROR) || ((I2cStatus == I2C_TX)))\
                                          {\
                                            IOCTRLInit;\
                                          }\
                                        }


#define   FULLDataTx                    if (I2cStatus == I2C_IDLE)\
                                        {\
                                          UBYTE I2cTmp, Sum;\
                                          pIrq                = (UBYTE*)&IoToAvr;\
                                          for(I2cTmp = 0, Sum = 0; I2cTmp < BYTES_TO_TX; I2cTmp++, pIrq++)\
                                          {\
                                            I2cOutBuffer[I2cTmp] = *pIrq;\
                                            Sum += *pIrq;\
                                          }\
                                          I2cOutBuffer[I2cTmp] = ~Sum;\
                                          I2cStatus = I2C_TX;\
                                          DISABLEI2cIrqs;\
                                          pIrq                = I2cOutBuffer;\
                                          Cnt                 = 0;\
                                          NoToTx              = NO_TO_TX;\
                                          *AT91C_AIC_ICCR     = (1L<<AT91C_ID_TWI);\
                                          *AT91C_TWI_MMR      = (AT91C_TWI_IADRSZ_NO | (DEVICE_ADR << 16)); /* no internal adr, write dir */\
                                          *AT91C_TWI_CR       = AT91C_TWI_MSEN | AT91C_TWI_START;\
                                          *AT91C_TWI_IER      = 0x000001C4;  /* Enable TX related irq */\
                                        }\
                                        else\
                                        {\
                                          if ((I2cStatus == I2C_ERROR) || ((I2cStatus == I2C_TX)))\
                                          {\
                                            IOCTRLInit;\
                                          }\
                                        }


#define   FULLDataRx                    {\
                                          if (I2cStatus == I2C_IDLE)\
                                          {\
                                            ULONG Tmp;\
                                            RxSum = 0;\
                                            I2cStatus = I2C_RX;\
                                            DISABLEI2cIrqs;\
                                            pIrq              = I2cInBuffer;\
                                            Cnt               = 0;\
                                            /* Reset error flags */\
                                            Tmp               = *AT91C_TWI_SR;\
                                            Tmp               = *AT91C_TWI_RHR;\
                                            Tmp               = Tmp;        /* Suppress warning */\
                                            *AT91C_TWI_MMR    = (AT91C_TWI_MREAD | AT91C_TWI_IADRSZ_NO | (DEVICE_ADR << 16));\
                                            *AT91C_TWI_CR     = AT91C_TWI_START | AT91C_TWI_MSEN;\
                                            Tmp               = *AT91C_TWI_SR;\
                                            *AT91C_TWI_IER    = 0x000001C2;\
                                          }\
                                          else\
                                          {\
                                            if ((I2cStatus == I2C_ERROR) || (I2cStatus == I2C_RX))\
                                            {\
                                              IOCTRLInit;\
                                            }\
                                          }\
                                        }

static    ULONG                         I2CTimerValue;
#define   CHECKTime(B)                  if (TIMEOUT < ((((*AT91C_PITC_PIIR) & AT91C_PITC_CPIV) - I2CTimerValue) & AT91C_PITC_CPIV))\
                                        {\
                                          B = TRUE;\
                                        }\
                                        else\
                                        {\
                                          B = FALSE;\
                                        }


#define   SETTime                       I2CTimerValue  = ((*AT91C_PITC_PIIR) & AT91C_PITC_CPIV)


__ramfunc void I2cHandler(void)
{

  ULONG Tmp;
  Tmp = *AT91C_TWI_SR;

  if (Tmp & AT91C_TWI_RXRDY)
  {
    Cnt++;
    if (Cnt < (NO_TO_RX - 1))
    {
      *pIrq = *AT91C_TWI_RHR;
      RxSum += *pIrq;
    }
    else
    {
      if (Cnt == (NO_TO_RX - 1))
      {

        /* Issue stop cond. (NACK) to indicate read last byte */
        ISSUEStopCond;
        *pIrq = *AT91C_TWI_RHR;
        RxSum += *pIrq;
      }
      else
      {
        if (Cnt == NO_TO_RX)
        {

          /* Now read last byte */
          *pIrq     = *AT91C_TWI_RHR;
          I2cStatus = I2C_IDLE;
          RxSum     = ~RxSum;
          if (RxSum == *pIrq)
          {
            UBYTE I2cIrqTmp;
            for (I2cIrqTmp = 0, pIrq = (UBYTE*)&IoFromAvr; I2cIrqTmp < BYTES_TO_RX; I2cIrqTmp++, pIrq++)
            {
              *pIrq = I2cInBuffer[I2cIrqTmp];
            }
          }
        }
      }
    }
    pIrq++;
  }

  else 
  {
    if (Tmp & AT91C_TWI_TXRDY)
    {
      if (Cnt < NoToTx)
      {

        /* When both shift and THR reg is empty - stop is sent automatically */
        *AT91C_TWI_CR  = AT91C_TWI_MSEN | AT91C_TWI_START;
        if (Cnt == (NoToTx - 1))
        {
          *AT91C_TWI_CR = AT91C_TWI_STOP;
        }
        *AT91C_TWI_THR = *pIrq;
        Cnt++;
        pIrq++;
      }
      else
      {
        I2cStatus = I2C_IDLE;
        DISABLEI2cIrqs;
      }
    }
  }

  /* Overrun error - byte received while rx buffer is full */
  if (Tmp & AT91C_TWI_OVRE)
  {
    I2cStatus = I2C_ERROR;
    ISSUEStopCond;
    DISABLEI2cIrqs;
  }

  /* Underrun error - trying to load invalid data to the shift register */
  if (Tmp & AT91C_TWI_UNRE)
  {
    I2cStatus = I2C_ERROR;
    DISABLEI2cIrqs;
  }

  /* NACK - Data byte has not been accepted by the reciever */
  if (Tmp & AT91C_TWI_NACK)
  {
    I2cStatus = I2C_ERROR;
    DISABLEI2cIrqs;
    IOCTRLInit;
  }
}


#endif


#ifdef    PCWIN


#endif
