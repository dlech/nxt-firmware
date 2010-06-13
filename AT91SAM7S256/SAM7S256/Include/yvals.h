/* yvals.h internal configuration header file. */
/* Copyright (c) 2001-2003 IAR Systems. All rights reserved. */

/* __INTRINSIC
 *
 * Note: Redefined each time yvals.h is included to ensure that intrinsic
 * support could be turned off individually for each system header file.
 */
#ifdef __INTRINSIC
  #undef __INTRINSIC
#endif /* __INTRINSIC */

#ifndef __NO_INTRINSIC
  #define __INTRINSIC __intrinsic
#else
  #define __INTRINSIC
#endif


#ifndef _YVALS
#define _YVALS

#ifndef _SYSTEM_BUILD
#pragma system_include
#endif

                /* Convenience macros */
#define _GLUE_B(x,y) x##y
#define _GLUE(x,y) _GLUE_B(x,y)

#define _GLUE3_B(x,y,z) x##y##z
#define _GLUE3(x,y,z) _GLUE3_B(x,y,z)

#define _STRINGIFY_B(x) #x
#define _STRINGIFY(x) _STRINGIFY_B(x)

                /* Versions */
#define _CPPLIB_VER     312

#ifndef __IAR_SYSTEMS_LIB__
  #define __IAR_SYSTEMS_LIB__ 3
#endif

#if (__IAR_SYSTEMS_ICC__ < 6) || (__IAR_SYSTEMS_ICC__ > 6)
  #error "<yvals.h>  compiled with wrong (version of IAR) compiler"
#endif

/*
 * Support for some C99 or other symbols
 *
 * This setting makes available some macros, functions, etc that are 
 * beneficial.
 * 
 * Default is to include them.
 */

#ifndef _DLIB_ADD_C99_SYMBOLS
  #define _DLIB_ADD_C99_SYMBOLS 1
#endif /* _DLIB_ADD_C99_SYMBOLS */

#ifndef _DLIB_ADD_EXTRA_SYMBOLS
  #define _DLIB_ADD_EXTRA_SYMBOLS 1
#endif /* _DLIB_ADD_EXTRA_SYMBOLS */


                /* Configuration */
#include <DLib_Defaults.h>

#define _HAS_PRAGMA_PRINTF_ARGS

#ifndef _NO_RETURN
  #define _NO_RETURN
#endif /* _NO_RETURN */

                /* Floating-point */
#ifndef _NO_FLOAT_FOLDING
  #if __FLOAT_SIZE__ == __DOUBLE_SIZE__
    #define _FLOAT_IS_DOUBLE
    #define _F_FNAME(fun) _##fun
    #define _F_FUN(fun)   fun
    #define _F_CTYPE      _Dcomplex
    #define _F_CONST(obj) _##obj._Double
    #define _F_PTRCAST    (double *)
    #define _F_CAST       (double)
  #else
    #define _F_FNAME(fun) _F##fun
    #define _F_FUN(fun)   fun##f
    #define _F_CTYPE      _Fcomplex
    #define _F_CONST(obj) _F##obj._Float
    #define _F_PTRCAST
    #define _F_CAST
  #endif
  #if __LONG_DOUBLE_SIZE__ == __DOUBLE_SIZE__
    #define _LONG_DOUBLE_IS_DOUBLE
    #define _L_FNAME(fun) _##fun
    #define _L_FUN(fun)   fun
    #define _L_CTYPE      _Dcomplex
    #define _L_CONST(obj) _##obj._Double
    #define _L_PTRCAST    (double *)
    #define _L_CAST       (double)
  #else
    #define _L_FNAME(fun) _L##fun
    #define _L_FUN(fun)   fun##l
    #define _L_CTYPE      _Lcomplex
    #define _L_CONST(obj) _L##obj._Long_double
    #define _L_PTRCAST
    #define _L_CAST
  #endif
#else /* _NO_FLOAT_FOLDING */
  #define _F_FNAME(fun) _F##fun
  #define _F_FUN(fun)   fun##f
  #define _F_CTYPE      _Fcomplex
  #define _F_CONST(obj) _F##obj._Float
  #define _F_PTRCAST
  #define _F_CAST
  #define _L_FNAME(fun) _L##fun
  #define _L_FUN(fun)   fun##l
  #define _L_CTYPE      _Lcomplex
  #define _L_CONST(obj) _L##obj._Long_double
  #define _L_PTRCAST
  #define _L_CAST
#endif /* !_NO_FLOAT_FOLDING */

                /* NAMING PROPERTIES */
