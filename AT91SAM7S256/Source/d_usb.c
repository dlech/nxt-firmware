//
// Programmer
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dkpechri                                        $
//
// Revision date   $Date:: 19-07-06 10:02                                    $
//
// Filename        $Workfile:: d_usb.c                                       $
//
// Version         $Revision:: 32                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_usb.c    $
//
// Platform        C
//

#include  "stdconst.h"
#include  "m_sched.h"
#include  "d_usb.h"
#include  "d_usb.r"

#define ENDPOINT_OUT 1          // HOST write
#define ENDPOINT_OUT_SIZE 64
#define ENDPOINT_IN 2           // HOST read
#define ENDPOINT_IN_SIZE 64

#define AT91C_UDP_ISR         ((AT91_REG *)   0xFFFB001C) // (UDP) Interrupt Status Register
#define AT91C_RSTC_URSTEN     ((unsigned int) 0x1 <<  0)  // (RSTC) User Reset Enable

                // Endpoint Control and Status Registers
#define AT91C_UDP_CSR0  ((AT91_REG *)   0xFFFB0030) // Endpoint 0 Control and Status Register
#define AT91C_UDP_CSR1  ((AT91_REG *)   0xFFFB0034) // Endpoint 1 Control and Status Register
#define AT91C_UDP_CSR2  ((AT91_REG *)   0xFFFB0038) // Endpoint 2 Control and Status Register
#define AT91C_UDP_CSR3  ((AT91_REG *)   0xFFFB003C) // Endpoint 3 Control and Status Register

                          // Endpoint FIFO Data Registers
#define AT91C_UDP_FDR0  ((AT91_REG *)   0xFFFB0050) // Endpoint 0 FIFO Data Register
#define AT91C_UDP_FDR1  ((AT91_REG *)   0xFFFB0054) // Endpoint 1 FIFO Data Register
#define AT91C_UDP_FDR2  ((AT91_REG *)   0xFFFB0058) // Endpoint 2 FIFO Data Register
#define AT91C_UDP_FDR3  ((AT91_REG *)   0xFFFB005C) // Endpoint 3 FIFO Data Register

const UBYTE DeviceDescriptor[] = {
  /* Device descriptor */
  0x12,   // bLength, size of this descriptor = 18 entries
  0x01,   // bDescriptorType = 1 = DEVICE
  0x00,   // bcdUSBL, USB spec. vers. 2.0
  0x02,   // bcdUSBH, -
  0x00,   // bDeviceClass
  0x00,   // bDeviceSubclass
  0x00,   // bDeviceProtocol
  0x08,   // bMaxPacketSize0, EndPointZero packet size = 8
  0x94,   // idVendorL, LEGO Group
  0x06,   // idVendorH, -
  0x02,   // idProductL, LEGO USB IR Tower = 0x01
  0x00,   // idProductH, -
  0x00,   // bcdDeviceL, device is version (zero)
  0x00,   // bcdDeviceH, -
  0x00,   // iManufacturer, index of string descriptor describing manufacturer
  0x00,   // iProduct, index of string descriptor describing product
  0x01,   // iSerialNumber,  index of string descriptor describing the device's
          // serial no.
  0x01    // bNumConfigs, number of possible configurations (only one)
};

/* USB standard request codes */

#define STD_GET_STATUS_ZERO           0x0080
#define STD_GET_STATUS_INTERFACE      0x0081
#define STD_GET_STATUS_ENDPOINT       0x0082

#define STD_CLEAR_FEATURE_ZERO        0x0100
#define STD_CLEAR_FEATURE_INTERFACE   0x0101
#define STD_CLEAR_FEATURE_ENDPOINT    0x0102

#define STD_SET_FEATURE_ZERO          0x0300
#define STD_SET_FEATURE_INTERFACE     0x0301
#define STD_SET_FEATURE_ENDPOINT      0x0302

#define STD_SET_ADDRESS               0x0500
#define STD_GET_DESCRIPTOR            0x0680
#define STD_SET_DESCRIPTOR            0x0700
#define STD_GET_CONFIGURATION         0x0880
#define STD_SET_CONFIGURATION         0x0900
#define STD_GET_INTERFACE             0x0A81
#define STD_SET_INTERFACE             0x0B01
#define STD_SYNCH_FRAME               0x0C82

/* USB constants, masks etc. */

