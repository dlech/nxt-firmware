/****************************************************************************
 **             - iom48.h -
 **
 **     This file declares the internal register addresses for ATmega48.
 **
 **     Used with iccAVR and aAVR.
 **
 **     Copyright IAR Systems 2003. All rights reserved.
 **
 **     File version: $Revision: 1 $
 **
 ***************************************************************************/

#include "iomacro.h"

#if TID_GUARD(1)
#error This file should only be compiled with iccavr or aavr whith processor option -v1
#endif /* TID_GUARD(1) */

/* Include the SFR part if this file has not been included before,
 * OR this file is included by the assembler (SFRs must be defined in
 * each assembler module). */
#if !defined(__IOM48_H) || defined(__IAR_SYSTEMS_ASM__)

#pragma language=extended

/*==========================*/
/* Predefined SFR Addresses */
/*==========================*/

/****************************************************************************
 * An example showing the SFR_B() macro call, 
 * the expanded result and usage of this result:
 *
 * SFR_B(AVR,   0x1F) Expands to:
 * __io union {
 *             unsigned char AVR;                 // The sfrb as 1 byte
 *             struct {                           // The sfrb as 8 bits
 *                     unsigned char AVR_Bit0:1,
 *                                   AVR_Bit1:1,
 *                                   AVR_Bit2:1,
 *                                   AVR_Bit3:1,
 *                                   AVR_Bit4:1,
 *                                   AVR_Bit5:1,
 *                                   AVR_Bit6:1,
 *                                   AVR_Bit7:1;
 *                    };
 *            } @ 0x1F;
 * Examples of how to use the expanded result:
 * AVR |= (1<<5);
 * or like this:
 * AVR_Bit5 = 1;
 ***************************************************************************/


/* Extended I/O space */
SFR_B(UDR0,     0xC6)      /* USART0 I/O Data Register */
SFR_W(UBRR0,    0xC4)      /* USART0 Baud Rate Register */

SFR_B(UCSR0C,   0xC2)      /* USART0 Control and Status Register C */
SFR_B(UCSR0B,   0xC1)      /* USART0 Control and Status Register B */
SFR_B(UCSR0A,   0xC0)      /* USART0 Control and Status Register A */

SFR_B(TWAMR,    0xBD)      /* 2-wire Serial Interface */
SFR_B(TWCR,     0xBC)      /* 2-wire Serial Interface Control Register */
SFR_B(TWDR,     0xBB)      /* 2-wire Serial Interface Data Register */
SFR_B(TWAR,     0xBA)      /* 2-wire Serial Interface Address Register */
SFR_B(TWSR,     0xB9)      /* 2-wire Serial Interface Status Register */
SFR_B(TWBR,     0xB8)      /* 2-wire Serial Interface Bit Rate Register */

SFR_B(ASSR,     0xB6)      /* Asynchronous mode Status Register */

SFR_B(OCR2B,    0xB4)      /* Timer/Counter 2 Output Compare Register B  */
SFR_B(OCR2A,    0xB3)      /* Timer/Counter 2 Output Compare Register A  */
SFR_B(TCNT2,    0xB2)      /* Timer/Counter 2 */
SFR_B(TCCR2B,   0xB1)      /* Timer/Counter 2 Control Register B */
SFR_B(TCCR2A,   0xB0)      /* Timer/Counter 2 Control Register A */

SFR_W(OCR1B,    0x8A)      /* Timer/Counter 1 - Output Compare Register B */
SFR_W(OCR1A,    0x88)      /* Timer/Counter 1 - Output Compare Register A */
SFR_W(ICR1,     0x86)      /* Timer/Counter 1 - Input Capture Register */
SFR_W(TCNT1,    0x84)      /* Timer/Counter 1 - Counter Register */

SFR_B(TCCR1C,   0x82)      /* Timer/Counter 1 Control Register C */
SFR_B(TCCR1B,   0x81)      /* Timer/Counter 1 Control Register B */
SFR_B(TCCR1A,   0x80)      /* Timer/Counter 1 Control Register A */
SFR_B(DIDR1,    0x7F)      /* Digital Input Disable Register 1 */
SFR_B(DIDR0,    0x7E)      /* Digital Input Disable Register 0 */

SFR_B(ADMUX,    0x7C)      /* ADC Multiplexer Selection Register */
SFR_B(ADCSRB,   0x7B)      /* ADC Control and Status Register B */
SFR_B(ADCSRA,   0x7A)      /* ADC Control and Status Register A */
SFR_W(ADC,      0x78)      /* ADC Data Register */

