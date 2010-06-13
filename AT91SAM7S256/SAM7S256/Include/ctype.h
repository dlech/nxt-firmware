/* ctype.h standard header */
#ifndef _CTYPE
#define _CTYPE

#ifndef _SYSTEM_BUILD
  #pragma system_include
#endif

#ifndef _YVALS
  #include <yvals.h>
#endif

#include <xlocale.h>

_C_STD_BEGIN

_C_LIB_DECL
__INTRINSIC int isalnum(int);
__INTRINSIC int isalpha(int);
#if _DLIB_ADD_C99_SYMBOLS
  __INTRINSIC int isblank(int);
#endif /* _DLIB__ADD_C99_SYMBOLS */
__INTRINSIC int iscntrl(int);
__INTRINSIC int isdigit(int);
__INTRINSIC int isgraph(int);
__INTRINSIC int islower(int);
__INTRINSIC int isprint(int);
__INTRINSIC int ispunct(int);
__INTRINSIC int isspace(int);
__INTRINSIC int isupper(int);
__INTRINSIC int isxdigit(int);
__INTRINSIC int tolower(int);
__INTRINSIC int toupper(int);
_END_C_LIB_DECL

#if _DLIB_ADD_C99_SYMBOLS
  #pragma inline
  int isblank(int _C)
  {
    return (   _C == ' '
            || _C == '\t'
            || isspace(_C));
  }
#endif /* _DLIB__ADD_C99_SYMBOLS */

#pragma inline
int isdigit(int _C)
{
  return _C >= '0' && _C <= '9';
}

#pragma inline
int isxdigit(int _C)
{
  return (   (_C >= 'a' && _C <= 'f')
          || (_C >= 'A' && _C <= 'F')
          || isdigit(_C));
}

#pragma inline
int isalnum(int _C)
{
  return (   isalpha(_C)
          || isdigit(_C));
}

#pragma inline
int isprint(int _C)
{
  return (   (_C >= ' ' && _C <= '\x7e')
          || isalpha(_C)
          || ispunct(_C));
}

#pragma inline
int isgraph(int _C)
{
  return (   _C != ' '
          && isprint(_C));
}


#if _DLIB_FULL_LOCALE_SUPPORT

  /* In full support locale mode proxy functions are defined in each
   * source file. */

#else /* _DLIB_FULL_LOCALE_SUPPORT */

  /* In non-full mode we redirect the corresponding locale function. */
  _EXTERN_C
  extern int _LOCALE_WITH_USED(toupper)(int);
  extern int _LOCALE_WITH_USED(tolower)(int);
  extern int _LOCALE_WITH_USED(isalpha)(int);
  extern int _LOCALE_WITH_USED(iscntrl)(int);
  extern int _LOCALE_WITH_USED(islower)(int);
  extern int _LOCALE_WITH_USED(ispunct)(int);
  extern int _LOCALE_WITH_USED(isspace)(int);
  extern int _LOCALE_WITH_USED(isupper)(int);
  _END_EXTERN_C

  #pragma inline
  int toupper(int _C)
  {
    return _LOCALE_WITH_USED(toupper)(_C);
  }

  #pragma inline
  int tolower(int _C)
  {
    return _LOCALE_WITH_USED(tolower)(_C);
  }

  #pragma inline
  int isalpha(int _C)
  {
    return _LOCALE_WITH_USED(isalpha)(_C);
  }

  #pragma inline
  int iscntrl(int _C)
  {
    return _LOCALE_WITH_USED(iscntrl)(_C);
  }

  #pragma inline
  int islower(int _C)
  {
    return _LOCALE_WITH_USED(islower)(_C);
  }

  #pragma inline
  int ispunct(int _C)
  {
    return _LOCALE_WITH_USED(ispunct)(_C);
  }

  #pragma inline
  int isspace(int _C)
  {
    return _LOCALE_WITH_USED(isspace)(_C);
  }

  #pragma inline
  int isupper(int _C)
  {
    return _LOCALE_WITH_USED(isupper)(_C);
  }

#endif /* _DLIB_FULL_LOCALE_SUPPORT */

_C_STD_END
#endif /* _CTYPE */

#ifdef _STD_USING
  using _CSTD isalnum; using _CSTD isalpha; using _CSTD iscntrl;
  using _CSTD isdigit; using _CSTD isgraph; using _CSTD islower;
  using _CSTD isprint; using _CSTD ispunct; using _CSTD isspace;
  using _CSTD isupper; using _CSTD isxdigit; using _CSTD tolower;
  using _CSTD toupper;
  #if _DLIB_ADD_C99_SYMBOLS
    uisng _CSTD isblank;
  #endif /* _DLIB__ADD_C99_SYMBOLS */
#endif /* _STD_USING */

/*
 * Copyright (c) 1992-2002 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V3.12:0576 */
