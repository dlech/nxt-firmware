/**************************************************************
 **             - INAVR.H -
 **
 **     Intrinsics for iccAVR
 **
 **     Used with iccAVR.
 **
 **     Copyright IAR Systems 1999. All rights reserved.
 **
 **     File version: $Revision: 1 $
 **
 **************************************************************/

#ifndef __INAVR_H
#define __INAVR_H

#ifndef __ICCAVR__
#error This file should only be compiled with iccAVR
#endif /* __ICCAVR__ */

__intrinsic void __no_operation(void);
__intrinsic void __enable_interrupt(void);
__intrinsic void __disable_interrupt(void);
__intrinsic void __sleep(void);
__intrinsic void __watchdog_reset(void);
#define __clear_watchdog_timer()  __watchdog_reset()
#pragma language=extended
__intrinsic unsigned char __load_program_memory(const unsigned char __flash *);
#ifdef __HAS_ELPM__
__intrinsic unsigned char __extended_load_program_memory(
                                             const unsigned char __farflash *);
#endif
#pragma language=default

__intrinsic void __insert_opcode(unsigned short op);

#if __MEMORY_MODEL__ == 4
#if __CPU__ < 2
#define __STR_MATTR__ __flash
#else
#define __STR_MATTR__ __hugeflash
#endif
#else
#define __STR_MATTR__
#endif


__intrinsic void __require(void *);

__intrinsic void __delay_cycles(unsigned long);

__intrinsic unsigned char __save_interrupt(void);
#define __get_interrupt_state() __save_interrupt()

__intrinsic void          __restore_interrupt(unsigned char);
#define __set_interrupt_state(STATE) __restore_interrupt(STATE)

__intrinsic unsigned char __swap_nibbles(unsigned char);

__intrinsic void          __indirect_jump_to(unsigned long);

#ifdef __HAS_ENHANCED_CORE__

#ifdef __HAS_MUL__
__intrinsic unsigned int  __multiply_unsigned(unsigned char, unsigned char);
__intrinsic signed int    __multiply_signed(signed char, signed char);
__intrinsic signed int    __multiply_signed_with_unsigned(signed char, unsigned char);

__intrinsic unsigned int  __fractional_multiply_unsigned(unsigned char, unsigned char);
__intrinsic signed int    __fractional_multiply_signed(signed char, signed char);
__intrinsic signed int    __fractional_multiply_signed_with_unsigned(signed char, signed char);
#endif

#pragma language=extended

/* SPM */
__intrinsic void __DataToR0ByteToSPMCR_SPM(unsigned char data, 
                                           unsigned char byte);
__intrinsic void __AddrToZByteToSPMCR_SPM(void __flash* addr, 
                                          unsigned char byte);
__intrinsic void __AddrToZWordToR1R0ByteToSPMCR_SPM(void __flash* addr, 
                                                    unsigned short word, 
                                                    unsigned char byte);

#define _SPM_LOCKBITS(Data)  \
  __DataToR0ByteToSPMCR_SPM((Data), 0x09)

#define _SPM_ERASE(Addr) \
  __AddrToZByteToSPMCR_SPM((void __flash*)(Addr), 0x03)

#define _SPM_FILLTEMP(Addr,Data)  \
  __AddrToZWordToR1R0ByteToSPMCR_SPM((void __flash*)(Addr), (Data), 0x01)

#define _SPM_PAGEWRITE(Addr) \
  __AddrToZByteToSPMCR_SPM((void __flash*)(Addr), (0x05))


__intrinsic unsigned char __AddrToZByteToSPMCR_LPM(void __flash* addr, 
                                                   unsigned char byte);

#define _SPM_GET_LOCKBITS()  \
  __AddrToZByteToSPMCR_LPM((void __flash*)0x0001, 0x09)

#define _SPM_GET_FUSEBITS()  \
  __AddrToZByteToSPMCR_LPM((void __flash*)0x0000, 0x09)