SFR_B(TIMSK2,   0x70)      /* Timer/Counter 2 Interrupt Mask Register */
SFR_B(TIMSK1,   0x6F)      /* Timer/Counter 1 Interrupt Mask Register */
SFR_B(TIMSK0,   0x6E)      /* Timer/Counter 0 Interrupt Mask Register */
SFR_B(PCMSK2,   0x6D)      /* Pin Change Mask Register 2 */
SFR_B(PCMSK1,   0x6C)      /* Pin Change Mask Register 1 */
SFR_B(PCMSK0,   0x6B)      /* Pin Change Mask Register 0 */

SFR_B(EICRA,    0x69)      /* External Interrupt Control Register A */
SFR_B(PCICR,    0x68)      /* Pin Change Interrupt Control Register */

SFR_B(OSCCAL,   0x66)      /* Oscillator Calibration Register */

SFR_B(PRR,      0x64)      /*  */

SFR_B(CLKPR,    0x61)      /* System Clock Prescaler */
SFR_B(WDTCSR,   0x60)      /* Watchdog Timer Control and Status Register */

/* Ordinary I/O space */
SFR_B(SREG,     0x3F)      /* Status Register */
SFR_W(SP,       0x3D)      /* Stack Pointer */

SFR_B(SPMCSR,   0x37)      /* Store Program Memory Control Register */

SFR_B(MCUCR,    0x35)      /* MCU Control Register */
SFR_B(MCUSR,    0x34)      /* MCU Status Register */
SFR_B(SMCR,     0x33)      /* Sleep Mode Control Register */

SFR_B(MONDR,    0x31)      /* Monitor Data Register */
SFR_B(ACSR,     0x30)      /* Analog Comparator Control and Status Register */

SFR_B(SPDR,     0x2E)      /* SPI Data Register  */
SFR_B(SPSR,     0x2D)      /* SPI Status Register */
SFR_B(SPCR,     0x2C)      /* SPI Control Register */
SFR_B(GPIOR2,   0x2B)      /* General Purpose I/O Register 2 */
SFR_B(GPIOR1,   0x2A)      /* General Purpose I/O Register 1 */

SFR_B(OCR0B,    0x28)      /* Timer/Counter 0 Output Compare Register B */
SFR_B(OCR0A,    0x27)      /* Timer/Counter 0 Output Compare Register A */
SFR_B(TCNT0,    0x26)      /* Timer/Counter 0 (8-bit) */
SFR_B(TCCR0B,   0x25)      /* Timer/Counter 0 Control Register B */
SFR_B(TCCR0A,   0x24)      /* Timer/Counter 0 Control Register A */
SFR_B(GTCCR,    0x23)      /* General Timer/Counter Control Register */
SFR_B(EEAR,     0x21)      /* EEPROM Address Register */
SFR_B(EEDR,     0x20)      /* EEPROM Data Register */
SFR_B(EECR,     0x1F)      /* EEPROM Control Register */
SFR_B(GPIOR0,   0x1E)      /* General Purpose I/O Register 0 */
SFR_B(EIMSK,    0x1D)      /* External Interrupt Mask Register */
SFR_B(EIFR,     0x1C)      /* External Interrupt Flag Register */
SFR_B(PCIFR,    0x1B)      /* Pin Change Interrupt Flag Register */

SFR_B(TIFR2,    0x17)      /* Timer/Counter 2 Interrupt Flag Register */
SFR_B(TIFR1,    0x16)      /* Timer/Counter 1 Interrupt Flag Register */
SFR_B(TIFR0,    0x15)      /* Timer/Counter 0 Interrupt Flag Register */

SFR_B(PORTD,    0x0B)      /* Data Register, Port D */
SFR_B(DDRD,     0x0A)      /* Data Direction Register, Port D */
SFR_B(PIND,     0x09)      /* Input Pins, Port D */
SFR_B(PORTC,    0x08)      /* Data Register, Port C */
SFR_B(DDRC,     0x07)      /* Data Direction Register, Port C */
SFR_B(PINC,     0x06)      /* Input Pins, Port C */
SFR_B(PORTB,    0x05)      /* Data Register, Port B */
SFR_B(DDRB,     0x04)      /* Data Direction Register, Port B */
SFR_B(PINB,     0x03)      /* Input Pins, Port B */


#ifndef __IOM48_H
#define __IOM48_H


/* SFRs are local in assembler modules (so this file may need to be */
/* included in more than one module in the same source file), */
/* but #defines must only be made once per source file. */

/*==============================*/
/* Interrupt Vector Definitions */
/*==============================*/

