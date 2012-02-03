/** @file debug_opcodes.S
 *  @brief ARM Debugger Opcode Parsing Routines
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

/* WARNING: The following excepted code from eCos arm_stub.c has bugs in
 * the next instruction address calculation logic. The C code has not been
 * updated since it is only used for documentation purposes.
 *
 * Correct code behavior should be determined from the ARMDEBUG source code
 * whenever there is conflict in the algorithms.
 *
 * Of note: ARM and Thumb mode BX PC handling (missing PC+8/PC+4 adjustment).
 *          LDM PC handling (missing Pre/Post Incr/Decr adjustment).
 */
/****************************************************************************
// Selected Routines from the eCos arm_stub.c related to next instruction address
// determination in ARM processors.

//========================================================================
//
//      arm_stub.c
//
//      Helper functions for stub, generic to all ARM processors
//
//========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later
// version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with eCos; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// As a special exception, if other files instantiate templates or use
// macros or inline functions from this file, or you compile this file
// and link it with other works to produce a work based on this file,
// this file does not by itself cause the resulting work to be covered by
// the GNU General Public License. However the source code for this file
// must still be made available in accordance with section (3) of the GNU
// General Public License v2.
//
// This exception does not invalidate any other reasons why a work based
// on this file might be covered by the GNU General Public License.
// -------------------------------------------
// ####ECOSGPLCOPYRIGHTEND####
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     Red Hat, gthomas
// Contributors:  Red Hat, gthomas, jskov
// Date:          1998-11-26
// Purpose:
// Description:   Helper functions for stub, generic to all ARM processors
// Usage:
//
//####DESCRIPTIONEND####
//
//========================================================================


static int
ins_will_execute(unsigned long ins)
{
    unsigned long psr = get_register(PS);  // condition codes
    int res = 0;
    switch ((ins & 0xF0000000) >> 28) {
    case 0x0: // EQ
        res = (psr & PS_Z) != 0;
        break;
    case 0x1: // NE
        res = (psr & PS_Z) == 0;
        break;
    case 0x2: // CS
        res = (psr & PS_C) != 0;
        break;
    case 0x3: // CC
        res = (psr & PS_C) == 0;
        break;
    case 0x4: // MI
        res = (psr & PS_N) != 0;
        break;
    case 0x5: // PL
        res = (psr & PS_N) == 0;
        break;
    case 0x6: // VS
        res = (psr & PS_V) != 0;
        break;
    case 0x7: // VC
        res = (psr & PS_V) == 0;
        break;
    case 0x8: // HI
        res = ((psr & PS_C) != 0) && ((psr & PS_Z) == 0);
        break;
    case 0x9: // LS
        res = ((psr & PS_C) == 0) || ((psr & PS_Z) != 0);
        break;
    case 0xA: // GE
        res = ((psr & (PS_N|PS_V)) == (PS_N|PS_V)) ||
            ((psr & (PS_N|PS_V)) == 0);
        break;
    case 0xB: // LT
        res = ((psr & (PS_N|PS_V)) == PS_N) ||
            ((psr & (PS_N|PS_V)) == PS_V);
        break;
    case 0xC: // GT
        res = ((psr & (PS_N|PS_V)) == (PS_N|PS_V)) ||
            ((psr & (PS_N|PS_V)) == 0);
        res = ((psr & PS_Z) == 0) && res;
        break;
    case 0xD: // LE
        res = ((psr & (PS_N|PS_V)) == PS_N) ||
            ((psr & (PS_N|PS_V)) == PS_V);
        res = ((psr & PS_Z) == PS_Z) || res;
        break;
    case 0xE: // AL
        res = TRUE;
        break;
    case 0xF: // NV
        if (((ins & 0x0E000000) >> 24) == 0xA)
	    res = TRUE;
	else
	    res = FALSE;
        break;
    }
    return res;
}

static unsigned long
RmShifted(int shift)
{
    unsigned long Rm = get_register(shift & 0x00F);
    int shift_count;
    if ((shift & 0x010) == 0) {
        shift_count = (shift & 0xF80) >> 7;
    } else {
        shift_count = get_register((shift & 0xF00) >> 8);
    }
    switch ((shift & 0x060) >> 5) {
    case 0x0: // Logical left
        Rm <<= shift_count;
        break;
    case 0x1: // Logical right
        Rm >>= shift_count;
        break;
    case 0x2: // Arithmetic right
        Rm = (unsigned long)((long)Rm >> shift_count);
        break;
    case 0x3: // Rotate right
        if (shift_count == 0) {
            // Special case, RORx
            Rm >>= 1;
            if (get_register(PS) & PS_C) Rm |= 0x80000000;
        } else {
            Rm = (Rm >> shift_count) | (Rm << (32-shift_count));
        }
        break;
    }
    return Rm;
}

// Decide the next instruction to be executed for a given instruction
static unsigned long *
target_ins(unsigned long *pc, unsigned long ins)
{
    unsigned long new_pc, offset, op2;
    unsigned long Rn;
    int i, reg_count, c;

    switch ((ins & 0x0C000000) >> 26) {
    case 0x0:
        // BX or BLX
        if ((ins & 0x0FFFFFD0) == 0x012FFF10) {
            new_pc = (unsigned long)get_register(ins & 0x0000000F);
            return ((unsigned long *)new_pc);
        }
        // Data processing
        new_pc = (unsigned long)(pc+1);
        if ((ins & 0x0000F000) == 0x0000F000) {
            // Destination register is PC
            if ((ins & 0x0FBF0000) != 0x010F0000) {
                Rn = (unsigned long)get_register((ins & 0x000F0000) >> 16);
                if ((ins & 0x000F0000) == 0x000F0000) Rn += 8;  // PC prefetch!
                if ((ins & 0x02000000) == 0) {
                    op2 = RmShifted(ins & 0x00000FFF);
                } else {
                    op2 = ins & 0x000000FF;
                    i = (ins & 0x00000F00) >> 8;  // Rotate count
                    op2 = (op2 >> (i*2)) | (op2 << (32-(i*2)));
                }
                switch ((ins & 0x01E00000) >> 21) {
                case 0x0: // AND
                    new_pc = Rn & op2;
                    break;
                case 0x1: // EOR
                    new_pc = Rn ^ op2;
                    break;
                case 0x2: // SUB
                    new_pc = Rn - op2;
                    break;
                case 0x3: // RSB
                    new_pc = op2 - Rn;
                    break;
                case 0x4: // ADD
                    new_pc = Rn + op2;
                    break;
                case 0x5: // ADC
                    c = (get_register(PS) & PS_C) != 0;
                    new_pc = Rn + op2 + c;
                    break;
                case 0x6: // SBC
                    c = (get_register(PS) & PS_C) != 0;
                    new_pc = Rn - op2 + c - 1;
                    break;
                case 0x7: // RSC
                    c = (get_register(PS) & PS_C) != 0;
                    new_pc = op2 - Rn +c - 1;
                    break;
                case 0x8: // TST
                case 0x9: // TEQ
                case 0xA: // CMP
                case 0xB: // CMN
                    break; // PC doesn't change
                case 0xC: // ORR
                    new_pc = Rn | op2;
                    break;
                case 0xD: // MOV
                    new_pc = op2;
                    break;
                case 0xE: // BIC
                    new_pc = Rn & ~op2;
                    break;
                case 0xF: // MVN
                    new_pc = ~op2;
                    break;
                }
            }
        }
        return ((unsigned long *)new_pc);
    case 0x1:
        if ((ins & 0x02000010) == 0x02000010) {
            // Undefined!
            return (pc+1);
        } else {
            if ((ins & 0x00100000) == 0) {
                // STR
                return (pc+1);
            } else {
                // LDR
                if ((ins & 0x0000F000) != 0x0000F000) {
                    // Rd not PC
                    return (pc+1);
                } else {
                    Rn = (unsigned long)get_register((ins & 0x000F0000) >> 16);
                    if ((ins & 0x000F0000) == 0x000F0000) Rn += 8;  // PC prefetch!
                    if (ins & 0x01000000) {
                        // Add/subtract offset before
                        if ((ins & 0x02000000) == 0) {
                            // Immediate offset
                            if (ins & 0x00800000) {
                                // Add offset
                                Rn += (ins & 0x00000FFF);
                            } else {
                                // Subtract offset
                                Rn -= (ins & 0x00000FFF);
                            }
                        } else {
                            // Offset is in a register
                            if (ins & 0x00800000) {
                                // Add offset
                                Rn += RmShifted(ins & 0x00000FFF);
                            } else {
                                // Subtract offset
                                Rn -= RmShifted(ins & 0x00000FFF);
                            }
                        }
                    }
                    return ((unsigned long *)*(unsigned long *)Rn);
                }
            }
        }
        return (pc+1);
    case 0x2:  // Branch, LDM/STM
        if ((ins & 0x02000000) == 0) {
            // LDM/STM
            if ((ins & 0x00100000) == 0) {
                // STM
                return (pc+1);
            } else {
                // LDM
                if ((ins & 0x00008000) == 0) {
                    // PC not in list
                    return (pc+1);
                } else {
                    Rn = (unsigned long)get_register((ins & 0x000F0000) >> 16);
                    if ((ins & 0x000F0000) == 0x000F0000) Rn += 8;  // PC prefetch!
                    offset = ins & 0x0000FFFF;
                    reg_count = 0;
                    for (i = 0;  i < 15;  i++) {
                        if (offset & (1<<i)) reg_count++;
                    }
                    if (ins & 0x00800000) {
                        // Add offset
                        Rn += reg_count*4;
                    } else {
                        // Subtract offset
                        Rn -= 4;
                    }
                    return ((unsigned long *)*(unsigned long *)Rn);
                }
            }
        } else {
            // Branch
            if (ins_will_execute(ins)) {
                offset = (ins & 0x00FFFFFF) << 2;
                if (ins & 0x00800000) offset |= 0xFC000000;  // sign extend
                new_pc = (unsigned long)(pc+2) + offset;
		// If its BLX, make new_pc a thumb address.
		if ((ins & 0xFE000000) == 0xFA000000) {
		    if ((ins & 0x01000000) == 0x01000000)
			new_pc |= 2;
		    new_pc = MAKE_THUMB_ADDR(new_pc);
		}
                return ((unsigned long *)new_pc);
            } else {
                // Falls through
                return (pc+1);
            }
        }
    case 0x3:  // Coprocessor & SWI
        if (((ins & 0x03000000) == 0x03000000) && ins_will_execute(ins)) {
           // SWI
           return (unsigned long *)(CYGNUM_HAL_VECTOR_SOFTWARE_INTERRUPT * 4);
        } else {
           return (pc+1);
        }
    default:
        // Never reached - but fixes compiler warning.
        return 0;
    }
}

// FIXME: target_ins also needs to check for CPSR/THUMB being set and
//        set the thumb bit accordingly.

static unsigned long
target_thumb_ins(unsigned long pc, unsigned short ins)
{
    unsigned long new_pc = MAKE_THUMB_ADDR(pc+2); // default is fall-through
                                        // to next thumb instruction
    unsigned long offset, arm_ins, sp;
    int i;

    switch ((ins & 0xf000) >> 12) {
    case 0x4:
        // Check for BX or BLX
        if ((ins & 0xff07) == 0x4700)
            new_pc = (unsigned long)get_register((ins & 0x00078) >> 3);
        break;
    case 0xb:
        // push/pop
        // Look for "pop {...,pc}"
        if ((ins & 0xf00) == 0xd00) {
            // find PC
            sp = (unsigned long)get_register(SP);

            for (offset = i = 0; i < 8; i++)
              if (ins & (1 << i))
                  offset += 4;

            new_pc = *(cyg_uint32 *)(sp + offset);

            if (!v5T_semantics())
                new_pc = MAKE_THUMB_ADDR(new_pc);
        }
        break;
    case 0xd:
        // Bcc | SWI
        // Use ARM function to check condition
        arm_ins = ((unsigned long)(ins & 0x0f00)) << 20;
        if ((arm_ins & 0xF0000000) == 0xF0000000) {
            // SWI
            new_pc = CYGNUM_HAL_VECTOR_SOFTWARE_INTERRUPT * 4;
        } else if (ins_will_execute(arm_ins)) {
            offset = (ins & 0x00FF) << 1;
            if (ins & 0x0080) offset |= 0xFFFFFE00;  // sign extend
            new_pc = MAKE_THUMB_ADDR((unsigned long)(pc+4) + offset);
        }
        break;
    case 0xe:
        // check for B
        if ((ins & 0x0800) == 0) {
            offset = (ins & 0x07FF) << 1;
            if (ins & 0x0400) offset |= 0xFFFFF800;  // sign extend
            new_pc = MAKE_THUMB_ADDR((unsigned long)(pc+4) + offset);
        }
        break;
    case 0xf:
        // BL/BLX (4byte instruction!)
        // First instruction (bit 11 == 0) holds top-part of offset
        if ((ins & 0x0800) == 0) {
	    offset = (ins & 0x07FF) << 12;
	    if (ins & 0x0400) offset |= 0xFF800000;  // sign extend
	    // Get second instruction
	    // Second instruction (bit 11 == 1) holds bottom-part of offset
	    ins = *(unsigned short*)(pc+2);
	    // Check for BL/BLX
	    if ((ins & 0xE800) == 0xE800) {
		offset |= (ins & 0x07ff) << 1;
		new_pc = (unsigned long)(pc+4) + offset;
		// If its BLX, force a full word alignment
		// Otherwise, its a thumb address.
		if (!(ins & 0x1000))
		    new_pc &= ~3;
		else
		    new_pc = MAKE_THUMB_ADDR(new_pc);
	    }
	}
        break;
    }

    return new_pc;
}

void __single_step (void)
{
    unsigned long pc = get_register(PC);
    unsigned long cpsr = get_register(PS);

    // Calculate address of next instruction to be executed
    if (cpsr & CPSR_THUMB_ENABLE) {
        // thumb
        ss_saved_pc = target_thumb_ins(pc, *(unsigned short*)pc);
    } else {
        // ARM
        unsigned long curins = *(unsigned long*)pc;
        if (ins_will_execute(curins)) {
            // Decode instruction to decide what the next PC will be
            ss_saved_pc = (unsigned long) target_ins((unsigned long*)pc,
                                                     curins);
        } else {
            // The current instruction will not execute (the conditions
            // don't hold)
            ss_saved_pc = pc+4;
        }
    }

    // Set breakpoint according to type
    if (IS_THUMB_ADDR(ss_saved_pc)) {
        // Thumb instruction
        unsigned long t_pc = UNMAKE_THUMB_ADDR(ss_saved_pc);
        ss_saved_thumb_instr = *(unsigned short*)t_pc;
        *(unsigned short*)t_pc = HAL_BREAKINST_THUMB;
    } else {
        // ARM instruction
        ss_saved_instr = *(unsigned long*)ss_saved_pc;
        *(unsigned long*)ss_saved_pc = HAL_BREAKINST_ARM;
    }
}

 ****************************************************************************/

