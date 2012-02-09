//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: d_hispeed.r                                   $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_hisp $
//
// Platform        C
//

#ifdef    SAM7S256

#if       defined (PROTOTYPE_PCB_3) || (PROTOTYPE_PCB_4)

#define   HIGHSPEED_RX_PIN              AT91C_PIO_PA5
#define   HIGHSPEED_TX_PIN              AT91C_PIO_PA6
#define   HIGHSPEED_RTS_PIN             AT91C_PIO_PA7

#else


#endif

#define   PER_ID6_UART_0        0x40
#define   UART0_INQ             0x40
#define   BAUD_RATE			    921600L

#define   SIZE_OF_INBUF         128
#define   NO_OF_INBUFFERS       2
#define   SIZE_OF_OUTBUF        128
#define   NO_OF_DMA_OUTBUFFERS  1

static    UBYTE  InBuf[NO_OF_INBUFFERS][SIZE_OF_INBUF];
static    ULONG  InBufPtrs[NO_OF_INBUFFERS];
static    UBYTE  InBufInPtr;

static    UBYTE  OutDma[NO_OF_DMA_OUTBUFFERS][SIZE_OF_OUTBUF];
static    UBYTE  DmaBufPtr;
static    UBYTE  *pBuffer;

static    UBYTE  MsgIn;
static    UBYTE  InBufOutCnt;

#define   HIGHSPEEDInit                 {\
										  *AT91C_PIOA_PER   = HIGHSPEED_TX_PIN | HIGHSPEED_RTS_PIN | HIGHSPEED_RX_PIN;   /* Enable PIO on PA07, PA06 & PA05 */\
										  *AT91C_PIOA_PPUDR = HIGHSPEED_RX_PIN | HIGHSPEED_TX_PIN | HIGHSPEED_RTS_PIN;   /* Disable Pull-up resistor */\
                                          *AT91C_PIOA_OER   = HIGHSPEED_TX_PIN | HIGHSPEED_RTS_PIN | HIGHSPEED_RX_PIN;   /* PA07 & PA06 set to Output  */\
                                          *AT91C_PIOA_CODR	= HIGHSPEED_TX_PIN | HIGHSPEED_RTS_PIN | HIGHSPEED_RX_PIN;	 /* Set output low */\
                                        }

