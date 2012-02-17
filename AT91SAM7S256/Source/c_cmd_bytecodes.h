#ifndef C_CMD_BYTECODES
#define C_CMD_BYTECODES
//
// opcode definitions
// symbol, bits, arg format
//
#define OPCODE_COUNT  0x51

//Family: Math
#define OP_ADD    	0x00	// dest, src1, src2
#define OP_SUB    	0x01	// dest, src1, src2
#define OP_NEG    	0x02	// dest, src
#define OP_MUL    	0x03	// dest, src1, src2
#define OP_DIV    	0x04	// dest, src1, src2
#define OP_MOD    	0x05	// dest, src1, src2

//Family: Logic
#define OP_AND    	0x06	// dest, src1, src2
#define OP_OR     	0x07	// dest, src1, src2
#define OP_XOR    	0x08	// dest, src1, src2
#define OP_NOT    	0x09	// dest, src

//Family: Bit manipulation
#define OP_CMNT     0x0A  // dest, src
#define OP_LSL      0x0B  // dest, src, bits
#define OP_LSR      0x0C  // dest, src, bits
#define OP_ASL      0x0D  // dest, src, bits
#define OP_ASR      0x0E  // dest, src, bits
#define OP_ROTL     0x0F  // dest, src, bits
#define OP_ROTR     0x10  // dest, src, bits

//Family: Comparison
#define OP_CMP    	0x11	// dest, src1, src2
#define OP_TST    	0x12	// dest, src
#define OP_CMPSET 	0x13	// dest, src, testsrc, testsrc
#define OP_TSTSET 	0x14	// dest, src, testsrc

//Family: Array ops
#define OP_INDEX  	0x15	// dest, src, index
#define OP_REPLACE	0x16	// dest, src, index, val
#define OP_ARRSIZE	0x17	// dest, src
#define OP_ARRBUILD	0x18	// instrsize, dest, src1, src2, …
#define OP_ARRSUBSET	0x19	// dest, src, index, length
#define OP_ARRINIT	0x1A	// dest, elem, length

//Family: Memory ops
#define OP_MOV    	0x1B	// dest, src
#define OP_SET    	0x1C	// dest, imm

//Family: String ops
#define OP_FLATTEN	0x1D	// dest, src
#define OP_UNFLATTEN	0x1E	// dest, err, src, type
#define OP_NUMTOSTRING	0x1F	// dest, src
#define OP_STRINGTONUM	0x20	// dest, offsetpast, src, offset, default
#define OP_STRCAT 	0x21	// instrsize, dest, src1, src2, …
#define OP_STRSUBSET	0x22	// dest, src, index, length
#define OP_STRTOBYTEARR	0x23	// dest, src
#define OP_BYTEARRTOSTR	0x24	// dest, src

//Family: Control flow
#define OP_JMP    	0x25	// offset
#define OP_BRCMP  	0x26	// offset, src1, src2
#define OP_BRTST  	0x27	// offset, src
#define OP_SYSCALL	0x28	// func, args
#define OP_STOP   	0x29	// stop?

//Family: Clump scheduling
#define OP_FINCLUMP	0x2A	// start, end
#define OP_FINCLUMPIMMED	0x2B	// clumpID
#define OP_ACQUIRE	0x2C	// mutexID
#define OP_RELEASE	0x2D	// mutexID
#define OP_SUBCALL	0x2E	// subroutine, callerID
#define OP_SUBRET 	0x2F	// callerID

//Family: IO ops
#define OP_SETIN  	0x30	// src, port, propid
#define OP_SETOUT 	0x31	// src, port, propid
#define OP_GETIN  	0x32	// dest, port, propid
#define OP_GETOUT 	0x33	// dest, port, propid

//Family: Timing
#define OP_WAIT   	0x34	// dest, src
#define OP_GETTICK	0x35	// dest

//Family: Math NEW
#define OP_SQRT   	0x36	// dest, src
#define OP_ABS    	0x37	// dest, src

