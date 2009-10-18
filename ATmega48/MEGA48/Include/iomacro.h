/**************************************************************
 **             - iomacro.h -
 **
 **     This file defines the Special Function Register Macros
 **     for Atmel AT90S.
 **
 **     Used with iccAVR and aAVR.
 **
 **     Copyright IAR Systems 1999. All rights reserved.
 **
 **     File version: $Revision: 1 $
 **
 **************************************************************/

#ifndef __IOMACRO_H
#define __IOMACRO_H

#define TID_GUARD(proc) ((__TID__ & 0x7FF0) != ((90 << 8) | ((proc) << 4)))

#if !(__IAR_SYSTEMS_ICC__) && !defined(__IAR_SYSTEMS_ASM__)
#error This file should only be compiled with iccavr,icca90 or aavr.
#endif /* !(__IAR_SYSTEMS_ICC__ > 2) && !defined __IAR_SYSTEMS_ASM__ */

/* The assembler uses a special set of macros... */
#ifdef __IAR_SYSTEMS_ASM__

/* Byte sized SFRs */
#define SFR_B_BITS(_NAME,_ADDR,_A,_B,_C,_D,_E,_F,_G,_H)\
	sfrb	_NAME = _ADDR
#define SFR_B_BITS_EXT(_NAME,_ADDR,_A,_B,_C,_D,_E,_F,_G,_H)\
	sfrb	_NAME = _ADDR
#define SFR_B_BITS_EXT_IO(_NAME,_ADDR,_A,_B,_C,_D,_E,_F,_G,_H)\
	sfrb	_NAME = _ADDR
#define SFR_B2_BITS(_NAME1,_NAME2,_ADDR,_A,_B,_C,_D,_E,_F,_G,_H)\
	ASMSFRB2 _NAME1, _NAME2, _ADDR

#define SFR_B_BITS_N(_NAME,_ADDR,_A,_B,_C,_D,_E,_F,_G,_H, \
                                 _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2)\
	sfrb	_NAME = _ADDR
#define SFR_B_BITS_EXT_N(_NAME,_ADDR,_A,_B,_C,_D,_E,_F,_G,_H, \
                                     _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2)\
	sfrb	_NAME = _ADDR
#define SFR_B_BITS_EXT_IO_N(_NAME,_ADDR,_A,_B,_C,_D,_E,_F,_G,_H, \
                                     _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2)\
	sfrb	_NAME = _ADDR
#define SFR_B2_BITS_N(_NAME1,_NAME2,_ADDR,_A,_B,_C,_D,_E,_F,_G,_H, \
                                          _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2)\
	ASMSFRB2 _NAME1, _NAME2, _ADDR

ASMSFRB2 MACRO
	sfrb	\1 = \3
	sfrb	\2 = \3
	ENDM


/* Word sized SFRs, needs to be expanded into an assembler macro first. */
#define SFR_W_BITS(_NAME, _ADDR, _A,_B,_C,_D,_E,_F,_G,_H, _I,_J,_K,_L,_M,_N,_O,_P)\
	ASMSFRW	_NAME, _ADDR
#define SFR_W_BITS_EXT_IO(_ADDR, _NAME, _A,_B,_C,_D,_E,_F,_G,_H, _I,_J,_K,_L,_M,_N,_O,_P)\
	ASMSFRW	_NAME, _ADDR
#define SFR_W_BITS_N(_NAME, _ADDR, _A,_B,_C,_D,_E,_F,_G,_H, _I,_J,_K,_L,_M,_N,_O,_P, \
                                   _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2, \
                                   _I2,_J2,_K2,_L2,_M2,_N2,_O2,_P2)\
	ASMSFRW	_NAME, _ADDR

ASMSFRW	MACRO
	sfrw	\1  = \2
	sfrb	\1L = (\2+0)
	sfrb	\1H = (\2+1)
	ENDM

#endif /* __IAR_SYSTEMS_ASM__ */

#ifdef __ICCAVR__

#define __BYTEBITS(_NAME,_A,_B,_C,_D,_E,_F,_G,_H) \
unsigned char _NAME ## _ ## _A:1, \
              _NAME ## _ ## _B:1, \
              _NAME ## _ ## _C:1, \
              _NAME ## _ ## _D:1, \
              _NAME ## _ ## _E:1, \
              _NAME ## _ ## _F:1, \
              _NAME ## _ ## _G:1, \
              _NAME ## _ ## _H:1;