#define END_OF_BUS_RESET        ((unsigned int) 0x1 << 12)
#define SUSPEND_INT                   ((unsigned int) 0x1 << 8)
#define SUSPEND_RESUME                ((unsigned int) 0x1 << 9)
#define WAKEUP                        ((unsigned int) 0x1 << 13)

//USB spec allows 500ms for control transfers
#define USB_MAX_TIMEOUT                500

static    UBYTE UsbHandleList[MAX_HANDLES];
static    UBYTE UsbHandleCnt;
static    UWORD RequestedData;
static    UBYTE BrickNameKnown;
enum
{
  USB_NOT_CONFIGURED,
  USB_CONFIGURED,
  USB_CONFIGURED_BUT_SUSPENDED
};
static    UBYTE UsbConnectionStates;


const UBYTE ConfigurationDescriptor[] = {
  /* ============== CONFIGURATION 1 =========== */
  /* Configuration 1 descriptor */
  0x09,   // bLength, descriptor size in bytes
  0x02,   // bDescriptorType, The constant Configuration
  0x20,   // wTotalLengthL for 2 EP + Control
  0x00,   // wTotalLengthH -
  0x01,   // bNumInterfaces, Number of interfaces in the configuration
  0x01,   // bConfigurationValue, Identifier for
          // Set_Configuration and Get_Configuration requests
  0x00,   // iConfiguration, Index of string descriptor for the configuration
  0xC0,   // bmAttributes, Bit 7 shall always be set. See e.g. page 108 in the book:
          // "USB Complete" by Jan Axelson. June 2001
          // Self powered only bit 6 = 1 (zero = buspowered USB 1.1 and up)
  0x00,   // MaxPower, power required (mA./2) We're SELF-POWERED, so ZERO

  /* Interface Descriptor */
  0x09,   // bLength, descriptor size in bytes
  0x04,   // bDescriptorType, the constant 0x04 = "INTERFACE"
  0x00,   // bInterfaceNumber, No. identifying this interface
  0x00,   // bAlternateSetting, value used to get an alternative interface
  0x02,   // bNumEndpoints, No. of supported endpoints in addition to endpoint 0
  0xFF,   // bInterfaceClass, Specifies the class code = VENDOR Specific
  0xFF,   // bInterfaceSubclass, Specifies the subclass code = VENDOR Specific
  0xFF,   // bInterfaceProtocol, protocol code = VENDOR Specific
  0x00,   // iInterface, index of string descriptor for the interface

  /* Endpoint 1 descriptor */
  0x07,               // bLength, descriptor length incl. this = 7
  0x05,               // bDescriptorType
  0x01,               // bEndpointAddress, Endpoint 01 - OUT
  0x02,               // bmAttributes      BULK
  ENDPOINT_OUT_SIZE,  // wMaxPacketSize
  0x00,               // -
  0x00,               // bInterval

  /* Endpoint 2 descriptor */
  0x07,               // bLength, descriptor length incl. this = 7
  0x05,               // bDescriptorType
  0x82,               // bEndpointAddress, Endpoint 02 - IN
  0x02,               // bmAttributes      BULK
  ENDPOINT_IN_SIZE,   // wMaxPacketSize
  0x00,               // -
  0x00                // bInterval
};

UBYTE SerialNumberDescriptor[] =
{
      0x1A,           // bLength, descriptor length incl. this = 16 bytes
      0x03,           // bDescriptorType

      0x31, 0x00,     // MSD of Lap (Lap[2,3]) in UNICode
      0x32, 0x00,     // Lap[4,5]
      0x33, 0x00,     // Lap[6,7]
      0x34, 0x00,     // Lap[8,9]
      0x35, 0x00,     // Lap[10,11]
      0x36, 0x00,     // Lap[12,13]
      0x37, 0x00,     // Lap[14,15]
      0x38, 0x00,     // LSD of Lap (Lap[16,17]) in UNICode

      0x30, 0x00,     // MSD of Nap (Nap[18,19]) in UNICode
      0x30, 0x00,     // LSD of Nap (Nap[20,21]) in UNICode

      0x39, 0x00,     // MSD of Uap in UNICode
      0x30, 0x00      // LSD of Uap in UNICode
};

const UBYTE LangIdDescriptor[] =
{
      0x04,           // Length
      0x03,           // Type, 3 = CONSTANT String
      0x09,           // English
      0x04            // subcode = U.S. English
};

static UCHAR CurrentConfiguration;  // Configured or not. We've only 1 conf. so... Boolean
static ULONG CurrentReceiveBank;    // Used for keep track of the PING-PONG buffers

