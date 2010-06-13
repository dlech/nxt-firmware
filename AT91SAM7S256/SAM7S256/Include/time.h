/* time.h standard header */
#ifndef _TIME
#define _TIME

#ifndef _SYSTEM_BUILD
#pragma system_include
#endif

#ifndef _YVALS
 #include <yvals.h>
#endif
#include <ysizet.h>
_C_STD_BEGIN

                /* macros */
#ifndef NULL
  #define NULL   _NULL
#endif /* NULL */

#define CLOCKS_PER_SEC  _CPS

                /* type definitions */
#if !defined(_CLOCK_T) && !defined(__clock_t_defined)
  #define _CLOCK_T
  #define __clock_t_defined
  #define _STD_USING_CLOCK_T
  typedef long clock_t;
#endif /* !defined(_CLOCK_T) && !defined(__clock_t_defined) */

#if !defined(_TIME_T) && !defined(__time_t_defined)
  #define _TIME_T
  #define __time_t_defined
  #define _STD_USING_TIME_T
  typedef long time_t;
#endif /* !defined(_TIME_T) && !defined(__time_t_defined) */

struct tm
{       /* date and time components */
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
};

_EXTERN_C       /* low-level functions */
__INTRINSIC time_t time(time_t *);
_END_EXTERN_C

_C_LIB_DECL     /* declarations */
__INTRINSIC char * asctime(const struct tm *);
__INTRINSIC clock_t clock(void);
__INTRINSIC char * ctime(const time_t *);
__INTRINSIC double difftime(time_t, time_t);
__INTRINSIC struct tm * gmtime(const time_t *);
__INTRINSIC struct tm * localtime(const time_t *);
__INTRINSIC time_t mktime(struct tm *);
__INTRINSIC size_t strftime(char *, size_t, const char *,
        const struct tm *);
_END_C_LIB_DECL
_C_STD_END
#endif /* _TIME */

#if defined(_STD_USING) && defined(__cplusplus)
  #ifdef _STD_USING_CLOCK_T
    using _CSTD clock_t;
  #endif /* _STD_USING_CLOCK_T */

  #ifdef _STD_USING_TIME_T
    using _CSTD time_t;
  #endif /* _STD_USING_TIME_T */

  #ifdef _STD_USING_CLOCKID_T
    using _CSTD clockid_t;
  #endif /* _STD_USING_CLOCKID_T */

  using _CSTD tm;
  using _CSTD asctime; using _CSTD clock; using _CSTD ctime;
  using _CSTD difftime; using _CSTD gmtime; using _CSTD localtime;
  using _CSTD mktime; using _CSTD strftime; using _CSTD time;
#endif /* defined(_STD_USING) && defined(__cplusplus) */

/*
 * Copyright (c) 1992-2002 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V3.12:0576 */