#define __ASSEMBLY__
#include "debug_stub.h"
#include "debug_internals.h"
#include "debug_macros.h"

.data
.align 4
/* Rm Shifted Shift Type Jump Table
 * On entry:
 *      R0: Register Rm
 *      R1: Shift/Rotate Amount
 * On exit:
 *      R0: RmShifted result
 *
 */
debug_regShiftJumpTable:
    .word   _reg_lsl                            /* 00 */
    .word   _reg_lsr                            /* 01 */
    .word   _reg_asr                            /* 02 */
    .word   _reg_ror                            /* 03 */
    .word   _reg_rrx                            /* 04 */

/* Data Processing Instruction Jump Table
 * On entry:
 *      R0: Register Rn (Operand 1) value
 *      R1: Operand 2 value
 *      R2: Default Next Instruction Address
 *      R5[3:0]: CPSR condition codes
 * On exit:
 *      R0: Calculated result
 *      R1, R2, R3: Destroyed
 *
 */
debug_dataInstrJumpTable:
    .word   _opcode_and                         /* 00 */
    .word   _opcode_eor                         /* 01 */
    .word   _opcode_sub                         /* 02 */
    .word   _opcode_rsb                         /* 03 */
    .word   _opcode_add                         /* 04 */
    .word   _opcode_adc                         /* 05 */
    .word   _opcode_sbc                         /* 06 */
    .word   _opcode_rsc                         /* 07 */
    .word   _opcode_tst                         /* 08 */
    .word   _opcode_teq                         /* 09 */
    .word   _opcode_cmp                         /* 0A */
    .word   _opcode_cmn                         /* 0B */
    .word   _opcode_orr                         /* 0C */
    .word   _opcode_mov                         /* 0D */
    .word   _opcode_bic                         /* 0E */
    .word   _opcode_mvn                         /* 0F */


