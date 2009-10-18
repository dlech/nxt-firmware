//
// Programmer
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dkpechri                                        $
//
// Revision date   $Date:: 19-07-06 10:02                                    $
//
// Filename        $Workfile:: d_usb.r                                       $
//
// Version         $Revision:: 9                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_usb.r    $
//
// Platform        C
//

#ifdef    SAM7S256

#ifdef    PROTOTYPE_PCB_3
#define   ENABLEUsbPU   *AT91C_PIOA_PER   = AT91C_PIO_PA16;     /* PIO allowed to control bit 16 */\
                        *AT91C_PIOA_OER   = AT91C_PIO_PA16;     /* Output pin 16 enabled */\
                        *AT91C_PIOA_SODR  = AT91C_PIO_PA16      /* Pin 16 set = enable USB pull-up */
#endif

#ifdef    PROTOTYPE_PCB_4
#define   ENABLEUsbPU   *AT91C_PIOA_PER   = AT91C_PIO_PA16;     /* PIO allowed to control bit 16 */\
                        *AT91C_PIOA_OER   = AT91C_PIO_PA16;     /* Output pin 16 enabled */\
                        *AT91C_PIOA_CODR  = AT91C_PIO_PA16      /* Pin 16 clear = enable USB pull-up */

#define   DISABLEUsbPU  *AT91C_PIOA_PER   = AT91C_PIO_PA16;     /* PIO allowed to control bit 16 */\
                        *AT91C_PIOA_OER   = AT91C_PIO_PA16;     /* Output pin 16 enabled */\
                        *AT91C_PIOA_SODR  = AT91C_PIO_PA16      /* Pin 16 set = disable USB pull-up */
#endif


#define   USBHwInit     *AT91C_CKGR_PLLR |= AT91C_CKGR_USBDIV_1;    /* Set the PLL USB Divider (96MHz/2) */\
                        *AT91C_PMC_SCER = AT91C_PMC_UDP;            /* WRITE-ONLY REG! Enables the 48MHz USB clock UDPCK (SysClk) */\
                        *AT91C_PMC_PCER = (1 << AT91C_ID_UDP);      /* WRITE-ONLY REG! Enable USB clock (Peripheral Clock) */\
                        \
                        /* Enable UDP PullUp (USB_DP_PUP) : enable & Clear of the corresponding PIO */  \
                        \
                        /* Removed 22022006 14:20 pc ENABLEUsbPU BlueCore delay , No pull up before OK serial-no rec. from B.C.*/


static    ULONG         USBTimeOut;

#define   USBTimedOut   (USB_TIMEOUT < ((((*AT91C_PITC_PIIR) & AT91C_PITC_CPIV) - USBTimeOut) & AT91C_PITC_CPIV))

#define   USBGetActualTime  USBTimeOut = ((*AT91C_PITC_PIIR) & AT91C_PITC_CPIV)

#define   USBReadADCValue(ADValue)  *ADValue = *AT91C_ADC_CDR4	

#define   USBExit

#define   USBDisconnect   DISABLEUsbPU

#define   USBConnect      ENABLEUsbPU

#endif

#ifdef    PCWIN

#endif