ULONG g_UsbTimeoutCounter;

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

void   dUsbDisconnect(void)
{
  USBDisconnect;
}

void   dUsbConnect(void)
{
  USBConnect;
}

void  dUsbStartTimeoutTimer(void)
{
   g_UsbTimeoutCounter = 0;

  USBGetActualTime;
}

// A longer version of the USB timer.
// Table 7-14 of the USB 2.0 spec allows up to 500ms for standard request completion.
UBYTE dUsbTimedOut(void)
{
   if(USBTimedOut)
   {
      g_UsbTimeoutCounter++;

      USBGetActualTime;
   }

   return (g_UsbTimeoutCounter >= USB_MAX_TIMEOUT) ? TRUE : FALSE;
}


UBYTE ConvertHighToHex(UBYTE TempChar)
{
  TempChar = (TempChar >> 4) & 0x0F;
  if (TempChar > 0x09)
    TempChar += 0x37;
  else
    TempChar += 0x30;
  return TempChar;
}

UBYTE ConvertLowToHex(UBYTE TempChar)
{
  TempChar &= 0x0F;
  if (TempChar > 0x09)
    TempChar += 0x37;
  else
    TempChar += 0x30;
  return TempChar;
}

void dUsbStoreBtAddress(UBYTE *pBtAddress)
{
  UBYTE NoToConvert;

  // make the Lap human readable (hmmm Hexadecimal)
  NoToConvert = *pBtAddress++;
  SerialNumberDescriptor[2] = ConvertHighToHex(NoToConvert);
  SerialNumberDescriptor[4] = ConvertLowToHex(NoToConvert);

  NoToConvert = *pBtAddress++;
  SerialNumberDescriptor[6] = ConvertHighToHex(NoToConvert);
  SerialNumberDescriptor[8] = ConvertLowToHex(NoToConvert);

  NoToConvert = *pBtAddress++;
  SerialNumberDescriptor[10] = ConvertHighToHex(NoToConvert);
  SerialNumberDescriptor[12] = ConvertLowToHex(NoToConvert);

  NoToConvert = *pBtAddress++;
  SerialNumberDescriptor[14] = ConvertHighToHex(NoToConvert);
  SerialNumberDescriptor[16] = ConvertLowToHex(NoToConvert);

  // make the Uap human readable (hmmm Hexadecimal)
  NoToConvert = *pBtAddress++;
  SerialNumberDescriptor[18] = ConvertHighToHex(NoToConvert);
  SerialNumberDescriptor[20] = ConvertLowToHex(NoToConvert);

  // make the Nap human readable (hmmm Hexadecimal)
  NoToConvert = *pBtAddress++;
  SerialNumberDescriptor[22] = ConvertHighToHex(NoToConvert);
  SerialNumberDescriptor[24] = ConvertLowToHex(NoToConvert);

  USBConnect;             // We're ready to participate in the real world
  BrickNameKnown = TRUE;  // OK for referencing :-)
}


ULONG dUsbRead(UBYTE *pData, ULONG Length)
{
  ULONG PacketSize, NumberOfBytesReceived;

  NumberOfBytesReceived = 0;

  while (Length)                  // Wished read size from user (Max length)
  {
    if ( !(BrickNameKnown))       // Right Brick???
      break;

    if ( !(dUsbIsConfigured()) )
      break;                      // Not configured - no time to waste

    if ( (*AT91C_UDP_CSR1) & CurrentReceiveBank )			// Data packet rx'ed in Current bank?
    {

    PacketSize = MIN((*AT91C_UDP_CSR1) >> 16, Length);		// Normalize number of bytes available in FIFO
    Length -= PacketSize;									// Rest of data to receive

    if (PacketSize < ENDPOINT_OUT_SIZE)						// If data less, we only have one loop
      Length = 0;

    while(PacketSize--)										// While more data in this very packet...
      pData[NumberOfBytesReceived++] = *AT91C_UDP_FDR1;		// Fill in buffer

    *AT91C_UDP_CSR1 &= ~(CurrentReceiveBank);				// Reset current bank pointer

    if (CurrentReceiveBank == AT91C_UDP_RX_DATA_BK0)		// Current Receive Bank 0?
      CurrentReceiveBank = AT91C_UDP_RX_DATA_BK1;			// We better use Bank 1
    else
      CurrentReceiveBank = AT91C_UDP_RX_DATA_BK0;			// Okay, go for Bank 0 :-)

    }

    else Length = 0;                                        // Leave and let's use the CPU cycles in a better way

  }

  return NumberOfBytesReceived;                 // Size of actually received stuff

}

