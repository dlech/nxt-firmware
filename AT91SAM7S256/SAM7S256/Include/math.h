/* math.h standard header */
#ifndef _MATH
#define _MATH

#ifndef _SYSTEM_BUILD
  #pragma system_include
#endif

#ifndef _YMATH
  #include <ymath.h>
#endif
_C_STD_BEGIN

                /* MACROS */
#define HUGE_VAL        _CSTD _Hugeval._Double
#if _DLIB_ADD_C99_SYMBOLS
  #define HUGE_VALF       _CSTD _FHugeval._Float
  #define HUGE_VALL       _CSTD _LHugeval._Long_Double

  #define INFINITY (0.Infinity)
  #define NAN      (0.NaN)

                /* typedefs */

  typedef float float_t;
  typedef double double_t;
#endif /* _DLIB_ADD_C99_SYMBOLS */



_C_LIB_DECL
                /* double declarations */
__INTRINSIC double acos(double);
__INTRINSIC double asin(double);
__INTRINSIC double atan(double);
__INTRINSIC double atan2(double, double);
__INTRINSIC double ceil(double);
__INTRINSIC double exp(double);
__INTRINSIC double fabs(double);
__INTRINSIC double floor(double);
__INTRINSIC double fmod(double, double);
__INTRINSIC double frexp(double, int *);
__INTRINSIC double ldexp(double, int);
__INTRINSIC double modf(double, double *);
__INTRINSIC double pow(double, double);
__INTRINSIC double sqrt(double);
__INTRINSIC double tan(double);
__INTRINSIC double tanh(double);

__INTRINSIC double cos(double);
__INTRINSIC double cosh(double);
__INTRINSIC double log(double);
__INTRINSIC double log10(double);
__INTRINSIC double sin(double);
__INTRINSIC double sinh(double);

#if _DLIB_ADD_C99_SYMBOLS

                /* float declarations */
  __INTRINSIC float acosf(float);
  __INTRINSIC float asinf(float);
  __INTRINSIC float atanf(float);
  __INTRINSIC float atan2f(float, float);
  __INTRINSIC float ceilf(float);
  __INTRINSIC float expf(float);
  __INTRINSIC float fabsf(float);
  __INTRINSIC float floorf(float);
  __INTRINSIC float fmodf(float, float);
  __INTRINSIC float frexpf(float, int *);
  __INTRINSIC float ldexpf(float, int);
  __INTRINSIC float modff(float, float *);
  __INTRINSIC float powf(float, float);
  __INTRINSIC float sqrtf(float);
  __INTRINSIC float tanf(float);
  __INTRINSIC float tanhf(float);

  __INTRINSIC float cosf(float);
  __INTRINSIC float coshf(float);
  __INTRINSIC float logf(float);
  __INTRINSIC float log10f(float);
  __INTRINSIC float sinf(float);
  __INTRINSIC float sinhf(float);

                /* long double declarations */
  __INTRINSIC long double acosl(long double);
  __INTRINSIC long double asinl(long double);
  __INTRINSIC long double atanl(long double);
  __INTRINSIC long double atan2l(long double, long double);
  __INTRINSIC long double ceill(long double);
  __INTRINSIC long double expl(long double);
  __INTRINSIC long double fabsl(long double);
  __INTRINSIC long double floorl(long double);
  __INTRINSIC long double fmodl(long double, long double);
  __INTRINSIC long double frexpl(long double, int *);
  __INTRINSIC long double ldexpl(long double, int);
  __INTRINSIC long double modfl(long double, long double *);
  __INTRINSIC long double powl(long double, long double);
  __INTRINSIC long double sqrtl(long double);
  __INTRINSIC long double tanl(long double);
  __INTRINSIC long double tanhl(long double);

  __INTRINSIC long double cosl(long double);
  __INTRINSIC long double coshl(long double);
  __INTRINSIC long double logl(long double);
  __INTRINSIC long double log10l(long double);
  __INTRINSIC long double sinl(long double);
  __INTRINSIC long double sinhl(long double);
#endif /* _DLIB_ADD_C99_SYMBOLS */

_END_C_LIB_DECL

