/** @file debug_test.h
 *  @brief C header file for debugger test routines
 *
 */

/* Copyright (C) 2007-2010 the NxOS developers
 *
 * Module Developed by: TC Wan <tcwan@cs.usm.my>
 *
 * See AUTHORS for a full list of the developers.
 *
 * See COPYING for redistribution license
 *
 */

#ifndef __DEBUG_TEST_H__
#define __DEBUG_TEST_H__

#include "_c_arm_macros.h"

#ifndef __ASSEMBLY__

/* Define C stuff */
/** @defgroup debug_public */
/*@{*/

FUNCDEF void dbg__test_arm_bkpt(void);
FUNCDEF void dbg__test_thumb_bkpt(void);

FUNCDEF void dbg__test_arm_instrstep(void);
FUNCDEF void dbg__test_thumb_instrstep(void);

 /*@}*/

#endif


#endif /* __DEBUG_TEST_H__ */
