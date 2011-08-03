//
// Date init       14.12.2004
//
// Revision date   $Date:: 19-02-09 18:51                                    $
//
// Filename        $Workfile:: d_lowspeed.r                                  $
//
// Version         $Revision:: 4                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_lows $
//
// Platform        C
//

#ifdef    SAM7S256

#if       defined (PROTOTYPE_PCB_3) || (PROTOTYPE_PCB_4)

#define   CHANNEL_ONE_CLK				AT91C_PIO_PA23 /* PA23 is Clk */
#define   CHANNEL_ONE_DATA				AT91C_PIO_PA18 /* PA18 is Data */

#define   CHANNEL_TWO_CLK				AT91C_PIO_PA28 /* PA28 is Clk */
#define   CHANNEL_TWO_DATA				AT91C_PIO_PA19 /* PA19 is Data */

#define   CHANNEL_THREE_CLK				AT91C_PIO_PA29 /* PA29 is Clk */
#define   CHANNEL_THREE_DATA			AT91C_PIO_PA20 /* PA20 is Data */

#define   CHANNEL_FOUR_CLK				AT91C_PIO_PA30 /* PA30 is Clk */
#define   CHANNEL_FOUR_DATA				AT91C_PIO_PA2 /* PA2 is Data */

#else

#define   CHANNEL_ONE_CLK				AT91C_PIO_PA28 /* PA28 is Clk */
#define   CHANNEL_ONE_DATA				AT91C_PIO_PA20 /* PA20 is Data */

#endif

typedef   struct
{
  UWORD MaskBit;
  UBYTE ChannelState;
  UBYTE TxState;
  UBYTE RxState;
  UBYTE ReStartState;
  UBYTE TxByteCnt;
  UBYTE RxByteCnt;
  UBYTE *pComOutBuffer;
  UBYTE *pComInBuffer;
  UBYTE AckStatus;
  UBYTE RxBitCnt;
  UBYTE ReStartBit;
  UBYTE ComDeviceAddress;
  UBYTE RxWaitCnt;
  UBYTE ClkStatus;
}LOWSPEEDPARAMETERS;

static LOWSPEEDPARAMETERS LowSpeedData[4];

ULONG DATA_PINS[4] = {CHANNEL_ONE_DATA, CHANNEL_TWO_DATA, CHANNEL_THREE_DATA, CHANNEL_FOUR_DATA};
ULONG CLK_PINS[4] = {CHANNEL_ONE_CLK, CHANNEL_TWO_CLK, CHANNEL_THREE_CLK, CHANNEL_FOUR_CLK};
const ULONG CLK_OR_DATA_PINS[4] = {CHANNEL_ONE_CLK | CHANNEL_ONE_DATA, 
                                   CHANNEL_TWO_CLK | CHANNEL_TWO_DATA, 
                                   CHANNEL_THREE_CLK | CHANNEL_THREE_DATA, 
                                   CHANNEL_FOUR_CLK | CHANNEL_FOUR_DATA};

#define   LOWSPEED_CHANNEL1	 0
#define   LOWSPEED_CHANNEL2	 1
#define   LOWSPEED_CHANNEL3	 2
#define   LOWSPEED_CHANNEL4	 3
#define   NO_OF_LOWSPEED_COM_CHANNEL	4

#define   MASK_BIT_8		 0x80

#define   PIO_INQ            0x04

//Used for variable ChannelState
#define   LOWSPEED_IDLE		         0x00
#define   LOWSPEED_TX_STOP_BIT		 0x01
#define   LOWSPEED_TRANSMITTING      0x02
#define   LOWSPEED_RECEIVING		 0x04
#define   LOWSPEED_TEST_WAIT_STATE   0x08
#define   LOWSPEED_RESTART_CONDITION 0x10
#define   LOWSPEED_WAIT_BEFORE_RX    0x20

//Used for variable TxState
#define   TX_IDLE					 0x00
#define   TX_DATA_MORE_DATA			 0x01
#define   TX_DATA_CLK_HIGH           0x02
#define   TX_EVALUATE_ACK_CLK_HIGH   0x03
#define   TX_DATA_READ_ACK_CLK_LOW   0x04
#define   TX_DATA_CLK_LOW            0x05
#define   TX_ACK_EVALUATED_CLK_LOW   0x06

//Used for variable RxState
#define   RX_IDLE					 0x00
#define   RX_START_BIT_CLK_HIGH		 0x01
#define   RX_DATA_CLK_HIGH           0x02
#define   RX_ACK_TX_CLK_HIGH         0x03
#define   RX_DATA_CLK_LOW			 0x04
#define   RX_DONE_OR_NOT_CLK_LOW     0x05

//Used for variable ReStart
#define   RESTART_STATE_IDLE		 0x00
#define   RESTART_STATE_ONE			 0x01
#define   RESTART_STATE_TWO			 0x02
#define   RESTART_STATE_THREE   	 0x03
#define   RESTART_STATE_FOUR		 0x04
#define   RESTART_STATE_FIVE		 0x05
#define   RESTART_STATE_SIX 		 0x06
#define   RESTART_STATE_SEVEN		 0x07

#define   LOWSpeedTxInit                {\
                                          LowSpeedData[LOWSPEED_CHANNEL1].ChannelState = 0;\
                                          LowSpeedData[LOWSPEED_CHANNEL2].ChannelState = 0;\
                                          LowSpeedData[LOWSPEED_CHANNEL3].ChannelState = 0;\
                                          LowSpeedData[LOWSPEED_CHANNEL4].ChannelState = 0;\
                                        }

