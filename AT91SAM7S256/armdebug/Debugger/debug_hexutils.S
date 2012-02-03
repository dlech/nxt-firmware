/** @file debug_hexutils.S
 *  @brief GDB hexadecimal conversion utility routines
 *
 */

/* Copyright (C) 2007-2011 the NxOS developers
 *
 * Module Developed by: TC Wan <tcwan@cs.usm.my>
 *
 * See AUTHORS for a full list of the developers.
 *
 * See COPYING for redistribution license
 *
 */


#define __ASSEMBLY__

#include "debug_internals.h"

.data
.align 4

hex2char_lut:
	/*	.ascii	"0123456789ABCDEF" */
	/* Need to use lowercase hex chars to avoid confusion with GDB Error (E NN) response */
	.ascii	"0123456789abcdef"

/* Macros
 */


/* _hex2char_lut
 *		Internal routine to intialize the LUT address pointer
 */
	.macro _hex2char_lut	addrptr
	ldr		\addrptr, =hex2char_lut
	.endm

/* _hex2char_cont
 *		Internal routine that assumes that the LUT has been loaded.
 *		This macro accepts a byte sized hex value as a parameter register(7:0) and returns the
 *		ASCII equivalent in in the same register(7:0)
 *		The second parameter is the LUT address pointer register to use (assumed to be initialized)
 *		WARNING: Assumes that the value in register is sanity checked before invoking macro
 */
	.macro	_hex2char_cont	reg, addrptr
	ldrb	\reg, [\addrptr, \reg]
	.endm

/* _hex2char
 *		This macro accepts a byte sized hex value as a parameter register(7:0) and returns the
 *		ASCII equivalent in in the same register(7:0)
 *		The second parameter is the LUT address pointer register to use (register content is destroyed)
 *		WARNING: Assumes that the value in register is sanity checked before invoking macro
 */
	.macro	_hex2char	reg, addrptr
	_hex2char_lut	\addrptr
	_hex2char_cont	\reg, \addrptr
	.endm

/* _char2hex
 *		This macro accepts an ASCII char as a parameter register(7:0) and returns the
 *		equivalent byte sized hex value in in the same register(7:0)
 *		WARNING: Assumes that the char in register is a valid hex char before invoking macro
 */
	.macro	_char2hex	reg
	cmp	\reg, #'A'									/* If Alpha */
	bichs	\reg, \reg, #ASCII_LOWER2UPPER_MASK		/* Convert to Uppercase */
	subhs	\reg, \reg, #7							/* Adjustment to allow for subtraction with 0x30 */
	sub		\reg, \reg, #0x30						/* get final hex value */
	.endm


.code 32
.text
.align 	4


/* Utility Routines
 *  GDB requires command parameters to be specified as Big Endian values.
 *  However, the read/write register command expect the register contents to be specified in target byte order.
 *  Hence we need both versions of multibyte conversion routines for word sized values.
 */

/* hex2char
 *		This routine accepts a byte sized hex value in R0(7:0) and returns the
 *		ASCII equivalent in R0(7:0)
 */
	.global	hex2char

hex2char:
	stmfd	sp!, {r1,lr}
	and		r0, #NIBBLE0			/* make sure that input is sane */
	_hex2char	r0, r1
	ldmfd	sp!, {r1,pc}

/* char2hex
 *		This routine accepts an ASCII character in R0(7:0) and returns the
 *		equivalent byte sized hex value in R0(7:0).
 *		It accepts lowercase and uppercase ASCII Hex char inputs.
 *      Invalid inputs return -1 as the value
*      On entry:
 *          R0: ASCII character
 *      On exit:
 *          R0: Hex value
 */
	.global	char2hex

char2hex:
	and		r0, r0, #BYTE0			/* make sure that input is sane */
	cmp		r0, #'0'
	blo		char2hex_error
	cmp     r0, #'9'
	bls     perform_char2hex
	cmp     r0, #'A'
	blo    char2hex_error
	cmp     r0, #'F'
	bls     perform_char2hex
    cmp     r0, #'a'
    blo    char2hex_error
	cmp		r0, #'f'
	bhi		char2hex_error
	/* Validated Hex Char */
perform_char2hex:
	_char2hex	r0					/* Return hex value in R0 */
	bx		lr

char2hex_error:
	mov     r0, #-1                 /* Set Return value to Error value */
	bx		lr

/* byte2ascii_cont
 *      (Shared routine, does not perform sanity checks)
 *      On entry:
 *          R0: ASCII buffer pointer
 *          R1[7:0]: byte value
 *      On exit:
 *          R0: Address of next empty char slot in buffer
 *          R1: Destroyed
 *
 *      This routine accepts an ASCII buffer pointer in R0 and a byte value in R1,
 *		and stores the ASCII equivalent byte value in the buffer pointed to by R0.
 *		Note: On return, R0 points to next empty char slot in buffer
 */
