//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 28-10-05 13:46                                    $
//
// Filename        $Workfile:: d_armcomm.r                                   $
//
// Version         $Revision:: 15                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/d_armcom $
//
// Platform        C
//

#ifdef    ATMEGAX8

/****************************************************************************
  TWI State codes
****************************************************************************/

#define TWI_START                  0x08  // START has been transmitted  
#define TWI_REP_START              0x10  // Repeated START has been transmitted
#define TWI_ARB_LOST               0x38  // Arbitration lost

// TWI Master Transmitter staus codes                      
#define TWI_MTX_ADR_ACK            0x18  // SLA+W has been tramsmitted and ACK received
#define TWI_MTX_ADR_NACK           0x20  // SLA+W has been tramsmitted and NACK received 
#define TWI_MTX_DATA_ACK           0x28  // Data byte has been tramsmitted and ACK received
#define TWI_MTX_DATA_NACK          0x30  // Data byte has been tramsmitted and NACK received 

// TWI Master Receiver staus codes  
#define TWI_MRX_ADR_ACK            0x40  // SLA+R has been tramsmitted and ACK received
#define TWI_MRX_ADR_NACK           0x48  // SLA+R has been tramsmitted and NACK received
#define TWI_MRX_DATA_ACK           0x50  // Data byte has been received and ACK tramsmitted
#define TWI_MRX_DATA_NACK          0x58  // Data byte has been received and NACK tramsmitted

// TWI Slave Transmitter staus codes
#define TWI_STX_ADR_ACK            0xA8  // Own SLA+R has been received; ACK has been returned
#define TWI_STX_ADR_ACK_M_ARB_LOST 0xB0  // Arbitration lost in SLA+R/W as Master; own SLA+R has been received; ACK has been returned
#define TWI_STX_DATA_ACK           0xB8  // Data byte in TWDR has been transmitted; ACK has been received
#define TWI_STX_DATA_NACK          0xC0  // Data byte in TWDR has been transmitted; NOT ACK has been received
#define TWI_STX_DATA_ACK_LAST_BYTE 0xC8  // Last data byte in TWDR has been transmitted (TWEA = “0”); ACK has been received

// TWI Slave Receiver staus codes
#define TWI_SRX_ADR_ACK            0x60  // Own SLA+W has been received ACK has been returned
#define TWI_SRX_ADR_ACK_M_ARB_LOST 0x68  // Arbitration lost in SLA+R/W as Master; own SLA+W has been received; ACK has been returned
#define TWI_SRX_GEN_ACK            0x70  // General call address has been received; ACK has been returned
#define TWI_SRX_GEN_ACK_M_ARB_LOST 0x78  // Arbitration lost in SLA+R/W as Master; General call address has been received; ACK has been returned
#define TWI_SRX_ADR_DATA_ACK       0x80  // Previously addressed with own SLA+W; data has been received; ACK has been returned
#define TWI_SRX_ADR_DATA_NACK      0x88  // Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
#define TWI_SRX_GEN_DATA_ACK       0x90  // Previously addressed with general call; data has been received; ACK has been returned
#define TWI_SRX_GEN_DATA_NACK      0x98  // Previously addressed with general call; data has been received; NOT ACK has been returned
#define TWI_SRX_STOP_RESTART       0xA0  // A STOP condition or repeated START condition has been received while still addressed as Slave

// TWI Miscellaneous status codes
#define TWI_NO_STATE               0xF8  // No relevant state information available; TWINT = “0”
#define TWI_BUS_ERROR              0x00  // Bus error due to an illegal START or STOP condition



/***********************************************************************************/
/***********************    Declaration of variables *******************************/
/***********************************************************************************/

#define   ADDRESS                       1  
#define   INBYTES                       BYTES_TO_TX // (sizeof(IoToAvr))
#define   OUTBYTES                      BYTES_TO_RX // (sizeof(IoFromAvr))

__flash   UBYTE CopyRightString[COPYRIGHTSTRINGLENGTH + 1] = COPYRIGHTSTRING;