/*
 * To determine the next instruction to execute, we need to check current (breakpointed) instruction
 * and determine whether it will be executed or not. This necessitates a mini instruction decoder
 * that can check the type of instruction, as well as if it'll affect the PC.
 * The instruction decoder used here is table based. Each entry in the table consists of:
 *		Instruction Identifier (IID), Instruction Bitmask (IBM), Instruction Handler Address (IHA)
 * Null entries are placed at the end of the table.
 *
 * This allows for a flexible approach to handling instructions that we're interested in, at the expense
 * of memory usage.
 *
 * For ARM, the IID & IBM are both 4 bytes, whereas the Thumb IID & IBM are 2 bytes.
 * The IHA is always 4 bytes.
 */

/* ARM Instruction Decode Table
 *	.word IID, IBM, IHA (12 bytes)
 */

/* WARNING: The sequence of matching instructions is important!
 *			Always check from more specific to more general IBMs
 *			for instructions sharing common opcode prefix bits.
 */
debug_armDecodeTable:
	.word	0x012fff10, 0x0ffffff0, _arm_bx_blx_handler		/* [Prefix:00] BX or BLX. Note v4t does not have BLX instr */
	.word	0x0000f000, 0x0c00f000, _arm_data_instr_handler	/* [Prefix:00] Data Processing instr with Rd = R15 */
/*	.word	0x06000010, 0x0e000010, _arm_undef_handler	*/	/* [Prefix:01] Undefined instr: shouldn't occur, as it would've been trapped already. See _dbg_following_instruction_addr */
	.word	0x0410f000, 0x0410f000, _arm_ldr_pc_handler		/* [Prefix:01] LDR with Rd = PC */
	.word	0x08108000, 0x0e108000, _arm_ldm_pc_handler		/* [Prefix:10] LDM {pc} */
	.word	0x0a000000, 0x0e000000, _arm_b_bl_blx_handler	/* [Prefix:10] B, BL or BLX. Note v4t does not have BLX instr */
	.word	0x0c000000, 0x0c000000, _arm_coproc_swi_handler	/* [Prefix:11] Coprocessor instr or SWI */
	.word	0x0,0x0,0x0										/* Null Entry */

/* Thumb Instruction Decode Table
 * 	.hword IID, IBM
 * 	.word IHA (8 bytes)
 */

/* WARNING: The sequence of matching instructions is important!
 *			Always check from more specific to more general IBMs
 *			for instructions sharing common opcode prefix bits.
 */
debug_thumbDecodeTable:
	.hword	0x4700, 0xff07
	.word	_thumb_bx_blx_handler				/* [Prefix:01] BX or BLX. Note: Link (L:b7) is not checked in the mask */
	.hword	0xbd00, 0xff00
	.word	_thumb_poppc_handler				/* [Prefix:10] PUSH/POP, specifically POP {Rlist,PC} */
	.hword	0xd000, 0xf000
	.word	_thumb_bcond_swi_handler			/* [Prefix:11] B<cond> or SWI */
	.hword	0xe000, 0xf800
	.word	_thumb_b_handler					/* [Prefix:11] B */
	.hword	0xf000, 0xf000
	.word	_thumb_long_bl_blx_handler			/* [Prefix:11] Long BL or BLX (4 bytes) Note: b11 (H) indicates 1st or 2nd instr */
	.hword	0x0,0x0
	.word	0x0									/* Null Entry */

/* ARM Condition Code Mapping Table
 * Converts Instruction encoding to SPSR Flags.
 * b31 b30 b29 b28
 *  N   Z   C   V
 * Indexed according to Instruction Encoding order (pg 30, Table 6, ATMEL ARM7TDMI Data Sheet)
 * Condition Code stored in MSN(set), LSN(clr) order
 * Note1: 0x00 = AL. NV is deprecated, treat as AL
 * Note2: 0xFF indicates that the condition checks needs to be handled separately (complex checks)
 *
 *		EQ: Z set
 *		NE: Z clr
 *		HS/CS: C set
 *		LO/CC: C clr
 *		MI: N set
 *		PL: N clr
 *		VS: V set
 *		VC: V clr
 */


debug_armCondCodeTable:
			/* EQ,   NE, HS/CS, LO/CC,   MI,   PL,   VS,   VC,   HI,   LS,   GE,   LT,   GT,   LE,   AL,   NV */
	.byte	 0x40, 0x04,  0x20,  0x02, 0x80, 0x08, 0x10, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00

/* ARM Complex Condition Code Mapping Table
 * Converts Instruction encoding to SPSR Flags.
 * b31 b30 b29 b28
 *  N   Z   C   V
 * Indexed according to Instruction Encoding order (pg 30, Table 6, ATMEL ARM7TDMI Data Sheet)
 * for HI, LS, GE, LT, GT and LE instructions only
 * Condition Code stored in the following order:
 *	 b7   b6   b5    b4   b3    b2     b1    b0
 *	AND  CHKZ CHKC CHKNV   -  Z set  C set  N==V		(bit set = 1)
 *   OR    -    -     -    -  Z clr  C clr  N!=V		(bit clr = 0)
 *
 *		HI: C set AND Z clr
 *		LS: C clr  OR Z set
 *		GE: N == V
 *		LT: N != V
 *		GT: Z clr AND (N == V)
 *		LE: Z set OR (N != V)
 */

#define COMPLEX_CONDCODE_START	    0x08
#define COMPLEX_CONDCODE_NEQV_MASK  0x01
#define COMPLEX_CONDCODE_CSET_MASK  0x02
#define COMPLEX_CONDCODE_ZSET_MASK  0x04
#define COMPLEX_CONDCODE_CHKNV_MASK 0x10
#define COMPLEX_CONDCODE_CHKC_MASK  0x20
#define COMPLEX_CONDCODE_CHKZ_MASK  0x40
#define COMPLEX_CONDCODE_ANDOR_MASK 0x80

#define COMPLEX_CONDCODE_NFLAG		0x08
#define COMPLEX_CONDCODE_ZFLAG		0x04
#define COMPLEX_CONDCODE_CFLAG		0x02
#define COMPLEX_CONDCODE_VFLAG		0x01


debug_armComplexCCTable:
			/* HI,   LS,   GE,   LT,   GT,   LE */
	.byte	 0xE2, 0x64, 0x11, 0x10, 0xD1, 0x54

.code 32
.text
.align 	4

/* dbg_following_instruction_addr
 *		Determine the address of the following instruction to execute.
 *      On entry:
 *          R0: Address of the instruction to be (re)executed
 *		On exit:
 *          R0: Destroyed
 *			R1: Following Instruction Address (31 bits, b0 = THUMB flag)
 *          R2-R7: Destroyed
 *
 *	Here we make use of the Debugger Stack which contains the address of the aborted instruction that will be reexecuted
 *	when we resume the program.
 *
 *	If it is a Manual Breakpoint inserted into the code, then we will need to update the aborted instruction
 *	address to skip the current aborted instruction and resume execution at the next instruction address,
 *  and the next instruction address to be returned to the calling routine is the following instruction
 *  address instead.
 *
 *	We need to check the aborted instruction type, to see if it is a branch instruction, before we can determine
 *	the next instruction address (for inserting a Breakpoint).
 */
	.global dbg_following_instruction_addr
dbg_following_instruction_addr:
    stmfd   sp!, {lr}