#define   LOWSpeedTimerInit             {\
                                          *AT91C_PMC_PCER       = 0x400;					/* Enable clock for PWM, PID10*/\
										  *AT91C_PWMC_MR	    = 0x01;						/* CLKA is output from prescaler */\
										  *AT91C_PWMC_MR	   |= 0x600;					/* Prescaler MCK divided with 64 */\
										  *AT91C_PWMC_CH0_CMR   = 0x06;						/* Channel 0 uses MCK divided by 64 */\
										  *AT91C_PWMC_CH0_CMR  &= 0xFFFFFEFF;				/* Left alignment on periode */\
										  *AT91C_PWMC_CH0_CPRDR = 0x20;						/* Set to 39 => 52uSecondes interrupt */\
										  *AT91C_PWMC_IDR	    = AT91C_PWMC_CHID0;			/* Disable interrupt for PWM output channel 0 */\
										  *AT91C_AIC_IDCR       = 0x400;					/* Disable AIC intterupt on ID10 PWM */\
                                           AT91C_AIC_SVR[10]    = (unsigned int)LowSpeedPwmIrqHandler;\
                                           AT91C_AIC_SMR[10]    = 0x01;						/* Enable trigger on level */\
                                          *AT91C_AIC_ICCR       = 0x400;					/* Clear interrupt register PID10*/\
										  *AT91C_PWMC_IER		= AT91C_PWMC_CHID0;			/* Enable interrupt for PWM output channel 0 */\
										  *AT91C_AIC_IECR       = 0x400;					/* Enable interrupt from PWM */\
										}

#define   LOWSpeedExit

#define   ENABLEDebugOutput				{\
                                          *AT91C_PIOA_PER   = AT91C_PIO_PA29; /* Enable PIO on PA029 */\
                                          *AT91C_PIOA_OER   = AT91C_PIO_PA29; /* PA029 set to Output  */\
                                          *AT91C_PIOA_CODR	= 0x20000000;\
                                        }

#define   SETDebugOutputHigh			*AT91C_PIOA_SODR	= 0x20000000

#define   SETDebugOutputLow				*AT91C_PIOA_CODR	= 0x20000000


#define	  SETClkComOneHigh		*AT91C_PIOA_SODR = CHANNEL_ONE_CLK

#define	  SETClkComOneLow		*AT91C_PIOA_CODR = CHANNEL_ONE_CLK

#define   GetClkComOnePinLevel		*AT91C_PIOA_PDSR & CHANNEL_ONE_CLK

#define	  SETClkComTwoHigh		*AT91C_PIOA_SODR = CHANNEL_TWO_CLK

#define	  SETClkComTwoLow		*AT91C_PIOA_CODR = CHANNEL_TWO_CLK

#define   GetClkComTwoPinLevel		*AT91C_PIOA_PDSR & CHANNEL_TWO_CLK

#define	  SETClkComThreeHigh		*AT91C_PIOA_SODR = CHANNEL_THREE_CLK

#define	  SETClkComThreeLow		*AT91C_PIOA_CODR = CHANNEL_THREE_CLK

#define   GetClkComThreePinLevel	*AT91C_PIOA_PDSR & CHANNEL_THREE_CLK

#define	  SETClkComFourHigh		*AT91C_PIOA_SODR = CHANNEL_FOUR_CLK

#define	  SETClkComFourLow		*AT91C_PIOA_CODR = CHANNEL_FOUR_CLK

#define   GetClkComFourPinLevel		*AT91C_PIOA_PDSR & CHANNEL_FOUR_CLK


#define	  SETDataComOneHigh		*AT91C_PIOA_SODR = CHANNEL_ONE_DATA

#define   SETDataComOneLow		*AT91C_PIOA_CODR = CHANNEL_ONE_DATA

#define   GetDataComOnePinLevel		*AT91C_PIOA_PDSR & CHANNEL_ONE_DATA

#define   GETDataComOnePinDirection	*AT91C_PIOA_OSR  & CHANNEL_ONE_DATA

#define	  SETDataComTwoHigh		*AT91C_PIOA_SODR = CHANNEL_TWO_DATA

#define   SETDataComTwoLow		*AT91C_PIOA_CODR = CHANNEL_TWO_DATA

#define   GetDataComTwoPinLevel		*AT91C_PIOA_PDSR & CHANNEL_TWO_DATA

#define   GETDataComTwoPinDirection	*AT91C_PIOA_OSR  & CHANNEL_TWO_DATA

#define	  SETDataComThreeHigh		*AT91C_PIOA_SODR = CHANNEL_THREE_DATA

#define   SETDataComThreeLow		*AT91C_PIOA_CODR = CHANNEL_THREE_DATA

#define   GetDataComThreePinLevel	*AT91C_PIOA_PDSR & CHANNEL_THREE_DATA

#define   GETDataComThreePinDirection	*AT91C_PIOA_OSR  & CHANNEL_THREE_DATA

#define	  SETDataComFourHigh		*AT91C_PIOA_SODR = CHANNEL_FOUR_DATA

#define   SETDataComFourLow		*AT91C_PIOA_CODR = CHANNEL_FOUR_DATA

#define   GetDataComFourPinLevel	*AT91C_PIOA_PDSR & CHANNEL_FOUR_DATA

#define   GETDataComFourPinDirection	*AT91C_PIOA_OSR  & CHANNEL_FOUR_DATA

#define   SETDataComOneToInput          *AT91C_PIOA_ODR  = CHANNEL_ONE_DATA;

#define   SETDataComOneToOutput		*AT91C_PIOA_OER  = CHANNEL_ONE_DATA;

#define   SETDataComTwoToInput          *AT91C_PIOA_ODR  = CHANNEL_TWO_DATA;

#define   SETDataComTwoToOutput		*AT91C_PIOA_OER  = CHANNEL_TWO_DATA;

#define   SETDataComThreeToInput        *AT91C_PIOA_ODR  = CHANNEL_THREE_DATA;

#define   SETDataComThreeToOutput	*AT91C_PIOA_OER  = CHANNEL_THREE_DATA;

#define   SETDataComFourToInput         *AT91C_PIOA_ODR  = CHANNEL_FOUR_DATA;

#define   SETDataComFourToOutput	*AT91C_PIOA_OER  = CHANNEL_FOUR_DATA;

#define   DISABLEPullupDataComOne	*AT91C_PIOA_PPUDR = CHANNEL_ONE_DATA;

#define   DISABLEPullupClkComOne	*AT91C_PIOA_PPUDR = CHANNEL_ONE_CLK;

#define   DISABLEPullupDataComTwo	*AT91C_PIOA_PPUDR = CHANNEL_TWO_DATA;

