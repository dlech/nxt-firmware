/* ymath.h internal header */
#ifndef _YMATH
#define _YMATH

#ifndef _SYSTEM_BUILD
#pragma system_include
#endif

#include <yvals.h>
_C_STD_BEGIN
_C_LIB_DECL

                /* MACROS FOR _Dtest RETURN (0 => ZERO) */
#define _DENORM         (-2)    /* C9X only */
#define _FINITE         (-1)
#define _INFCODE        1
#define _NANCODE        2

                /* TYPE DEFINITIONS */

#if __SHORT_SIZE__ != 2
#error "Float implementation assumes short is 2 bytes"
#endif

typedef union
{       /* pun float types as integer array */
  unsigned short _Word[__LONG_DOUBLE_SIZE__ / 2];
  float _Float;
  double _Double;
  long double _Long_double;
} _Dconst;

                /* double DECLARATIONS */
__INTRINSIC double _Cosh(double, double);
__INTRINSIC short _Dtest(double);
__INTRINSIC short _Exp(double *, double, short);
__INTRINSIC double _Log(double, int);
__INTRINSIC double _Sin(double, unsigned int);
__INTRINSIC double _Sinh(double, double);
extern const _Dconst _Denorm, _Hugeval, _Inf, _Nan, _Snan;

                /* float DECLARATIONS */
#ifndef _FLOAT_IS_DOUBLE
  __INTRINSIC float _FCosh(float, float);
  __INTRINSIC short _FDtest(float);
  __INTRINSIC short _FExp(float *, float, short);
  __INTRINSIC float _FLog(float, int);
  __INTRINSIC float _FSin(float, unsigned int);
  __INTRINSIC float _FSinh(float, float);
  extern const _Dconst _FDenorm, _FHugeval, _FInf, _FNan, _FSnan;
#endif /* _FLOAT_IS_DOUBLE */

                /* long double DECLARATIONS */
#ifndef _LONG_DOUBLE_IS_DOUBLE
  __INTRINSIC long double _LCosh(long double, long double);
  __INTRINSIC short _LDtest(long double);
  __INTRINSIC short _LExp(long double *, long double, short);
  __INTRINSIC long double _LLog(long double, int);
  __INTRINSIC long double _LSin(long double, unsigned int);
  __INTRINSIC long double _LSinh(long double, long double);
  extern const _Dconst _LDenorm, _LInf, _LNan, _LSnan;
#endif /* _LONG_DOUBLE_IS_DOUBLE */

                /* long double ADDITIONS TO math.h NEEDED FOR complex */
__INTRINSIC long double (atan2l)(long double, long double);
__INTRINSIC long double (cosl)(long double);
__INTRINSIC long double (expl)(long double);
__INTRINSIC long double (ldexpl)(long double, int);
__INTRINSIC long double (logl)(long double);
__INTRINSIC long double (powl)(long double, long double);
__INTRINSIC long double (sinl)(long double);
__INTRINSIC long double (sqrtl)(long double);
__INTRINSIC long double (tanl)(long double);
                /* float ADDITIONS TO math.h NEEDED FOR complex */
__INTRINSIC float (atan2f)(float, float);
__INTRINSIC float (cosf)(float);
__INTRINSIC float (expf)(float);
__INTRINSIC float (ldexpf)(float, int);
__INTRINSIC float (logf)(float);
__INTRINSIC float (powf)(float, float);
__INTRINSIC float (sinf)(float);
__INTRINSIC float (sqrtf)(float);
__INTRINSIC float (tanf)(float);
_END_C_LIB_DECL
_C_STD_END
#endif /* _YMATH */

/*
 * Copyright (c) 1992-2002 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V3.12:0576 */