#ifndef _NO_DEFINITIONS_IN_HEADER_FILES
                /* double INLINES, FOR C and C++ */
  #pragma inline 
  double cos(double _X)
  {       /* return cosine */
    return (_Sin(_X, 1));
  }

  #pragma inline 
  double cosh(double _X)
  {       /* return hyperbolic cosine */
    return (_Cosh(_X, 1));
  }

  #pragma inline 
  double log(double _X)
  {       /* return natural logarithm */
    return (_Log(_X, 0));
  }

  #pragma inline 
  double log10(double _X)
  {       /* return base-10 logarithm */
    return (_Log(_X, 1));
  }

  #pragma inline 
  double sin(double _X)
  {       /* return sine */
    return (_Sin(_X, 0));
  }

  #pragma inline
  double sinh(double _X)
  {       /* return hyperbolic sine */
    return (_Sinh(_X, 1));
  }

  #ifdef __cplusplus
    inline double abs(double _X)    /* OVERLOADS */
    {       /* return absolute value */
      return (fabs(_X));
    }

    inline double pow(double _X, int _Y)
    {       /* raise to integer power */
      unsigned int _N = _Y;
      if (_Y < 0)
        _N = 0 - _N;

      for (double _Z = 1; ; _X *= _X)
      {
        if ((_N & 1) != 0)
          _Z *= _X;
        if ((_N >>= 1) == 0)
          return (_Y < 0 ? (double)(1) / _Z : _Z);
      }
    }
  #endif /* __cplusplus */


                /* float INLINES, FOR C and C++ */
  #if _DLIB_ADD_C99_SYMBOLS
    #pragma inline 
    float cosf(float _X)
    {       /* return cosine */
      return (_F_FNAME(Sin)(_X, 1));
    }

    #pragma inline 
    float coshf(float _X)
    {       /* return hyperbolic cosine */
      return (_F_FNAME(Cosh)(_X, 1));
    }

    #pragma inline 
    float logf(float _X)
    {       /* return natural logarithm */
      return (_F_FNAME(Log)(_X, 0));
    }

    #pragma inline 
    float log10f(float _X)
    {       /* return base-10 logarithm */
      return (_F_FNAME(Log)(_X, 1));
    }

    #pragma inline 
    float sinf(float _X)
    {       /* return sine */
      return (_F_FNAME(Sin)(_X, 0));
    }

    #pragma inline 
    float sinhf(float _X)
    {       /* return hyperbolic sine */
      return (_F_FNAME(Sinh)(_X, 1));
    }
  #endif /* _DLIB_ADD_C99_SYMBOLS */

  #ifdef __cplusplus
    inline float abs(float _X)      /* OVERLOADS */
    {       /* return absolute value */
      return (_F_FUN(fabs)(_X));
    }

    inline float acos(float _X)
    {       /* return arccosine */
      return (_F_FUN(acos)(_F_CAST _X));
    }

    inline float asin(float _X)
    {       /* return arcsine */
      return (_F_FUN(asin)(_F_CAST _X));
    }

    inline float atan(float _X)
    {       /* return arctangent */
      return (_F_FUN(atan)(_F_CAST _X));
    }

    inline float atan2(float _Y, float _X)
    {       /* return arctangent */
      return (_F_FUN(atan2)(_F_CAST _Y,_F_CAST  _X));
    }

    inline float ceil(float _X)
    {       /* return ceiling */
      return (_F_FUN(ceil)(_F_CAST _X));
    }

    inline float cos(float _X)
    {       /* return cosine */
      return (_F_FNAME(Sin)(_X, 1));
    }

    inline float cosh(float _X)
    {       /* return hyperbolic cosine */
      return (_F_FNAME(Cosh)(_X, 1));
    }

    inline float exp(float _X)
    {       /* return exponential */
      return (_F_FUN(exp)(_F_CAST _X));
    }

    inline float fabs(float _X)
    {       /* return absolute value */
      return (_F_FUN(fabs)(_F_CAST _X));
    }

    inline float floor(float _X)
    {       /* return floor */
      return (_F_FUN(floor)(_F_CAST _X));
    }

    inline float fmod(float _X, float _Y)
    {       /* return modulus */
      return (_F_FUN(fmod)(_F_CAST _X,_F_CAST  _Y));
    }

    inline float frexp(float _X, int *_Y)
    {       /* unpack exponent */
      return (_F_FUN(frexp)(_F_CAST _X, _Y));
    }

    inline float ldexp(float _X, int _Y)
    {       /* pack exponent */
      return (_F_FUN(ldexp)(_F_CAST _X, _Y));
    }

    inline float log(float _X)
    {       /* return natural logarithm */
      return (_F_FNAME(Log)(_X, 0));
    }

    inline float log10(float _X)
    {       /* return base-10 logarithm */
      return (_F_FNAME(Log)(_X, 1));
    }

    inline float modf(float _X, float *_Y)
    {       /* unpack fraction */
      return (_F_FUN(modf)(_F_CAST _X,_F_PTRCAST  _Y));
    }

    inline float pow(float _X, float _Y)
    {       /* raise to power */
      return (_F_FUN(pow)(_F_CAST _X,_F_CAST  _Y));
    }

    inline float pow(float _X, int _Y)
    {       /* raise to integer power */
    #ifdef _FLOAT_IS_DOUBLE
      return (float) pow((double) _X, _Y);
    #else
      unsigned int _N = _Y;
      if (_Y < 0)
        _N = 0 - _N;

      for (float _Z = 1; ; _X *= _X)
      {
        if ((_N & 1) != 0)
          _Z *= _X;
        if ((_N >>= 1) == 0)
          return (_Y < 0 ? (float)(1) / _Z : _Z);
      }
    #endif /* _FLOAT_IS_DOUBLE */
    }

    inline float sin(float _X)
    {       /* return sine */
      return (_F_FNAME(Sin)(_X, 0));
    }

    inline float sinh(float _X)
    {       /* return hyperbolic sine */
      return (_F_FNAME(Sinh)(_X, 1));
    }

    inline float sqrt(float _X)
    {       /* return square root */
      return (_F_FUN(sqrt)(_F_CAST _X));
    }

    inline float tan(float _X)
    {       /* return tangent */
      return (_F_FUN(tan)(_F_CAST _X));
    }

    inline float tanh(float _X)
    {       /* return hyperbolic tangent */
      return (_F_FUN(tanh)(_F_CAST _X));
    }
  #endif /* __cplusplus */

                /* long double INLINES, FOR C and C++ */
  #if _DLIB_ADD_C99_SYMBOLS
    #pragma inline 
    long double cosl(long double _X)
    {       /* return cosine */
      return (_L_FNAME(Sin)(_X, 1));
    }

    #pragma inline 
    long double coshl(long double _X)
    {       /* return hyperbolic cosine */
      return (_L_FNAME(Cosh)(_X, 1));
    }

    #pragma inline 
    long double logl(long double _X)
    {       /* return natural logarithm */
      return (_L_FNAME(Log)(_X, 0));
    }

    #pragma inline 
    long double log10l(long double _X)
    {       /* return base-10 logarithm */
      return (_L_FNAME(Log)(_X, 1));
    }

    #pragma inline 
    long double sinl(long double _X)
    {       /* return sine */
      return (_L_FNAME(Sin)(_X, 0));
    }

    #pragma inline 
    long double sinhl(long double _X)
    {       /* return hyperbolic sine */
      return (_L_FNAME(Sinh)(_X, 1));
    }
  #endif /* _DLIB_ADD_C99_SYMBOLS */

  #ifdef __cplusplus
    inline long double abs(long double _X)  /* OVERLOADS */
    {       /* return absolute value */
      return (_L_FUN(fabs)(_L_CAST _X));
    }

    inline long double acos(long double _X)
    {       /* return arccosine */
      return (_L_FUN(acos)(_L_CAST _X));
    }

    inline long double asin(long double _X)
    {       /* return arcsine */
      return (_L_FUN(asin)(_L_CAST _X));
    }

    inline long double atan(long double _X)
    {       /* return arctangent */
      return (_L_FUN(atan)(_L_CAST _X));
    }

    inline long double atan2(long double _Y, long double _X)
    {       /* return arctangent */
      return (_L_FUN(atan2)(_L_CAST _Y, _L_CAST _X));
    }

    inline long double ceil(long double _X)
    {       /* return ceiling */
      return (_L_FUN(ceil)(_L_CAST _X));
    }

    inline long double cos(long double _X)
    {       /* return cosine */
      return (_L_FNAME(Sin)(_X, 1));
    }

    inline long double cosh(long double _X)
    {       /* return hyperbolic cosine */
      return (_L_FNAME(Cosh)(_X, 1));
    }

    inline long double exp(long double _X)
    {       /* return exponential */
      return (_L_FUN(exp)(_L_CAST _X));
    }

    inline long double fabs(long double _X)
    {       /* return absolute value */
      return (_L_FUN(fabs)(_L_CAST _X));
    }

    inline long double floor(long double _X)
    {       /* return floor */
      return (_L_FUN(floor)(_L_CAST _X));
    }

    inline long double fmod(long double _X, long double _Y)
    {       /* return modulus */
      return (_L_FUN(fmod)(_L_CAST _X,_L_CAST _Y));
    }

    inline long double frexp(long double _X, int *_Y)
    {       /* unpack exponent */
      return (_L_FUN(frexp)(_L_CAST _X, _Y));
    }

    inline long double ldexp(long double _X, int _Y)
    {       /* pack exponent */
      return (_L_FUN(ldexp)(_L_CAST _X, _Y));
    }

    inline long double log(long double _X)
    {       /* return natural logarithm */
      return (_L_FNAME(Log)(_X, 0));
    }

    inline long double log10(long double _X)
    {       /* return base-10 logarithm */
      return (_L_FNAME(Log)(_X, 1));
    }

    inline long double modf(long double _X, long double *_Y)
    {       /* unpack fraction */
      return (_L_FUN(modf)(_L_CAST _X, _L_PTRCAST _Y));
    }

    inline long double pow(long double _X, long double _Y)
    {       /* raise to power */
      return (_L_FUN(pow)(_L_CAST _X, _L_CAST _Y));
    }

    inline long double pow(long double _X, int _Y)
    {       /* raise to integer power */
    #ifdef _LONG_DOUBLE_IS_DOUBLE
      return (long double) pow((double) _X, _Y);
    #else
      unsigned int _N = _Y;
      if (_Y < 0)
        _N = 0 - _N;

      for (long double _Z = 1; ; _X *= _X)
      {
        if ((_N & 1) != 0)
          _Z *= _X;
        if ((_N >>= 1) == 0)
          return (_Y < 0 ? (long double)(1) / _Z : _Z);
      }
    #endif /* _LONG_DOUBLE_IS_DOUBLE */
    }

    inline long double sin(long double _X)
    {       /* return sine */
      return (_L_FNAME(Sin)(_X, 0));
    }

    inline long double sinh(long double _X)
    {       /* return hyperbolic sine */
      return (_L_FNAME(Sinh)(_X, 1));
    }

    inline long double sqrt(long double _X)
    {       /* return square root */
      return (_L_FUN(sqrt)(_L_CAST _X));
    }

    inline long double tan(long double _X)
    {       /* return tangent */
      return (_L_FUN(tan)(_L_CAST _X));
    }

    inline long double tanh(long double _X)
    {       /* return hyperbolic tangent */
      return (_L_FUN(tanh)(_L_CAST _X));
    }
  #endif /* __cplusplus */
