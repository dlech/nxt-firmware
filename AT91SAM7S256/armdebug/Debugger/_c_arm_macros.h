/** @file _c_arm_macros.h
 *  @brief Define macros to support shared C and ASM headers
 *
 */

/* Copyright (C) 2010 the NxOS developers
 *
 * Module Developed by: TC Wan <tcwan@cs.usm.my>
 *
 * Thanks to Bartli (forum post @ embdev.net ARM programming with GCC/GNU tools forum)
 *
 * See AUTHORS for a full list of the developers.
 *
 * See COPYING for redistribution license
 *
 */

#ifndef __C_ARM_MACROS__
#define __C_ARM_MACROS__


#ifdef __ASSEMBLY__

#define NULL   0x0
#define FALSE   0
#define TRUE    ~FALSE

#define TYPEDEF @
#define FUNCDEF @

  .set last_enum_value, 0
  .macro enum_val name
  .equiv \name, last_enum_value
  .set last_enum_value, last_enum_value + 1
  .endm

#define ENUM_BEGIN  .set last_enum_value, 0

#define ENUM_VAL(name) enum_val name
#define ENUM_VALASSIGN(name, value)            \
  .set last_enum_value, value                 ;\
  enum_val name
#define ENUM_END(enum_name)

#else  /* C Defines */
/** Macro to control typedef generation
 *
 */
#define TYPEDEF typedef

/** Macro to control extern generation
 *
 */
#ifndef FUNCDEF
#define FUNCDEF extern
#endif

/** Macro to control typedef enum generation
 *
 */
#define ENUM_BEGIN typedef enum {

/** Macro to specify enum instance (auto value assignment)
 *
 */
#define ENUM_VAL(name) name,

/** Macro to control enum specification and value assignment
*
*/
#define ENUM_VALASSIGN(name, value) name = value,

/** Macro to control enum named type generation
 *
 */
#define ENUM_END(enum_name) } enum_name;

#endif

/* Example of how to use the ENUM definition macros
ENUM_BEGIN
ENUM_VAL(INIT)
ENUM_VAL(RESET)
ENUM_VAL(CONFIGURED)
ENUM_END(enum_label)
*/

#endif /* __C_ARM_MACROS__ */
