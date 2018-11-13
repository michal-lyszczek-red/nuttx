/************************************************************************************************
 * arch/arm/include/max326xx/max32660.h
 *
 *   Copyright (C) 2018 Gregory Nutt. All rights reserved.
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
 ************************************************************************************************/

/* This file should never be included directed but, rather, only indirectly through
 * nuttx/irq.h
 */

#ifndef __ARCH_ARM_INCLUDE_MAX326XX_MAX32660_IRQ_H
#define __ARCH_ARM_INCLUDE_MAX326XX_MAX32660_IRQ_H

/************************************************************************************************
 * Included Files
 ************************************************************************************************/

/************************************************************************************************
 * Pre-processor Definitions
 ************************************************************************************************/

/* External interrupts (vectors >= 16) */
/* REVISIT: Interrupt Vectors are not documented in the User Manual.  Here I am just assuming
 * that the vectors are the same for the other family members, but with unsupported options
 * removed.
 */
                                                             /*  0  Reserved */
#define MAX326XX_IRQ_SUPPLY     (MAX326XX_IRQ_EXTINT + 1)    /*  1  Power Supply Reset */
#define MAX326XX_IRQ_FLASHC     (MAX326XX_IRQ_EXTINT + 2)    /*  2  SRAM/FLASH Controller */
#define MAX326XX_IRQ_ALARM0     (MAX326XX_IRQ_EXTINT + 3)    /*  3  RTC Time-of-Day Alarm 0 */
#define MAX326XX_IRQ_ALARM1     (MAX326XX_IRQ_EXTINT + 4)    /*  4  RTC Time-of-Day Alarm 1 */
#define MAX326XX_IRQ_INTERVAL   (MAX326XX_IRQ_EXTINT + 5)    /*  5  RTC Interval Alarm */
#define MAX326XX_IRQ_RTCOVF     (MAX326XX_IRQ_EXTINT + 6)    /*  6  RTC Counter Overflow */
#define MAX326XX_IRQ_PMU        (MAX326XX_IRQ_EXTINT + 7)    /*  7  PMU */
#define MAX326XX_IRQ_USB        (MAX326XX_IRQ_EXTINT + 8)    /*  8-9  Reserved */
#define MAX326XX_IRQ_MMA        (MAX326XX_IRQ_EXTINT + 10)   /* 10  MMA */
#define MAX326XX_IRQ_WDT0       (MAX326XX_IRQ_EXTINT + 11)   /* 11  Watchdog 0 Timeout Interrupt */
                                                             /* 12-14  Watchdog 0 Pre-window Interrupt */
#define MAX326XX_IRQ_GPIO0      (MAX326XX_IRQ_EXTINT + 15)   /* 15  GPIO P0.1-7 External Interrupts */
                                                             /* 16-21 Reserved */
#define MAX326XX_IRQ_TIM0_0     (MAX326XX_IRQ_EXTINT + 22)   /* 22  Timer 0 Interrupt 0 */
#define MAX326XX_IRQ_TIM0_1     (MAX326XX_IRQ_EXTINT + 23)   /* 23  Timer 0 Interrupt 1 */
#define MAX326XX_IRQ_TIM1_0     (MAX326XX_IRQ_EXTINT + 24)   /* 24  Timer 1 Interrupt 0 */
#define MAX326XX_IRQ_TIM1_1     (MAX326XX_IRQ_EXTINT + 25)   /* 25  Timer 1 Interrupt 1 */
#define MAX326XX_IRQ_TIM2_0     (MAX326XX_IRQ_EXTINT + 26)   /* 26  Timer 2 Interrupt 0 */
#define MAX326XX_IRQ_TIM2_1     (MAX326XX_IRQ_EXTINT + 27)   /* 27  Timer 2 Interrupt 1 */
#define MAX326XX_IRQ_TIM3_0     (MAX326XX_IRQ_EXTINT + 28)   /* 28-33 Reserved */
#define MAX326XX_IRQ_UART0      (MAX326XX_IRQ_EXTINT + 34)   /* 34  UART 0 */
#define MAX326XX_IRQ_UART1      (MAX326XX_IRQ_EXTINT + 35)   /* 34  UART 1 */
#define MAX326XX_IRQ_UART2      (MAX326XX_IRQ_EXTINT + 36)   /* 36-38 Reserved */
#define MAX326XX_IRQ_I2CM0      (MAX326XX_IRQ_EXTINT + 39)   /* 39  I2C0 Master */
#define MAX326XX_IRQ_I2CM1      (MAX326XX_IRQ_EXTINT + 40)   /* 40  I2C1 Master */
                                                             /* 41-42 Reserved */
#define MAX326XX_IRQ_SPIM0      (MAX326XX_IRQ_EXTINT + 43)   /* 43-44  SPI Master 0 */
#define MAX326XX_IRQ_SPIM1      (MAX326XX_IRQ_EXTINT + 44)   /* 44  SPI Master 1 */
#define MAX326XX_IRQ_SPIM2      (MAX326XX_IRQ_EXTINT + 45)   /* 45  SPI Master 2 */
                                                             /* 46-48  Reserved */
#define MAX326XX_IRQ_SPIS       (MAX326XX_IRQ_EXTINT + 49)   /* 49  SPI Slave */

/* Number of external interrupts and total number of vectors */

#define MAX326XX_IRQ_NEXTINT    50
#define NR_IRQS                 (MAX326XX_IRQ_EXTINT + MAX326XX_IRQ_NEXTINT)

/************************************************************************************************
 * Public Types
 ************************************************************************************************/

/************************************************************************************************
 * Inline functions
 ************************************************************************************************/

/************************************************************************************************
 * Public Data
 ************************************************************************************************/

/************************************************************************************************
 * Public Function Prototypes
 ************************************************************************************************/

#ifndef __ASSEMBLY__
#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

#undef EXTERN
#ifdef __cplusplus
}
#endif
#endif

#endif /* __ARCH_ARM_INCLUDE_MAX326XX_MAX32660_IRQ_H */