/* We assume that any BKPT instructions in the code will be Manual Breakpoints,
 * i.e., the Debugger does not leave stray Single Step / Auto / Normal breakpoints in memory
 */
    mov     r6, r0                                      /* Keep instruction address in R6 */
    _getdbgregister DBGSTACK_USERCPSR_INDEX, r1         /* Retrieve User CPSR into R1 */
    and     r0, r1, #CPSR_THUMB                         /* store Thumb Mode status in R0 */
    mov     r5, r1, lsr #28                             /* store CPSR condition flags in R5[3:0] */

_dbg_get_aborted_instr:
1:  teq     r0, #0                                      /* Check if it is ARM or Thumb instruction */
    ldrneh  r4, [r6]                                    /* Load Thumb instruction opcode using Addr in R6 into R4 */
    ldrne   r2, =(BKPT16_INSTR | BKPT16_MANUAL_BKPT)    /* check for Thumb Manual Breakpoint Instruction */
    ldreq   r4, [r6]                                    /* Load ARM instruction opcode using Addr in R6 into R4 */
    ldreq   r2, =(BKPT32_INSTR | BKPT32_MANUAL_BKPT)    /* check for ARM Manual Breakpoint Instruction */
    teq     r4, r2                                      /* Is instruction opcode (R4) == Manual Breakpoint opcode (R2)? */
    bne     2f                                          /* Not Manual breakpoint */
    teq     r0, #0                                      /* Check if it is ARM or Thumb Manual Breakpoint */
    addne   r6, r6, #2                                  /* Is Manual Breakpoint, Skip to next Thumb instruction */
    addeq   r6, r6, #4                                  /* Is Manual Breakpoint, Skip to next ARM instruction */
    b       1b                                          /* To protect against a sequence of Manual Breakpoint Instructions */

/* Here, R4 contains the instruction opcode which will be (re)executed when program resumes.
 * We need to dissect it to see if it is a branch instruction.
 * For ARM instructions, we also need to evaluate the current (breakpointed) instruction to see if it'll execute.
 * If not, then the following instruction is at the address following the address of the opcode in R4 (Default Following Instruction Address in R6).
 */
2:
    teq     r0, #0                                      /* Check if current instruction is ARM or Thumb instruction */
    beq     _following_instr_addr_for_arm
_following_instr_addr_for_thumb:
    add     r6, r6, #2                                  /* Store default following Thumb instruction address to R6 */
#if 0
	/* Flag Thumb instruction only within the instruction handler */
    orr     r6, r6, #BKPT_STATE_THUMB_FLAG              /* Set b0 to indicate Thumb instruction */
#endif
    /* R4: Candidate Instruction Opcode
     * R5[3:0]: CPSR condition codes
     * R6: Default Following Instruction Address (PC+2)
     */
    bl      _eval_thumb_instruction						/* following address is either ARM or Thumb */
    /* We must set this the Thumb bit only within the instruction handler since BX would switch modes */
    b       _exit_dbg_following_instruction_addr

_following_instr_addr_for_arm:
    add     r6, r6, #4                                  /* Store default following ARM instruction address to R6 */
    /* R4: Candidate Instruction Opcode
     * R5[3:0]: CPSR condition codes
     * R6: Default Following Instruction Address (PC+4)
     */
    bl      _eval_arm_instruction

_exit_dbg_following_instruction_addr:
    mov     r1, r0                                      /* Return Actual Following Instruction Address in R1 (B0 set to indicate Thumb mode) */
    ldmfd   sp!, {pc}


/* _eval_arm_instruction
 *      Evaluate ARM instruction to determine following instruction address
 *      On entry:
 *          R4: Opcode of instruction to be executed
 *          R5[3:0]: CPSR condition codes
 *          R6: Default Following Instruction Address (PC+4)
 *      On exit:
 *          R0: following instruction address (B0 set to indicate Thumb mode)
 *          R1-R7: destroyed
 */
_eval_arm_instruction:
    stmfd   sp!, {lr}
    bl      _dbg_check_arm_condcode		/* Returns R0: will_execute (boolean) */
    teq     r0, #FALSE
    moveq   r0, r6                      /* If False (don't execute), so use Default Following Instruction Address */
    beq     _exit_eval_arm_instruction	/* and Return to caller */

_will_execute_arm_instr:
    mov     r0, #0                      /* initialize ARM Decode Entry Table index register */
1:
    _dbg_armDecodeEntry r1, r2, r3, r0  /* instrreg (R1), instrmask (R2), codehandler (R3), indexreg (R0) */
    teq     r1, #0                      /* Check for Null Entry (End of Table marker) */
    moveq   r0, r6                      /* End of Table, no match found, so use Default Following Instruction Address */
    beq     _exit_eval_arm_instruction
    and     r7, r4, r2                  /* Use R7 to check masked instruction opcode (from R4) to see if it matches template (in R1) */
    teq     r7, r1
    addne   r0, r0, #1                  /* No match, so keep looking */
    bne     1b

_call_arm_code_handler:
    mov     lr, pc
    bx      r3                          /* Call Code Handler with R4: Instruction Opcode, R5[3:0]: CPSR, R6: Default Following Instruction Address */
_exit_eval_arm_instruction:
    /* Returned Following Address Instruction in R0 (B0 set to indicate Thumb mode)  */
    ldmfd   sp!, {pc}

/* _eval_thumb_instruction
 *      Evaluate Thumb instruction to determine following instruction address
 *      On entry:
 *          R4: Opcode of instruction to be executed
 *          R5[3:0]: CPSR condition codes
 *          R6: Default Following Instruction Address (PC+2)
 *      On exit:
 *          R0: following instruction address (B0 set to indicate Thumb mode)
 *          R1-R7: destroyed
 */
_eval_thumb_instruction:
    stmfd   sp!, {lr}
	/* Only B<cond> instructions are conditionally executed, deal with it in that Code Handler */
    mov     r0, #0                      /* initialize Thumb Decode Entry Table index register */
1:
    _dbg_thumbDecodeEntry r1, r2, r3, r0  /* instrreg (R1), instrmask (R2), codehandler (R3), indexreg (R0) */
    teq     r1, #0                      /* Check for Null Entry (End of Table marker) */
    moveq   r0, r6                      /* End of Table, no match found, so use Default Following Instruction Address */
    orreq  	r0, r0, #BKPT_STATE_THUMB_FLAG  /* Set R0[0] to flag Thumb mode */
    beq     _exit_eval_thumb_instruction

    and     r7, r4, r2                  /* Use R5 to check masked instruction opcode (from R4) to see if it matches template (in R1) */
    teq     r7, r1
    addne   r0, r0, #1                  /* No match, so keep looking */
    bne     1b

_call_thumb_code_handler:
    mov     lr, pc
    bx      r3                          /* Call Code Handler with R4: Instruction Opcode, R5[3:0]: CPSR, R6: Default Following Instruction Address */
_exit_eval_thumb_instruction:
    /* Returned Following Address Instruction in R0 */
    ldmfd   sp!, {pc}


/****************************************************************************
 *
 * Instruction Decode Routines
 *
 ****************************************************************************/

/* _dbg_check_arm_condcode
 *		Check ARM conditional execution code
 *		On entry:
 *			R4: Opcode of instruction to be executed
 *			R5[3:0]: CPSR condition codes
 *		On exit:
 *			R0: will_execute (boolean)
 *          R1-R3: Destroyed
 */

_dbg_check_arm_condcode:
	mov		r0, #TRUE								/* Default will_execute value */
	mov		r3, r4, lsr #28							/* convert opcode's condition code to index (0-F) */
	ldr		r2, =debug_armCondCodeTable
	ldrb	r1, [r2, r3]							/* Get condition code mask */
/*
 * The following check is unnecessary as it is covered by the _dbg_cond_simple_checks checking algorithm
 	teq		r1, #0
	beq		_dbg_check_arm_condcode_exit
*/
	teq		r1, #0xFF
	bne		_dbg_cond_simple_checks