#define SFR_B_BITS(_NAME, _ADDR, _A,_B,_C,_D,_E,_F,_G,_H) \
    __io union { \
      unsigned char   _NAME;           /* The sfrb as 1 byte */ \
      struct {                        /* The sfrb as 8 bits */ \
        __BYTEBITS(_NAME, _A,_B,_C,_D,_E,_F,_G,_H) \
      };  \
    } @ _ADDR;

#define SFR_B_BITS_N(_NAME, _ADDR, _A,_B,_C,_D,_E,_F,_G,_H, \
                                   _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2) \
    __io union { \
      unsigned char   _NAME;           /* The sfrb as 1 byte */ \
      struct {                        /* The sfrb as 8 bits */ \
        __BYTEBITS(_NAME, _A,_B,_C,_D,_E,_F,_G,_H) \
      };  \
      struct {                        /* The sfrb as 8 bits */ \
        __BYTEBITS(_NAME, _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2) \
      };  \
    } @ _ADDR;

#define SFR_B2_BITS(_NAME1, _NAME2, _ADDR, _A,_B,_C,_D,_E,_F,_G,_H) \
    __io union { \
      unsigned char   _NAME1;           /* The sfrb as 1 byte */ \
      unsigned char   _NAME2;           /* The sfrb as 1 byte */ \
      struct {                        /* The sfrb as 8 bits */ \
        __BYTEBITS(_NAME1, _A,_B,_C,_D,_E,_F,_G,_H) \
      };  \
      struct {                        /* The sfrb as 8 bits */ \
        __BYTEBITS(_NAME2, _A,_B,_C,_D,_E,_F,_G,_H) \
      };  \
    } @ _ADDR;

#define SFR_B2_BITS_N(_NAME1, _NAME2, _ADDR, _A,_B,_C,_D,_E,_F,_G,_H, \
                                             _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2) \
    __io union { \
      unsigned char   _NAME1;           /* The sfrb as 1 byte */ \
      unsigned char   _NAME2;           /* The sfrb as 1 byte */ \
      struct {                        /* The sfrb as 8 bits */ \
        __BYTEBITS(_NAME1, _A,_B,_C,_D,_E,_F,_G,_H) \
      };  \
      struct {                        /* The sfrb as 8 bits */ \
        __BYTEBITS(_NAME2, _A,_B,_C,_D,_E,_F,_G,_H) \
      };  \
      struct {                        /* The sfrb as 8 bits */ \
        __BYTEBITS(_NAME1, _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2) \
      };  \
      struct {                        /* The sfrb as 8 bits */ \
        __BYTEBITS(_NAME2, _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2) \
      };  \
    } @ _ADDR;

#define SFR_B_BITS_EXT(_NAME, _ADDR, _A,_B,_C,_D,_E,_F,_G,_H) \
    __near __no_init volatile union { \
      unsigned char   _NAME;           /* The sfrb as 1 byte */ \
      struct {                        /* The sfrb as 8 bits */ \
        __BYTEBITS(_NAME, _A,_B,_C,_D,_E,_F,_G,_H) \
      };  \
    } @ _ADDR;

#define SFR_B_BITS_EXT_IO(_NAME, _ADDR, _A,_B,_C,_D,_E,_F,_G,_H) \
    __ext_io union { \
      unsigned char   _NAME;           /* The sfrb as 1 byte */ \
      struct {                        /* The sfrb as 8 bits */ \
        __BYTEBITS(_NAME, _A,_B,_C,_D,_E,_F,_G,_H) \
      };  \
    } @ _ADDR;


#define SFR_B_BITS_EXT_N(_NAME, _ADDR, _A,_B,_C,_D,_E,_F,_G,_H, \
                                       _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2) \
    __near __no_init volatile union { \
      unsigned char   _NAME;           /* The sfrb as 1 byte */ \
      struct {                        /* The sfrb as 8 bits */ \
        __BYTEBITS(_NAME, _A,_B,_C,_D,_E,_F,_G,_H) \
      };  \
      struct {                        /* The sfrb as 8 bits */ \
        __BYTEBITS(_NAME, _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2) \
      };  \
    } @ _ADDR;