ULONG dUsbWrite( const UBYTE *pData, ULONG Length)
{
  ULONG CharsEachTx = 0;

                                                      // Send the very first (or only) packet
  CharsEachTx = MIN(Length, ENDPOINT_IN_SIZE);        // First transmission size
  Length -= CharsEachTx;                              // Adjust the rest of transmission size

  while (CharsEachTx--)                               // While more chars in this chunk
    *AT91C_UDP_FDR2 = *pData++;                       // Get rid off it one by one
                                                      // Pushing the data into the UDP TX-fifo
  *AT91C_UDP_CSR2 |= AT91C_UDP_TXPKTRDY;              // Signal "DO THE TX" the stuff is delivered...

  while (Length)                                      // While more bytes (I.e. packets) ín total transmission
  {                                                   // Start filling the second bank

    CharsEachTx = MIN(Length, ENDPOINT_IN_SIZE);
    Length -= CharsEachTx;                            // Adjust total length

    while (CharsEachTx--)                             // While more chars in this chunk
      *AT91C_UDP_FDR2 = *pData++;

          dUsbStartTimeoutTimer();
    while ( !((*AT91C_UDP_CSR2) & AT91C_UDP_TXCOMP) )			// Wait for the the first bank to be sent
          if (dUsbTimedOut() || !(dUsbIsConfigured()) )			// Communication down..... Bail out
            return Length;										// Invalid function - return job length not done

          (*AT91C_UDP_CSR2) &= ~(AT91C_UDP_TXCOMP);				// Reset transmit interrupt flag

    while ((*AT91C_UDP_CSR2) & AT91C_UDP_TXCOMP);				// Wait until flag (H/W) is reset

    (*AT91C_UDP_CSR2) |= AT91C_UDP_TXPKTRDY;					// We're ready to send next bank

  }																// Loop while bytes to tx

        dUsbStartTimeoutTimer();								// Arm the timeout timing
        while ( !((*AT91C_UDP_CSR2) & AT91C_UDP_TXCOMP) )		// Wait for transmission to complete
          if ( !(dUsbIsConfigured()) || dUsbTimedOut())         // Communication down..... Bail out
            return Length;										// Invalid function - return job length not done

  (*AT91C_UDP_CSR2) &= ~(AT91C_UDP_TXCOMP);           // Reset Interrupt flag

  while ((*AT91C_UDP_CSR2) & AT91C_UDP_TXCOMP);       // Wait for H/W to settle.....

  return Length;                                      // Return byte count NOT x-ferred

}

static void dUsbSendStall(void)
{
  (*AT91C_UDP_CSR0) |= AT91C_UDP_FORCESTALL;                                // Set STALL condition
  while ( !((*AT91C_UDP_CSR0) & AT91C_UDP_ISOERROR) );                      // Wait until stall ack'ed

  (*AT91C_UDP_CSR0) &= ~(AT91C_UDP_FORCESTALL | AT91C_UDP_ISOERROR);        // Reset again
  while ((*AT91C_UDP_CSR0) & (AT91C_UDP_FORCESTALL | AT91C_UDP_ISOERROR));  // Wait until H/W really reset
}

static void dUsbSendZeroLengthPackage(void)
{
   // Signal that buffer is ready to send
   (*AT91C_UDP_CSR0) |= AT91C_UDP_TXPKTRDY;

   dUsbStartTimeoutTimer();

   // Wait for ACK handshake from host
   while ( !((*AT91C_UDP_CSR0) & AT91C_UDP_TXCOMP) && !dUsbTimedOut());
   // Clear handshake flag
   (*AT91C_UDP_CSR0) &= ~(AT91C_UDP_TXCOMP);
   while ((*AT91C_UDP_CSR0) & AT91C_UDP_TXCOMP);
}