#define   DISABLEPullupClkComTwo	*AT91C_PIOA_PPUDR = CHANNEL_TWO_CLK;

#define   DISABLEPullupDataComThree	*AT91C_PIOA_PPUDR = CHANNEL_THREE_DATA;

#define   DISABLEPullupClkComThree	*AT91C_PIOA_PPUDR = CHANNEL_THREE_CLK;

#define   DISABLEPullupDataComFour	*AT91C_PIOA_PPUDR = CHANNEL_FOUR_DATA;

#define   DISABLEPullupClkComFour	*AT91C_PIOA_PPUDR = CHANNEL_FOUR_CLK;

#define   ENABLEPullupDataComOne	*AT91C_PIOA_PPUER = CHANNEL_ONE_DATA;

#define   ENABLEPullupClkComOne 	*AT91C_PIOA_PPUER = CHANNEL_ONE_CLK;

#define   ENABLEPullupDataComTwo	*AT91C_PIOA_PPUER = CHANNEL_TWO_DATA;

#define   ENABLEPullupClkComTwo 	*AT91C_PIOA_PPUER = CHANNEL_TWO_CLK;

#define   ENABLEPullupDataComThree	*AT91C_PIOA_PPUER = CHANNEL_THREE_DATA;

#define   ENABLEPullupClkComThree	*AT91C_PIOA_PPUER = CHANNEL_THREE_CLK;

#define   ENABLEPullupDataComFour	*AT91C_PIOA_PPUER = CHANNEL_FOUR_DATA;

#define   ENABLEPullupClkComFour	*AT91C_PIOA_PPUER = CHANNEL_FOUR_CLK;

#define SETClkLow(ChannelNr)  {\
  *AT91C_PIOA_CODR = CLK_PINS[ChannelNr];\
  LowSpeedData[ChannelNr].ClkStatus = 0;\
                                         }

#define SETClkHigh(ChannelNr)  {\
  *AT91C_PIOA_SODR = CLK_PINS[ChannelNr];\
  LowSpeedData[ChannelNr].ClkStatus = 1;\
                                         }

#define SETDataLow(ChannelNr)  {\
  *AT91C_PIOA_CODR = DATA_PINS[ChannelNr];\
}

#define SETDataHigh(ChannelNr)  {\
  *AT91C_PIOA_SODR = DATA_PINS[ChannelNr];\
}

#define SETDataToInput(ChannelNr)  {\
  *AT91C_PIOA_ODR  = DATA_PINS[ChannelNr];\
}

#define SETDataToOutput(ChannelNr)  {\
  *AT91C_PIOA_OER  = DATA_PINS[ChannelNr];\
}

#define GetClkPinLevel(ChannelNr)	(*AT91C_PIOA_PDSR & CLK_PINS[ChannelNr])
#define GetDataPinLevel(ChannelNr)	(*AT91C_PIOA_PDSR & DATA_PINS[ChannelNr])
#define GETDataPinDirection(ChannelNr)	(*AT91C_PIOA_OSR  & DATA_PINS[ChannelNr])

#define   ENABLEPWMTimerForLowCom  {\
  *AT91C_PWMC_ENA = AT91C_PWMC_CHID0;	/* Enable PWM output channel 0 */\
										}

#define    DISABLEPWMTimerForLowCom  {\
  *AT91C_PWMC_DIS = AT91C_PWMC_CHID0;	/* Disable PWM output channel 0 */\
										}
										
#define    OLD_DISABLEPWMTimerForLowCom	{\
										  *AT91C_PWMC_DIS	 = AT91C_PWMC_CHID0;			/* Disable PWM output channel 0 */\
                                          *AT91C_PWMC_IDR	 = AT91C_PWMC_CHID0;			/* Disable interrupt from PWM output channel 0 */\
										  *AT91C_AIC_IDCR    = 0x400;						/* Disable Irq from PID10 */\
                                          *AT91C_AIC_ICCR    = 0x400;						/* Clear interrupt register PID10*/\
                                          *AT91C_PMC_PCDR    = 0x400;						/* Disable clock for PWM, PID10*/\
										}