static    UBYTE I2CInByte;
static    UBYTE I2CInBuffer[INBYTES + 1];
static    UBYTE *pI2CInBuffer;
static    UBYTE I2CInPointer;
static    UBYTE I2COutBuffer[OUTBYTES + 1];
static    UBYTE *pI2COutBuffer;
static    UBYTE I2COutPointer;
static    UBYTE Chksum;
static    UBYTE I2CInState;
static    UBYTE ArmCommFlag;
static    UBYTE ArmCopyRightValid;

#define   ARMCOMMInit                   TWAR = (UBYTE)(ADDRESS << 1);\
                                        TWCR = 0xC5;\
                                        ArmCommFlag = FALSE;\
                                        ArmCopyRightValid = FALSE


#pragma vector=TWI_vect
__interrupt void I2CInterrupt(void)
{
  switch ((TWSR & 0xF8))
  {
  
    // Write command
    
    case TWI_SRX_ADR_ACK :
    {
      I2CInPointer  = 0;
      I2CInState    = 0;
    }
    break;
    
    case TWI_SRX_ADR_DATA_ACK :
    {
      I2CInByte = TWDR;
    
      switch (I2CInState)
      {
        case 0 :
        {
          if (I2CInByte != 0xCC)
          {
            I2CInBuffer[I2CInPointer++] = I2CInByte;
            I2CInState++;
          }
          else
          {
            I2CInState = 2;
          }
        }
        break;
      
        case 1 :
        {
          I2CInBuffer[I2CInPointer++] = I2CInByte;
          if (I2CInPointer >= (INBYTES + 1))
          {
            Chksum = 0;  
            for (I2CInPointer = 0;I2CInPointer < (INBYTES + 1);I2CInPointer++)
            {
              Chksum += I2CInBuffer[I2CInPointer];
            }
            
            if (Chksum == 0xFF)
            {
              pI2CInBuffer = (UBYTE*)&IoToAvr;
              for (I2CInPointer = 0;I2CInPointer < INBYTES;I2CInPointer++)
              {
                *pI2CInBuffer = I2CInBuffer[I2CInPointer];
                pI2CInBuffer++;
              }
              ArmCommFlag = TRUE;
            }
            I2CInState = 99;
          }       
        }
        break;
      
        case 2 :
        {
          if (I2CInByte == CopyRightString[I2CInPointer++])
          {
            if (I2CInPointer >= COPYRIGHTSTRINGLENGTH)
            {
              ArmCopyRightValid = TRUE;
            }
          }
          else
          {
            I2CInState = 99;
          }
        }
        break;
        
        default :
        {
        }
        break;
        
      }
    }
    break;

    // Read command
  
    case TWI_STX_ADR_ACK :
    {
      Chksum = 0;
      pI2COutBuffer = (UBYTE*)&IoFromAvr;
      for (I2COutPointer = 0;I2COutPointer < OUTBYTES;I2COutPointer++)
      {
        I2COutBuffer[I2COutPointer] = *pI2COutBuffer;
        Chksum += *pI2COutBuffer;
        pI2COutBuffer++;
      }
      I2COutBuffer[I2COutPointer] = ~Chksum;
      I2COutPointer = 0;
      TWDR = I2COutBuffer[I2COutPointer++];
    }
    break;
    
    case TWI_STX_DATA_ACK :
    {
      if (I2COutPointer >= (OUTBYTES + 1))
      {
        
      }
      else
      {
        TWDR = I2COutBuffer[I2COutPointer++];
      }    
    }
    break;
    case TWI_NO_STATE:
    {
      TWCR |= 0x90;
    }
    break;
    case TWI_BUS_ERROR:
    {
      UBYTE volatile Tmp;
      Tmp = 1;
      TWCR &= ~0x20;
      Tmp = 0;
      TWCR |= 0x90;
      Tmp = 2;
    }
    break;
  
    default:
    {
    }
    break;
    
  }
  TWCR  |= 0x80;
}

UBYTE     ArmCommCheck(void)
{
  UBYTE   Result;
  
  Result      = ArmCommFlag;
  ArmCommFlag = FALSE;

  return (Result);
}


#define   ARMCOMMCheck                  ArmCommCheck()

#define   ARMCOMMCopyRight              ArmCopyRightValid

#define   ARMCOMMExit                   PORTC &= ~0x30;\
                                        DDRC  |=  0x30;\
                                        TWCR   =  0x80

#endif
