/* xlocale.h internal header file */
/* Copyright (C) 2003 IAR Systems.  All rights reserved. */

#ifndef _XLOCALE_H
#define _XLOCALE_H

#ifndef _SYSTEM_BUILD
  #pragma system_include
#endif

#ifndef _YVALS
  #include <yvals.h>
#endif

#include <xtls.h>

#if _DLIB_FULL_LOCALE_SUPPORT

#include <wchar.h>

  /*
   * ======================================================================
   * Full support, it is possible to define several locales and switch
   * between them.
   */

  #ifndef _LOCALE_USE_C
    #error "_LOCALE_USE_C must be defined for _DLIB_FULL_LOCALE_SUPPORT"
  #endif


  __INTRINSIC int _LocaleForCat(int cat);
  __INTRINSIC int _LocaleEncoding(void);


  /*
   * _LOCALE_LIST and _LOCALE_LIST1 -- Macros that can be used in
   * conjunction with _LOCALE_LIST_HELPER and _LOCALE_LIST_HELPER1,
   * respectively, to iterate over the defined locales.
   */

  /* Add the "C" locale, then include "localelist" to add the rest. */

  #define _LOCALE_LIST0_0     _LOCALE_LIST_HELPER(C)
  #define _LOCALE_LIST1_0(a1) _LOCALE_LIST_HELPER1(C,a1)

  #include <xlocalelist.h>


  /*
   * Define unique id:s for each locale.
   */

  #define _LOCALE_LIST_HELPER(n) _Locale##n##_id,

  enum
  {
    _LOCALE_LIST
    _LocaleCount /* This eats last "," */
  };

  #undef _LOCALE_LIST_HELPER


  /*
   * The current lconv structure.
   */

  _TLS_DATA_DECL(struct lconv, _Locale_lconv);

  _EXTERN_C
  #define _LOCALE_LIST_HELPER1(n,f)            \
    extern int _Locale##n##_##f(int);
  _LOCALE_LIST1(toupper)                             
  _LOCALE_LIST1(tolower)                             
  _LOCALE_LIST1(isalpha)                             
  _LOCALE_LIST1(iscntrl)                             
  _LOCALE_LIST1(islower)                             
  _LOCALE_LIST1(ispunct)                             
  _LOCALE_LIST1(isspace)                             
  _LOCALE_LIST1(isupper)                             
  #undef _LOCALE_LIST_HELPER1
  #define _LOCALE_LIST_HELPER1(n,f)            \
    extern wint_t _Locale##n##_##f(wint_t);
  _LOCALE_LIST1(towupper)                             
  _LOCALE_LIST1(towlower)                             
  #undef _LOCALE_LIST_HELPER1
  #define _LOCALE_LIST_HELPER1(n,f)            \
    extern int _Locale##n##_##f(wint_t);
  _LOCALE_LIST1(iswalpha)                             
  _LOCALE_LIST1(iswcntrl)                             
  _LOCALE_LIST1(iswlower)                             
  _LOCALE_LIST1(iswpunct)                             
  _LOCALE_LIST1(iswspace)                             
  _LOCALE_LIST1(iswupper)                             
  _LOCALE_LIST1(iswdigit)                             
  _LOCALE_LIST1(iswxdigit)                             
  #undef _LOCALE_LIST_HELPER1
  _END_EXTERN_C



#else /* !_DLIB_FULL_LOCALE_SUPPORT */

  /*
   * ======================================================================
   * Reduced support.  One locale (possibly "C") is hardwired.
   */

  /*
   * This defined the Macro _LOCALE_WITH_USED (i.e. With used
   * locale). Expands "f" to the corresponding identifier in the
   * selected locale.
   */

  #include <xlocaleuse.h>

  #ifdef _LOCALE_USE_C
    #define _LOCALE_DECIMAL_POINT ('.')
    #include <xlocale_c.h>
  #endif

#endif /* _DLIB_FULL_LOCALE_SUPPORT */


#ifndef _LOCALE_DECIMAL_POINT
  #define _LOCALE_DECIMAL_POINT (localeconv()->decimal_point[0])
#endif

#endif /* _XLOCALE_H */