byte2ascii_cont:
	stmfd	sp!, {r2,r3,r4, lr}
	lsl		r2, r1, #24			    /* Keep copy of input byte value R1[7:0], shifted to MSB R2[31:24] */
	mov		r4, #2					/* Loop counter */
	_hex2char_lut	r3				/* initialize LUT pointer */
1:	ror		r2, r2, #28 			/* Rotate MSNibble R2[31:28] into LSNibble position R2[3:0] */
	and		r1, r2, #NIBBLE0		/* Mask out everything else, store Nibble in R1 */
	_hex2char_cont	r1, r3			/* Convert nibble to ASCII char */
	strb	r1, [r0], #1
	subs	r4, r4, #1				/* decrement loop counter */
	bne		1b
	ldmfd	sp!, {r2,r3,r4, pc}

/* byte2ascii
 *      On entry:
 *          R0: ASCII buffer pointer
 *          R1[7:0]: Byte value
 *      On exit:
 *          R0: Address of next empty char slot in buffer
 *          R1: Original Address of Buffer
 *
 *		This routine accepts an ASCII buffer pointer in R0 and a byte value in R1,
 *		and stores the ASCII equivalent byte value in the buffer pointed to by R0.
 *		Note: On return, R0 points to the next empty char slot in buffer
 */
	.global	byte2ascii

byte2ascii:
	stmfd	sp!, {r0, lr}			/* Keep ASCII buffer pointer */
	and		r1, #BYTE0				/* sanitize input */
	bl		byte2ascii_cont
	ldmfd	sp!, {r1, pc}			/* return original string pointer in R1 */

/* halfword2ascii_be
 *      Big Endian version of halfword2ascii conversion routine
 *      On entry:
 *          R0: ASCII buffer pointer
 *          R1[15:0]: Halfword value
 *      On exit:
 *          R0: Address of next empty char slot in buffer
 *          R1: Original Address of Buffer
 *
 *      This routine accepts an ASCII buffer pointer in R0 and a halfword value in R1,
 *      and stores the ASCII equivalent halfword value in the buffer pointed to by R0.
 *      Note: On return, R0 points to the next empty char slot in buffer
 */
	.global	halfword2ascii_be
halfword2ascii_be:
	stmfd	sp!, {r0,r2,r3, lr}		/* Keep ASCII buffer pointer */
    mov     r3, #2                  /* Loop Counter */
    mov     r2, r1, lsl #16         /* copy of input halfword value R1[15:0], shifted to MSH R2[31:16] */
    b       _conv_byte2ascii_be     /* goto Byte conversion loop */

/* halfword2ascii_le
 *      Little Endian version of halfword2ascii conversion routine
 *      On entry:
 *          R0: ASCII buffer pointer
 *          R1[15:0]: Halfword value
 *      On exit:
 *          R0: Address of next empty char slot in buffer
 *          R1: Original Address of Buffer
 *
 *      This routine accepts an ASCII buffer pointer in R0 and a halfword value in R1,
 *      and stores the ASCII equivalent halfword value in the buffer pointed to by R0.
 *      Note: On return, R0 points to the next empty char slot in buffer
 */
    .global halfword2ascii_le
halfword2ascii_le:
    stmfd   sp!, {r0,r2,r3, lr}     /* Keep ASCII buffer pointer */
    mov     r3, #2                  /* Loop Counter */
    b       _conv_byte2ascii_le     /* goto Byte conversion loop */


/* word2ascii_be
 *      Big Endian version of word2ascii conversion routine
 *      On entry:
 *          R0: ASCII buffer pointer
 *          R1[31:0]: Word value
 *      On exit:
 *          R0: Address of next empty char slot in buffer
 *          R1: Original Address of Buffer
 *
 *      This routine accepts an ASCII buffer pointer in R0 and a word value in R1,
 *      and stores the ASCII equivalent word value in the buffer pointed to by R0.
 *      Note: On return, R0 points to the next empty char slot in buffer
 */
	.global	word2ascii_be
word2ascii_be:
	stmfd	sp!, {r0,r2,r3, lr}		/* Keep ASCII buffer pointer */
	mov		r2, r1					/* copy of input word value R1[31:0] */
	mov		r3, #4					/* Loop Counter */

	/* Fall through to byte coversion loop */


