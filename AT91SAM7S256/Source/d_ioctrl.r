//
// Date init       14.12.2004
//
// Revision date   $Date:: 7-12-07 14:09                                     $
//
// Filename        $Workfile:: d_ioctrl.r                                    $
//
// Version         $Revision:: 4                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_ioct $
//
// Platform        C
//


#ifdef    SAM7S256

extern    void     I2cHandler(void);

enum
{
  I2C_IDLE  =  1,
  I2C_ERROR =  2,
  I2C_TX    =  3,
  I2C_RX    =  4
};

#define   NO_TO_TX                      BYTES_TO_TX + 1
#define   NO_TO_RX                      BYTES_TO_RX + 1
#define   TIMEOUT                       (((OSC/16)/1000)*30) /* 100 ms timeout on I2C*/
#define   I2CCLK                        400000L
#define   TIME400KHZ                    (((OSC/16L)/(I2CCLK * 2)) + 1)
#define   CLDIV                         (((OSC/I2CCLK)/2)-3)
#define   DEVICE_ADR                    0x01


static    UBYTE                         *pIrq;
static    UBYTE volatile                Cnt;
static    UBYTE                         I2cStatus;
static    UBYTE                         I2cLastStatus;
static    UBYTE                         I2cInBuffer[NO_TO_RX];
static    UBYTE                         I2cOutBuffer[COPYRIGHTSTRINGLENGTH + 1];
static    UBYTE                         RxSum;
static    ULONG                         I2CTimerValue;


#define   DISABLEI2cIrqs                *AT91C_TWI_IDR  = 0x000001C7
#define   ISSUEStopCond                 *AT91C_TWI_CR   = AT91C_TWI_STOP
#define   INSERTPower(Power)            IoToAvr.Power   = Power
#define   INSERTPwm(Pwm)                IoToAvr.PwmFreq = Pwm
#define   SETTime                       I2CTimerValue   = ((*AT91C_PITC_PIIR) & AT91C_PITC_CPIV)


#define   DISABLETwi                    *AT91C_PIOA_PPUDR = (AT91C_PA4_TWCK | AT91C_PA3_TWD);/* no pull up             */\
                                        *AT91C_PIOA_MDER  = (AT91C_PA4_TWCK | AT91C_PA3_TWD);/* SCL + SDA is open drain*/\
                                        *AT91C_PIOA_SODR  = (AT91C_PA4_TWCK | AT91C_PA3_TWD);/* SCL + SDA is high      */\
                                        *AT91C_PIOA_OER   = (AT91C_PA4_TWCK | AT91C_PA3_TWD);/* SCL + SDA is output    */\
                                        *AT91C_PIOA_PER   = (AT91C_PA4_TWCK | AT91C_PA3_TWD);/* Disable peripheal      */\


#define   STARTIrqTx                    I2cStatus      = I2C_TX;\
                                        I2cLastStatus  = I2C_TX;\
                                        pIrq           = I2cOutBuffer;\
                                        *AT91C_TWI_CR  = AT91C_TWI_MSEN;\
                                        *AT91C_TWI_MMR = (AT91C_TWI_IADRSZ_NO | (DEVICE_ADR << 16)); /* no int. adr, write dir */\
                                        *AT91C_TWI_IER = 0x00000104;                                 /* Enable TX related irq */\
                                        *AT91C_TWI_THR = *pIrq


#define   WAITClk                       {\
                                          ULONG PitTmr;\
                                          PitTmr = (*AT91C_PITC_PIIR & AT91C_PITC_CPIV) + TIME400KHZ;\
                                          if (PitTmr >= (*AT91C_PITC_PIMR & AT91C_PITC_CPIV))\
                                          {\
                                            PitTmr -= (*AT91C_PITC_PIMR & AT91C_PITC_CPIV);\
                                          }\
                                          while ((*AT91C_PITC_PIIR & AT91C_PITC_CPIV) < PitTmr);\
                                        }


#define   RESETI2c                      {\
                                          UBYTE Tmp;\
                                          DISABLETwi;\
                                          Tmp = 0;\
                                          /* Clock minimum 9 times and both SCK and SDA should be high */\
                                          while((!(*AT91C_PIOA_PDSR & AT91C_PA3_TWD)) || (Tmp <= 9))\
                                          {\
                                            if ((*AT91C_PIOA_PDSR) & AT91C_PA4_TWCK)    /* Clk strectching?   */\
                                            {\
                                              *AT91C_PIOA_CODR = AT91C_PA4_TWCK;        /* SCL is low         */\
                                              WAITClk;\
                                              *AT91C_PIOA_SODR = AT91C_PA4_TWCK;        /* SCL is high        */\
                                              WAITClk;\
                                              Tmp++;\
                                            }\
                                          }\
                                          *AT91C_TWI_CR   = AT91C_TWI_MSDIS;\
                                          *AT91C_TWI_CR   = AT91C_TWI_SWRST;\
                                          *AT91C_PIOA_ASR = (AT91C_PA4_TWCK | AT91C_PA3_TWD); /* Sel. per. A     */\
                                          *AT91C_PIOA_PDR = (AT91C_PA4_TWCK | AT91C_PA3_TWD); /* Sel. per on pins*/\
                                        }


