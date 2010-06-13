/* xmtx.h internal header */
#ifndef _XMTX
#define _XMTX

#ifndef _SYSTEM_BUILD
#pragma system_include
#endif

#include <stdlib.h>
#ifndef _YVALS
  #include <yvals.h>
#endif

_C_LIB_DECL
typedef void *_Rmtx;

__INTRINSIC void _Mtxinit(_Rmtx *);
__INTRINSIC void _Mtxdst(_Rmtx *);
__INTRINSIC void _Mtxlock(_Rmtx *);
__INTRINSIC void _Mtxunlock(_Rmtx *);

#if !_MULTI_THREAD
  #define _Mtxinit(mtx)
  #define _Mtxdst(mtx)
  #define _Mtxlock(mtx)
  #define _Mtxunlock(mtx)

  typedef char _Once_t;

  #define _Once(cntrl, func)    if (*(cntrl) == 0) (func)(), *(cntrl) = 2
  #define _ONCE_T_INIT  0
#else
  #error "unknown library type"
#endif /* _MULTI_THREAD */
_END_C_LIB_DECL
#endif /* _XMTX */

/*
 * Copyright (c) 1992-2002 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V3.12:0576 */