/* NB! vectors are specified as byte addresses */

#define  RESET_vect           (0x00) /*  External Pin, Power-on Reset, Brownout
                                         Reset and Watchdog Reset */
#define  INT0_vect            (0x02) /* External Interrupt Request 0 */
#define  INT1_vect            (0x04) /* External Interrupt Request 1 */
#define  PCINT0_vect          (0x06) /* Pin Change Interrupt Request 0 */
#define  PCINT1_vect          (0x08) /* Pin Change Interrupt Request 1 */
#define  PCINT2_vect          (0x0A) /* Pin Change Interrupt Request 2 */
#define  WDT_vect             (0x0C) /* Watchdog Time-out Interrupt */
#define  TIMER2_COMPA_vect    (0x0E) /* Timer/Counter2 Compare Match A */
#define  TIMER2_COMPB_vect    (0x10) /* Timer/Counter2 Compare Match B */
#define  TIMER2_OVF_vect      (0x12) /* Timer/Counter2 Overflow */
#define  TIMER1_CAPT_vect     (0x14) /* Timer/Counter1 Capture Event */
#define  TIMER1_COMPA_vect    (0x16) /* Timer/Counter1 Compare Match A */
#define  TIMER1_COMPB_vect    (0x18) /* Timer/Coutner1 Compare Match B */
#define  TIMER1_OVF_vect      (0x1A) /* Timer/Counter1 Overflow */
#define  TIMER0_COMPA_vect    (0x1C) /* Timer/Counter0 Compare Match A */
#define  TIMER0_COMPB_vect    (0x1E) /* Timer/Counter0 Compare Match B */
#define  TIMER0_OVF_vect      (0x20) /* Timer/Counter0 Overflow */
#define  SPI_STC_vect         (0x22) /* SPI Serial Transfer Complete */
#define  USART_RX_vect        (0x24) /* USART Rx Complete */
#define  USART_UDRE_vect      (0x26) /* USART, Data Register Empty */
#define  USART_TX_vect        (0x28) /* USART, Tx Complete */
#define  ADC_vect             (0x2A) /* ADC Conversion Complete */
#define  EE_RDY_vect          (0x2C) /* EEPROM Ready */
#define  ANA_COMP_vect        (0x2E) /* Analog Comparator */
#define  TWI_vect             (0x30) /* 2-wire Serial Interface */
#define  SPM_READY_vect       (0x32) /* Store Program Memory Ready */

#ifdef __IAR_SYSTEMS_ASM__   
#ifndef ENABLE_BIT_DEFINITIONS
#define  ENABLE_BIT_DEFINITIONS
#endif /* ENABLE_BIT_DEFINITIONS */
#endif /* __IAR_SYSTEMS_ASM__ */

#ifdef ENABLE_BIT_DEFINITIONS


/* Bit definitions for use with the IAR Assembler   
   The Register Bit names are represented by their bit number (0-7).
*/

/*UCSR0C*/
#define UMSEL01   7
#define UMSEL00   6
#define UPM01     5
#define UPM00     4
#define USBS0     3
#define UDORD0    2
#define UCPHA0    1
#define UCPOL0    0

#define UCSZ01    UDORD0
#define UCSZ00    UCPHA0

/*UCSR0B*/
#define RXCIE0    7
#define TXCIE0    6
#define UDRIE0    5
#define RXEN0     4
#define TXEN0     3
#define UCSZ02    2
#define RXB80     1
#define TXB80     0

/*UCSR0A*/
#define RXC0      7
#define TXC0      6
#define UDRE0     5
#define FE0       4
#define DOR0      3
#define UPE0      2
#define U2X0      1
#define MPCM0     0


/*TWAMR*/
#define TWAM6     7
#define TWAM5     6
#define TWAM4     5
#define TWAM3     4
#define TWAM2     3
#define TWAM1     2
#define TWAM0     1

/*TWCR*/
#define TWINT     7
#define TWEA      6
#define TWSTA     5
#define TWSTO     4
#define TWWC      3
#define TWEN      2
#define TWIE      0

/*TWAR*/
#define TWA6      7
#define TWA5      6
#define TWA4      5
#define TWA3      4
#define TWA2      3
#define TWA1      2
#define TWA0      1
#define TWGCE     0

/*TWSR*/
#define TWS7      7
#define TWS6      6
#define TWS5      5
#define TWS4      4
#define TWS3      3
#define TWPS1     1
#define TWPS0     0


/*ASSR*/
#define EXCLK     6
#define AS2       5
#define TCN2UB    4
#define OCR2AUB   3
#define OCR2BUB   2
#define TCR2AUB   1
#define TCR2BUB   0