/* #define _STD_LINKAGE defines C names as extern "C++" */
/* #define _STD_USING   exports C names from std to global, else reversed */
#define _HAS_STRICT_LINKAGE           0       /* extern "C" in function type */

                /* THREAD AND LOCALE CONTROL */
#ifndef _MULTI_THREAD
  #define _MULTI_THREAD 0     /* 0 for no locks, 1 for multithreaded library */
#else
  #error "IARs specific library routines can't do this currently."
#endif /* _MULTI_THREAD */
#define _GLOBAL_LOCALE  0       /* 0 for per-thread locales, 1 for shared */
#define _FILE_OP_LOCKS  0       /* 0 for no file atomic locks, 1 for atomic */

                /* THREAD-LOCAL STORAGE */
#define _COMPILER_TLS   0       /* 1 if compiler supports TLS directly */
#define _TLS_QUAL       /* TLS qualifier, such as __declspec(thread), if any */

#define _HAS_EXCEPTIONS 0
#define _HAS_NAMESPACE  0
#ifdef __WCHAR_T
  #define _HAS_WCHAR_TYPE 1
#endif /* __WCHAR_T */

#if defined(__cplusplus)
  #ifndef __ARRAY_OPERATORS
    #error "<yvals.h> __ARRAY_OPERATORS not defined (c++)"
  #endif /* __ARRAY_OPERATORS */
#endif /* __cplusplus */

                /* NAMESPACE CONTROL */
