/****************************************************************************
 * include/nuttx/qencoder.h
 *
 *   Copyright (C) 2012, 2015, 2017 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef __INCLUDE_NUTTX_SENSORS_QENCODER_H
#define __INCLUDE_NUTTX_SENSORS_QENCODER_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/fs/ioctl.h>

#ifdef CONFIG_SENSORS_QENCODER

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************
 * CONFIG_SENSORS_QENCODER - Enables support for the quadrature encoder upper half
 */

/* IOCTL Commands ***********************************************************/

/* The Quadrature Encode module uses a standard character driver framework.
 * However, since the driver is a device control interface rather than a
 * data transfer interface, the majority of the functionality is implemented
 * in driver ioctl calls.  The PWM ioctl commands are listed below:
 *
 * QEIOC_POSITION - Get the current position from the encoder.
 *   Argument: int32_t pointer to the location to return the position.
 * QEIOC_RESET - Reset the position to zero.
 *   Argument: None
 */

#define QEIOC_POSITION     _QEIOC(0x0001) /* Arg: int32_t* pointer */
#define QEIOC_RESET        _QEIOC(0x0002) /* Arg: None */

#define QE_FIRST           0x0001         /* First required command */
#define QE_NCMDS           2              /* Two required commands */

/* User defined ioctl commands are also supported. These will be forwarded
 * by the upper-half QE driver to the lower-half QE driver via the ioctl()
 * method fo the QE lower-half interface.  However, the lower-half driver
 * must reserve a block of commands as follows in order prevent IOCTL
 * command numbers from overlapping.
 */

/* See arch/arm/src/tiva/tiva_qencoder.h (Not usable at that location) */

#define QE_TIVA_FIRST      (QE_FIRST + QE_NCMDS)
#define QE_TIVA_NCMDS      5

/* See include/nuttx/sensors/as5048b.h */

#define QE_AS5048B_FIRST   (QE_TIVA_FIRST + QEIOC_TIVA_NCMDS)
#define QE_AS5048B_NCMDS   4

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* This is the vtable that is used to by the upper half quadrature encoder
 * to call back into the lower half quadrature encoder.
 */

struct qe_lowerhalf_s;
struct qe_ops_s
{
  /* This method is called when the driver is opened.  The lower half driver
   * should configure and initialize the device so that it is ready for use.
   * The initial position value should be zero.
   */

  CODE int (*setup)(FAR struct qe_lowerhalf_s *lower);

  /* This method is called when the driver is closed.  The lower half driver
   * should stop data collection, free any resources, disable timer hardware,
   * and put the system into the lowest possible power usage state
   */

  CODE int (*shutdown)(FAR struct qe_lowerhalf_s *lower);

  /* Return the current position measurement. */

  CODE int (*position)(FAR struct qe_lowerhalf_s *lower, FAR int32_t *pos);

  /* Reset the position measurement to zero. */

  CODE int (*reset)(FAR struct qe_lowerhalf_s *lower);

  /* Lower-half logic may support platform-specific ioctl commands */

  CODE int (*ioctl)(FAR struct qe_lowerhalf_s *lower,
                    int cmd, unsigned long arg);
};

/* This is the interface between the lower half quadrature encoder driver
 * and the upper half quadrature encoder driver.  A (device-specific)
 * instance of this structure is passed to the upper-half driver when the
 * quadrature encoder driver is registered.
 *
 * Normally that lower half logic will have its own, custom state structure
 * that is simply cast to struct qe_lowerhalf_s.  In order to perform such
 * casts, the initial fields of the custom state structure match the initial
 * fields of the following generic lower half state structure.
 */

struct qe_lowerhalf_s
{
  /* The first field of this state structure must be a pointer to the lower-
   * half callback structure:
   */

  FAR const struct qe_ops_s *ops;

  /* The custom timer state structure may include additional fields after
   * the pointer to the callback structure.
   */
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: qe_register
 *
 * Description:
 *   Register the Quadrature Encoder lower half device as 'devpath'
 *
 * Input Parameters:
 *   devpath - The full path to the driver to register. E.g., "/dev/qe0"
 *   lower - An instance of the lower half interface
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

int qe_register(FAR const char *devpath, FAR struct qe_lowerhalf_s *lower);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* CONFIG_SENSORS_QENCODER */
#endif /* __INCLUDE_NUTTX_SENSORS_QENCODER_H */