/*TCCR2B*/
#define FOC2A     7
#define FOC2B     6
#define WGM22     3
#define CS22      2
#define CS21      1
#define CS20      0

/*TCCR2A*/
#define COM2A1    7
#define COM2A0    6
#define COM2B1    5
#define COM2B0    4
#define WGM21     1
#define WGM20     0


/*TCCR1C*/
#define FOC1A     7
#define FOC1B     6

/*TCCR1B*/
#define ICNC1     7
#define ICES1     6
#define WGM13     4
#define WGM12     3
#define CS12      2
#define CS11      1
#define CS10      0

/*TCCR1A*/
#define COM1A1    7
#define COM1A0    6
#define COM1B1    5
#define COM1B0    4
#define WGM11     1
#define WGM10     0

/*DIDR1*/
#define AIN1D     1
#define AIN0D     0

/*DIDR0*/
#define ADC5D     5
#define ADC4D     4
#define ADC3D     3
#define ADC2D     2
#define ADC1D     1
#define ADC0D     0


/*ADMUX*/
#define REFS1     7
#define REFS0     6
#define ADLAR     5
#define MUX3      3
#define MUX2      2
#define MUX1      1
#define MUX0      0

/*ADCSRB*/
#define ACME      6
#define ADTS2     2
#define ADTS1     1
#define ADTS0     0

/*ADCSRA*/
#define ADEN      7
#define ADSC      6
#define ADATE     5
#define ADIF      4
#define ADIE      3
#define ADPS2     2
#define ADPS1     1
#define ADPS0     0


/*TIMSK2*/
#define OCIE2B    2
#define OCIE2A    1
#define TOIE2     0

/*TIMSK1*/
#define ICIE1     5
#define OCIE1B    2
#define OCIE1A    1
#define TOIE1     0

/*TIMSK0*/
#define OCIE0B    2
#define OCIE0A    1
#define TOIE0     0

/*PCMSK2*/
#define PCINT23   7
#define PCINT22   6
#define PCINT21   5
#define PCINT20   4
#define PCINT19   3
#define PCINT18   2
#define PCINT17   1
#define PCINT16   0

/*PCMSK1*/
#define PCINT14   6
#define PCINT13   5
#define PCINT12   4
#define PCINT11   3
#define PCINT10   2
#define PCINT9    1
#define PCINT8    0

/*PCMSK0*/
#define PCINT7    7
#define PCINT6    6
#define PCINT5    5
#define PCINT4    4
#define PCINT3    3
#define PCINT2    2
#define PCINT1    1
#define PCINT0    0


/*EICRA*/
#define ISC11     3
#define ISC10     2
#define ISC01     1
#define ISC00     0

/*PCICR*/
#define PCIE2     2
#define PCIE1     1
#define PCIE0     0


/*PRR*/
#define PRTW1     7
#define PRTIM2    6
#define PRTIM0    5
#define PRTIM1    3
#define PRSPI     2
#define PRUSART0  1
#define PRADC     0


/*CLKPR*/
#define CLKPCE    7
#define CLKPS3    3
#define CLKPS2    2
#define CLKPS1    1
#define CLKPS0    0

/*WDTCSR*/
#define WDIF      7
#define WDIE      6
#define WDP3      5
#define WDCE      4
#define WDE       3
#define WDP2      2
#define WDP1      1
#define WDP0      0

/* Ordinary I/O space */

/*SPH*/
#define SP9       1
#define SP8       0

/*SPL*/
#define SP7       7
#define SP6       6
#define SP5       5
#define SP4       4
#define SP3       3
#define SP2       2
#define SP1       1
#define SP0       0


/*SPMCSR*/
#define SPMIE     7
#define BLBSET    3
#define PGWRT     2
#define PGERS     1
#define SPMEN     0


/*MCUCR*/
#define PUD       4
#define IVSEL     1
#define IVCE      0

/*MCUSR*/
#define WDRF      3
#define BORF      2
#define EXTRF     1
#define PORF      0

/*SMCR*/
#define SM2       3
#define SM1       2
#define SM0       1
#define SE        0


/*ACSR*/
#define ACD       7
#define ACBG      6
#define ACO       5
#define ACI       4
#define ACIE      3
#define ACIC      2
#define ACIS1     1
#define ACIS0     0


/*SPSR*/
#define SPIF      7
#define WCOL      6
#define SPI2X     0

/*SPCR*/
#define SPIE      7
#define SPE       6
#define DORD      5
#define MSTR      4
#define CPOL      3
#define CPHA      2
#define SPR1      1
#define SPR0      0