// JCH ADDS
#define OP_WAITI           0x64  // immed
#define OP_WAITV           0x65  // var 
#define OP_SIGN            0x66  // dest, src
#define OP_STOPCLUMPIMMED  0x67  // clumpID
#define OP_STARTCLUMPIMMED 0x68  // clumpID
#define OP_PRIORITY        0x69  // clumpID, pri
#define OP_FMTNUM          0x6a  // dest, fmt, src1
#define OP_ARROP           0x6b  // cmd, dest, src, idx, len

// math ops (float)
#define OP_ACOS     0x6c  // dest, src
#define OP_ASIN     0x6d  // dest, src
#define OP_ATAN     0x6e  // dest, src
#define OP_CEIL     0x6f  // dest, src
#define OP_EXP      0x70  // dest, src
#define OP_FLOOR    0x71  // dest, src
#define OP_TAN      0x72  // dest, src
#define OP_COS      0x74  // dest, src
#define OP_LOG      0x76  // dest, src
#define OP_LOG10    0x77  // dest, src
#define OP_SIN      0x78  // dest, src
#define OP_TRUNC    0x7a  // dest, src
#define OP_FRAC     0x7b  // dest, src

#define OP_ATAN2    0x7c  // dest, src1, src2
#define OP_POW      0x7d  // dest, src1, src2

#define OP_MULDIV   0x7e  // dest, src1, src2, src3

// transcendental opcodes that use degrees instead of radians
#define OP_ACOSD    0x7f  // dest, src
#define OP_ASIND    0x80  // dest, src
#define OP_ATAND    0x81  // dest, src
#define OP_TAND     0x82  // dest, src
#define OP_COSD     0x84  // dest, src
#define OP_SIND     0x86  // dest, src
#define OP_ATAN2D   0x88  // dest, src1, src2


// hyperbolic transcendental functions
#define OP_TANH     0x73  // dest, src
#define OP_COSH     0x75  // dest, src
#define OP_SINH     0x79  // dest, src
#define OP_TANHD    0x83  // dest, src
#define OP_COSHD    0x85  // dest, src
#define OP_SINHD    0x87  // dest, src

// misc other JCH additions
#define OP_ADDROF   0x89  // dest, src, rel

#define OP_FINCLUMPVAR   0x90 // clumpID
#define OP_SUBCALLVAR    0x91 // subroutine, callerID
#define OP_STOPCLUMPVAR  0x92 // clumpID
#define OP_STARTCLUMPVAR 0x93 // clumpID

#define OP_JMPABSVAR     0x94 // pcvar
#define OP_BRCMPABSVAR   0x95 // pcvar, src1, src2
#define OP_BRTSTABSVAR   0x96 // pcvar, src

// array operation definitions
#define OPARR_SUM     0x00
#define OPARR_MEAN    0x01
#define OPARR_SUMSQR  0x02
#define OPARR_STD     0x03
#define OPARR_MIN     0x04
#define OPARR_MAX     0x05
#define OPARR_SORT    0x06
#define OPARR_TOUPPER 0x07
#define OPARR_TOLOWER 0x08

// condition code definitions
#define OPCC1_LT   0x00
#define OPCC1_GT   0x01
#define OPCC1_LTEQ 0x02
#define OPCC1_GTEQ 0x03
#define OPCC1_EQ   0x04
#define OPCC1_NEQ  0x05



//
// short op definitions
//
#define USE_SHORT_OPS
#define	SHORT_OP_MOV	0
#define	SHORT_OP_ACQUIRE	1
#define	SHORT_OP_RELEASE	2
#define	SHORT_OP_SUBCALL	3

//
// short op mapping table
//
static UBYTE ShortOpMap[4] = 
{
	OP_MOV,
	OP_ACQUIRE,
	OP_RELEASE,
	OP_SUBCALL
};

#endif  // C_CMD_BYTECODES