static void dUsbSendViaControl(const UBYTE *pData, ULONG Length)
{
  ULONG BytesToTx = 0;
  AT91_REG Temp_Csr;
  UBYTE HaveToTxZeroLength = FALSE;
  UBYTE ZeroCouldBeNeeded = FALSE;

   // If the amount of data requested is more than what can be sent, a 0-length
   // packet may be required
   if (RequestedData > Length)
   {
      ZeroCouldBeNeeded = TRUE;  // Exact same size would be interpreted as EOP @ host
   }

  do
  {
		// The endpoint size is 8 bytes.  Limit each data phase to 8 bytes.
		
		BytesToTx = MIN(Length, 8);
		Length -= BytesToTx;

		// If this is the last data phase containing data, but the host requested
		// more, a 0-byte packet will be needed to terminate the data phase.
		if(ZeroCouldBeNeeded && (Length == 0) && (BytesToTx == 8))
		{
			HaveToTxZeroLength = TRUE;
		}

		// Copy data to endpoint buffer
		while (BytesToTx--)
		{
			(*AT91C_UDP_FDR0) = *pData++;
		}

		// Signal that buffer is ready to send
		(*AT91C_UDP_CSR0) |= AT91C_UDP_TXPKTRDY;

				dUsbStartTimeoutTimer();

		// Wait for ACK handshake from host
		do
		{
			Temp_Csr = (*AT91C_UDP_CSR0);

			// Return if the status phase occurs before the packet is accepted
			if (Temp_Csr & AT91C_UDP_RX_DATA_BK0)
			{
				// Clear the PKTRDY flag
				(*AT91C_UDP_CSR0) &= ~(AT91C_UDP_TXPKTRDY);
				// Clear the status phase flag
				(*AT91C_UDP_CSR0) &= ~(AT91C_UDP_RX_DATA_BK0);
				return;
			}
		}
		while (!(Temp_Csr & AT91C_UDP_TXCOMP) && !dUsbTimedOut());

		// Clear handshake flag
		(*AT91C_UDP_CSR0) &= ~(AT91C_UDP_TXCOMP);
	 
		while ((*AT91C_UDP_CSR0) & AT91C_UDP_TXCOMP);

  } while (Length);

   if(HaveToTxZeroLength)
   {
      dUsbSendZeroLengthPackage();
   }

   dUsbStartTimeoutTimer();

   // Wait for Status Phase
   while(!((*AT91C_UDP_CSR0) & AT91C_UDP_RX_DATA_BK0) && !dUsbTimedOut());
   // Clear flag
   (*AT91C_UDP_CSR0) &= ~(AT91C_UDP_RX_DATA_BK0);
}

