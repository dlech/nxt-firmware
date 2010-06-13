#ifndef C_CMD_BYTECODES
#define C_CMD_BYTECODES
//
// opcode definitions
// symbol, bits, arg format
//
#define OPCODE_COUNT	0x38

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
#define OP_CMNT   	0x0A	// dest, src
#define OP_LSL    	0x0B	// dest, src
#define OP_LSR    	0x0C	// dest, src
#define OP_ASL    	0x0D	// dest, src
#define OP_ASR    	0x0E	// dest, src
#define OP_ROTL   	0x0F	// dest, src
#define OP_ROTR   	0x10	// dest, src

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
