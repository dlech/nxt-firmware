/* xencoding_limits.h internal header file */
/* Copyright (C) 2003 IAR Systems.  All rights reserved. */

#ifndef _XENCODING_LIMITS_H
#define _XENCODING_LIMITS_H

#ifndef _SYSTEM_BUILD
  #pragma system_include
#endif

#ifndef _YVALS
  #include <yvals.h>
#endif

                                /* Multibyte encoding length. */
#define _EncodingSb_LenMax   1

#if __WCHAR_T_MAX__ <= 0xFF
  #define _EncodingUtf8_LenMax 1
#elif __WCHAR_T_MAX__ <= 0xFFFF
  #define _EncodingUtf8_LenMax 3
#else
  #define _EncodingUtf8_LenMax 6
#endif


#if _DLIB_FULL_LOCALE_SUPPORT

  #define _ENCODING_LEN_MAX _EncodingSb_LenMax

  #ifdef _ENCODING_USE_UTF8
    #if _ENCODING_LEN_MAX < _EncodingUtf8_LenMax
      #undef _ENCODING_LEN_MAX
      #define _ENCODING_LEN_MAX _EncodingUtf8_LenMax
    #endif
  #endif

  #define _ENCODING_CUR_MAX (_Mbcurmax())

#else /* _DLIB_FULL_LOCALE_SUPPORT */

                                /* Utility macro */
  #ifdef _ENCODING_USE_UTF8
    #define _ENCODING_WITH_USED(x) _EncodingUtf8_##x
  #else
    #define _ENCODING_WITH_USED(x) _EncodingSb_##x
  #endif


  #define _ENCODING_LEN_MAX _ENCODING_WITH_USED(LenMax)
  #define _ENCODING_CUR_MAX _ENCODING_LEN_MAX

#endif /* _DLIB_FULL_LOCALE_SUPPORT */

#endif /* _XENCODING_LIMITS_H */