/*TCCR0B*/
#define FOC0A     7
#define FOC0B     6
#define WGM02     3
#define CS02      2
#define CS01      1
#define CS00      0

/*TCCR0A*/
#define COM0A1    7
#define COM0A0    6
#define COM0B1    5
#define COM0B0    4
#define WGM01     1
#define WGM00     0

/*GTCCR*/
#define TSM       7
#define PSR2      1
#define PSR10     0


/*EECR*/
#define EERIE     3
#define EEMPE     2
#define EEPE      1
#define EERE      0

/*EIMSK*/
#define INT1      1
#define INT0      0

/*EIFR*/
#define INTF1     1
#define INTF0     0

/*PCIFR*/
#define PCIF2     2
#define PCIF1     1
#define PCIF0     0


/*TIFR2*/
#define OCF2B     2
#define OCF2A     1
#define TOV2      0

/*TIFR1*/
#define ICF1      5
#define OCF1B     2
#define OCF1A     1
#define TOV1      0

/*TIFR0*/
#define OCF0B     2
#define OCF0A     1
#define TOV0      0


/*PORTD*/
#define PORTD7    7
#define PORTD6    6
#define PORTD5    5
#define PORTD4    4
#define PORTD3    3
#define PORTD2    2
#define PORTD1    1
#define PORTD0    0

#define PD7       7
#define PD6       6
#define PD5       5
#define PD4       4
#define PD3       3
#define PD2       2
#define PD1       1
#define PD0       0

/*DDRD*/
#define DDD7      7
#define DDD6      6
#define DDD5      5
#define DDD4      4
#define DDD3      3
#define DDD2      2
#define DDD1      1
#define DDD0      0

/*PIND*/
#define PIND7     7
#define PIND6     6
#define PIND5     5
#define PIND4     4
#define PIND3     3
#define PIND2     2
#define PIND1     1
#define PIND0     0

/*PORTC*/
#define PORTC6    6
#define PORTC5    5
#define PORTC4    4
#define PORTC3    3
#define PORTC2    2
#define PORTC1    1
#define PORTC0    0

#define PC6       6
#define PC5       5
#define PC4       4
#define PC3       3
#define PC2       2
#define PC1       1
#define PC0       0

/*DDRC*/
#define DDC6      6
#define DDC5      5
#define DDC4      4
#define DDC3      3
#define DDC2      2
#define DDC1      1
#define DDC0      0

/*PINC*/
#define PINC6     6
#define PINC5     5
#define PINC4     4
#define PINC3     3
#define PINC2     2
#define PINC1     1
#define PINC0     0

/*PORTB*/
#define PORTB7    7
#define PORTB6    6
#define PORTB5    5
#define PORTB4    4
#define PORTB3    3
#define PORTB2    2
#define PORTB1    1
#define PORTB0    0

#define PB7       7
#define PB6       6
#define PB5       5
#define PB4       4
#define PB3       3
#define PB2       2
#define PB1       1
#define PB0       0

/*DDRB*/
#define DDB7      7
#define DDB6      6
#define DDB5      5
#define DDB4      4
#define DDB3      3
#define DDB2      2
#define DDB1      1
#define DDB0      0

/*PINB*/
#define PINB7     7
#define PINB6     6
#define PINB5     5
#define PINB4     4
#define PINB3     3
#define PINB2     2
#define PINB1     1
#define PINB0     0


/* Extended Fuse Byte */
#define SELFPRGEN 0

/* High Fuse Byte */
#define RSTDISBL  7
#define DWEN      6
#define SPIEN     5
#define WDTON     4
#define EESAVE    3
#define BODLEVEL2 2
#define BODLEVEL1 1
#define BODLEVEL0 0

/* Low Fuse Byte */
#define CKDIV8    7
#define CKOUT     6
#define SUT1      5
#define SUT0      4
#define CKSEL3    3
#define CKSEL2    2
#define CKSEL1    1
#define CKSEL0    0

/* Pointer definition */
#define    XL     r26
#define    XH     r27
#define    YL     r28
#define    YH     r29
#define    ZL     r30
#define    ZH     r31

/* Contants */
#define    RAMEND   0x02FF    /*Last On-Chip SRAM location*/
#define    XRAMEND  0x02FF
#define    E2END    0x00FF
#define    FLASHEND 0x0FFF

#endif /* ENABLE_BIT_DEFINITIONS */ 
#endif /* __IOM48_H (define part) */
#endif /* __IOM48_H (SFR part) */