__ramfunc void LowSpeedPwmIrqHandler(void)
{
  ULONG TestVar;
  ULONG PinStatus;
  UBYTE ChannelNr;

  TestVar = *AT91C_PWMC_ISR;
  TestVar = TestVar;
  PinStatus = *AT91C_PIOA_PDSR;

  for (ChannelNr = 0; ChannelNr < NO_OF_LOWSPEED_COM_CHANNEL; ChannelNr++)
  {
    if (((LowSpeedData[ChannelNr].ClkStatus == 1) && (PinStatus & CLK_PINS[ChannelNr])) || (((LowSpeedData[ChannelNr].ClkStatus == 0) && (!(PinStatus & CLK_PINS[ChannelNr])))))
    {
      switch(LowSpeedData[ChannelNr].ChannelState)
      {
        case LOWSPEED_IDLE:
        {
        }
        break;

        case LOWSPEED_TX_STOP_BIT:
        {
          SETDataHigh(ChannelNr);
          LowSpeedData[ChannelNr].ChannelState = LOWSPEED_IDLE;                                     //Now we have send a STOP sequence, disable this channel
        }
        break;

        case LOWSPEED_TRANSMITTING:
        {
          switch(LowSpeedData[ChannelNr].TxState)
          {
            case TX_DATA_MORE_DATA:
            {
              PinStatus |= CLK_PINS[ChannelNr];
              LowSpeedData[ChannelNr].TxState = TX_DATA_CLK_HIGH;
            }
            break;

            case TX_DATA_CLK_HIGH:
            {
              SETClkLow(ChannelNr);
              for (int a=0; a<60; a++);  // let the line settle
              if (LowSpeedData[ChannelNr].MaskBit == 0)     				                           //Is Byte Done, then we need a ack from receiver
            {
              SETDataToInput(ChannelNr);                                                             //Set datapin to input
              LowSpeedData[ChannelNr].TxState = TX_DATA_READ_ACK_CLK_LOW;
            }
            else
            {
              if (*LowSpeedData[ChannelNr].pComOutBuffer & LowSpeedData[ChannelNr].MaskBit)          //Setup data pin in relation to the data
              {
	            SETDataHigh(ChannelNr);       						                               //Set data output high
              }
	          else
              {
	            SETDataLow(ChannelNr);                                                               //Set data output low
              }
              LowSpeedData[ChannelNr].TxState = TX_DATA_CLK_LOW;
            }
          }
          break;

          case TX_EVALUATE_ACK_CLK_HIGH:
          {
            SETClkLow(ChannelNr);
            if (LowSpeedData[ChannelNr].AckStatus == 1)	
	        {
	          LowSpeedData[ChannelNr].TxByteCnt--;
	          if (LowSpeedData[ChannelNr].TxByteCnt > 0)	        		    			   //Here initialise to send next byte
	          {
	            LowSpeedData[ChannelNr].MaskBit = MASK_BIT_8;
	            LowSpeedData[ChannelNr].pComOutBuffer++;
	          }
	          LowSpeedData[ChannelNr].TxState = TX_ACK_EVALUATED_CLK_LOW;                    //Received ack, now make a stop sequence or send next byte
	        }
	        else
	        { //Data communication error !
	          LowSpeedData[ChannelNr].TxByteCnt = 0;
	          SETClkHigh(ChannelNr);
              LowSpeedData[ChannelNr].ChannelState = LOWSPEED_TX_STOP_BIT;                     //Received ack, now make a stop sequence or send next byte.
	        }
          }
          break;

          case TX_DATA_READ_ACK_CLK_LOW:
          {
            if (!(PinStatus & DATA_PINS[ChannelNr]))
	        {
	          LowSpeedData[ChannelNr].AckStatus = 1;										//Read ack signal from receiver
	        }	        	        	
	        SETDataToOutput(ChannelNr);
	        SETDataLow(ChannelNr);
	        LowSpeedData[ChannelNr].TxState = TX_EVALUATE_ACK_CLK_HIGH;
	        SETClkHigh(ChannelNr);
          }
          break;

          case TX_DATA_CLK_LOW:
          {
            LowSpeedData[ChannelNr].MaskBit = LowSpeedData[ChannelNr].MaskBit >> 1;			//Get ready for the next bit which should be clk out next time
            SETClkHigh(ChannelNr);															//Clk goes high = The reciever reads the data
            LowSpeedData[ChannelNr].TxState = TX_DATA_CLK_HIGH;
          }
          break;

          case TX_ACK_EVALUATED_CLK_LOW:
          {
            if (LowSpeedData[ChannelNr].MaskBit != 0)
            {
              LowSpeedData[ChannelNr].TxState = TX_DATA_MORE_DATA;
            }
            else
            {
              if (LowSpeedData[ChannelNr].ReStartBit != 0)
              {
                LowSpeedData[ChannelNr].ChannelState = LOWSPEED_RESTART_CONDITION;
                LowSpeedData[ChannelNr].ReStartState = RESTART_STATE_ONE;
                SETDataLow(ChannelNr);
                SETClkHigh(ChannelNr);                                                         //Clk goes high = The reciever reads the data
              }
              else
              {
                if (LowSpeedData[ChannelNr].RxByteCnt != 0)
                {
                  LowSpeedData[ChannelNr].ChannelState = LOWSPEED_WAIT_BEFORE_RX;
                }
                else
                {
                  LowSpeedData[ChannelNr].ChannelState = LOWSPEED_TX_STOP_BIT;
                  SETClkHigh(ChannelNr);                                                         //Clk goes high = The reciever reads the data
                }
              }
              LowSpeedData[ChannelNr].TxState = TX_IDLE;
            }
          }
          break;
        }
      }
      break;

      case LOWSPEED_RESTART_CONDITION:
      {
        switch(LowSpeedData[ChannelNr].ReStartState)
        {
          case RESTART_STATE_ONE:
          {
            LowSpeedData[ChannelNr].ReStartState = RESTART_STATE_TWO;
          }
          break;

          case RESTART_STATE_TWO:
          {
            SETDataHigh(ChannelNr);
            LowSpeedData[ChannelNr].ReStartState = RESTART_STATE_THREE;
          }
          break;

          case RESTART_STATE_THREE:
          {
            SETClkLow(ChannelNr);
            LowSpeedData[ChannelNr].ReStartState = RESTART_STATE_FOUR;
          }
          break;

          case RESTART_STATE_FOUR:
          {
            SETClkHigh(ChannelNr);
            LowSpeedData[ChannelNr].ReStartState = RESTART_STATE_FIVE;
          }
          break;

          case RESTART_STATE_FIVE:
          {
            SETDataLow(ChannelNr);
            LowSpeedData[ChannelNr].ReStartState = RESTART_STATE_SIX;
          }
          break;

          case RESTART_STATE_SIX:
          {
            LowSpeedData[ChannelNr].ReStartState = RESTART_STATE_SEVEN;
          }
          break;

          case RESTART_STATE_SEVEN:
          {
            SETClkLow(ChannelNr);
            LowSpeedData[ChannelNr].ReStartState = RESTART_STATE_IDLE;
            LowSpeedData[ChannelNr].ReStartBit = 0;
            LowSpeedData[ChannelNr].pComOutBuffer = &LowSpeedData[ChannelNr].ComDeviceAddress;
            *LowSpeedData[ChannelNr].pComOutBuffer += 0x01;
			LowSpeedData[ChannelNr].ChannelState = LOWSPEED_TRANSMITTING;
			LowSpeedData[ChannelNr].MaskBit = MASK_BIT_8;
			LowSpeedData[ChannelNr].TxByteCnt = 0x01;
			LowSpeedData[ChannelNr].TxState = TX_DATA_CLK_HIGH;
			LowSpeedData[ChannelNr].AckStatus = 0;
          }
          break;
        }
      }
      break;

      case LOWSPEED_WAIT_BEFORE_RX:
      {
        LowSpeedData[ChannelNr].RxWaitCnt++;
        if (LowSpeedData[ChannelNr].RxWaitCnt > 5)
        {
          LowSpeedData[ChannelNr].ChannelState = LOWSPEED_RECEIVING;
          SETDataToInput(ChannelNr);
        }
      }
      break;

      case LOWSPEED_RECEIVING:
      {
        switch(LowSpeedData[ChannelNr].RxState)
        {
          case RX_START_BIT_CLK_HIGH:
          {
            SETClkLow(ChannelNr);
            LowSpeedData[ChannelNr].RxState = RX_DATA_CLK_LOW;
          }
          break;

          case RX_DATA_CLK_HIGH:
          {
            LowSpeedData[ChannelNr].RxBitCnt++;
            if(PinStatus & DATA_PINS[ChannelNr])
            {
              *LowSpeedData[ChannelNr].pComInBuffer |= 0x01;
            }
            SETClkLow(ChannelNr);
            if (LowSpeedData[ChannelNr].RxBitCnt < 8)
            {
              *LowSpeedData[ChannelNr].pComInBuffer = *LowSpeedData[ChannelNr].pComInBuffer << 1;
            }
            else
            {
              if (LowSpeedData[ChannelNr].RxByteCnt > 1)
              {
                SETDataToOutput(ChannelNr);
                SETDataLow(ChannelNr);
              }
            }
            LowSpeedData[ChannelNr].RxState = RX_DATA_CLK_LOW;
          }
          break;

          case RX_ACK_TX_CLK_HIGH:
          {
            SETClkLow(ChannelNr);
            SETDataToInput(ChannelNr);
            LowSpeedData[ChannelNr].pComInBuffer++;
		    LowSpeedData[ChannelNr].RxByteCnt--;
		    LowSpeedData[ChannelNr].RxBitCnt = 0;
	        LowSpeedData[ChannelNr].RxState = RX_DONE_OR_NOT_CLK_LOW;
          }
          break;

          case RX_DATA_CLK_LOW:
          {
            SETClkHigh(ChannelNr);
            if (LowSpeedData[ChannelNr].RxBitCnt == 8)
            {
              LowSpeedData[ChannelNr].RxState = RX_ACK_TX_CLK_HIGH;
            }
            else
            {
              LowSpeedData[ChannelNr].RxState = RX_DATA_CLK_HIGH;
            }
          }
          break;

          case RX_DONE_OR_NOT_CLK_LOW:
          {
            if (LowSpeedData[ChannelNr].RxByteCnt == 0)
            {
              LowSpeedData[ChannelNr].ChannelState = LOWSPEED_IDLE;
              LowSpeedData[ChannelNr].RxState = RX_IDLE;
              SETClkHigh(ChannelNr);
            }
            else
            {
              LowSpeedData[ChannelNr].RxState = RX_START_BIT_CLK_HIGH;
            }
          }
          break;
        }
      }
      break;

      default:
      break;
      }
    }
    else
    {

      if (LOWSPEED_IDLE != LowSpeedData[ChannelNr].ChannelState)
      {
        //Data communication error !
        LowSpeedData[ChannelNr].TxByteCnt = 0;
        SETClkHigh(ChannelNr);
        LowSpeedData[ChannelNr].ChannelState = LOWSPEED_TX_STOP_BIT;
      }
    }
  }
}


