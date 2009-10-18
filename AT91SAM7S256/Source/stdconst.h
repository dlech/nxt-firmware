//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Us8jamlo                                        $
//
// Revision date   $Date:: 3/04/05 2:59p                                     $
//
// Filename        $Workfile:: stdconst.h                                    $
//
// Version         $Revision:: 3                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/stdconst.h $
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