#define SFR_B_BITS_EXT_IO_N(_NAME, _ADDR, _A,_B,_C,_D,_E,_F,_G,_H, \
                                       _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2) \
    __ext_io union { \
      unsigned char   _NAME;           /* The sfrb as 1 byte */ \
      struct {                        /* The sfrb as 8 bits */ \
        __BYTEBITS(_NAME, _A,_B,_C,_D,_E,_F,_G,_H) \
      };  \
      struct {                        /* The sfrb as 8 bits */ \
        __BYTEBITS(_NAME, _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2) \
      };  \
    } @ _ADDR;
  
#define SFR_W_BITS(_NAME, _ADDR, _A,_B,_C,_D,_E,_F,_G,_H, _I,_J,_K,_L,_M,_N,_O,_P) \
    __io union { \
      unsigned short  _NAME;  /* The sfrw as 1 short */ \
      struct {                /* The sfrw as 16 bits */ \
        __BYTEBITS(_NAME, _A,_B,_C,_D,_E,_F,_G,_H)   /* Bit names defined by user */  \
        __BYTEBITS(_NAME, _I,_J,_K,_L,_M,_N,_O,_P)   /* Bit names defined by user */  \
      };  \
      struct { /* The sfrw as 2 bytes */ \
        unsigned char _NAME ## L; \
        unsigned char _NAME ## H; \
      };  \
      struct {                          /* The sfrw as 2 x 8 bits */ \
        __BYTEBITS(_NAME ## L, Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7)  /* Bit names hard coded to 0-7 */ \
        __BYTEBITS(_NAME ## H, Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7)  /* Bit names hard coded to 0-7 */ \
      };  \
    } @ _ADDR;

#define SFR_W_BITS_EXT_IO(_ADDR, _NAME, _A,_B,_C,_D,_E,_F,_G,_H, _I,_J,_K,_L,_M,_N,_O,_P) \
    __ext_io union { \
      unsigned short  _NAME;  /* The sfrw as 1 short */ \
      struct {                /* The sfrw as 16 bits */ \
        __BYTEBITS(_NAME, _A,_B,_C,_D,_E,_F,_G,_H)   /* Bit names defined by user */  \
        __BYTEBITS(_NAME, _I,_J,_K,_L,_M,_N,_O,_P)   /* Bit names defined by user */  \
      };  \
      struct { /* The sfrw as 2 bytes */ \
        unsigned char _NAME ## L; \
        unsigned char _NAME ## H; \
      };  \
      struct {                          /* The sfrw as 2 x 8 bits */ \
        __BYTEBITS(_NAME ## L, Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7)  /* Bit names hard coded to 0-7 */ \
        __BYTEBITS(_NAME ## H, Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7)  /* Bit names hard coded to 0-7 */ \
      };  \
    } @ _ADDR;

#define SFR_W_BITS_N(_NAME, _ADDR, _A,_B,_C,_D,_E,_F,_G,_H, _I,_J,_K,_L,_M,_N,_O,_P, \
                                   _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2, \
                                   _I2,_J2,_K2,_L2,_M2,_N2,_O2,_P2) \
    __io union { \
      unsigned short  _NAME;  /* The sfrw as 1 short */ \
      struct {                /* The sfrw as 16 bits */ \
        __BYTEBITS(_NAME, _A,_B,_C,_D,_E,_F,_G,_H)   /* Bit names defined by user */  \
        __BYTEBITS(_NAME, _I,_J,_K,_L,_M,_N,_O,_P)   /* Bit names defined by user */  \
      };  \
      struct {                /* The sfrw as 16 bits */ \
        __BYTEBITS(_NAME, _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2)   /* Bit names defined by user */  \
        __BYTEBITS(_NAME, _I2,_J2,_K2,_L2,_M2,_N2,_O2,_P2)   /* Bit names defined by user */  \
      };  \
      struct { /* The sfrw as 2 bytes */ \
        unsigned char _NAME ## L; \
        unsigned char _NAME ## H; \
      };  \
      struct {                          /* The sfrw as 2 x 8 bits */ \
        __BYTEBITS(_NAME ## L, Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7)  /* Bit names hard coded to 0-7 */ \
        __BYTEBITS(_NAME ## H, Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7)  /* Bit names hard coded to 0-7 */ \
      };  \
      struct {                /* The sfrw as 2 x 8 bits */ \
        __BYTEBITS(_NAME ## L, _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2)   /* Bit names defined by user */  \
        __BYTEBITS(_NAME ## H, _I2,_J2,_K2,_L2,_M2,_N2,_O2,_P2)   /* Bit names defined by user */  \
      };  \
    } @ _ADDR;
#else
#ifndef __IAR_SYSTEMS_ASM__
 /* Special for the icca90 */

/* Byte sized SFRs */
#define SFR_B_BITS(_NAME,_ADDR,_A,_B,_C,_D,_E,_F,_G,_H)\
	sfrb _NAME = _ADDR;
#define SFR_B_BITS_EXT(_NAME,_ADDR,_A,_B,_C,_D,_E,_F,_G,_H)\
	sfrb _NAME = _ADDR;
#define SFR_B2_BITS(_NAME1,_NAME2,_ADDR,_A,_B,_C,_D,_E,_F,_G,_H)\
	sfrb _NAME1 = _ADDR; sfrb _NAME2 = _ADDR;

#define SFR_B_BITS_N(_NAME,_ADDR,_A,_B,_C,_D,_E,_F,_G,_H, \
                                 _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2)\
	sfrb	_NAME = _ADDR;
#define SFR_B_BITS_EXT_N(_NAME,_ADDR,_A,_B,_C,_D,_E,_F,_G,_H, \
                                     _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2)\
	sfrb	_NAME = _ADDR;
#define SFR_B2_BITS_N(_NAME1,_NAME2,_ADDR,_A,_B,_C,_D,_E,_F,_G,_H, \
                                          _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2)\
	sfrb _NAME1 = _ADDR; sfrb _NAME2 = _ADDR;

/* Word sized SFRs */
#define SFR_W_BITS(_NAME, _ADDR, _A,_B,_C,_D,_E,_F,_G,_H, _I,_J,_K,_L,_M,_N,_O,_P)\
	sfrw _NAME = _ADDR; sfrb _NAME##L = _ADDR; sfrb _NAME##H = (_ADDR+1);
#define SFR_W_BITS_N(_NAME, _ADDR, _A,_B,_C,_D,_E,_F,_G,_H, _I,_J,_K,_L,_M,_N,_O,_P, \
                                   _A2,_B2,_C2,_D2,_E2,_F2,_G2,_H2, \
                                   _I2,_J2,_K2,_L2,_M2,_N2,_O2,_P2)\
	sfrw _NAME = _ADDR; sfrb _NAME##L = _ADDR; sfrb _NAME##H = (_ADDR+1);

#endif
#endif /* !__ICCAVR__ */

#define SFR_B(_NAME, _ADDR) SFR_B_BITS(_NAME, _ADDR, \
                                    Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7)
/*
  SFR_B(SREG,   0x3F) Expands to:
  __io union {
              unsigned char   SREG;          // The sfrb as 1 byte
              struct {                        // The sfrb as 8 bits
                      unsigned char SREG_Bit0:1,
                                    SREG_Bit1:1,
                                    SREG_Bit2:1,
                                    SREG_Bit3:1,
                                    SREG_Bit4:1,
                                    SREG_Bit5:1,
                                    SREG_Bit6:1,
                                    SREG_Bit7:1;
                     };
             } @ 0x3F;
*/
#define SFR_B2(_NAME1, _NAME2, _ADDR) SFR_B2_BITS(_NAME1, _NAME2, _ADDR, \
                                    Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7)
#define SFR_B_EXT(_NAME, _ADDR) SFR_B_BITS_EXT(_NAME, _ADDR, \
                                    Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7)