/*
 * Complex Checks:
 *		We assume that CHKNV and CHKC are mutually exclusive.
 *		In addition, it is possible for CHKNV, CHKC and CHKZ to
 *		be cleared, in which case it'll return True (default)
 *
 *
 * will_execute = TRUE [default condition]
 * If (CHKNV) set
 *		// Only N/V, and Z flags are involved
 *		NEQV_Flag = (N == V)
 *		will_execute = (NEQV_Flag == NEQV_Mask)
 *
 * If (CHKC) set
 *		// Only C and Z flags are involved
 *		will_execute = (C_Flag == CSet_Mask)
 *
 * If (CHKZ) set
 *		z_match = (Z_Flag == ZSet_Mask)
 *		If (AND bit set)
 *			will_execute = will_execute && z_match
 *		else
 *			will_execute = will_execute || z_match
 *
 */
_dbg_cond_complex_checks:
	sub		r3, r3, #COMPLEX_CONDCODE_START			/* Convert complex condition code in R3 to new index (0-3) */
	ldr		r2, =debug_armComplexCCTable
	ldrb	r1, [r2, r3]							/* Get complex condition code bitmap in R1 */

_cond_check_nv:
	tst		r1, #COMPLEX_CONDCODE_CHKNV_MASK
	beq		_cond_check_c							/* CHECKNV not set, so skip */
	ands	r2, r5, #(COMPLEX_CONDCODE_NFLAG | COMPLEX_CONDCODE_VFLAG)	/* Is (N == V == 0)? */
	teqne	r2, #(COMPLEX_CONDCODE_NFLAG | COMPLEX_CONDCODE_VFLAG)		/* No, Is (N == V == 1)? */

	moveq	r2, #COMPLEX_CONDCODE_NEQV_MASK			/* EQ: Either (N == V == 0) or (N == V == 1), set R2: COMPLEX_CONDCODE_NEQV_MASK */
	movne	r2, #0									/* NE: N != V, clear R2 */
	and		r3, r1, #COMPLEX_CONDCODE_NEQV_MASK		/* R3: Extract NEQV Mask Value */
	teq		r2, r3									/* Does N/V Condition match NEQV Mask value? */
	movne	r0, #FALSE								/* No, so will_execute = FALSE (for now) */
	b		_cond_check_z

#if 0
	bne		_cond_nnev								/* No, so (N != V) */

	/* EQ: Either (N == V == 0) or (N == V == 1) */
_cond_neqv:
	tst		r1, #COMPLEX_CONDCODE_NEQV_MASK			/* Is (N == V) mask set? */
	moveq	r0, #FALSE								/* No, so will_execute = FALSE (for now) */
	b		_cond_check_z

	/* Else, N != V */
_cond_nnev:
	tst		r1, #COMPLEX_CONDCODE_NEQV_MASK			/* Is (N == V) mask set? */
	movne	r0, #FALSE								/* Yes, so will_execute = FALSE (for now) */
	b		_cond_check_z
#endif

_cond_check_c:
	tst		r1, #COMPLEX_CONDCODE_CHKC_MASK
	beq		_cond_check_z							/* CHECKC not set, so skip */

	/* Use R2 to store C Flag, R3 to store CSet Mask */
	and		r2, r5, #COMPLEX_CONDCODE_CFLAG			/* r2 = C flag */
	and		r3, r1, #COMPLEX_CONDCODE_CSET_MASK		/* r3 = CSet mask */
	teq		r2, r3									/* Does C flag == CSet mask */
	movne	r0, #FALSE								/* No, so C flag failed match */

_cond_check_z:
	tst		r1, #COMPLEX_CONDCODE_CHKZ_MASK
	beq		_dbg_check_arm_condcode_exit			/* No additional checks needed, exit */

	/* Use R2 to store Z Flag, R3 to store ZSet Mask */
	and		r2, r5, #COMPLEX_CONDCODE_ZFLAG			/* r2 = Z flag */
	and		r3, r1, #COMPLEX_CONDCODE_ZSET_MASK		/* r3 = ZSet mask */
	teq		r2, r3									/* Does Z flag == ZSet mask */
	moveq	r3, #TRUE								/* Zero, so z flag matched */
	movne	r3, #FALSE								/* Non-zero, so z flag failed match */

_cond_andor:
	tst		r1, #COMPLEX_CONDCODE_ANDOR_MASK		/* Is ANDOR mask set? */
	andne	r0, r0, r3								/* Yes, so AND with will_execute */
	orreq	r0, r0, r3								/* No, so OR with will_execute */
	b		_dbg_check_arm_condcode_exit			/* Return will_execute (R0) */

/*
 * Simple Checks:
 *		We take advantage of the fact that only 1 bit would be set
 *		in the bitmask, by generating the corresponding actual
 *		CondSet[7:4], CondClr[3:0] value for comparison.
 *
 * will_execute = TRUE [default condition, equivalent to 0x00 (AL) ]
 * Generate CondSetClr[7:0] from CPSR[3:0]
 * will_execute = ((CondSetClr & BitMask) == BitMask)
 *
 */
_dbg_cond_simple_checks:
	eor		r2, r5, #NIBBLE0						/* R2: CondClr[3:0] = Invert CPSR[3:0] */
	orr		r2, r2, r5, lsl #4						/* R2: CondSet[7:4] | CondClr[3:0] */
	and		r2, r2, r1								/* R2: CondSetClr[7:0] & Bitmask */
	teq		r2, r1									/* ((cond_code & SetBitMask) == SetBitMask)? */
	movne	r0, #FALSE								/* Not equal, check failed */

_dbg_check_arm_condcode_exit:
	bx		lr										/* Return to caller */

/* _arm_rmshifted_val
 *      Calculate value of Shifted Rm (operand)
 *      On entry:
 *          R0[11:0]: Shifted Rm operand
 *      On exit:
 *          R0: value of Shifted Rm
 *          R1, R2, R3: destroyed
 */
_arm_rmshifted_val:
    stmfd   sp!, {lr}
    ldr     r3, =(NIBBLE2|BYTE0)
    and     r3, r0, r3                      /* 12 bit Shifted Register operand, copied to R3 */
    and     r2, r3, #NIBBLE0                /* Register Rn Enum in R2 */
    _regenum2index  r2, r2                  /* Convert Enum into Index in R2 */
    _getdbgregisterfromindex r2, r0         /* Retrieve Register Rn contents from Index (R2) into R0 */

    tst     r3, #0x10                       /* B4: Immediate (0) or Register (1) shift count */
    /* check bitshift op */
    and     r3, r3, #0x60                   /* shift type */
    mov     r3, r3, lsr #5                  /* convert into shift type jumptable index */
    bne     _arm_get_reg_shift              /* Flags set previously via TST r3 (B4) */
_arm_calc_const_shift:
    movs    r1, r3, lsr #7                  /* Immediate shift count, 5 bit unsigned value in R1 */
    bne     _arm_calc_shifted_rm_val        /* Non-zero shift count, process normally */
    /* Must check for RRX == ROR #0 */
    teq     r3, #0x3                        /* ROR == 0x3 */
    addeq   r3, r3, #1
    b       _arm_calc_shifted_rm_val

_arm_get_reg_shift:
    mov     r2, r3, lsr #8                  /* Register-based shift count, 4 bit register enum in R2 */
    _regenum2index  r2, r2                  /* Convert Enum into Index in R2 */
    _getdbgregisterfromindex r2, r1         /* Retrieve Register value (shift count) from Index (R2) into R1 */

_arm_calc_shifted_rm_val:
    _dbg_jumpTableHandler   debug_regShiftJumpTable, r2, r3     /* Calculate RmShifted value from R0: Rn Register val, R1: Shift/Rotate val */
    ldmfd   sp!, {pc}

/* Rm Shifted Shift Type Jump Table Routines
 * On entry:
 *      R0: Register Rm
 *      R1: Shift/Rotate Amount
 * On exit:
 *      R0: RmShifted result
 *      R1: destroyed
 *
 */
_reg_lsl:
    lsl     r0, r0, r1
    bx      lr

_reg_lsr:
    lsr     r0, r0, r1
    bx      lr

_reg_asr:
    asr     r0, r0, r1
    bx      lr

_reg_ror:
    ror     r0, r0, r1
    bx      lr