#define ENABLETxPins(ChannelNumber)  {\
  *AT91C_PIOA_PER   = CLK_OR_DATA_PINS[ChannelNumber]; /* Enable PIO on PA20 & PA28 */\
  *AT91C_PIOA_PPUDR = CLK_OR_DATA_PINS[ChannelNumber]; /* Disable Pull-up resistor  */\
  *AT91C_PIOA_ODR   = CLK_OR_DATA_PINS[ChannelNumber]; /* PA20 & PA28 set to Input  */\
}

#define TxData(ChannelNumber, Status, DataOutBuffer, NumberOfByte) {\
  if ((GetDataPinLevel(ChannelNumber) && GetClkPinLevel(ChannelNumber)) && (LowSpeedData[ChannelNumber].ChannelState == LOWSPEED_IDLE))\
  {\
    *AT91C_PIOA_PER   = CLK_OR_DATA_PINS[ChannelNumber]; /* Enable PIO on PA20 & PA28 */\
    *AT91C_PIOA_OER   = CLK_OR_DATA_PINS[ChannelNumber]; /* PA20 & PA28 set to Output  */\
    *AT91C_PIOA_PPUDR = CLK_OR_DATA_PINS[ChannelNumber]; /* Disable Pull-up resistor  */\
    SETClkHigh(ChannelNumber);\
    SETDataLow(ChannelNumber);\
    LowSpeedData[ChannelNumber].ClkStatus = 1;\
    LowSpeedData[ChannelNumber].pComOutBuffer = DataOutBuffer;\
    LowSpeedData[ChannelNumber].ComDeviceAddress = *LowSpeedData[ChannelNumber].pComOutBuffer;\
    LowSpeedData[ChannelNumber].MaskBit = MASK_BIT_8;\
    LowSpeedData[ChannelNumber].TxByteCnt = NumberOfByte;\
    LowSpeedData[ChannelNumber].TxState = TX_DATA_CLK_HIGH;\
    LowSpeedData[ChannelNumber].AckStatus = 0;\
    LowSpeedData[ChannelNumber].ChannelState = LOWSPEED_TRANSMITTING;\
    Status = 1;\
  }\
  else\
  {\
    Status = 0;\
  }\
}

#define RxData(ChannelNumber, DataInBuffer, RxBytes, NoRestart)  {\
  LowSpeedData[ChannelNumber].pComInBuffer = DataInBuffer;\
  LowSpeedData[ChannelNumber].RxBitCnt = 0;\
  LowSpeedData[ChannelNumber].RxByteCnt = RxBytes;\
  LowSpeedData[ChannelNumber].RxState = RX_DATA_CLK_LOW;\
  LowSpeedData[ChannelNumber].ReStartBit = (1 - (NoRestart & (1<<ChannelNumber)));\
  LowSpeedData[ChannelNumber].RxWaitCnt = 0;\
											         }
											
											