#define   HIGHSPEEDSetupUart(_spd, _baud, _mode, _umode) {\
                                          UBYTE Tmp;\
                                          InBufInPtr = 0;\
                                          for(Tmp = 0; Tmp < NO_OF_INBUFFERS; Tmp++)\
                                          {\
                                            InBufPtrs[Tmp]   = (ULONG)&(InBuf[Tmp][0]);\
                                          }\
                                          *AT91C_PMC_PCER = PER_ID6_UART_0;                        /* Enable PMC clock for UART 0 */\
                                          *AT91C_PIOA_PPUDR = HIGHSPEED_RX_PIN | HIGHSPEED_TX_PIN | HIGHSPEED_RTS_PIN; /* Disable Pull-up resistor */\
                                          *AT91C_PIOA_PDR = HIGHSPEED_TX_PIN | HIGHSPEED_RTS_PIN | HIGHSPEED_RX_PIN; /* Disable Per. A on PA5, PA6 & PA7 */\
                                          *AT91C_PIOA_ASR = HIGHSPEED_TX_PIN | HIGHSPEED_RTS_PIN | HIGHSPEED_RX_PIN;; /* Enable Per. A on PA5, PA6 & PA7 */\
                                          *AT91C_US0_CR   = AT91C_US_RSTSTA;                       /* Resets pins on UART0 */\
                                          *AT91C_US0_CR   = AT91C_US_STTTO;                        /* Start timeout functionality after 1 byte */\
                                          *AT91C_US0_RTOR = 2400;                                  /* Approxitely 20 mS,x times bit time with 115200 bit pr s */\
                                          *AT91C_US0_IDR  = AT91C_US_TIMEOUT;                      /* Disable interrupt on timeout */\
                                          *AT91C_AIC_IDCR = UART0_INQ;                             /* Disable UART0 interrupt */\
                                          *AT91C_AIC_ICCR = UART0_INQ;                             /* Clear interrupt register */\
                                          *AT91C_US0_MR   = (_umode);                              /* Set UART to RUN RS485 Mode*/\
                                          *AT91C_US0_MR  &= ~AT91C_US_SYNC;                        /* Set UART in asynchronous mode */\
                                          *AT91C_US0_MR  |= AT91C_US_CLKS_CLOCK;                   /* Clock setup MCK*/\
                                          *AT91C_US0_MR  |= AT91C_US_OVER;                         /* UART is using over sampling mode */\
                                          *AT91C_US0_MR  |= (_mode);                               /* default is 8n1 */\
                                          *AT91C_US0_BRGR = ((OSC/8/(_baud)) | (((OSC/8) - ((OSC/8/(_baud)) * (_baud))) / (((_baud) + 4)/8)) << 16);\
                                          *AT91C_US0_PTCR = (AT91C_PDC_RXTDIS | AT91C_PDC_TXTDIS); /* Disable of TX & RX with DMA */\
                                          *AT91C_US0_RCR  = 0;                                     /* Receive Counter Register */\
                                          *AT91C_US0_TCR  = 0;                                     /* Transmit Counter Register */\
                                          *AT91C_US0_RNPR = 0;\
                                          *AT91C_US0_TNPR = 0;\
                                          Tmp             = *AT91C_US0_RHR;\
                                          Tmp             = *AT91C_US0_CSR;\
                                          *AT91C_US0_RPR  = (unsigned int)&(InBuf[InBufInPtr][0]); /* Initialise receiver buffer using DMA */\
                                          *AT91C_US0_RCR  = SIZE_OF_INBUF;\
                                          *AT91C_US0_RNPR = (unsigned int)&(InBuf[(InBufInPtr + 1)%NO_OF_INBUFFERS][0]);\
                                          *AT91C_US0_RNCR = SIZE_OF_INBUF;\
                                          MsgIn           = 0;\
                                          InBufOutCnt     = 0;\
                                          *AT91C_US0_CR   = AT91C_US_RXEN | AT91C_US_TXEN;         /* Enable Tx & Rx on UART 0*/\
                                          *AT91C_US0_PTCR = (AT91C_PDC_RXTEN | AT91C_PDC_TXTEN);   /* Enable of TX & RX with DMA */\
                                        }

#define HIGHSPEEDInitReceiver(InputBuffer)\
                                        {\
										  UBYTE Tmp;\
                                          pBuffer     = InputBuffer;\
                                          *AT91C_US0_PTCR = (AT91C_PDC_RXTDIS | AT91C_PDC_TXTDIS); /* Disable of TX & RX with DMA */\
                                          *AT91C_US0_RCR  = 0;                                     /* Receive Counter Register */\
                                          *AT91C_US0_TCR  = 0;                                     /* Transmit Counter Register */\
                                          *AT91C_US0_RNPR = 0;\
                                          *AT91C_US0_TNPR = 0;\
                                          Tmp             = *AT91C_US0_RHR;\
                                          Tmp             = *AT91C_US0_CSR;\
                                          Tmp			  = Tmp;\
                                          *AT91C_US0_RPR  = (unsigned int)&(InBuf[InBufInPtr][0]);     /* Initialise receiver buffer using DMA */\
                                          *AT91C_US0_RCR  = SIZE_OF_INBUF;\
                                          *AT91C_US0_RNPR = (unsigned int)&(InBuf[(InBufInPtr + 1)%NO_OF_INBUFFERS][0]);\
                                          *AT91C_US0_RNCR = SIZE_OF_INBUF;\
                                          MsgIn           = 0;\
                                          InBufOutCnt     = 0;\
                                          *AT91C_US0_CR   = AT91C_US_RXEN | AT91C_US_TXEN;         /* Enable Tx & Rx on UART 0*/\
                                          *AT91C_US0_PTCR = (AT91C_PDC_RXTEN | AT91C_PDC_TXTEN);   /* Enable of TX & RX with DMA */\
                                        }