#endif /* _NO_DEFINITIONS_IN_HEADER_FILES */
_C_STD_END

#if _DLIB_ADD_C99_SYMBOLS
#if 0

/* C99 floating point functionality */

Fyll i
  #define FP_ILOGB0
  #define FP_ILOGBNAN

  #define MATH_ERRNO 1
  #define MATH_ERREXCEPT 2
  #define math_errhandling MATH_ERRNO


  #define FP_INFINITE  _INFCODE  
  #define FP_NAN       _NANCODE
  #define FP_NORMAL    _FINITE
  #define FP_SUBNORMAL _DENORM
  #define FP_ZERO      0

  #if _LONG_DOUBLE_IS_DOUBLE
    #error "Must add long double handling to the macros below"
  #endif

  #define fpclassify(x) \
    (sizeof(x) == __DOUBLE_SIZE__ ? __fpclassifyd(x) : __fpclassifyf(x))

  #pragma inline
  int __fpclassifyd(double x)
  {
    return Dtest(x);
  }

  #ifndef _FLOAT_IS_DOUBLE
    #pragma inline
    int __fpclassifyf(float x)
    {
      return _F_FNAME(Dtest)(x);
    }
  #endif /* _FLOAT_IS_DOUBLE */

  #define isfinite(x) __isfinite(fpclassify(x))

  #pragma inline
  int __isfinite(int x)
  {
    return x == FP_ZERO || x == FP_NORMAL || x == FP_SUBNORMAL;
  }

  #define isinf(x)    (fpclassify(x) == FP_INFINITE)
  #define isnan(x)    (fpclassify(x) == FP_NAN)
  #define isnormal(x) (fpclassify(x) == FP_NORMAL)

  #define signbit(x) \
    (sizeof(x) == __DOUBLE_SIZE__ ? __signbitd(x) : __signbitf(x))

  #include "xxtd.h"
  #pragma inline
  int __signbitd(double x)
  {
    unsigned short *ps = (unsigned short *)&px;

    return ((ps[_X0] & _XSIGN) == _XSIGN;
  }
  #include "xxtdundef.h"

  #ifndef _FLOAT_IS_DOUBLE
    #include "xxtf.h"
    #pragma inline
    int __signbitf(float x)
    {
      unsigned short *ps = (unsigned short *)&px;

      return (ps[_X0] & _XSIGN) == _XSIGN;
    }
    #include "xxtfundef.h"
  #endif /* _FLOAT_IS_DOUBLE */
#endif /* 0 */
#endif /* _DLIB_ADD_C99_SYMBOLS */



#if defined(_STD_USING) && defined(__cplusplus)
  using _CSTD abs;

  using _CSTD acos; using _CSTD asin;
  using _CSTD atan; using _CSTD atan2; using _CSTD ceil;
  using _CSTD cos; using _CSTD cosh; using _CSTD exp;
  using _CSTD fabs; using _CSTD floor; using _CSTD fmod;
  using _CSTD frexp; using _CSTD ldexp; using _CSTD log;
  using _CSTD log10; using _CSTD modf; using _CSTD pow;
  using _CSTD sin; using _CSTD sinh; using _CSTD sqrt;
  using _CSTD tan; using _CSTD tanh;

  #if _DLIB_ADD_C99_SYMBOLS
    using _CSTD acosf; using _CSTD asinf;
    using _CSTD atanf; using _CSTD atan2f; using _CSTD ceilf;
    using _CSTD cosf; using _CSTD coshf; using _CSTD expf;
    using _CSTD fabsf; using _CSTD floorf; using _CSTD fmodf;
    using _CSTD frexpf; using _CSTD ldexpf; using _CSTD logf;
    using _CSTD log10f; using _CSTD modff; using _CSTD powf;
    using _CSTD sinf; using _CSTD sinhf; using _CSTD sqrtf;
    using _CSTD tanf; using _CSTD tanhf;

    using _CSTD acosl; using _CSTD asinl;
    using _CSTD atanl; using _CSTD atan2l; using _CSTD ceill;
    using _CSTD cosl; using _CSTD coshl; using _CSTD expl;
    using _CSTD fabsl; using _CSTD floorl; using _CSTD fmodl;
    using _CSTD frexpl; using _CSTD ldexpl; using _CSTD logl;
    using _CSTD log10l; using _CSTD modfl; using _CSTD powl;
    using _CSTD sinl; using _CSTD sinhl; using _CSTD sqrtl;
    using _CSTD tanl; using _CSTD tanhl;
  #endif /* _DLIB_ADD_C99_SYMBOLS */

#endif /* defined(_STD_USING) && defined(__cplusplus) */


#endif /* _MATH */

/*
 * Copyright (c) 1992-2002 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V3.12:0576 */