#define STATUSTxCom(ChannelNumber, Status)     {\
											     if (LowSpeedData[ChannelNumber].ChannelState != 0)\
											     {\
											       if ((LowSpeedData[ChannelNumber].TxByteCnt == 0) && (LowSpeedData[ChannelNumber].ChannelState != LOWSPEED_RESTART_CONDITION))\
											       {\
											         if (LowSpeedData[ChannelNumber].MaskBit == 0)\
											         {\
											           if (LowSpeedData[ChannelNumber].AckStatus == 1)\
											           {\
													     Status = 0x01;  /* TX SUCCESS  */\
											           }\
											           else\
											           {\
                                                                                                     Status = 0xFF; /* TX ERROR */\
											           }\
											         }\
											         else\
											         {\
											           Status = 0;\
											         }\
											       }\
											       else\
											       {\
											         Status = 0;\
											       }\
											     }\
											     else\
											     {\
											       if (LowSpeedData[ChannelNumber].RxByteCnt == 0)\
											       {\
											         if (LowSpeedData[ChannelNumber].AckStatus == 1)\
											         {\
													   Status = 0x01;  /* TX SUCCESS  */\
											         }\
											         else\
											         {\
											           Status = 0xFF; /* TX ERROR */\
											         }\
											       }\
											       else\
											       {\
											         Status = 0xFF; /* TX ERROR */\
											       }\
											     }\
											   }
											
#define STATUSRxCom(ChannelNumber, Status)     {\
                                                 if (LowSpeedData[ChannelNumber].ChannelState == LOWSPEED_IDLE)\
											     {\
											       if (LowSpeedData[ChannelNumber].RxByteCnt == 0)\
											       {\
											         Status = 0x01;  /* RX SUCCESS  */\
											       }\
											       else\
											       {\
											         Status = 0xFF; /* RX ERROR */\
											       }\
											     }\
											     else\
											     {\
											       Status = 0;\
											     }\
                                               }

typedef enum {
  I2C_DISABLED = 0,
  I2C_IDLE,
  I2C_ACTIVEIDLE,
  I2C_COMPLETE,
  I2C_BEGIN,
  I2C_NEWRESTART,
  I2C_NEWSTART,
  I2C_NEWREAD,
  I2C_START1,
  I2C_START2,
  I2C_DELAY,
  I2C_RXDATA1,
  I2C_RXDATA2,
  I2C_RXDATA3,
  I2C_RXENDACK,
  I2C_RXACK1,
  I2C_RXACK2,
  I2C_TXDATA1,
  I2C_TXDATA2,
  I2C_TXACK1,
  I2C_TXACK2,
  I2C_STOP1,
  I2C_STOP2,
  I2C_STOP3,
  I2C_ENDLEGO1,
  I2C_ENDLEGO2,
  I2C_END,
  I2C_ENDSTOP1,
  I2C_ENDSTOP2,
  I2C_ENDSTOP3,
  I2C_FAULT,
  I2C_RELEASE,
} i2c_port_state;

struct i2c_partial_transaction {
  i2c_port_state  state; // Initial state for this transaction
  UWORD nbits;	// N bits to transfer
  UBYTE * data;	// Data buffer
};

#define I2C_ERR_INVALID_PORT -1
#define I2C_ERR_BUSY -2
#define I2C_ERR_FAULT -3
#define I2C_ERR_INVALID_LENGTH -4
#define I2C_ERR_BUS_BUSY -5

#define I2C_BUF_SIZE 16
#define I2C_HS_CLOCK 125000
#define I2C_MAX_PARTIAL_TRANSACTIONS 5
// Fast pulse stretch
#define I2C_CLOCK_RETRY 5
// Timeout for pulse stretch
#define I2C_MAX_STRETCH 100
#define I2C_N_PORTS 4

#define TIME4US ((OSC/16)/250000L)

#define IO_COMPLETE_MASK 0xf
#define BUS_FREE_MASK 0xf00
#define BUS_FREE_SHIFT 8

// Take the clock line high but allow slave devices to extend the low clock
// state by pulling the clock line low.
static int rPulseStretch(ULONG scl_pin, SWORD * pDelay, i2c_port_state * pState)
{
  // Allow pulse stretching, make clock line float
  *AT91C_PIOA_ODR = scl_pin;
  // Has the pulse been stretched too much? If so report an error
  *pDelay -= 1;
  if (*pDelay <= 0)
  {
    *pState = I2C_FAULT;
    return 1;
  }
  // Implement a fast wait routine, to avoid having to wait for an entire
  // clock cycle.
  int i = 0;
  while(!(*AT91C_PIOA_PDSR & scl_pin))
    if (i++ >= I2C_CLOCK_RETRY)
      return 1;
  return 0;
}

