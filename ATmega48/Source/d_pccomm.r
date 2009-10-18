//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 22-02-05 11:10                                    $
//
// Filename        $Workfile:: d_pccomm.r                                    $
//
// Version         $Revision:: 6                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/d_pccomm $
//
// Platform        C
//

#ifdef    ATMEGAX8

#define   BAUD_RATE                     4800L

#define   RX_BUFFERSIZE                 (BYTES_TO_TX)
#define   TX_BUFFERSIZE                 (BYTES_TO_RX)

UBYTE     RxBuffer[RX_BUFFERSIZE];
UBYTE     RxPointer;

UBYTE     TxBuffer[TX_BUFFERSIZE];
UBYTE     TxPointer;

#pragma   language=extended
#pragma   vector = USART_RX_vect
__interrupt void RxInterrupt(void)
{
  UBYTE   *pBuffer;
  
  RxBuffer[RxPointer] = UDR0;
  RxPointer++;
  if (RxPointer >= RX_BUFFERSIZE)
  {
    pBuffer   = (UBYTE*)&IoToAvr;
    for (RxPointer = 0;RxPointer < RX_BUFFERSIZE;RxPointer++)
    {
      *pBuffer = RxBuffer[RxPointer];
      pBuffer++;
    }
    RxPointer = 0;
    pBuffer   = (UBYTE*)&IoFromAvr;
    for (TxPointer = 0;TxPointer < TX_BUFFERSIZE;TxPointer++)
    {
      TxBuffer[TxPointer] = *pBuffer;
      pBuffer++;
    }
    TxPointer = 0;
    UDR0      = TxBuffer[TxPointer];
    TxPointer++;
    UCSR0B   |= 0x40;
  }
}

#pragma   language=extended
#pragma   vector = USART_TX_vect
__interrupt void TxInterrupt(void)
{
  UDR0 = TxBuffer[TxPointer];
  TxPointer++;
  if (TxPointer >= TX_BUFFERSIZE)
  {
    UCSR0B &= ~0x40;
    TxPointer = 0;
    RxPointer = 0;
  }
}

#define   INITPcComm                    {\
                                          DDRD     |=  0x02;\
                                          DDRD     &= ~0x01;\
                                          UBRR0     =  (UWORD)((OSC/(16 * BAUD_RATE)) - 1);\
                                          UCSR0A    =  0x40;\
                                          UCSR0B    =  0x98;\
                                          UCSR0C    =  0x36;\
                                          RxPointer =  0;\
                                        }

#define   EXITPcComm                    {\
                                          UCSR0B    =  0x00;\
                                          PORTD    &= ~0x01;\
                                          DDRD     |=  0x01;\
                                        }


#endif