_reg_rrx:
    _getdbgregister DBGSTACK_USERCPSR_INDEX, r1	/* Retrieve CPSR contents into R1 */
    ands    r1, r1, #CPSR_CFLAG            		/* Keep C Flag */
    movne   r1, #0x80000000                 	/* Set B31 if C Flag set */
    lsr     r0, r0, #1                      	/* Rm >> 1 */
    orr     r0, r0, r1                      	/* Put C flag into B31 */
    bx      lr


/* _arm_data_instr_handler
 *      ARM Data Processing Instruction with Rd == R15
 *      On entry:
 *          R4: Opcode of instruction to be executed
 *          R5[3:0]: CPSR condition codes
 *          R6: Default Following Instruction Address (PC+4)
 *      On exit:
 *          R0: following instruction address
 *          R1-R7: Destroyed
 */
_arm_data_instr_handler:
    stmfd   sp!, {lr}
    ldr     r1, =ARM_DATA_INSTR_MASK
    and     r3, r4, r1                      /* Keep base instruction Opcode in R3 */
    ldr     r1, =ARM_DATA_INSTR_MSRMRS
    teq     r3, r1                          /* Check for MSR / MRS instruction */

_arm_is_msr_mrs_instr:
    moveq   r0, r6                          /* Copy default next instruciton address to R0 */
    beq     _exit_arm_data_instr_handler    /* Return default next instruction address */

    /* Not MSR / MRS, so process normally */
_arm_check_operand2_type:
    tst     r4, #ARM_DATA_INSTR_IMMREG      /* Check for Immediate (1) or Register (0) Operand 2 */
    beq     _arm_op2_is_reg

_arm_op2_is_imm:
    and     r1, r4, #BYTE0                  /* 8 bit unsigned constant in R1 */
    and     r2, r4, #NIBBLE2                /* (rotate count / 2) in R2[11:8] */
    lsr     r2, r2, #7                      /* actual rotate count in R2[4:0] */
    ror     r1, r1, r2                      /* Rotated constant in R1 */
    b       _arm_get_operand1_val

_arm_op2_is_reg:
    ldr     r1, =(NIBBLE2|BYTE0)
    and     r0, r4, r1                      /* 12 bit register operand in R1 */
    bl      _arm_rmshifted_val              /* R0 contains the Rm shifted val */
    mov     r1, r0                          /* R1: Operand2 val */

_arm_get_operand1_val:
	bl		_dbg_data_instr_retrieve_op1val	/* R0: Register Rn (Operand1) val */

_arm_calc_data_instr_val:
    and     r3, r4, #ARM_DATA_INSTR_NORMAL  /* Mask Instruction Opcode into R3[24:21] */
    lsr     r3, r3, #21                     /* Shift Data Processing Opcode into R3[3:0] */
    /* Calculate data instruction value from R0: Register Rn (Operand1) val, R1: Operand2 val, R5[3:0]: CPSR, R6: Default Next Instr Addr */
    _dbg_jumpTableHandler   debug_dataInstrJumpTable, r2, r3    /* Next Instruction Address in R0 */
_exit_arm_data_instr_handler:
    ldmfd   sp!, {pc}

/* _dbg_data_instr_retrieve_op1val
 * Retrieve Data Instruction Operand 1 value
 * On entry:
 *      R4: Opcode of instruction to be executed
 *      R5[3:0]: CPSR condition codes
 *      R6: Default Next Instruction Address (PC+4)
 * On exit:
 *      R0: Register Rn (Operand 1) value
 *      R2, R3: Destroyed
 *
 */
_dbg_data_instr_retrieve_op1val:
    and     r3, r4, #NIBBLE4             	/* Store Rn (Operand1) Register Enum into R3[19:16] */
    lsr     r3, r3, #16                     /* Shift into R3[3:0] */
    _regenum2index  r3, r2                  /* Convert Enum into Index in R2 */
    _getdbgregisterfromindex r2, r0         /* Retrieve Register contents from Index (R2) into R0 */
    teq     r3, #REG_PC                     /* Check if it is PC relative */
    addeq   r0, r0, #8                      /* R0: Register Rn (Operand1) val; adjust for PC relative (+8) */
	bx		lr

/* Data Processing Instruction Jump Table Routines
 * On entry:
 *      R0: Register Rn (Operand 1) value
 *      R1: Operand 2 value
 *      R5[3:0]: CPSR condition codes
 *      R6: Default Next Instruction Address (PC+4)
 * On exit:
 *      R0: Calculated result
 *      R1, R2, R3: Destroyed
 *
 */
_opcode_and:
    and r0, r0, r1
    bx  lr

_opcode_eor:
    eor r0, r0, r1
    bx  lr

_opcode_sub:
    sub r0, r0, r1
    bx  lr

_opcode_rsb:
    rsb r0, r0, r1
    bx  lr

_opcode_add:
    add r0, r0, r1
    bx  lr

_opcode_adc:
    /* Op1 + Op2 + C */
    tst     r5, #(CPSR_CFLAG>> 28)          /* R5[3:0] is shifted CPSR value: Test C Flag */
    add     r0, r0, r1
    addne   r0, r0, #1                      /* Add C if set */
    bx      lr

_opcode_sbc:
    /* Op1 - Op2 + C - 1 */
    tst     r5, #(CPSR_CFLAG>> 28)          /* R5[3:0] is shifted CPSR value: Test C Flag */
    sub     r0, r0, r1
    subeq   r0, r0, #1                      /* If C clear, subtract 1, else (C - 1) = 0 */
    bx      lr

_opcode_rsc:
    /* Op2 - Op1 + C - 1 */
    tst     r5, #(CPSR_CFLAG>> 28)          /* R5[3:0] is shifted CPSR value: Test C Flag */
    rsb     r0, r0, r1
    subeq   r0, r0, #1                      /* If C clear, subtract 1, else (C - 1) = 0 */
    bx  lr

_opcode_tst:
_opcode_teq:
_opcode_cmp:
_opcode_cmn:
    mov r0, r6          /* Next Instruction Address is not modified */
    bx  lr

_opcode_orr:
    orr r0, r0, r1
    bx  lr

_opcode_mov:
    mov r0, r1          /* Operand 1 is ignored */
    bx  lr

_opcode_bic:
    bic r0, r0, r1
    bx  lr

_opcode_mvn:
    mvn r0, r1          /* Operand 1 is ignored */
    bx  lr

/* _arm_bx_blx_handler
 *      BX or BLX Rm Handler. Note v4t does not have BLX instr
 *      On entry:
 *          R4: Opcode of instruction to be executed
 *          R5[3:0]: CPSR condition codes
 *          R6: Default Following Instruction Address (PC+4)
 *      On exit:
 *          R0: following instruction address (B0 set to indicate Thumb mode)
 *          R1,R2: destroyed
 */
_arm_bx_blx_handler:
    stmfd   sp!, {lr}
    and     r2, r4, #NIBBLE0                /* Register Rn Enum in R2 */
    _regenum2index  r2, r1                  /* Convert Enum into Index in R1 */
    _getdbgregisterfromindex r1, r0         /* Retrieve Register contents from Index (R1) into R0 */
	teq		r2, #REG_PC
	addeq	r0, r0, #8						/* Adjust PC relative register value (for BX PC) */
	/* Here, the register value would have B0 set to indicate switch to Thumb mode  */
    ldmfd   sp!, {pc}

/* _arm_ldr_pc_handler
 *      LDR with Rd = PC
 *      On entry:
 *          R4: Opcode of instruction to be executed
 *          R5[3:0]: CPSR condition codes
 *          R6: Default Following Instruction Address (PC+4)
 *      On exit:
 *          R0: following instruction address
 *          R1, R2, R3, R4, R5: destroyed
 */

