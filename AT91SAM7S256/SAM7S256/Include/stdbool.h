/* stdbool.h header */
/* Copyright (C) 2003 IAR Systems.  All rights reserved. */

/* NOTE: IAR Extensions must be enabled in order to use the bool type! */

#ifndef _STDBOOL
#define _STDBOOL

#ifndef _SYSTEM_BUILD
#pragma system_include
#endif


#ifndef __C99_BOOL__
  #error "<stdbool.h>  compiled with wrong (version of IAR) compiler"
#endif

#ifndef __cplusplus

#define bool _Bool
#define true 1
#define false 0

#endif /* !__cplusplus */

#define __bool_true_false_are_defined 1

#endif /* !_STDBOOL */