/* Big Endian Multibyte Convert: Rotate then convert */
_conv_byte2ascii_be:
	ror		r2, r2, #24			    /* Rotate MSB R2[31:24] into LSB position R2[7:0] */
	and		r1, r2, #BYTE0			/* Copy byte value in R2[7:0] into R1 */
	bl		byte2ascii_cont         /* R0: next ASCII buffer location pointer, R1: destroyed */
	subs	r3, r3, #1
	bne		_conv_byte2ascii_be
	ldmfd	sp!, {r1,r2,r3, pc}

/* word2ascii_le
 *      Little Endian version of word2ascii conversion routine
 *      On entry:
 *          R0: ASCII buffer pointer
 *          R1[31:0]: Word value
 *      On exit:
 *          R0: Address of next empty char slot in buffer
 *          R1: Original Address of Buffer
 *
 *      This routine accepts an ASCII buffer pointer in R0 and a word value in R1,
 *      and stores the ASCII equivalent word value in the buffer pointed to by R0.
 *      Note: On return, R0 points to the next empty char slot in buffer
 */
    .global word2ascii_le
word2ascii_le:
    stmfd   sp!, {r0,r2,r3, lr}     /* Keep ASCII buffer pointer */
    mov     r2, r1                  /* copy of input word value R1[31:0] */
    mov     r3, #4                  /* Loop Counter */

    /* Fall through to byte coversion loop */

/* Little Endian Multibyte Convert: Convert then rotate */
_conv_byte2ascii_le:
    and     r1, r2, #BYTE0          /* Copy byte value in R2[7:0] into R1 */
    bl      byte2ascii_cont         /* R0: next ASCII buffer location pointer, R1: destroyed */
    ror     r2, r2, #8              /* Rotate LSB+1 R2[15:8] into LSB position R2[7:0] */
    subs    r3, r3, #1
    bne     _conv_byte2ascii_le
    ldmfd   sp!, {r1,r2,r3, pc}


/* ascii2hex_varlen_be
 *      Big Endian version of ascii2hex_varlen conversion routine
 *      (There is no Little Endian Version)
 *      On entry:
 *          R0: ASCII buffer pointer
 *      On exit:
 *          R0: Hex value
 *          R1: Address of next char slot in buffer
 *
 *      This routine accepts an ASCII buffer pointer in R0,
 *      and returns the hex value in R0 for up to 8 Hex characters.
 *      Note: On return, R1 points to the ASCII buffer location after the hex value chars.
 */
    .global ascii2hex_varlen_be

ascii2hex_varlen_be:
    stmfd   sp!, {r2,r3, lr}
    mov     r3, #CMD_REG_REGPARAMLEN /* Set max count to 8 (Max Register size) */
    mov     r1, r0                  /* Use R1 as ASCII buffer pointer */
    mov     r2, #0                  /* Initialize Cummulative Results */
2:  ldrb    r0, [r1]	            /* Load ASCII char for Hex Value */
    bl      char2hex                /* on return, hex value in R0, -1 for error */
    cmp     r0, #-1
    beq     _exit_ascii2hex_varlen
    orr     r2, r0, r2, lsl #4      /* combined byte value */
    subs    r3, r3, #1              /* Decrement Counter */
    add		r1, r1, #1				/* Go to next char slot */
    bne     2b
_exit_ascii2hex_varlen:
    mov     r0, r2                  /* Return results in R0 */
    ldmfd   sp!, {r2,r3, pc}


/* ascii2byte
 *      On entry:
 *          R0: ASCII buffer pointer
 *      On exit:
 *          R0[7:0]: Byte value
 *          R1: Address of next char slot in buffer
 *
 *		This routine accepts an ASCII buffer pointer in R0,
 *		and returns the byte value in R0[7:0].
 *		Note: On return, R1 points to the ASCII buffer location after the current 2 chars.
 *		WARNING: This routine assumes that the input buffer was sanitized and contains valid Hex chars,
 *				 otherwise it will return invalid results.
 */
	.global	ascii2byte

ascii2byte:
    stmfd   sp!, {r2, lr}
    mov     r1, r0                  /* Use R1 as ASCII buffer pointer */
    ldrb    r0, [r1], #1            /* Load ASCII char for MSN */
    bl      char2hex                /* on return, hex value in R0, -1 for error (ignored) */
    mov     r2, r0, lsl #4          /* Intermediate Results register */
    ldrb    r0, [r1], #1            /* Load ASCII char for LSN */
    bl      char2hex                /* on return, hex value in R0, -1 for error (ignored) */
    orr     r0, r2, r0              /* combined byte value */
    ldmfd   sp!, {r2, pc}