static void dUsbEnumerate(void)
{
  UBYTE bmRequestType, bRequest;
  UWORD wValue, wIndex, wLength, wStatus;

  if ( !((*AT91C_UDP_CSR0) & AT91C_UDP_RXSETUP) )         // No setup package available
    return;
                                                          // Bytes are popped from the FIFO one by one

  bmRequestType = *AT91C_UDP_FDR0;
  bRequest      = *AT91C_UDP_FDR0;
  wValue        = ((*AT91C_UDP_FDR0) & 0xFF);
  wValue       |= ((*AT91C_UDP_FDR0) << 8);
  wIndex        = ((*AT91C_UDP_FDR0) & 0xFF);
  wIndex       |= ((*AT91C_UDP_FDR0) << 8);
  wLength       = ((*AT91C_UDP_FDR0) & 0xFF);
  wLength      |= ((*AT91C_UDP_FDR0) << 8);

  if (bmRequestType & 0x80)                               // If a DEVICE-TO-HOST request
  {
    *AT91C_UDP_CSR0 |= AT91C_UDP_DIR;                     // Enables data IN transaction in the control data stage
    while ( !((*AT91C_UDP_CSR0) & AT91C_UDP_DIR) );       // Repeat until the DIR bit is set
  }

  *AT91C_UDP_CSR0 &= ~AT91C_UDP_RXSETUP;                  // Device firmware has read the setup data in FIFO
  while ( ((*AT91C_UDP_CSR0)  & AT91C_UDP_RXSETUP)  );    // Wait until bit cleared

  // Handle supported standard device request from Table 9-3 in USB specification Rev 2.0

   switch ((bRequest << 8) | bmRequestType)
   {
      case STD_GET_DESCRIPTOR:

      RequestedData = wLength;

      if (wValue == 0x100)       // Return Device Descriptor
      {
         if (sizeof(DeviceDescriptor) > wLength)
         {
            dUsbSendViaControl(DeviceDescriptor, wLength);
         }
         else
         {
            dUsbSendViaControl(DeviceDescriptor, sizeof(DeviceDescriptor));
         }
      }
      else if (wValue == 0x200)  // Return Configuration Descriptor
      {
         if (sizeof(ConfigurationDescriptor) > wLength)
         {
			dUsbSendViaControl(ConfigurationDescriptor, wLength);
         }
         else
         {
            dUsbSendViaControl(ConfigurationDescriptor, sizeof(ConfigurationDescriptor));
         }
      }
      else if ((wValue & 0xF00) == 0x300)
      {
        switch(wValue & 0xFF)
        {
          case 0x00:
            if ((sizeof(LangIdDescriptor)) > wLength)
			{
				dUsbSendViaControl(LangIdDescriptor, wLength);
			}
            else
			{
				dUsbSendViaControl(LangIdDescriptor, sizeof(LangIdDescriptor));
			}
          break;

          case 0x01:
            if ((sizeof(SerialNumberDescriptor)) > wLength)
			{
				dUsbSendViaControl(SerialNumberDescriptor, wLength);
			}
            else
			{
				dUsbSendViaControl(SerialNumberDescriptor, sizeof(SerialNumberDescriptor));
			}
          break;

          default:
			dUsbSendStall();  // Illegal request :-(
          break;
        }
      }
      else
        dUsbSendStall();      // Illegal request :-(

      break;

    case STD_SET_ADDRESS:

        // Status IN transfer
        (*AT91C_UDP_CSR0) |= AT91C_UDP_TXPKTRDY;

        dUsbStartTimeoutTimer();

        while((*AT91C_UDP_CSR0) & AT91C_UDP_TXPKTRDY && !dUsbTimedOut());

              *AT91C_UDP_FADDR = (AT91C_UDP_FEN | wValue);            // Set device address. No check for invalid address.
                                                                      // Function endpoint enabled.
              *AT91C_UDP_GLBSTATE  = (wValue) ? AT91C_UDP_FADDEN : 0; // If Device address != 0 then flag device
                                                                      // in ADDRESS STATE
              break;

    case STD_SET_CONFIGURATION:

              CurrentConfiguration = wValue;                          // Low byte of wValue = wanted configuration
              UsbConnectionStates = USB_CONFIGURED;
              dUsbSendZeroLengthPackage();                            // Signal request processed OK

              *AT91C_UDP_GLBSTATE  = (wValue) ? AT91C_UDP_CONFG : AT91C_UDP_FADDEN;           // If wanted configuration != 0

              *AT91C_UDP_CSR1 = (wValue) ? (AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_BULK_OUT) : 0; // Endpoint 1 enabled and set as BULK OUT
              *AT91C_UDP_CSR2 = (wValue) ? (AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_BULK_IN)  : 0; // Endpoint 2 enabled and set as BULK IN
              *AT91C_UDP_CSR3 = (wValue) ? (AT91C_UDP_EPTYPE_INT_IN)   : 0;                   // Endpoint 3 disabled and set as INTERRUPT IN

              break;

    case STD_GET_CONFIGURATION:                                       // The actual configuration value is sent to HOST

              RequestedData = sizeof(CurrentConfiguration);

              dUsbSendViaControl((UBYTE *) &(CurrentConfiguration), sizeof(CurrentConfiguration));

              break;

    case STD_GET_STATUS_ZERO:

              wStatus = 0x01;                                         // Atmel has a 0x00, but we're not BUS-powered
              RequestedData = sizeof(wStatus);

              dUsbSendViaControl((UBYTE *) &wStatus, sizeof(wStatus));

              break;

    case STD_GET_STATUS_INTERFACE:                                    // Everything reset to zero (reserved)

              wStatus = 0;
              RequestedData = sizeof(wStatus);

              dUsbSendViaControl((UBYTE *) &wStatus, sizeof(wStatus));

              break;

    case STD_GET_STATUS_ENDPOINT:

              wStatus = 0;
              RequestedData = sizeof(wStatus);
              wIndex &= 0x0F;                                                 // Mask the endpoint #

              if (((*AT91C_UDP_GLBSTATE) & AT91C_UDP_CONFG) && (wIndex <= 3)) // If device in CONFIGURED state
              {                                                               // and ENDPOINT selected in valid range

                switch (wIndex)
                {

                  case 1: wStatus = ((*AT91C_UDP_CSR1) & AT91C_UDP_EPEDS) ? 0 : 1;  // If an endpoint is halted, the HALT
                                                                                    // feature is set to 1, else reset
                          break;

                  case 2: wStatus = ((*AT91C_UDP_CSR2) & AT91C_UDP_EPEDS) ? 0 : 1;

                          break;

                  case 3: wStatus = ((*AT91C_UDP_CSR3) & AT91C_UDP_EPEDS) ? 0 : 1;

                          break;
                  default:
                          // We'll never come here, but we'll never say never.......
                          break;
                }

                dUsbSendViaControl((UBYTE *) &wStatus, sizeof(wStatus));

              }

              else if (((*AT91C_UDP_GLBSTATE) & AT91C_UDP_FADDEN) && (wIndex == 0))
              {
                wStatus = ((*AT91C_UDP_CSR0) & AT91C_UDP_EPEDS) ? 0 : 1;            // Return 1 if device in ADRESSED state

                dUsbSendViaControl((UBYTE *) &wStatus, sizeof(wStatus));
              }
              else

                dUsbSendStall();                                // Illegal request :-(

              break;

    case STD_SET_FEATURE_ZERO:

              dUsbSendStall();                                  // Illegal request :-(

              break;

    case STD_SET_FEATURE_INTERFACE:

              dUsbSendZeroLengthPackage();                      // TextBook

              break;

    case STD_SET_FEATURE_ENDPOINT:

              wIndex &= 0x0F;

              if ((wValue == 0) && wIndex && (wIndex <= 3))     // Feature Selector = 0 ENDPOINT HALT and
              {                                                 // endpoint isolated and validated

                switch (wIndex)
                {

                case 1:   (*AT91C_UDP_CSR1) = 0;

                          break;

                case 2:   (*AT91C_UDP_CSR2) = 0;

                          break;

                case 3:   (*AT91C_UDP_CSR3) = 0;

                          break;

                default:
                          // We'll never come here, but we'll never say never.......
                break;

                }

                dUsbSendZeroLengthPackage();

              }
              else

                dUsbSendStall();                              // Illegal request :-(

              break;

    case STD_CLEAR_FEATURE_ZERO:

              dUsbSendStall();                                // Illegal request :-(

              break;

    case STD_CLEAR_FEATURE_INTERFACE:

              dUsbSendZeroLengthPackage();                    // No special

              break;

    case STD_CLEAR_FEATURE_ENDPOINT:

              wIndex &= 0x0F;

              if ((wValue == 0) && wIndex && (wIndex <= 3))   // Feature Selector = 0 => ENABLE A HALTED endpoint
              {                                               // and endpoint isolated and validated

                if (wIndex == 1)
                  (*AT91C_UDP_CSR1) = (AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_BULK_OUT);  // On duty again
                else if (wIndex == 2)
                  (*AT91C_UDP_CSR2) = (AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_BULK_IN);   // -
                else if (wIndex == 3)
                  (*AT91C_UDP_CSR3) = (AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_INT_IN);    // -

                dUsbSendZeroLengthPackage();

              }
              else

                dUsbSendStall();				// Illegal request :-(

              break;

    default:

              dUsbSendStall();					// Illegal request :-(

              break;
    }
}