_arm_ldr_pc_handler:
    stmfd   sp!, {lr}

	mov		r1, #0							/* R1: Post-Indexed Offset (cleared) */
    tst     r4, #ARM_LDR_INSTR_PREPOST      /* Pre (1) or Post (0) Indexed */
	beq		_get_rn_val						/* If Post-Indexed, just use Rn directly */

    /* Pre-Indexed */
    ldr     r0, =(NIBBLE2|BYTE0)
    and     r0, r4, r0                      /* R0: 12 bit Immediate value or Shifted Reg operand */
    tst     r4, #ARM_LDR_INSTR_REGIMM       /* Register (1) or Immediate (0) */
    beq     _calc_ldr_pc_offset             /* Immediate value is already in R0 */

_get_shiftedreg_val:
    bl      _arm_rmshifted_val              /* Convert Rm shifted operand in R0 into value in R0 */

_calc_ldr_pc_offset:
	mov		r1, r0							/* Keep Offset in R1 */
_get_rn_val:
	bl		_dbg_data_instr_retrieve_op1val	/* R0: Register Rn (Operand1) val */
_calc_op1val_with_offset:
    tst     r4, #ARM_LDR_INSTR_UPDOWN       /* Add (1) or Subtract (0) */
    addne   r0, r0, r1                      /* If Add, R0 = Rn + Offset */
    subeq   r0, r0, r1                      /* If Sub, R0 = Rn - Offset */

_get_ldr_pc_val_from_mem:
	ldr		r0, [r0]						/* Retrieve value from Memory at address given in R0 */
    ldmfd   sp!, {pc}

/* _arm_ldm_pc_handler
 *      LDM {pc}
 *      On entry:
 *          R4: Opcode of instruction to be executed
 *          R5[3:0]: CPSR condition codes
 *          R6: Default Following Instruction Address (PC+4)
 *      On exit:
 *          R0: following instruction address
 *          R2, R3: destroyed
 *
 *      Note: The algorithm from eCos arm_stub.c does not deal with the Pre/Post-Indexed addressing (P) bit.
 *			The algorithm here loads different content using LDM based on the value of the P bit.
 */
_arm_ldm_pc_handler:
    stmfd   sp!, {lr}
	bl		_dbg_data_instr_retrieve_op1val	/* R0: Register Rn (Operand1) val */

_arm_get_regcount:
    mov     r2, #0                          /* Initialize reg_count (R2) to 0 */
    mov     r3, r4, lsl #16					/* Keep HLFWORD0 containing vector bits in R3[31:16] */
	/* This shortens the checking to a max of 16 iterations, since the PC bit should be set */
1:  movs    r3, r3, lsl #1                  /* count number of '1' bits */
    addcs   r2, r2, #1                      /* increment reg_count (R2) if C Flag set */
    bne     1b                              /* continue until vector is empty */

	/* Pre-Incr:  Rn += reg_count x 4
	 * Post-Incr: Rn += (reg_count - 1) x 4
	 * Pre-Decr:  Rn -= 4
	 * Post-Decr: Rn
	 */

_arm_check_updown_offset:
    tst     r4, #ARM_LDM_INSTR_UPDOWN       /* Check Up (1) or Down (0) */
    beq		_arm_check_prepost_decr

_arm_check_prepost_incr:
    tst     r4, #ARM_LDM_INSTR_PREPOST      /* Check Pre (1) or Post (0) */
    subeq	r2, r2, #1						/* Post-Incr: Decrement reg_count in R2 */
    add		r0, r0, r2, lsl #2              /* Increment Offset: Rn (R0) += reg_count (R2) x 4 */
	b		_get_ldm_pc_val_from_mem

_arm_check_prepost_decr:
    tst     r4, #ARM_LDM_INSTR_PREPOST      /* Check Pre (1) or Post (0) */
	/* Post-Decr: Rn unchanged */
    subne   r0, r0, #4                      /* Pre-Decr: Rn (R0) -= 4 */

_get_ldm_pc_val_from_mem:
    ldr     r0, [r0]                        /* Retrieve stack content for new PC value */
    ldmfd   sp!, {pc}


/* _arm_b_bl_blx_handler
 *      B, BL or BLX <offset>. Note v4t does not have BLX instr
 *      On entry:
 *          R4: Opcode of instruction to be executed
 *          R5[3:0]: CPSR condition codes
 *          R6: Default Following Instruction Address (PC+4)
 *      On exit:
 *          R0: following instruction address
 *          R1: destroyed
 */

_arm_b_bl_blx_handler:
    stmfd   sp!, {lr}

_arm_b_bl_blx_get_offset:
    and     r0, r4, #(BYTE2|BYTE1|BYTE0)    /* Encoded Branch offset in R4[23:0] */
    lsl		r0, r0, #(32-24)				/* Shift to R0[31:8] */
    asr		r0, r0, #(32-26)				/* Actual Signed offset = Encode Offset x 4 in R0[25:0] */
	add		r1, r6, #4						/* R1: (PC+4) + 4 */
    add     r0, r0, r1                      /* Calculate Branch Target Address R0: (PC+8) + signed offset */

#ifndef __ARM6OR7__
	/* armv5t or later, has BLX support */
	and		r1, r4, #ARM_BLX_INSTR_MASK		/* Mask out Condition Code and Opcode */
	teq		r1, #ARM_BLX_INSTR_BLX			/* Look for BLX */
	bne		_exit_arm_b_bl_blx_handler		/* No, it is a B/BL instruction */
	tst		r4, #ARM_BLX_INSTR_HBIT			/* H bit for Thumb Halfword Address */
	orrne	r0, r0, #0x02					/* Set Halfword Address R0[1] */
    orr   	r0, r0, #BKPT_STATE_THUMB_FLAG  /* Set R0[0] since BLX instr used to switch to Thumb mode */
#endif

_exit_arm_b_bl_blx_handler:
    ldmfd   sp!, {pc}

/* _arm_coproc_swi_handler
 *      SVC (SWI) or Coprocessor instruction
 *      On entry:
 *          R4: Opcode of instruction to be executed
 *          R5[3:0]: CPSR condition codes
 *          R6: Default Following Instruction Address (PC+4)
 *      On exit:
 *          R0: following instruction address
 */

_arm_coproc_swi_handler:
    and     r0, r4, #ARM_SWI_INSTR_MASK
    teq     r0, #ARM_SWI_INSTR_VAL          /* SVC (SWI) instruction */

    ldreq   r0, =SVC_VECTOR                 /* SWI: Return SVC Vector Address */
    movne   r0, r6                          /* CoProc: Use default Following Instruction Address */
_exit_arm_coproc_swi_handler:
	bx	    lr

/* _thumb_bx_blx_handler
 *      BX or BLX Handler. Note: Link (L:b7) is not checked in the mask; armv4t does not support BLX.
 *      On entry:
 *          R4: Opcode of instruction to be executed
 *          R5[3:0]: CPSR condition codes
 *          R6: Default Following Instruction Address (PC+2)
 *      On exit:
 *          R0: following instruction address (B0 set to indicate Thumb mode)
 *          R1, R2: destroyed
 */
_thumb_bx_blx_handler:
    stmfd   sp!, {lr}
    and     r2, r4, #THUMB_BLX_INSTR_REG_RNMASK	/* Register Rn Enum in R2[6:3] (Hi-Reg indicated by B6) */
    mov		r2, r2, lsr #3					/* Shift Rn Enum to R2[3:0] */
    _regenum2index  r2, r1                  /* Convert Enum into Index in R1 */
    _getdbgregisterfromindex r1, r0         /* Retrieve Register contents from Index (R1) into R0 */
	teq		r2, #REG_PC
	addeq	r0, r0, #4						/* Adjust PC relative register value (for BX PC) */
	/* Here, the register value would have R0[0] set to indicate switch to Thumb mode  */
    ldmfd   sp!, {pc}

/* _thumb_poppc_handler
 *     PUSH/POP, specifically POP {Rlist,PC}
 *      On entry:
 *          R4: Opcode of instruction to be executed
 *          R5[3:0]: CPSR condition codes
 *          R6: Default Following Instruction Address (PC+2)
 *      On exit:
 *          R0: following instruction address (B0 set to indicate Thumb mode)
 *          R1-R3: destroyed
 */
_thumb_poppc_handler:
    stmfd   sp!, {lr}

_thumb_get_SP_val:
    _getdbgregister DBGSTACK_USERSP_INDEX, r1	/* Retrieve SP contents into R1 */

