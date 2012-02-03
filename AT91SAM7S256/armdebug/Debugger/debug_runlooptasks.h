/** @file debug_runlooptasks.h
 *  @brief Shared C/ASM header file for debugger communications
 *
 */

/* Copyright (C) 2007-2011 the NxOS developers
 *
 * Module Developed by: TC Wan <tcwan@cs.usm.my>
 *
 * See AUTHORS for a full list of the developers.
 *
 * See COPYING for redistribution license
 *
 */

#ifndef __DEBUG_RUNLOOPTASKS_H__
#define __DEBUG_RUNLOOPTASKS_H__

#include "_c_arm_macros.h"

/* This is a place holder header file to allow for interfacing with C Routines in either
 * NxOS or NXT Firmware.
 *
 * Since the header files from the original source trees were meant for C programs, we can't
 * include them directly. Here we just use .extern to reference the routines.
 */

#ifdef __NXOS__
  .extern		nx__abort_info
  .extern		nx_systick_wait_ms

  .extern		nx_usb_is_connected
  .extern       nx_usb_can_write
  .extern       nx_usb_write
  .extern       nx_usb_data_written
  .extern       nx_usb_read
  .extern       nx_usb_data_read
  .extern       nx_core_reset
  .extern		nx_core_halt

#else           /* NXT Firmware */

  .extern		cCommInit
  .extern		cCommCtrl
  .extern		cCommExit
  .extern		dUsbWrite
  .extern		dUsbRead
  .extern		dUsbIsConfigured
  .extern		dBtSendMsg
  .equ			nxt_UBYTE_TRUE,		1
  .equ			nxt_UBYTE_FALSE,	0
  .equ			USB_CMD_READY,		0x01			/* From c_comm.iom */
  .equ			BT_CMD_READY,		0x02			/* From c_comm.iom */

  .extern		dIOCtrlSetPower
  .extern		dIOCtrlSetPwm
  .extern		dIOCtrlTransfer
  .equ			BOOT, 0xA55A                                  /* from c_ioctrl.iom */
  .equ			POWERDOWN, 0x5A00                             /* from c_ioctrl.iom */

#endif

#endif