UBYTE dUsbIsConfigured(void)
{

	if (*AT91C_UDP_ISR & END_OF_BUS_RESET)        // If "End Of Bus Reset Interrupt"
	{                                             // Somebody fallen in the wire? ;-)


		*AT91C_UDP_ICR = END_OF_BUS_RESET;          // Reset "End Of Bus Reset Interrupt"
		*AT91C_UDP_ICR = SUSPEND_RESUME;            // State unknown after reset, so we better clear
		*AT91C_UDP_ICR = WAKEUP;                    // As above

		CurrentConfiguration = 0;                   // We're new and ready
		UsbConnectionStates = USB_NOT_CONFIGURED;

		*AT91C_UDP_RSTEP = 0xFFFFFFFF;              // Reset all implemented endpoints "and a few more"
		*AT91C_UDP_RSTEP = 0x0;                     // Restored as zeroes
													// Below our main crash thing, if it is missing ;-)
		CurrentReceiveBank = AT91C_UDP_RX_DATA_BK0; // Start the PING-PONG buffers at a known state and order

		*AT91C_UDP_FADDR = AT91C_UDP_FEN;           // Set FEN in the Function Address Register
													// USB device is able to receive and transfer data

		*AT91C_UDP_CSR0 = (AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_CTRL);  // Configure endpoint 0
																	// AT91C_UDP_EPEDS = Endpoint enable
																	// AT91C_UDP_EPTYPE_CTRL = Endpoint type CONTROL
	}
	
	else if (*AT91C_UDP_ISR & SUSPEND_INT)
	{
		if (UsbConnectionStates == USB_CONFIGURED)
		{
			UsbConnectionStates = USB_CONFIGURED_BUT_SUSPENDED;
		}
		else
		{
			UsbConnectionStates = USB_NOT_CONFIGURED;
		}

		*AT91C_UDP_ICR = SUSPEND_INT;
		CurrentReceiveBank = AT91C_UDP_RX_DATA_BK0; // Start the PING-PONG buffers at a known state and order
	}

    else if (*AT91C_UDP_ISR & SUSPEND_RESUME)
    {
        if (UsbConnectionStates == USB_CONFIGURED_BUT_SUSPENDED)
		{
			UsbConnectionStates = USB_CONFIGURED;
		}
        else
		{
			UsbConnectionStates = USB_NOT_CONFIGURED;
		}

        *AT91C_UDP_ICR = WAKEUP;
        *AT91C_UDP_ICR = SUSPEND_RESUME;
    }
	
	else if (*AT91C_UDP_ISR &  AT91C_UDP_EPINT0)	// If "Endpoint 0 Interrupt"
	{
		*AT91C_UDP_ICR = AT91C_UDP_EPINT0;          // Reset "Endpoint 0 Interrupt"
		if (BrickNameKnown)
			dUsbEnumerate();						// Let's date & exchange "personal data"
	}
	
	if (UsbConnectionStates == USB_CONFIGURED)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}