_thumb_get_regcount:
    mov     r3, r4, lsl #24					/* Keep BYTE0 containing vector bits in R3[31:24] */
	/* POP is equivalent to LDMFD. Load PC is encoded in b8,
	 * the 8-bit vector is for Lo registers.
	 * This shortens the checking to a max of 8 iterations
	 */
1:  movs    r3, r3, lsl #1                  /* count number of '1' bits */
	addcs	r1, r1, #4						/* Walk the stack to locate the PUSHed LR (POP PC) value */
    bne     1b                              /* continue until vector is empty */
	ldr		r0, [r1]						/* Retrieve new PC value */
#if 0
	/* PC Value should have B0 set */
    orr     r0, r0, #BKPT_STATE_THUMB_FLAG	/* Force R0[0] since it is used to indicates Thumb mode */
#endif
    ldmfd   sp!, {pc}

/* _thumb_bcond_swi_handler
 *      B<cond> or SWI (SVC)
 *      On entry:
 *          R4: Opcode of instruction to be executed
 *          R5[3:0]: CPSR condition codes
 *          R6: Default Following Instruction Address (PC+2)
 *      On exit:
 *          R0: following instruction address (B0 set to indicate Thumb mode)
 *          R1-R3: destroyed
 */
_thumb_bcond_swi_handler:
    stmfd   sp!, {lr}
    and     r2, r4, #THUMB_BCOND_SWI_INSTR_CONDMASK		/* Keep Condition Code R2[11:8] */
    teq     r2, #THUMB_BCOND_SWI_INSTR_SWI              /* SVC (SWI) instruction */
_thumb_swi_instr:
    ldreq   r0, =SVC_VECTOR                 /* Return SVC Vector Address */
    beq     _exit_thumb_bcond_swi_handler	/* Switch to ARM mode for SVC */
_thum_bcond_unused_instr:
    teq		r2, #THUMB_BCOND_SWI_COND_UNUSED
    moveq   r0, r6                          /* False (don't execute), so use Default Following Instruction Address */
    beq     _exit_thumb_bcond_instr

_thumb_bcond_instr:
	stmfd	sp!, {r4}						/* Preserve Opcode in R4 */
	lsl		r4, r2, #(32-12)		        /* Shift condition code in R2[11:8] to R0[31:28] to match ARM cond-code format */
	bl		_dbg_check_arm_condcode	        /* Use ARM condition code checking routine to test (R4, R6 unchanged) */
	ldmfd	sp!, {r4}						/* Restore Opcode in R4 */
    teq     r0, #FALSE
    moveq   r0, r6                          /* False (don't execute), so use Default Following Instruction Address */
    beq     _exit_thumb_bcond_instr

_thumb_calc_bcond_offset:
    lsl		r0, r4, #(32-8)					/* Shift 8-bit offset in R4[7:0] to R0[31:24] */
    asr		r0, r0, #(32-9)					/* Convert into 9-bit signed offset in R0[8:0] */
    add		r0, r6, r0						/* PC+2 + signed offset */
    add		r0, r0, #2						/* PC+4 + signed offset */
_exit_thumb_bcond_instr:
    orr     r0, r0, #BKPT_STATE_THUMB_FLAG  /* Set R0[0] since it is used to indicates Thumb mode */
_exit_thumb_bcond_swi_handler:
    ldmfd   sp!, {pc}

/* _thumb_b_handler
 *      B
 *      On entry:
 *          R4: Opcode of instruction to be executed
 *          R5[3:0]: CPSR condition codes
 *          R6: Default Following Instruction Address (PC+2)
 *      On exit:
 *          R0: following instruction address (B0 set to indicate Thumb mode)
 *          R1: destroyed
 *		Note: The signed offset is 12-bits (encoded value x 2)
 */
_thumb_b_handler:
    stmfd   sp!, {lr}
    lsl		r0, r4, #(32-11)		/* Shift 11-bit offset in R4[10:0] to R0[31:21] */
    asr		r0, r0, #(32-12)		/* Convert into 12-bit signed offset in R0[11:0] */
    add		r0, r6, r0				/* PC+2 + signed offset */
    add		r0, r0, #2				/* PC+4 + signed offset */
    orr     r0, r0, #BKPT_STATE_THUMB_FLAG /* Set R0[0] since it is used to indicates Thumb mode */
    ldmfd   sp!, {pc}

/* _thumb_long_bl_blx_handler
 *      Long BL or BLX (4 bytes) Note: b11 (H) indicates 1st or 2nd instr; armv4t does not support BLX.
 *      On entry:
 *          R4: Opcode of instruction to be executed
 *          R5[3:0]: CPSR condition codes
 *          R6: Default Following Instruction Address (PC+2)
 *      On exit:
 *          R0: following instruction address (B0 set to indicate Thumb mode)
 *          R1, R2, R3: destroyed
 *			R6: Subseqent Instruction Address (PC+4) if first instruction is valid, else unchanged (PC+2)
 *		Note: The BL instruction (0xFxxx) should be in pairs (Dual 16-bit instructions).
 *		The first instruction should have (H=0) to indicate the upper 11 bits of the encoded offset
 *		The second instruction should have (H=1) to indicate the lower 11 bits of the encoded offset
 *		The signed offset is 23 bits (encoded value x 2)
 *
 *		Note2: The BLX instruction (0xExxx) encodes the first instruciton using BL (0xFxxx) with H=0,
 *		while the second instruction has a different opcode value (0xExxx), with H=1.
 *		BLX is only used to switch to an ARM target.
 */
_thumb_long_bl_blx_handler:
    stmfd   sp!, {lr}
_thumb_check_1st_bl_blx_instruction:
	tst		r4, #THUMB_BLX_INSTR_IMM_HBIT	/* Check H bit */
	bne		_return_default_thumb_following_instr	/* H=1 as first instruction shouldn't happen */
_thumb_check_2nd_bl_blx_instruction:
    ldrh	r3, [r6]						/* Get second instruction in pair at PC+2 into R3 */
	add		r6, r6, #2						/* Skip to Subsequent Instruction (PC+4) */
	tst		r3, #THUMB_BLX_INSTR_IMM_HBIT	/* Check H bit */
	beq		_return_default_thumb_following_instr	/* H=0 as second instruction shouldn't happen */

_thumb_concat_branch_offset:
    lsl		r0, r4, #(32-11)			/* Shift first instruction 11-bit offset in R4[10:0] to R0[31:21] */
    asr		r0, r0, #(32-23)			/* Convert into 12-bit signed offset in R0[22:12] */
    lsl		r2, r3, #(32-11)			/* Shift second instruction 11-bit offset in R3[10:0] to R2[31:21] */
    lsr		r2, r2, #(32-12)			/* Convert into 12-bit unsigned offset in R2[11:0] */
	orr		r0, r0, r2					/* Combine offsets */
    add		r0, r6, r0					/* PC+4 + signed offset */

_thumb_check_bl_blx_pair:
    and		r3, r3, #THUMB_BLX_INSTR_IMM_MASK	/* Keep second instruction opcode in R3 */
	teq		r3, #THUMB_BLX_INSTR_IMM_BL			/* Look for BL */
	beq		_flag_thumb_instr_addr				/* Return BL target address in R0 */

#ifndef __ARM6OR7__
	/* v5t or higher architecture */
	teq		r3, #THUMB_BLX_INSTR_IMM_BLX		/* Look for BLX */
	biceq	r0, r0, #0x03						/* Match, Force ARM address */
    beq		_exit_thumb_long_bl_blx_handler
#endif

_return_default_thumb_following_instr:
	/* FIXME: This assumes that once the 4-byte sequence check fails, it will return PC+4,
	 * regardless of whether the second instruction is a valid BL/BLX instruction or not.
	 */
	mov		r0, r6								/* Return default Following/Subsequent Instruction Address  */
_flag_thumb_instr_addr:
    orr		r0, r0, #BKPT_STATE_THUMB_FLAG      /* Set R0[0] since it is used to indicates Thumb mode */

_exit_thumb_long_bl_blx_handler:
    ldmfd   sp!, {pc}