/* ascii2halfword_be
 *      Big Endian version of ascii2halfword conversion routine
 *      On entry:
 *          R0: ASCII buffer pointer
 *      On exit:
 *          R0[15:0]: Halfword value
 *          R1: Address of next char slot in buffer
 *
 *		This routine accepts an ASCII buffer pointer in R0,
 *		and returns the Halfword value in R0[15:0].
 *		Note: On return, R1 points to the ASCII buffer location after the current 4 chars.
 *		WARNING: This routine assumes that the input buffer was sanitized and contains valid Hex chars,
 *				 otherwise it will return invalid results.
 */
	.global	ascii2halfword_be

ascii2halfword_be:
	stmfd	sp!, {r2,r3, lr}
	mov		r3, #2					/* Loop counter */
    b       _conv_ascii2byte_be

/* ascii2halfword_le
 *      Little Endian version of ascii2halfword conversion routine
 *      On entry:
 *          R0: ASCII buffer pointer
 *      On exit:
 *          R0[15:0]: Halfword value
 *          R1: Address of next char slot in buffer
 *
 *      This routine accepts an ASCII buffer pointer in R0,
 *      and returns the Halfword value in R0[15:0].
 *      Note: On return, R1 points to the ASCII buffer location after the current 4 chars.
 *      WARNING: This routine assumes that the input buffer was sanitized and contains valid Hex chars,
 *               otherwise it will return invalid results.
 */
    .global ascii2halfword_le

ascii2halfword_le:
    stmfd   sp!, {r2,r3, lr}
    mov     r3, #2                  /* Loop counter */
    b       _conv_ascii2byte_le


/* ascii2word_be
 *      Big Endian version of ascii2word conversion routine
 *      On entry:
 *          R0: ASCII buffer pointer
 *      On exit:
 *          R0[31:0]: Word value
 *          R1: Address of next char slot in buffer
 *
 *		This routine accepts an ASCII buffer pointer in R0,
 *		and returns the word value in R0[31:0].
 *		Note: On return, R1 points to the ASCII buffer location after the current 8 chars.
 *		WARNING: This routine assumes that the input buffer was sanitized and contains valid Hex chars,
 *				 otherwise it will return invalid results.
 */
	.global	ascii2word_be

ascii2word_be:
	stmfd	sp!, {r2,r3, lr}
	mov		r3, #4					/* Loop counter */

	/* Fall through to byte coversion loop */

_conv_ascii2byte_be:
	teq		r0, #0
	beq		_exit_conv_ascii2byte_be	/* exit if NULL pointer in R0 */
    mov     r2, #0                  /* Initialize Cummulative value */
2:  bl      ascii2byte
    orr     r2, r0, r2, lsl #8      /* Merge current byte with cummulative value */
    mov     r0, r1                  /* Copy next char pointer to R0 for next byte */
    subs    r3, r3, #1
    bne     2b
    mov     r0, r2                  /* Copy it to R0 as return value */

_exit_conv_ascii2byte_be:
	ldmfd	sp!, {r2,r3, pc}		/* return hex value in R0 */

/* ascii2word_le
 *      Litle Endian version of ascii2word conversion routine
 *      On entry:
 *          R0: ASCII buffer pointer
 *      On exit:
 *          R0[31:0]: Word value
 *          R1: Address of next char slot in buffer
 *
 *      This routine accepts an ASCII buffer pointer in R0,
 *      and returns the word value in R0[31:0].
 *      Note: On return, R1 points to the ASCII buffer location after the current 8 chars.
 *      WARNING: This routine assumes that the input buffer was sanitized and contains valid Hex chars,
 *               otherwise it will return invalid results.
 */
    .global ascii2word_le

ascii2word_le:
    stmfd   sp!, {r2,r3, lr}
    mov     r3, #4                  /* Loop counter */

    /* Fall through to byte coversion loop */

_conv_ascii2byte_le:
    teq     r0, #0
    beq     _exit_conv_ascii2byte_le    /* exit if NULL pointer in R0 */
    push    {r3}                    /* Need to keep couter for final value adjustment */
    mov     r2, #0                  /* Initialize Cummulative value */
2:  bl      ascii2byte
    orr     r2, r0, r2, ror #8       /* Merge current byte with cummulative value */
    mov     r0, r1                  /* Copy next char pointer to R0 for next byte */
    subs    r3, r3, #1
    bne     2b
    /* Cummulative value done, need to rotate it into the correct position for return value */
    pop     {r3}                    /* retrieve counter */
    rsb     r3, r3, #5              /* 5 - count */
    lsl     r3, r3, #3              /* [(5-count) x 8] bits to rotate */
    mov     r0, r2, ror r3           /* Copy it to R0 as return value */

_exit_conv_ascii2byte_le:
    ldmfd   sp!, {r2,r3, pc}        /* return hex value in R0 */