#define SFR_W(_NAME, _ADDR)  SFR_W_BITS(_NAME, _ADDR, \
                                    Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7, \
                                    Bit8,Bit9,Bit10,Bit11,Bit12,Bit13,Bit14,Bit15)

#define SFR_B_R(_ADDR, _NAME) SFR_B_BITS(_NAME, _ADDR, \
                                    Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7)
#define SFR_B_EXT_IO_R(_ADDR, _NAME) SFR_B_BITS_EXT_IO(_NAME, _ADDR, \
                                    Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7)
#define SFR_W_EXT_IO_R(_NAME, _ADDR) SFR_W_BITS_EXT_IO(_NAME, _ADDR, \
                                    Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7, \
                                    Bit8,Bit9,Bit10,Bit11,Bit12,Bit13,Bit14,Bit15)
/*
  SFR_B(0x3F,   SREG) Expands to:
  __io union {
              unsigned char   SREG;          // The sfrb as 1 byte
              struct {                        // The sfrb as 8 bits
                      unsigned char SREG_Bit0:1,
                                    SREG_Bit1:1,
                                    SREG_Bit2:1,
                                    SREG_Bit3:1,
                                    SREG_Bit4:1,
                                    SREG_Bit5:1,
                                    SREG_Bit6:1,
                                    SREG_Bit7:1;
                     };
             } @ 0x3F;
*/
#define SFR_B2_R(_ADDR, _NAME1, _NAME2) SFR_B2_BITS(_NAME1, _NAME2, _ADDR, \
                                    Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7)