void      dUsbInsertHandle(UBYTE Handle)
{
  UBYTE   Tmp;

  Tmp = 0;
  while((UsbHandleList[Tmp] != MAX_HANDLES) && (Tmp < MAX_HANDLES))
  {
    Tmp++;
  }
  UsbHandleList[Tmp] = Handle;
}

void      dUsbRemoveHandle(UBYTE Handle)
{
  UBYTE   Tmp;

  Tmp = 0;
  while (Tmp < MAX_HANDLES)
  {
    if (Handle == UsbHandleList[Tmp])
    {
      UsbHandleList[Tmp] = MAX_HANDLES;
    }
    Tmp++;
  }
}

UWORD     dUsbGetFirstHandle(void)
{
  UWORD   RtnVal;

  UsbHandleCnt = 0;
  RtnVal = dUsbGetNextHandle();

  return(RtnVal);
}

UWORD     dUsbGetNextHandle(void)
{
  UBYTE   Tmp;
  UWORD   RtnVal;

  RtnVal = 0;
  Tmp = UsbHandleCnt;
  while((Tmp < MAX_HANDLES) && (MAX_HANDLES == UsbHandleList[Tmp]))
  {
    Tmp++;
  }
  UsbHandleCnt = Tmp + 1;

  if (Tmp < MAX_HANDLES)
  {
    RtnVal |= UsbHandleList[Tmp];
  }
  else
  {
    RtnVal = 0x8100;
  }

  return(RtnVal);
}

UWORD     dUsbCheckConnection(void)
{
  UWORD   ADValue;
  UWORD   Return;

  Return = FALSE;
  USBReadADCValue(&ADValue);

  if (ADValue > 512)
  {
    Return = TRUE;
  }
  return(Return);
}

void dUsbInit(void)
{
  UBYTE   Tmp;

  // We could come from a SAMBA session and then we need
  // to "introduce ourself in a polite way for the PNP manager
  // We will pull the carpet and start a new session by removing
  // the pull up of the D+ wire

  BrickNameKnown = FALSE;
  dUsbStartTimeoutTimer();								// Let H/W settle
  dUsbDisconnect();										// Pull the carpet
  while(!USBTimedOut);									// wait 1 mS.

  USBHwInit;											// New session

  CurrentConfiguration = 0;								// We're new born     
  UsbConnectionStates = USB_NOT_CONFIGURED;
  CurrentReceiveBank   = AT91C_UDP_RX_DATA_BK0;			// Always start from Bank 0
  RequestedData = 0;

  for(Tmp = 0; Tmp < MAX_HANDLES; Tmp++)
  {
    UsbHandleList[Tmp] = MAX_HANDLES;
  }
}

void      dUsbResetConfig(void)
{
  CurrentConfiguration = 0;								// We've lost the connection
  UsbConnectionStates = USB_NOT_CONFIGURED;
}

void dUsbExit(void)
{
//  USBExit;
}