#define HIGHSPEEDReceivedData(pByteCnt)\
                                        {\
                                          UWORD InCnt;\
                                          *pByteCnt = 0;\
                                          InCnt = (SIZE_OF_INBUF - *AT91C_US0_RCR);\
                                          if (*AT91C_US0_RNCR == 0)\
                                          {\
                                            InCnt = SIZE_OF_INBUF;\
                                          }\
                                          InCnt -= InBufOutCnt; /* Remove already read bytes */\
                                          if(InCnt)\
                                          {\
                                            while(InCnt > 0)\
                                            {\
                                              pBuffer[MsgIn] = InBuf[InBufInPtr][InBufOutCnt];\
                                              MsgIn++;\
                                              InBufOutCnt++;\
                                              InCnt--;\
                                            }\
                                            *pByteCnt       = MsgIn;\
                                            MsgIn			= 0;\
                                          }\
                                          if ((*AT91C_US0_RNCR == 0) && (SIZE_OF_INBUF == InBufOutCnt))\
                                          {\
                                            InBufOutCnt     = 0;\
                                            *AT91C_US0_RNPR = (unsigned int)InBufPtrs[InBufInPtr];\
                                            *AT91C_US0_RNCR = SIZE_OF_INBUF;\
                                            InBufInPtr      = (InBufInPtr + 1) % NO_OF_INBUFFERS;\
                                          }\
                                        }

#define   AVAILOutBuf(Avail)            if (!(*AT91C_US0_TNCR))\
                                        {\
                                          Avail = SIZE_OF_OUTBUF;\
                                        }\
                                        else\
                                        {\
                                          Avail = 0;\
                                        }
                                        
#define HIGHSPEEDSendDmaData(OutputBuffer, BytesToSend)\
                                        {\
                                          UWORD Avail;\
                                          AVAILOutBuf(Avail);\
                                          if (BytesToSend < ((SWORD)Avail - 1))\
                                          {\
                                            memcpy((PSZ)&(OutDma[DmaBufPtr][0]), OutputBuffer, BytesToSend);\
                                            *AT91C_US0_TNPR = (unsigned int)&(OutDma[DmaBufPtr][0]);\
                                            *AT91C_US0_TNCR = BytesToSend;\
                                            DmaBufPtr = (DmaBufPtr + 1) % NO_OF_DMA_OUTBUFFERS;\
                                          }\
                                        }                     

#define HIGHSPEEDExit                   {\
                                          *AT91C_PMC_PCDR = PER_ID6_UART_0;    /* Disable PMC clock for UART 0*/\
                                          *AT91C_PIOA_PER   = HIGHSPEED_TX_PIN | HIGHSPEED_RTS_PIN | HIGHSPEED_RX_PIN;   /* Enable PIO on PA07, PA06 & PA05 */\
										  *AT91C_PIOA_PPUDR = HIGHSPEED_RX_PIN | HIGHSPEED_TX_PIN | HIGHSPEED_RTS_PIN;   /* Disable Pull-up resistor */\
                                          *AT91C_PIOA_OER   = HIGHSPEED_TX_PIN | HIGHSPEED_RTS_PIN | HIGHSPEED_RX_PIN;   /* PA07 & PA06 set to Output  */\
                                          *AT91C_PIOA_CODR	= HIGHSPEED_TX_PIN | HIGHSPEED_RTS_PIN | HIGHSPEED_RX_PIN;	 /* Set output low */\
                                        }


#define   BYTESToSend(Bts)            {\
                                        (Bts) = *AT91C_US0_TNCR;\
                                        (Bts) += *AT91C_US0_TCR;\
                                      }
                                        
#endif

#ifdef    PCWIN

#endif