#define SFR_W_R(_ADDR, _NAME)  SFR_W_BITS(_NAME, _ADDR, \
                                    Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7, \
                                    Bit8,Bit9,Bit10,Bit11,Bit12,Bit13,Bit14,Bit15)

#define SFR_B_N(_ADDR, _NAME, _B7, _B6, _B5, _B4, _B3, _B2, _B1, _B0) \
                 SFR_B_BITS_N(_NAME, _ADDR, \
                              Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7, \
                              _B0,_B1,_B2,_B3,_B4,_B5,_B6,_B7)
/*
  SFR_B_N(0x3F,SREG,I,T,H,S,V,N,Z,C) Expands to:
  __io union {
              unsigned char   SREG;          // The sfrb as 1 byte
              struct {                        // The sfrb as 8 bits
                      unsigned char SREG_Bit0:1,
                                    SREG_Bit1:1,
                                    SREG_Bit2:1,
                                    SREG_Bit3:1,
                                    SREG_Bit4:1,
                                    SREG_Bit5:1,
                                    SREG_Bit6:1,
                                    SREG_Bit7:1;
                     };
              struct {                        // The sfrb as 8 bits
                      unsigned char SREG_C:1,
                                    SREG_Z:1,
                                    SREG_N:1,
                                    SREG_V:1,
                                    SREG_S:1,
                                    SREG_H:1,
                                    SREG_T:1,
                                    SREG_I:1;
                     };
             } @ 0x3F;
*/
#define SFR_B2_N(_ADDR, _NAME1, _NAME2, _B7, _B6, _B5, _B4, _B3, _B2, _B1, _B0) \
                 SFR_B2_BITS_N(_NAME1, _NAME2, _ADDR, \
                               Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7, \
                               _B0,_B1,_B2,_B3,_B4,_B5,_B6,_B7)

#define SFR_B_EXT_N(_ADDR, _NAME, _B7, _B6, _B5, _B4, _B3, _B2, _B1, _B0) \
                    SFR_B_BITS_EXT_N(_NAME, _ADDR, \
                                     Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7, \
                                     _B0,_B1,_B2,_B3,_B4,_B5,_B6,_B7)

#define SFR_B_EXT_IO_N(_ADDR, _NAME, _B7, _B6, _B5, _B4, _B3, _B2, _B1, _B0) \
                    SFR_B_BITS_EXT_IO_N(_NAME, _ADDR, \
                                     Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7, \
                                     _B0,_B1,_B2,_B3,_B4,_B5,_B6,_B7)

#define SFR_W_N(_ADDR, _NAME, _B15, _B14, _B13, _B12, _B11, _B10, _B9, _B8, \
                              _B7, _B6, _B5, _B4, _B3, _B2, _B1, _B0) \
                SFR_W_BITS_N(_NAME, _ADDR, \
                             Bit0,Bit1,Bit2,Bit3,Bit4,Bit5,Bit6,Bit7, \
                             Bit8,Bit9,Bit10,Bit11,Bit12,Bit13,Bit14,Bit15, \
                             _B0,_B1,_B2,_B3,_B4,_B5,_B6,_B7, \
                             _B8,_B9,_B10,_B11,_B12,_B13,_B14,_B15)

#endif /* __IOMACRO_H */
