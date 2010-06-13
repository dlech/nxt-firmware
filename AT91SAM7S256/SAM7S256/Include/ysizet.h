/* ysizet.h internal header file. */
/* Copyright (C) 2003 IAR Systems.  All rights reserved. */

#ifndef _YSIZET_H
#define _YSIZET_H

#ifndef _SYSTEM_BUILD
#pragma system_include
#endif

#ifndef _YVALS
 #include <yvals.h>
#endif

_C_STD_BEGIN
                /* type definitions */
#if !defined(_SIZE_T) && !defined(_SIZET)
  #define _SIZE_T
  #define _SIZET
  #define _STD_USING_SIZE_T
typedef _Sizet size_t;
#endif /* !defined(_SIZE_T) && !defined(_SIZET) */

#define __DATA_PTR_MEM_HELPER1__(M, I) \
typedef __DATA_MEM##I##_SIZE_TYPE__ M##_size_t;
__DATA_PTR_MEMORY_LIST1__()
#undef __DATA_PTR_MEM_HELPER1__

_C_STD_END

#if defined(_STD_USING) && defined(__cplusplus)
  #ifdef _STD_USING_SIZE_T
using _CSTD size_t;
  #endif /* _STD_USING_SIZE_T */
#endif /* defined(_STD_USING) && defined(__cplusplus) */

#endif /* _YSIZET_H */
