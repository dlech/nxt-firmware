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

#include "config.h"

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

#ifdef __GNUC__
#define DEFINE_DATA(type, name) \
    extern const type name ## _; \
    const type * const name = &name ## _; \
    const type name ## _
#define BEGIN_DATA {
#define END_DATA }
#define POINTER_TO_DATA(name) (&name ## _)
#define SIZEOF_DATA(name) (sizeof_ ## name)
#else
#define DEFINE_DATA(type, name) \
    const type name[]
#define BEGIN_DATA
#define END_DATA
#define POINTER_TO_DATA(name) (name)
#define SIZEOF_DATA(name) (sizeof (name))
#endif

#ifdef __GNUC__
#define __ramfunc __attribute__ ((section (".fastrun"), optimize ("no-jump-tables")))
#define __ramdata __attribute__ ((section (".data")))
#else
#define __ramdata
#endif

#endif