#if defined(__cplusplus)
  #if _HAS_NAMESPACE
    #define _STD_BEGIN  namespace std {
    #define _STD_END    }
    #define _STD        std::

    #ifdef _STD_USING
      #define _C_STD_BEGIN    namespace std { /* only if *.c compiled as C++ */
      #define _C_STD_END      }
      #define _CSTD     std::
          {
            __dtor_rec const * * pp = (__dtor_rec const * *) (rec + 1);
            /* Point to pointer */
            rec->next = pp;
            rec->object = NULL;

    #else /* _STD_USING */
      #define _GLOBAL_USING    /* *.h in global namespace, c* imports to std */

      #define _C_STD_BEGIN
      #define _C_STD_END
      #define _CSTD     ::
    #endif /* _STD_USING */

    #define _C_LIB_DECL         extern "C" {    /* C has extern "C" linkage */
    #define _END_C_LIB_DECL     }
    #define _EXTERN_C                   extern "C" {
    #define _END_EXTERN_C               }
  #else /* _HAS_NAMESPACE */
    #define _STD_BEGIN
    #define _STD_END
    #define _STD        ::

    #define _C_STD_BEGIN
    #define _C_STD_END
    #define _CSTD       ::

    #define _C_LIB_DECL         extern "C" {
    #define _END_C_LIB_DECL     }
    #define _EXTERN_C                   extern "C" {
    #define _END_EXTERN_C               }
  #endif /* _HAS_NAMESPACE */

#else /* __cplusplus */
  #define _STD_BEGIN
  #define _STD_END
  #define _STD

  #define _C_STD_BEGIN
  #define _C_STD_END
  #define _CSTD

  #define _C_LIB_DECL
  #define _END_C_LIB_DECL
  #define _EXTERN_C
  #define _END_EXTERN_C
#endif /* __cplusplus */

#ifdef __cplusplus
  _STD_BEGIN
  typedef bool _Bool;
  _STD_END
#endif /* __cplusplus */


/* Map IAR compiler interface for long longs */
#define __LONGLONG_SIZE__         __LONG_LONG_SIZE__
#define __SIGNED_LONGLONG_MAX__   __SIGNED_LONG_LONG_MAX__
#define __SIGNED_LONGLONG_MIN__   __SIGNED_LONG_LONG_MIN__
#define __UNSIGNED_LONGLONG_MAX__ __UNSIGNED_LONG_LONG_MAX__

#ifdef __LONG_LONG_SIZE__
  #define _LONGLONG     long long
  #define _ULONGLONG    unsigned long long
  #define _LLONG_MAX    __SIGNED_LONGLONG_MAX__
  #define _ULLONG_MAX   __UNSIGNED_LONGLONG_MAX__
#endif /* __LONGLONG_SIZE__ */

_C_STD_BEGIN
                /* errno PROPERTIES */
#define _EDOM   33
#define _ERANGE 34
#define _EFPOS  35
#define _EILSEQ 36
#define _ERRMAX 37

                /* FLOATING-POINT PROPERTIES */
#if __FLOAT_SIZE__ == 4
  #define _FBIAS 0x7e    /* IEEE 754 float properties */
  #define _FOFF  7
  #define _FMANTISSA 23
  #if __LITTLE_ENDIAN__
    #define _F0    1
  #else
    #define _F0    0
  #endif
#else
  #error "<yvals.h> __FLOAT_SIZE__ not 4"
#endif /* __FLOAT_SIZE__ */

                /* double properties */
#if __DOUBLE_SIZE__ == 8
  #define _DBIAS 0x3fe   /* IEEE 754 double properties */
  #define _DOFF  4
  #define _DMANTISSA 52
  #if __LITTLE_ENDIAN__
    #define _D0    3
  #else
    #define _D0    0
  #endif
#elif __DOUBLE_SIZE__ == 4
  #define _DBIAS 0x7e
  #define _DOFF          7
  #define _DMANTISSA 23
  #if __LITTLE_ENDIAN__
    #define _D0    1
  #else
    #define _D0    0
  #endif
#else
  #error "<yvals.h> __DOUBLE_SIZE__ not 4 or 8"
#endif /* __DOUBLE_SIZE__ */

                /* long double properties */
#if __LONG_DOUBLE_SIZE__ == 10
  #define _DLONG 1       /* IEEE 754 long double properties */
  #define _LBIAS 0x3ffe
  #define _LOFF  15
  #define _LMANTISSA 63
  #if __LITTLE_ENDIAN__
    #define _L0    4
  #else
    #define _L0    0
  #endif
#elif __LONG_DOUBLE_SIZE__ == 16
  #define _LMANTISSA 112
  #error "<yvals.h> __LONG_DOUBLE_SIZE__ 16 isn't supported yet"
#elif __LONG_DOUBLE_SIZE__ == 8
  #define _DLONG 0
  #define _LBIAS 0x3fe
  #define _LOFF  4
  #define _LMANTISSA 52
  #if __LITTLE_ENDIAN__
    #define _L0    3
  #else
    #define _L0    0
  #endif
#elif __LONG_DOUBLE_SIZE__ == 4
  #define _DLONG 0
  #define _LBIAS 0x7e
  #define _LOFF          7
  #define _LMANTISSA 23
  #if __LITTLE_ENDIAN__
    #define _L0    1
  #else
    #define _L0    0
  #endif
#else
  #error "<yvals.h> __LONG_DOUBLE_SIZE__ not 4, 8 or 10"
#endif /* __LONG_DOUBLE_SIZE__ */

#include <xencoding_limits.h>

                /* INTEGER PROPERTIES */
#define _C2             1       /* 0 if not 2's complement */
                                /* MB_LEN_MAX */
#define _MBMAX          _ENCODING_LEN_MAX

#define _MAX_EXP_DIG    8       /* for parsing numerics */
#define _MAX_INT_DIG    32
#define _MAX_SIG_DIG    36

#ifdef _LONGLONG
  typedef _LONGLONG _Longlong;
  typedef _ULONGLONG _ULonglong;
#else /* _LONGLONG */
  typedef long _Longlong;
  typedef unsigned long _ULonglong;
  #define _LLONG_MAX  __SIGNED_LONG_MAX__
  #define _ULLONG_MAX __UNSIGNED_LONG_MAX__
#endif /* _LONGLONG */

#ifdef __cplusplus
  #define _WCHART
  typedef wchar_t _Wchart;
  typedef wchar_t _Wintt;
#else
  typedef __WCHAR_T_TYPE__ _Wchart;
  typedef __WCHAR_T_TYPE__ _Wintt;
#endif

#ifdef __SIGNED_WCHAR_T__
  #define _WCMIN  __WCHAR_T_MIN__
  #define _WIMIN  __WCHAR_T_MIN__
#else
  #define _WCMIN  0
  #define _WIMIN  0
#endif
#define _WCMAX  __WCHAR_T_MAX__
#define _WIMAX  __WCHAR_T_MAX__

#if __INT_SIZE__ == 2
  #define _ILONG 0
#elif __INT_SIZE__ == 4
  #define _ILONG 1
#else
  #error "__INT_SIZE__ must be 2 or 4"
#endif /* __INT_SIZE__ */

                /* POINTER PROPERTIES */
#define _NULL           0       /* 0L if pointer same as long */

typedef __PTRDIFF_T_TYPE__  _Ptrdifft;
typedef __SIZE_T_TYPE__     _Sizet;

                /* signal PROPERTIES */
#define _SIGABRT        22
#define _SIGMAX         32

                /* stdarg PROPERTIES */
#ifndef _VA_DEFINED
  #ifndef _VA_LIST_STACK_MEMORY_ATTRIBUTE
    #define _VA_LIST_STACK_MEMORY_ATTRIBUTE
  #endif

  typedef struct
  {
    char _VA_LIST_STACK_MEMORY_ATTRIBUTE *_Ap;
  } __Va_list;
#else /* _VA_DEFINED */
  typedef _VA_LIST __Va_list;
#endif /* !_VA_DEFINED */

                /* stdlib PROPERTIES */
#define _EXFAIL 1       /* EXIT_FAILURE */

_EXTERN_C
__INTRINSIC void _Atexit(void (*)(void));
_END_EXTERN_C

typedef struct _Mbstatet
{       /* state of a multibyte translation */
  unsigned long _Wchar;
  unsigned short _Byte, _State;
} _Mbstatet;

                /* stdio PROPERTIES */
#define _FNAMAX 260
#define _FOPMAX 20
#define _TNAMAX 16

#if _DLIB_FILE_DESCRIPTOR
#define _Filet  FILE
#endif

typedef struct _Fpost
{       /* file position */
  long _Off;    /* can be system dependent */
  _Mbstatet _Wstate;
} _Fpost;

#ifndef _FPOSOFF
  #define _FPOSOFF(fp)  ((fp)._Off)
#endif

#define _FD_VALID(fd)   (0 <= (fd))     /* fd is signed integer */
#define _FD_INVALID     (-1)

                /* time PROPERTIES */
#define _CPS    1
/* Bias between 1900 (struct tm) and 1970 time_t. */
#define _TBIAS_DAYS (70 * 365L + 17)
#define _TBIAS  (_TBIAS_DAYS * 86400LU)
_C_STD_END

                /* MULTITHREAD PROPERTIES */
#if _MULTI_THREAD
  _C_STD_BEGIN
  _EXTERN_C
  __INTRINSIC void _Locksyslock(unsigned int);
  __INTRINSIC void _Unlocksyslock(unsigned int);
  _END_EXTERN_C
  _C_STD_END

#else /* _MULTI_THREAD */
  #define _Locksyslock(x)       (void)0
  #define _Unlocksyslock(x)     (void)0
#endif /* _MULTI_THREAD */

                /* LOCK MACROS */
#define _LOCK_LOCALE    0
#define _LOCK_MALLOC    1
#define _LOCK_STREAM    2
#define _MAX_LOCK       3       /* one more than highest lock number */

#ifdef __cplusplus
  _STD_BEGIN
                // CLASS _Lockit
  class _Lockit
  {     // lock while object in existence -- MUST NEST
  public:
  #if _MULTI_THREAD
    #define _LOCKIT(x)  lockit x
    explicit _Lockit()
      : _Locktype(0)
    {   // set default lock
      _Locksyslock(_Locktype);
    }

    explicit _Lockit(int _Type)
      : _Locktype(_Type)
    {   // set the lock
      _Locksyslock(_Locktype);
    }

    ~_Lockit()
    {   // clear the lock
      _Unlocksyslock(_Locktype);
    }

    private:
    _Lockit(const _Lockit&);            // not defined
    _Lockit& operator=(const _Lockit&); // not defined

    int _Locktype;
  #else /* _MULTI_THREAD */
    #define _LOCKIT(x)
    explicit _Lockit()
    {   // do nothing
    }

    explicit _Lockit(int)
    {   // do nothing
    }

    ~_Lockit()
    {   // do nothing
    }
  #endif /* _MULTI_THREAD */
  };

  class _Mutex
  {     // lock under program control
  public:
  #if _MULTI_THREAD
    _Mutex();
    ~_Mutex();
    void _Lock();
    void _Unlock();

    private:
    _Mutex(const _Mutex&);              // not defined
    _Mutex& operator=(const _Mutex&);   // not defined
    void *_Mtx;
  #else /* _MULTI_THREAD */
    void _Lock()
    {   // do nothing
    }

    void _Unlock()
    {   // do nothing
    }
  #endif /* _MULTI_THREAD */
  };
_STD_END
#endif /* __cplusplus */

                /* MISCELLANEOUS MACROS AND FUNCTIONS*/
/* #define _ATEXIT_T    void */
#define _Mbstinit(x)    mbstate_t x = {0, 0}

#define _MAX    max
#define _MIN    min

#pragma inline
static char _LC(char _C)
{  /* Convert character to lower case. */
  return ((_C) | ('a' - 'A'));
}

#if _HAS_NAMESPACE
  #if defined(__cplusplus)
    _STD_BEGIN
    typedef ::va_list va_list;
    _STD_END
  #endif /* __cplusplus */
#else
#endif /* _HAS_NAMESPACE */

#endif /* _YVALS */

/*
 * Copyright (c) 1992-2002 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V3.12:0576 */