#ifdef __HAS_ELPM__
__intrinsic void __AddrToZ24ByteToSPMCR_SPM(void __farflash* addr, 
                                            unsigned char byte);
__intrinsic void __AddrToZ24WordToR1R0ByteToSPMCR_SPM(void __farflash* addr, 
                                                      unsigned short word, 
                                                      unsigned char byte);
#define _SPM_24_ERASE(Addr) \
  __AddrToZ24ByteToSPMCR_SPM((void __farflash*)(Addr), 0x03)

#define _SPM_24_FILLTEMP(Addr,Data)  \
  __AddrToZ24WordToR1R0ByteToSPMCR_SPM((void __farflash*)(Addr), (Data), 0x01)

#define _SPM_24_PAGEWRITE(Addr) \
  __AddrToZ24ByteToSPMCR_SPM((void __farflash*)(Addr), (0x05))

__intrinsic unsigned char __AddrToZ24ByteToSPMCR_ELPM(void __farflash* addr, 
                                                      unsigned char byte);
#endif
#pragma language=default

#endif //__HAS_ENHANCED_CORE__

/* Include a file appropriate for the processor used, 
 * that defines EECR, EEAR and EEDR (e.g. io2312.h). */
#ifdef __HAS_EEPROM__
#define __EEPUT(ADR,VAL)  (*((unsigned char __eeprom *)ADR) = VAL)
#define __EEGET(VAR, ADR) (VAR = *((unsigned char __eeprom *)ADR))
#else /* !__HAS_EEPROM__ */
#define __EEPUT(ADR,VAL)  {while (EECR & 0x02); \
 EEAR = (ADR); EEDR = (VAL); EECR = 0x04; EECR = 0x02;}
 
#define __EEGET(VAR, ADR) {while (EECR & 0x02); \
        EEAR = (ADR); EECR = 0x01; (VAR) = EEDR;}
#endif /* __HAS_EEPROM__ */

/* PORT is a sfrb defined variable */
#define input(PORT) (PORT)
#define output(PORT,VAL) ((PORT)=(VAL))

#define input_block_dec(PORT,ADDRESS,COUNT)\
{ \
  unsigned char i;\
  unsigned char *addr=(ADDRESS);\
  for(i=0;i<(COUNT);i++)\
    *addr--=(PORT);\
}

#define input_block_inc(PORT,ADDRESS,COUNT)\
{ \
  unsigned char i;\
  unsigned char *addr=(ADDRESS);\
  for(i=0;i<(COUNT);i++)\
    *addr++=(PORT);\
}

#define output_block_dec(PORT,ADDRESS,COUNT)\
{ \
  unsigned char i;\
  unsigned char *addr=(ADDRESS);\
  for(i=0;i<(COUNT);i++)\
    (PORT)=*addr--;\
}

#define output_block_inc(PORT,ADDRESS,COUNT)\
{ \
  unsigned char i;\
  unsigned char *addr=(ADDRESS);\
  for(i=0;i<(COUNT);i++)\
    (PORT)=*addr++;\
}


//Nice to have macros 

#define __out_word(BaseName, value)\
{\
  unsigned char _tH=(value) >>   8;\
  unsigned char _tL=(value) & 0xFF;\
  BaseName ## H = _tH;\
  BaseName ## L = _tL;\
}


#define __out_word_atomic(BaseName, value)\
{\
 unsigned char _t=__save_interrupt();\
 __disable_interrupt();\
 __out_word(BaseName,value);\
 __restore_interrupt(_t);\
}

#define __in_word(BaseName, value)\
{\
 (value) = (BaseName ## L);\
 (value) |= (unsigned short)BaseName ## H << 8;\
}


#define __in_word_atomic(BaseName, value)\
{\
 unsigned char _t=__save_interrupt();\
 __disable_interrupt();\
 __in_word(BaseName, value);\
 __restore_interrupt(_t);\
}
#endif /* __INAVR_H */
