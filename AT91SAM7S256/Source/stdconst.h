//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dkandlun                                        $
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: stdconst.h                                    $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/stdcon $
//
// Platform        C
//


#ifndef   STDCONST
#define   STDCONST

#ifndef NULL
#define NULL    ((void *)0)
#endif

#define   TRUE                          1
#define   FALSE                         0

typedef   unsigned char                 UCHAR;
typedef   unsigned short                USHORT;

typedef   unsigned char                 UBYTE;
typedef   signed char                   SBYTE;
typedef   unsigned short int            UWORD;
typedef   signed short int              SWORD;
typedef   unsigned long                 ULONG;
typedef   signed long                   SLONG;

typedef   ULONG*                        PULONG;
typedef   USHORT*                       PUSHORT;  
typedef   UCHAR*                        PUCHAR;
typedef   char*                         PSZ;

#define   BASETYPES


#endif
