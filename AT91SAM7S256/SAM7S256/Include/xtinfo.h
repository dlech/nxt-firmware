/* xtinfo.h internal header */
#ifndef _XTINFO
#define _XTINFO

#ifndef _SYSTEM_BUILD
#pragma system_include
#endif

#include <time.h>
#ifndef _YVALS
  #include <yvals.h>
#endif

#include <xlocale.h>

_C_STD_BEGIN

                /* type definitions */
typedef struct
{       /* format strings for date and time */
  const char *_Am_pm;
  const char *_Days;
  const char *_Abday;
  const char *_Day;
  const char *_Months;
  const char *_Abmon;
  const char *_Mon;
  const char *_Formats;
  const char *_D_t_fmt;
  const char *_D_fmt;
  const char *_T_fmt;
  const char *_T_fmt_ampm;
  const char *_Era_Formats;
  const char *_Era_D_t_fmt;
  const char *_Era_D_fmt;
  const char *_Era_T_fmt;
  const char *_Era_T_fmt_ampm;
  const char *_Era;
  const char *_Alt_digits;
  const char *_Isdst;
  const char *_Tzone;
} _Tinfo;

                /* declarations */
_C_LIB_DECL
__INTRINSIC size_t _CStrftime(char *, size_t, const char *,
                              const struct tm *, const _Tinfo *);
__INTRINSIC const _Tinfo *_Getptimes(void);
__INTRINSIC const _Tinfo *_GetptimesFor(int /* Id */);

#if !_DLIB_FULL_LOCALE_SUPPORT

#pragma inline
const _Tinfo * _Getptimes(void)
{
  extern const _Tinfo _LOCALE_WITH_USED(Tinfo);
  return &_LOCALE_WITH_USED(Tinfo);
}
#endif

_END_C_LIB_DECL
_C_STD_END
#endif /* _XTINFO */

/*
 * Copyright (c) 1992-2002 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V3.12:0576 */