#define   IOCTRLInit                    *AT91C_AIC_IDCR   = (1L<<AT91C_ID_TWI);          /* Disable AIC irq    */\
                                        *AT91C_PMC_PCER   = (1L<<AT91C_ID_TWI);          /* Enable TWI Clock   */\
                                        DISABLEI2cIrqs;                                  /* Disable TWI irq    */\
                                        if (((*AT91C_AIC_ISR & 0x1F) == AT91C_ID_TWI))\
                                        {\
                                          *AT91C_AIC_EOICR = 1;\
                                        }\
                                        RESETI2c;\
                                        IoToAvr.Power     = 0;\
                                        *AT91C_AIC_ICCR   = (1L<<AT91C_ID_TWI);          /* Clear AIC irq      */\
                                        AT91C_AIC_SVR[AT91C_ID_TWI] = (unsigned int)I2cHandler;\
                                        AT91C_AIC_SMR[AT91C_ID_TWI] = ((AT91C_AIC_PRIOR_HIGHEST) | (AT91C_AIC_SRCTYPE_INT_EDGE_TRIGGERED));\
                                        *AT91C_AIC_IECR   = (1L<<AT91C_ID_TWI);          /* Enables AIC irq    */\
                                        *AT91C_TWI_CWGR   = (CLDIV | (CLDIV << 8));      /* 400KHz clock       */\
                                        UNLOCKTx


#define   IOCTRLExit                    DISABLEI2cIrqs;\
                                        *AT91C_AIC_IDCR   = (1L<<AT91C_ID_TWI);               /* Disable AIC irq  */\
                                        *AT91C_AIC_ICCR   = (1L<<AT91C_ID_TWI);               /* Clear AIC irq    */\
                                        *AT91C_PMC_PCDR   = (1L<<AT91C_ID_TWI);               /* Disable clock    */\
                                        DISABLETwi


#define   UNLOCKTx                      I2cOutBuffer[0] = 0xCC;                       /* CC is the Unlock cmd  */\
                                        memcpy(&I2cOutBuffer[1], (UBYTE*)COPYRIGHTSTRING, COPYRIGHTSTRINGLENGTH);\
                                        Cnt = COPYRIGHTSTRINGLENGTH + 1;              /* +1 is the 0xCC command*/\
                                        STARTIrqTx;\
                                        SETTime


#define   I2CTransfer                   if ((I2cStatus == I2C_IDLE) && (*AT91C_TWI_SR & AT91C_TWI_TXCOMP))\
                                        {\
                                          DISABLEI2cIrqs;\
                                          if (I2cLastStatus == I2C_TX)\
                                          {\
                                            RxSum          = 0;\
                                            I2cStatus      = I2C_RX;\
                                            I2cLastStatus  = I2C_RX;\
                                            pIrq           = I2cInBuffer;\
                                            Cnt            = NO_TO_RX;\
                                            *AT91C_TWI_CR  = AT91C_TWI_MSEN;\
                                            *AT91C_TWI_MMR = (AT91C_TWI_MREAD | AT91C_TWI_IADRSZ_NO | (DEVICE_ADR << 16));\
                                            *AT91C_TWI_CR  = AT91C_TWI_START;\
                                            *AT91C_TWI_IER = 0x00000102;\
                                          }\
                                          else\
                                          {\
                                            /* Now TX (last time was RX) */\
                                            UBYTE I2cTmp, Sum;\
                                            /* Copy rx'ed data bytes so they can be read by controllers   */\
                                            if (RxSum == 0xFF)\
                                            {\
                                              memcpy((UBYTE*)&IoFromAvr,I2cInBuffer,BYTES_TO_RX);\
                                            }\
                                            pIrq                = (UBYTE*)&IoToAvr;\
                                            for(I2cTmp = 0, Sum = 0; I2cTmp < BYTES_TO_TX; I2cTmp++, pIrq++)\
                                            {\
                                              I2cOutBuffer[I2cTmp] = *pIrq;\
                                              Sum += *pIrq;\
                                            }\
                                            I2cOutBuffer[I2cTmp] = ~Sum;\
                                            Cnt                  = NO_TO_TX;\
                                            STARTIrqTx;\
                                          }\
                                          SETTime;\
                                        }\
                                        else\
                                        {\
                                          if ((I2cStatus == I2C_ERROR) || (TIMEOUT < (((*AT91C_PITC_PIIR) - I2CTimerValue) & AT91C_PITC_CPIV)))\
                                          {\
                                            IOCTRLInit;\
                                          }\
                                        }



__ramfunc void I2cHandler(void)
{

  ULONG Tmp;
  Tmp = *AT91C_TWI_SR;
  if (Tmp & AT91C_TWI_RXRDY)
  {
    *pIrq = *AT91C_TWI_RHR;
    RxSum += *pIrq;
    if (1 == --Cnt)
    {
      ISSUEStopCond;
    }
    else
    {
      if (0 == Cnt)
      {
        I2cStatus = I2C_IDLE;
      }
    }
    pIrq++;
  }
  else
  {
    if (Tmp & AT91C_TWI_TXRDY)
    {
      if (Cnt--)
      {

        /* When both shift and THR reg is empty - stop is sent automatically */
        *AT91C_TWI_THR = *pIrq;
        pIrq++;
      }
      else
      {

        /* All bytes TX'ed - TXCOMP checked in I2CTransfer*/
        I2cStatus = I2C_IDLE;
      }
    }
  }

  /* NACK - Data byte has not been accepted by the reciever */
  if (Tmp & AT91C_TWI_NACK)
  {
    I2cStatus = I2C_ERROR;
  }
}


#endif


#ifdef    PCWIN


#endif