// Start a transaction. 
SBYTE rI2CFastStart(UBYTE port, UBYTE address, UBYTE *write_data, UBYTE write_len, UBYTE *pReadLen, UBYTE *data_out)
{ 
  if (port >= I2C_N_PORTS)
    return -1;
  ULONG p_scl_pin;
  ULONG p_sda_pin;
  ULONG p_ready_mask;
  UBYTE p_buffer[I2C_BUF_SIZE+2];
  struct i2c_partial_transaction p_partial_transaction[I2C_MAX_PARTIAL_TRANSACTIONS];
  struct i2c_partial_transaction *p_current_pt;
  i2c_port_state p_state;
  UBYTE *p_data;
  ULONG p_nbits;
  SWORD p_delay;
  UBYTE p_bits;
  UBYTE p_fault;
  UBYTE p_lego_mode = FALSE;
  UBYTE p_always_active = FALSE;
  UBYTE p_no_release = FALSE;
  UBYTE p_high_speed = TRUE;

  ULONG pinmask;

  // enable the port in hi speed mode
  {
    p_scl_pin = CLK_PINS[port];
    p_sda_pin = DATA_PINS[port];
    pinmask = CLK_OR_DATA_PINS[port];
    p_state = I2C_IDLE;
    // Set data & clock to be enabled for output with
    // pullups disabled.

    *AT91C_PIOA_SODR  = pinmask;
    *AT91C_PIOA_OER   = pinmask;
    *AT91C_PIOA_PPUDR = pinmask;
//    // If we are always active, we never drop below the ACTIVEIDLE state 
//    p_lego_mode = ((mode & I2C_LEGO_MODE) ? 1 : 0);
//    p_no_release = ((mode & I2C_NO_RELEASE) ? 1 : 0);
//    p_always_active = ((mode & I2C_ALWAYS_ACTIVE) ? 1 : 0);
    if (p_always_active)
      p_state = I2C_ACTIVEIDLE;

    // Select which lines to test to see if the bus is busy.
    // If the clock line is being driven by us we do not test it.
    if (p_lego_mode || p_no_release)
      p_ready_mask = p_sda_pin;
    else
      p_ready_mask = pinmask;
    // Release whichever lines we can
    *AT91C_PIOA_ODR = p_ready_mask;
  }  
  
  struct i2c_partial_transaction *pt;
  UBYTE *data;

  // check buffer size
  if (*pReadLen > I2C_BUF_SIZE) return I2C_ERR_INVALID_LENGTH;
  if (write_len > I2C_BUF_SIZE) return I2C_ERR_INVALID_LENGTH;   
  // must have some data to transfer
  if (*pReadLen + write_len <= 0) return I2C_ERR_INVALID_LENGTH;

  pt = p_partial_transaction;
  p_current_pt = pt;
  data = p_buffer;
  
  // process the write data (if any)
  if (write_len > 0){
    *data++ = address; // This is a write
    pt->nbits = (write_len + 1)*8;
    // copy the write data
    memcpy(data, write_data, write_len);
    data += write_len;
    pt->data = p_buffer;
    pt->state = I2C_NEWSTART;
    // We add an extra stop for the odd Lego i2c sensor, but only on a read
    if (*pReadLen > 0 && p_lego_mode) {
      pt++;
      pt->state = I2C_STOP1;
    }
    pt++;
  }
  // now add the read transaction (if any)
  if (*pReadLen > 0)
  {
    // first we have to write the device address
    pt->state = (data != p_buffer ? I2C_NEWRESTART : I2C_NEWSTART);
    pt->data = data;
    *data++ = address |  1; // this is a read
    pt->nbits = 8;
    pt++;
    // now we have the read
    pt->state = I2C_NEWREAD;
    pt->data = p_buffer;
    pt->nbits = (*pReadLen)*8;
    pt++;
  }
  // define what happens at the end of the operation
  if (p_lego_mode)
    pt->state = (*pReadLen > 0 ? I2C_ENDLEGO1 : I2C_ENDSTOP1);
  else
    pt->state = (p_no_release ? I2C_END : I2C_ENDSTOP1);
//  // We save the number of bytes to read for completion
//  p->read_len = read_len;
  // Start the transaction
  p_state = I2C_BEGIN;
//  if (!p_always_active)
//    build_active_list();
  while(p_state != I2C_COMPLETE)
  {
    ULONG PitTmr = (*AT91C_PITC_PIIR);
    // The main i2c state machine. Move the port state from one step to another
    // toggling the clock line on alternate calls.
    switch (p_state) {
    default:
    case I2C_DISABLED:
    case I2C_IDLE:		// Not in a transaction
    case I2C_ACTIVEIDLE:	// Not in a transaction but active
    case I2C_COMPLETE:          // Transaction completed
      break;
    case I2C_BEGIN:		
      // Start new transaction
      *AT91C_PIOA_OER = p_sda_pin|p_scl_pin;
      p_fault = 0;
      p_state = p_current_pt->state;
      break;
    case I2C_NEWRESTART:		
      // restart a new partial transaction
      // Take the clock low
      *AT91C_PIOA_CODR = p_scl_pin;
      // FALLTHROUGH
    case I2C_NEWSTART:		
      // Start the current partial transaction
      p_data = p_current_pt->data;
      p_nbits = p_current_pt->nbits;
      p_bits = *(p_data);
      p_state = I2C_START1;
      p_delay = I2C_MAX_STRETCH;
      *AT91C_PIOA_SODR = p_sda_pin;
      *AT91C_PIOA_OER = p_sda_pin;
      break;
    case I2C_START1:
      // SDA high, take SCL high
      if (p_high_speed && rPulseStretch(p_scl_pin, &p_delay, &p_state))
        break;
      *AT91C_PIOA_SODR = p_scl_pin;
      *AT91C_PIOA_OER = p_scl_pin;
      p_state = I2C_START2;
      break;
    case I2C_START2:		
      // Take SDA low while SCL is high
      *AT91C_PIOA_CODR = p_sda_pin;
      p_state = I2C_TXDATA1;
      break;
    case I2C_DELAY:
      if (p_delay == 0)
        p_state = I2C_RXDATA1;
      else
        p_delay--;
      break;
    case I2C_TXDATA1:
      // Take SCL low
      *AT91C_PIOA_CODR = p_scl_pin;
      p_delay = I2C_MAX_STRETCH;
      p_state = I2C_TXDATA2;
      break;
    case I2C_TXDATA2:
      // set the data line
      if (p_bits & 0x80)
        *AT91C_PIOA_SODR = p_sda_pin;
      else
        *AT91C_PIOA_CODR = p_sda_pin;
      *AT91C_PIOA_OER = p_sda_pin;
      if (p_high_speed && rPulseStretch(p_scl_pin, &p_delay, &p_state))
        break;
      p_bits <<= 1;
      p_nbits--;
      if((p_nbits & 7) == 0) 
        p_state = I2C_TXACK1;
      else
        p_state = I2C_TXDATA1;
      // Take SCL high
      *AT91C_PIOA_SODR = p_scl_pin;
      *AT91C_PIOA_OER = p_scl_pin;
      break;
    case I2C_TXACK1:
      // Take SCL low
      *AT91C_PIOA_CODR = p_scl_pin;
      p_state = I2C_TXACK2;
      // release the data line
      *AT91C_PIOA_ODR = p_sda_pin;
      break;
    case I2C_TXACK2:
      // Take SCL High
      *AT91C_PIOA_SODR = p_scl_pin;
      if(*AT91C_PIOA_PDSR & p_sda_pin)
        p_state = I2C_FAULT;
      else if (p_nbits == 0)
      {
        p_current_pt++;
        p_state = p_current_pt->state;
      }
      else
      {
        p_data++;
        p_bits = *(p_data);
        p_state = I2C_TXDATA1;
      }
      break;
    case I2C_NEWREAD:		
      // Start the read partial transaction
      p_data = p_current_pt->data;
      p_nbits = p_current_pt->nbits;
      p_bits = 0;
      // Take SCL Low
      *AT91C_PIOA_CODR = p_scl_pin;
      p_state = I2C_RXDATA1;
      // get ready to read
      *AT91C_PIOA_ODR = p_sda_pin;
      break;
    case I2C_RXDATA1:
      p_delay = I2C_MAX_STRETCH;
      p_state = I2C_RXDATA2;
      // Fall through
    case I2C_RXDATA2:
      if (p_high_speed && rPulseStretch(p_scl_pin, &p_delay, &p_state))
        break;
      // Take SCL High
      *AT91C_PIOA_SODR = p_scl_pin;
      *AT91C_PIOA_OER = p_scl_pin;
      p_state = I2C_RXDATA3;
      break;
    case I2C_RXDATA3:
      // Receive a bit.
      p_bits <<= 1;
      if(*AT91C_PIOA_PDSR & p_sda_pin)
        p_bits |= 1;
      // Take SCL Low
      *AT91C_PIOA_CODR = p_scl_pin;
      p_nbits--;
      if((p_nbits & 7) == 0){
        *(p_data) = p_bits;
        if (p_nbits)
          p_state = I2C_RXACK1;
        else
          p_state = I2C_RXENDACK;
      }
      else
        p_state = I2C_RXDATA1;
      break;
    case I2C_RXACK1:
      // take data low
      *AT91C_PIOA_CODR = p_sda_pin;
      *AT91C_PIOA_OER = p_sda_pin;
      // Move on to next byte
      p_data++;
      p_bits = 0;
      p_state = I2C_RXACK2;
      // Clock high
      *AT91C_PIOA_SODR = p_scl_pin;
      break;
    case I2C_RXACK2:
      // Take SCL Low
      *AT91C_PIOA_CODR = p_scl_pin;
      p_state = I2C_RXDATA1;
      // get ready to read
      *AT91C_PIOA_ODR = p_sda_pin;
      break;
    case I2C_RXENDACK:
      // take data high
      *AT91C_PIOA_SODR = p_sda_pin;
      *AT91C_PIOA_OER = p_sda_pin;
      // get ready to move to the next state
      p_current_pt++;
      p_state = p_current_pt->state;
      // Clock high data is already high
      *AT91C_PIOA_SODR = p_scl_pin;
      break;
    case I2C_STOP1:
      // Issue a Stop state
      // SCL is high, take it low
      *AT91C_PIOA_CODR = p_scl_pin;
      p_state = I2C_STOP2;
      *AT91C_PIOA_CODR = p_sda_pin;
      *AT91C_PIOA_OER = p_sda_pin;
      break;
    case I2C_STOP2:
      // Take SCL high
      *AT91C_PIOA_SODR = p_scl_pin;
      p_state = I2C_STOP3;
      break;  
    case I2C_STOP3:
      // Take SDA pin high while the clock is high
      *AT91C_PIOA_SODR = p_sda_pin;
      // and move to the next state
      p_current_pt++;
      p_state = p_current_pt->state;
      break;
    case I2C_ENDLEGO1:
      // Lego mode end case, does not issue stop. Used at end of rx
      // Clock low data is already high, release the data line
      *AT91C_PIOA_CODR = p_scl_pin;
      p_state = I2C_ENDLEGO2;
      *AT91C_PIOA_ODR = p_sda_pin;
      break;
    case I2C_ENDLEGO2:
      // Clock high, data is already high and we are done
      *AT91C_PIOA_SODR = p_scl_pin;
      p_state = I2C_RELEASE;
      break;
    case I2C_END:
      // End the transaction but hold onto the bus, keeping the clock low
      // Clock low and keep it active
      *AT91C_PIOA_CODR = p_scl_pin;
      p_state = I2C_RELEASE;
      break;
    case I2C_ENDSTOP1:
      // Issue a Stop state
      // SCL is high, take it low
      *AT91C_PIOA_CODR = p_scl_pin;
      p_delay = I2C_MAX_STRETCH;
      p_state = I2C_ENDSTOP2;
      *AT91C_PIOA_CODR = p_sda_pin;
      *AT91C_PIOA_OER = p_sda_pin;
      break;
    case I2C_ENDSTOP2:
      if (p_high_speed && rPulseStretch(p_scl_pin, &p_delay, &p_state))
        break;
      // Take SCL high
      *AT91C_PIOA_SODR = p_scl_pin;
      *AT91C_PIOA_OER = p_scl_pin;
      p_state = I2C_ENDSTOP3;
      break;  
    case I2C_ENDSTOP3:
      // Take SDA pin high while the clock is high
      *AT91C_PIOA_SODR = p_sda_pin;
      p_state = I2C_RELEASE;
      break;
    case I2C_FAULT:
      p_fault = 1;
      p_state = I2C_ENDSTOP1;
      break;
    case I2C_RELEASE:
      // Release whichever lines we can
      *AT91C_PIOA_ODR = p_ready_mask;
      // All done
      p_state = I2C_COMPLETE;
      break;
    }
    while (((*AT91C_PITC_PIIR) - PitTmr) < TIME4US);
  }
  
  if (!p_always_active)
    p_state = I2C_IDLE;
  else
    p_state = I2C_ACTIVEIDLE;
  if (p_fault)
    return I2C_ERR_FAULT;
  if (*pReadLen > 0)
  {
    *pReadLen = (UBYTE)(p_data - p_buffer + 1);
    if (data_out)
      memcpy(data_out, p_buffer, *pReadLen);
  }
  return *pReadLen;
  
}

        
#endif

#ifdef    PCWIN

#endif
