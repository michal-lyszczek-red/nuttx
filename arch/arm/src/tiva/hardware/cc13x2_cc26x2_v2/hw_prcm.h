/******************************************************************************
 *  Filename:       hw_prcm_h
 *  Revised:        2018-05-14 12:24:52 +0200 (Mon, 14 May 2018)
 *  Revision:       51990
 *
 * Copyright (c) 2015 - 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3) Neither the name NuttX nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#ifndef __ARCH_ARM_SRC_TIVA_HARDWARE_CC13X2_CC26X2_V2_PRCM_H
#define __ARCH_ARM_SRC_TIVA_HARDWARE_CC13X2_CC26X2_V2_PRCM_H

/******************************************************************************
 * Pre-processor Definitions
 ******************************************************************************/

/******************************************************************************
 *
 * This section defines the register offsets of
 * PRCM component
 *
 ******************************************************************************
 * Infrastructure Clock Division Factor For Run Mode
 */

#define PRCM_INFRCLKDIVR_OFFSET                                     0x00000000

/* Infrastructure Clock Division Factor For Sleep Mode */

#define PRCM_INFRCLKDIVS_OFFSET                                     0x00000004

/* Infrastructure Clock Division Factor For DeepSleep Mode */

#define PRCM_INFRCLKDIVDS_OFFSET                                    0x00000008

/* MCU Voltage Domain Control */

#define PRCM_VDCTL_OFFSET                                           0x0000000c

/* Load PRCM Settings To CLKCTRL Power Domain */

#define PRCM_CLKLOADCTL_OFFSET                                      0x00000028

/* RFC Clock Gate */

#define PRCM_RFCCLKG_OFFSET                                         0x0000002c

/* VIMS Clock Gate */

#define PRCM_VIMSCLKG_OFFSET                                        0x00000030

/* SEC (PKA And TRNG And CRYPTO) And UDMA Clock Gate For Run And All Modes */

#define PRCM_SECDMACLKGR_OFFSET                                     0x0000003c

/* SEC (PKA And TRNG And CRYPTO) And UDMA Clock Gate For Sleep Mode */

#define PRCM_SECDMACLKGS_OFFSET                                     0x00000040

/* SEC (PKA And TRNG and CRYPTO) And UDMA Clock Gate For Deep Sleep Mode */

#define PRCM_SECDMACLKGDS_OFFSET                                    0x00000044

/* GPIO Clock Gate For Run And All Modes */

#define PRCM_GPIOCLKGR_OFFSET                                       0x00000048

/* GPIO Clock Gate For Sleep Mode */

#define PRCM_GPIOCLKGS_OFFSET                                       0x0000004c

/* GPIO Clock Gate For Deep Sleep Mode */

#define PRCM_GPIOCLKGDS_OFFSET                                      0x00000050

/* GPT Clock Gate For Run And All Modes */

#define PRCM_GPTCLKGR_OFFSET                                        0x00000054

/* GPT Clock Gate For Sleep Mode */

#define PRCM_GPTCLKGS_OFFSET                                        0x00000058

/* GPT Clock Gate For Deep Sleep Mode */

#define PRCM_GPTCLKGDS_OFFSET                                       0x0000005c

/* I2C Clock Gate For Run And All Modes */

#define PRCM_I2CCLKGR_OFFSET                                        0x00000060

/* I2C Clock Gate For Sleep Mode */

#define PRCM_I2CCLKGS_OFFSET                                        0x00000064

/* I2C Clock Gate For Deep Sleep Mode */

#define PRCM_I2CCLKGDS_OFFSET                                       0x00000068

/* UART Clock Gate For Run And All Modes */

#define PRCM_UARTCLKGR_OFFSET                                       0x0000006c

/* UART Clock Gate For Sleep Mode */

#define PRCM_UARTCLKGS_OFFSET                                       0x00000070

/* UART Clock Gate For Deep Sleep Mode */

#define PRCM_UARTCLKGDS_OFFSET                                      0x00000074

/* SSI Clock Gate For Run And All Modes */

#define PRCM_SSICLKGR_OFFSET                                        0x00000078

/* SSI Clock Gate For Sleep Mode */

#define PRCM_SSICLKGS_OFFSET                                        0x0000007c

/* SSI Clock Gate For Deep Sleep Mode */

#define PRCM_SSICLKGDS_OFFSET                                       0x00000080

/* I2S Clock Gate For Run And All Modes */

#define PRCM_I2SCLKGR_OFFSET                                        0x00000084

/* I2S Clock Gate For Sleep Mode */

#define PRCM_I2SCLKGS_OFFSET                                        0x00000088

/* I2S Clock Gate For Deep Sleep Mode */

#define PRCM_I2SCLKGDS_OFFSET                                       0x0000008c

/* Internal */

#define PRCM_SYSBUSCLKDIV_OFFSET                                    0x000000b4

/* Internal */

#define PRCM_CPUCLKDIV_OFFSET                                       0x000000b8

/* Internal */

#define PRCM_PERBUSCPUCLKDIV_OFFSET                                 0x000000bc

/* Internal */

#define PRCM_PERDMACLKDIV_OFFSET                                    0x000000c4

/* I2S Clock Control */

#define PRCM_I2SBCLKSEL_OFFSET                                      0x000000c8

/* GPT Scalar */

#define PRCM_GPTCLKDIV_OFFSET                                       0x000000cc

/* I2S Clock Control */

#define PRCM_I2SCLKCTL_OFFSET                                       0x000000d0

/* MCLK Division Ratio */

#define PRCM_I2SMCLKDIV_OFFSET                                      0x000000d4

/* BCLK Division Ratio */

#define PRCM_I2SBCLKDIV_OFFSET                                      0x000000d8

/* WCLK Division Ratio */

#define PRCM_I2SWCLKDIV_OFFSET                                      0x000000dc

/* RESET For SEC (PKA And TRNG And CRYPTO) And UDMA */

#define PRCM_RESETSECDMA_OFFSET                                     0x000000f0

/* RESET For GPIO IPs */

#define PRCM_RESETGPIO_OFFSET                                       0x000000f4

/* RESET For GPT Ips */

#define PRCM_RESETGPT_OFFSET                                        0x000000f8

/* RESET For I2C IPs */

#define PRCM_RESETI2C_OFFSET                                        0x000000fc

/* RESET For UART IPs */

#define PRCM_RESETUART_OFFSET                                       0x00000100

/* RESET For SSI IPs */

#define PRCM_RESETSSI_OFFSET                                        0x00000104

/* RESET For I2S IP */

#define PRCM_RESETI2S_OFFSET                                        0x00000108

/* Power Domain Control */

#define PRCM_PDCTL0_OFFSET                                          0x0000012c

/* RFC Power Domain Control */

#define PRCM_PDCTL0RFC_OFFSET                                       0x00000130

/* SERIAL Power Domain Control */

#define PRCM_PDCTL0SERIAL_OFFSET                                    0x00000134

/* PERIPH Power Domain Control */

#define PRCM_PDCTL0PERIPH_OFFSET                                    0x00000138

/* Power Domain Status */

#define PRCM_PDSTAT0_OFFSET                                         0x00000140

/* RFC Power Domain Status */

#define PRCM_PDSTAT0RFC_OFFSET                                      0x00000144

/* SERIAL Power Domain Status */

#define PRCM_PDSTAT0SERIAL_OFFSET                                   0x00000148

/* PERIPH Power Domain Status */

#define PRCM_PDSTAT0PERIPH_OFFSET                                   0x0000014c

/* Power Domain Control */

#define PRCM_PDCTL1_OFFSET                                          0x0000017c

/* CPU Power Domain Direct Control */

#define PRCM_PDCTL1CPU_OFFSET                                       0x00000184

/* RFC Power Domain Direct Control */

#define PRCM_PDCTL1RFC_OFFSET                                       0x00000188

/* VIMS Mode Direct Control */

#define PRCM_PDCTL1VIMS_OFFSET                                      0x0000018c

/* Power Manager Status */

#define PRCM_PDSTAT1_OFFSET                                         0x00000194

/* BUS Power Domain Direct Read Status */

#define PRCM_PDSTAT1BUS_OFFSET                                      0x00000198

/* RFC Power Domain Direct Read Status */

#define PRCM_PDSTAT1RFC_OFFSET                                      0x0000019c

/* CPU Power Domain Direct Read Status */

#define PRCM_PDSTAT1CPU_OFFSET                                      0x000001a0

/* VIMS Mode Direct Read Status */

#define PRCM_PDSTAT1VIMS_OFFSET                                     0x000001a4

/* Control To RFC */

#define PRCM_RFCBITS_OFFSET                                         0x000001cc

/* Selected RFC Mode */

#define PRCM_RFCMODESEL_OFFSET                                      0x000001d0

/* Allowed RFC Modes */

#define PRCM_RFCMODEHWOPT_OFFSET                                    0x000001d4

/* Power Profiler Register */

#define PRCM_PWRPROFSTAT_OFFSET                                     0x000001e0

/* MCU SRAM configuration */

#define PRCM_MCUSRAMCFG_OFFSET                                      0x0000021c

/* Memory Retention Control */

#define PRCM_RAMRETEN_OFFSET                                        0x00000224

/* Oscillator Interrupt Mask */

#define PRCM_OSCIMSC_OFFSET                                         0x00000290

/* Oscillator Raw Interrupt Status */

#define PRCM_OSCRIS_OFFSET                                          0x00000294

/* Oscillator Raw Interrupt Clear */

#define PRCM_OSCICR_OFFSET                                          0x00000298

/******************************************************************************
 *
 * Register: PRCM_INFRCLKDIVR
 *
 ******************************************************************************
 * Field:   [1:0] RATIO
 *
 * Division rate for clocks driving modules in the MCU_AON domain when system
 * CPU is in run mode. Division ratio affects both infrastructure clock and
 * perbusull clock.
 * ENUMs:
 * DIV32                    Divide by 32
 * DIV8                     Divide by 8
 * DIV2                     Divide by 2
 * DIV1                     Divide by 1
 */

#define PRCM_INFRCLKDIVR_RATIO_MASK                                 0x00000003
#define PRCM_INFRCLKDIVR_RATIO_SHIFT                                         0
#define PRCM_INFRCLKDIVR_RATIO_DIV32                                0x00000003
#define PRCM_INFRCLKDIVR_RATIO_DIV8                                 0x00000002
#define PRCM_INFRCLKDIVR_RATIO_DIV2                                 0x00000001
#define PRCM_INFRCLKDIVR_RATIO_DIV1                                 0x00000000

/******************************************************************************
 *
 * Register: PRCM_INFRCLKDIVS
 *
 ******************************************************************************
 * Field:   [1:0] RATIO
 *
 * Division rate for clocks driving modules in the MCU_AON domain when system
 * CPU is in sleep mode. Division ratio affects both infrastructure clock and
 * perbusull clock.
 * ENUMs:
 * DIV32                    Divide by 32
 * DIV8                     Divide by 8
 * DIV2                     Divide by 2
 * DIV1                     Divide by 1
 */

#define PRCM_INFRCLKDIVS_RATIO_MASK                                 0x00000003
#define PRCM_INFRCLKDIVS_RATIO_SHIFT                                         0
#define PRCM_INFRCLKDIVS_RATIO_DIV32                                0x00000003
#define PRCM_INFRCLKDIVS_RATIO_DIV8                                 0x00000002
#define PRCM_INFRCLKDIVS_RATIO_DIV2                                 0x00000001
#define PRCM_INFRCLKDIVS_RATIO_DIV1                                 0x00000000

/******************************************************************************
 *
 * Register: PRCM_INFRCLKDIVDS
 *
 ******************************************************************************
 * Field:   [1:0] RATIO
 *
 * Division rate for clocks driving modules in the MCU_AON domain when system
 * CPU is in seepsleep mode. Division ratio affects both infrastructure clock
 * and perbusull clock.
 * ENUMs:
 * DIV32                    Divide by 32
 * DIV8                     Divide by 8
 * DIV2                     Divide by 2
 * DIV1                     Divide by 1
 */

#define PRCM_INFRCLKDIVDS_RATIO_MASK                                0x00000003
#define PRCM_INFRCLKDIVDS_RATIO_SHIFT                                        0
#define PRCM_INFRCLKDIVDS_RATIO_DIV32                               0x00000003
#define PRCM_INFRCLKDIVDS_RATIO_DIV8                                0x00000002
#define PRCM_INFRCLKDIVDS_RATIO_DIV2                                0x00000001
#define PRCM_INFRCLKDIVDS_RATIO_DIV1                                0x00000000

/******************************************************************************
 *
 * Register: PRCM_VDCTL
 *
 ******************************************************************************
 * Field:     [0] ULDO
 *
 * Request PMCTL to switch to uLDO.
 *
 * 0: No request
 * 1: Assert request when possible
 *
 * The bit will have no effect before the following requirements are met:
 * 1. PDCTL1.CPU_ON = 0
 * 2. PDCTL1.VIMS_MODE = x0
 * 3. SECDMACLKGDS.DMA_CLK_EN = 0 and S.CRYPTO_CLK_EN] = 0 and
 * SECDMACLKGR.DMA_AM_CLK_EN = 0 (Note: Settings must be loaded with
 * CLKLOADCTL.LOAD)
 * 4. SECDMACLKGDS.CRYPTO_CLK_EN = 0 and  SECDMACLKGR.CRYPTO_AM_CLK_EN = 0
 * (Note: Settings must be loaded with CLKLOADCTL.LOAD)
 * 5. I2SCLKGDS.CLK_EN = 0 and I2SCLKGR.AM_CLK_EN = 0 (Note: Settings must be
 * loaded with CLKLOADCTL.LOAD)
 * 6. RFC do no request access to BUS
 * 7. System CPU in deepsleep
 */

#define PRCM_VDCTL_ULDO                                             0x00000001
#define PRCM_VDCTL_ULDO_MASK                                        0x00000001
#define PRCM_VDCTL_ULDO_SHIFT                                                0

/******************************************************************************
 *
 * Register: PRCM_CLKLOADCTL
 *
 ******************************************************************************
 * Field:     [1] LOAD_DONE
 *
 * Status of LOAD.
 * Will be cleared to 0 when any of the registers requiring a LOAD is written
 * to, and be set to 1 when a LOAD is done.
 * Note that writing no change to a register will result in the LOAD_DONE being
 * cleared.
 *
 * 0 : One or more registers have been write accessed after last LOAD
 * 1 : No registers are write accessed after last LOAD
 */

#define PRCM_CLKLOADCTL_LOAD_DONE                                   0x00000002
#define PRCM_CLKLOADCTL_LOAD_DONE_MASK                              0x00000002
#define PRCM_CLKLOADCTL_LOAD_DONE_SHIFT                                      1

/* Field:     [0] LOAD
 *
 *
 * 0: No action
 * 1: Load settings to CLKCTRL. Bit is HW cleared.
 *
 * Multiple changes to settings may be done before LOAD is written once so all
 * changes takes place at the same time. LOAD can also be done after single
 * setting updates.
 *
 * Registers that needs to be followed by LOAD before settings being applied
 * are:
 * - SYSBUSCLKDIV
 * - CPUCLKDIV
 * - PERBUSCPUCLKDIV
 * - PERDMACLKDIV
 * - PERBUSCPUCLKG
 * - RFCCLKG
 * - VIMSCLKG
 * - SECDMACLKGR
 * - SECDMACLKGS
 * - SECDMACLKGDS
 * - GPIOCLKGR
 * - GPIOCLKGS
 * - GPIOCLKGDS
 * - GPTCLKGR
 * - GPTCLKGS
 * - GPTCLKGDS
 * - GPTCLKDIV
 * - I2CCLKGR
 * - I2CCLKGS
 * - I2CCLKGDS
 * - SSICLKGR
 * - SSICLKGS
 * - SSICLKGDS
 * - UARTCLKGR
 * - UARTCLKGS
 * - UARTCLKGDS
 * - I2SCLKGR
 * - I2SCLKGS
 * - I2SCLKGDS
 * - I2SBCLKSEL
 * - I2SCLKCTL
 * - I2SMCLKDIV
 * - I2SBCLKDIV
 * - I2SWCLKDIV
 */

#define PRCM_CLKLOADCTL_LOAD                                        0x00000001
#define PRCM_CLKLOADCTL_LOAD_MASK                                   0x00000001
#define PRCM_CLKLOADCTL_LOAD_SHIFT                                           0

/******************************************************************************
 *
 * Register: PRCM_RFCCLKG
 *
 ******************************************************************************
 * Field:     [0] CLK_EN
 *
 *
 * 0: Disable Clock
 * 1: Enable clock if RFC power domain is on
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_RFCCLKG_CLK_EN                                         0x00000001
#define PRCM_RFCCLKG_CLK_EN_MASK                                    0x00000001
#define PRCM_RFCCLKG_CLK_EN_SHIFT                                            0

/******************************************************************************
 *
 * Register: PRCM_VIMSCLKG
 *
 ******************************************************************************
 * Field:   [1:0] CLK_EN
 *
 * 00: Disable clock
 * 01: Disable clock when SYSBUS clock is disabled
 * 11: Enable clock
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_VIMSCLKG_CLK_EN_MASK                                   0x00000003
#define PRCM_VIMSCLKG_CLK_EN_SHIFT                                           0

/******************************************************************************
 *
 * Register: PRCM_SECDMACLKGR
 *
 ******************************************************************************
 * Field:    [24] DMA_AM_CLK_EN
 *
 *
 * 0: No force
 * 1: Force clock on for all modes (Run, Sleep and Deep Sleep)
 *
 * Overrides DMA_CLK_EN, SECDMACLKGS.DMA_CLK_EN and SECDMACLKGDS.DMA_CLK_EN
 * when enabled.
 *
 * SYSBUS clock will always run when enabled
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_SECDMACLKGR_DMA_AM_CLK_EN                              0x01000000
#define PRCM_SECDMACLKGR_DMA_AM_CLK_EN_MASK                         0x01000000
#define PRCM_SECDMACLKGR_DMA_AM_CLK_EN_SHIFT                                24

/* Field:    [19] PKA_ZERIOZE_RESET_N
 *
 * Zeroization logic hardware reset.
 *
 * 0: pka_zeroize logic inactive.
 * 1: pka_zeroize of memory  is enabled.
 *
 * This register must remain active until the memory are completely zeroized
 * which requires 256 periods on systembus clock.
 */

#define PRCM_SECDMACLKGR_PKA_ZERIOZE_RESET_N                        0x00080000
#define PRCM_SECDMACLKGR_PKA_ZERIOZE_RESET_N_MASK                   0x00080000
#define PRCM_SECDMACLKGR_PKA_ZERIOZE_RESET_N_SHIFT                          19

/* Field:    [18] PKA_AM_CLK_EN
 *
 *
 * 0: No force
 * 1: Force clock on for all modes (Run, Sleep and Deep Sleep)
 *
 * Overrides PKA_CLK_EN, SECDMACLKGS.PKA_CLK_EN and SECDMACLKGDS.PKA_CLK_EN
 * when enabled.
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_SECDMACLKGR_PKA_AM_CLK_EN                              0x00040000
#define PRCM_SECDMACLKGR_PKA_AM_CLK_EN_MASK                         0x00040000
#define PRCM_SECDMACLKGR_PKA_AM_CLK_EN_SHIFT                                18

/* Field:    [17] TRNG_AM_CLK_EN
 *
 *
 * 0: No force
 * 1: Force clock on for all modes (Run, Sleep and Deep Sleep)
 *
 * Overrides TRNG_CLK_EN, SECDMACLKGS.TRNG_CLK_EN and SECDMACLKGDS.TRNG_CLK_EN
 * when enabled.
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_SECDMACLKGR_TRNG_AM_CLK_EN                             0x00020000
#define PRCM_SECDMACLKGR_TRNG_AM_CLK_EN_MASK                        0x00020000
#define PRCM_SECDMACLKGR_TRNG_AM_CLK_EN_SHIFT                               17

/* Field:    [16] CRYPTO_AM_CLK_EN
 *
 *
 * 0: No force
 * 1: Force clock on for all modes (Run, Sleep and Deep Sleep)
 *
 * Overrides CRYPTO_CLK_EN, SECDMACLKGS.CRYPTO_CLK_EN and
 * SECDMACLKGDS.CRYPTO_CLK_EN when enabled.
 *
 * SYSBUS clock will always run when enabled
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_SECDMACLKGR_CRYPTO_AM_CLK_EN                           0x00010000
#define PRCM_SECDMACLKGR_CRYPTO_AM_CLK_EN_MASK                      0x00010000
#define PRCM_SECDMACLKGR_CRYPTO_AM_CLK_EN_SHIFT                             16

/* Field:     [8] DMA_CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by DMA_AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_SECDMACLKGR_DMA_CLK_EN                                 0x00000100
#define PRCM_SECDMACLKGR_DMA_CLK_EN_MASK                            0x00000100
#define PRCM_SECDMACLKGR_DMA_CLK_EN_SHIFT                                    8

/* Field:     [2] PKA_CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by PKA_AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_SECDMACLKGR_PKA_CLK_EN                                 0x00000004
#define PRCM_SECDMACLKGR_PKA_CLK_EN_MASK                            0x00000004
#define PRCM_SECDMACLKGR_PKA_CLK_EN_SHIFT                                    2

/* Field:     [1] TRNG_CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by TRNG_AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_SECDMACLKGR_TRNG_CLK_EN                                0x00000002
#define PRCM_SECDMACLKGR_TRNG_CLK_EN_MASK                           0x00000002
#define PRCM_SECDMACLKGR_TRNG_CLK_EN_SHIFT                                   1

/* Field:     [0] CRYPTO_CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by CRYPTO_AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_SECDMACLKGR_CRYPTO_CLK_EN                              0x00000001
#define PRCM_SECDMACLKGR_CRYPTO_CLK_EN_MASK                         0x00000001
#define PRCM_SECDMACLKGR_CRYPTO_CLK_EN_SHIFT                                 0

/******************************************************************************
 *
 * Register: PRCM_SECDMACLKGS
 *
 ******************************************************************************
 * Field:     [8] DMA_CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by SECDMACLKGR.DMA_AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_SECDMACLKGS_DMA_CLK_EN                                 0x00000100
#define PRCM_SECDMACLKGS_DMA_CLK_EN_MASK                            0x00000100
#define PRCM_SECDMACLKGS_DMA_CLK_EN_SHIFT                                    8

/* Field:     [2] PKA_CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by SECDMACLKGR.PKA_AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_SECDMACLKGS_PKA_CLK_EN                                 0x00000004
#define PRCM_SECDMACLKGS_PKA_CLK_EN_MASK                            0x00000004
#define PRCM_SECDMACLKGS_PKA_CLK_EN_SHIFT                                    2

/* Field:     [1] TRNG_CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by SECDMACLKGR.TRNG_AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_SECDMACLKGS_TRNG_CLK_EN                                0x00000002
#define PRCM_SECDMACLKGS_TRNG_CLK_EN_MASK                           0x00000002
#define PRCM_SECDMACLKGS_TRNG_CLK_EN_SHIFT                                   1

/* Field:     [0] CRYPTO_CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by SECDMACLKGR.CRYPTO_AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_SECDMACLKGS_CRYPTO_CLK_EN                              0x00000001
#define PRCM_SECDMACLKGS_CRYPTO_CLK_EN_MASK                         0x00000001
#define PRCM_SECDMACLKGS_CRYPTO_CLK_EN_SHIFT                                 0

/******************************************************************************
 *
 * Register: PRCM_SECDMACLKGDS
 *
 ******************************************************************************
 * Field:     [8] DMA_CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by SECDMACLKGR.DMA_AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_SECDMACLKGDS_DMA_CLK_EN                                0x00000100
#define PRCM_SECDMACLKGDS_DMA_CLK_EN_MASK                           0x00000100
#define PRCM_SECDMACLKGDS_DMA_CLK_EN_SHIFT                                   8

/* Field:     [2] PKA_CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by SECDMACLKGR.PKA_AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_SECDMACLKGDS_PKA_CLK_EN                                0x00000004
#define PRCM_SECDMACLKGDS_PKA_CLK_EN_MASK                           0x00000004
#define PRCM_SECDMACLKGDS_PKA_CLK_EN_SHIFT                                   2

/* Field:     [1] TRNG_CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * SYSBUS clock will always run when enabled
 *
 * Can be forced on by SECDMACLKGR.TRNG_AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_SECDMACLKGDS_TRNG_CLK_EN                               0x00000002
#define PRCM_SECDMACLKGDS_TRNG_CLK_EN_MASK                          0x00000002
#define PRCM_SECDMACLKGDS_TRNG_CLK_EN_SHIFT                                  1

/* Field:     [0] CRYPTO_CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * SYSBUS clock will always run when enabled
 *
 * Can be forced on by SECDMACLKGR.CRYPTO_AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_SECDMACLKGDS_CRYPTO_CLK_EN                             0x00000001
#define PRCM_SECDMACLKGDS_CRYPTO_CLK_EN_MASK                        0x00000001
#define PRCM_SECDMACLKGDS_CRYPTO_CLK_EN_SHIFT                                0

/******************************************************************************
 *
 * Register: PRCM_GPIOCLKGR
 *
 ******************************************************************************
 * Field:     [8] AM_CLK_EN
 *
 *
 * 0: No force
 * 1: Force clock on for all modes (Run, Sleep and Deep Sleep)
 *
 * Overrides CLK_EN,  GPIOCLKGS.CLK_EN and  GPIOCLKGDS.CLK_EN when enabled.
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_GPIOCLKGR_AM_CLK_EN                                    0x00000100
#define PRCM_GPIOCLKGR_AM_CLK_EN_MASK                               0x00000100
#define PRCM_GPIOCLKGR_AM_CLK_EN_SHIFT                                       8

/* Field:     [0] CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_GPIOCLKGR_CLK_EN                                       0x00000001
#define PRCM_GPIOCLKGR_CLK_EN_MASK                                  0x00000001
#define PRCM_GPIOCLKGR_CLK_EN_SHIFT                                          0

/******************************************************************************
 *
 * Register: PRCM_GPIOCLKGS
 *
 ******************************************************************************
 * Field:     [0] CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by GPIOCLKGR.AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_GPIOCLKGS_CLK_EN                                       0x00000001
#define PRCM_GPIOCLKGS_CLK_EN_MASK                                  0x00000001
#define PRCM_GPIOCLKGS_CLK_EN_SHIFT                                          0

/******************************************************************************
 *
 * Register: PRCM_GPIOCLKGDS
 *
 ******************************************************************************
 * Field:     [0] CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by GPIOCLKGR.AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_GPIOCLKGDS_CLK_EN                                      0x00000001
#define PRCM_GPIOCLKGDS_CLK_EN_MASK                                 0x00000001
#define PRCM_GPIOCLKGDS_CLK_EN_SHIFT                                         0

/******************************************************************************
 *
 * Register: PRCM_GPTCLKGR
 *
 ******************************************************************************
 * Field:  [11:8] AM_CLK_EN
 *
 * Each bit below has the following meaning:
 *
 * 0: No force
 * 1: Force clock on for all modes (Run, Sleep and Deep Sleep)
 *
 * Overrides CLK_EN,  GPTCLKGS.CLK_EN and  GPTCLKGDS.CLK_EN when enabled.
 *
 * ENUMs can be combined
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 * ENUMs:
 * AM_GPT3                  Enable clock for GPT3  in all modes
 * AM_GPT2                  Enable clock for GPT2  in all modes
 * AM_GPT1                  Enable clock for GPT1  in all modes
 * AM_GPT0                  Enable clock for GPT0 in all modes
 */

#define PRCM_GPTCLKGR_AM_CLK_EN_MASK                                0x00000f00
#define PRCM_GPTCLKGR_AM_CLK_EN_SHIFT                                        8
#define PRCM_GPTCLKGR_AM_CLK_EN_AM_GPT3                             0x00000800
#define PRCM_GPTCLKGR_AM_CLK_EN_AM_GPT2                             0x00000400
#define PRCM_GPTCLKGR_AM_CLK_EN_AM_GPT1                             0x00000200
#define PRCM_GPTCLKGR_AM_CLK_EN_AM_GPT0                             0x00000100

/* Field:   [3:0] CLK_EN
 *
 * Each bit below has the following meaning:
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by AM_CLK_EN
 *
 * ENUMs can be combined
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 * ENUMs:
 * GPT3                     Enable clock for GPT3
 * GPT2                     Enable clock for GPT2
 * GPT1                     Enable clock for GPT1
 * GPT0                     Enable clock for GPT0
 */

#define PRCM_GPTCLKGR_CLK_EN_MASK                                   0x0000000f
#define PRCM_GPTCLKGR_CLK_EN_SHIFT                                           0
#define PRCM_GPTCLKGR_CLK_EN_GPT3                                   0x00000008
#define PRCM_GPTCLKGR_CLK_EN_GPT2                                   0x00000004
#define PRCM_GPTCLKGR_CLK_EN_GPT1                                   0x00000002
#define PRCM_GPTCLKGR_CLK_EN_GPT0                                   0x00000001

/******************************************************************************
 *
 * Register: PRCM_GPTCLKGS
 *
 ******************************************************************************
 * Field:   [3:0] CLK_EN
 *
 * Each bit below has the following meaning:
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by GPTCLKGR.AM_CLK_EN
 *
 * ENUMs can be combined
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 * ENUMs:
 * GPT3                     Enable clock for GPT3
 * GPT2                     Enable clock for GPT2
 * GPT1                     Enable clock for GPT1
 * GPT0                     Enable clock for GPT0
 */

#define PRCM_GPTCLKGS_CLK_EN_MASK                                   0x0000000f
#define PRCM_GPTCLKGS_CLK_EN_SHIFT                                           0
#define PRCM_GPTCLKGS_CLK_EN_GPT3                                   0x00000008
#define PRCM_GPTCLKGS_CLK_EN_GPT2                                   0x00000004
#define PRCM_GPTCLKGS_CLK_EN_GPT1                                   0x00000002
#define PRCM_GPTCLKGS_CLK_EN_GPT0                                   0x00000001

/******************************************************************************
 *
 * Register: PRCM_GPTCLKGDS
 *
 ******************************************************************************
 * Field:   [3:0] CLK_EN
 *
 * Each bit below has the following meaning:
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by GPTCLKGR.AM_CLK_EN
 *
 * ENUMs can be combined
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 * ENUMs:
 * GPT3                     Enable clock for GPT3
 * GPT2                     Enable clock for GPT2
 * GPT1                     Enable clock for GPT1
 * GPT0                     Enable clock for GPT0
 */

#define PRCM_GPTCLKGDS_CLK_EN_MASK                                  0x0000000f
#define PRCM_GPTCLKGDS_CLK_EN_SHIFT                                          0
#define PRCM_GPTCLKGDS_CLK_EN_GPT3                                  0x00000008
#define PRCM_GPTCLKGDS_CLK_EN_GPT2                                  0x00000004
#define PRCM_GPTCLKGDS_CLK_EN_GPT1                                  0x00000002
#define PRCM_GPTCLKGDS_CLK_EN_GPT0                                  0x00000001

/******************************************************************************
 *
 * Register: PRCM_I2CCLKGR
 *
 ******************************************************************************
 * Field:     [8] AM_CLK_EN
 *
 *
 * 0: No force
 * 1: Force clock on for all modes (Run, Sleep and Deep Sleep)
 *
 * Overrides CLK_EN,  I2CCLKGS.CLK_EN and  I2CCLKGDS.CLK_EN when enabled.
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_I2CCLKGR_AM_CLK_EN                                     0x00000100
#define PRCM_I2CCLKGR_AM_CLK_EN_MASK                                0x00000100
#define PRCM_I2CCLKGR_AM_CLK_EN_SHIFT                                        8

/* Field:     [0] CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_I2CCLKGR_CLK_EN                                        0x00000001
#define PRCM_I2CCLKGR_CLK_EN_MASK                                   0x00000001
#define PRCM_I2CCLKGR_CLK_EN_SHIFT                                           0

/******************************************************************************
 *
 * Register: PRCM_I2CCLKGS
 *
 ******************************************************************************
 * Field:     [0] CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by I2CCLKGR.AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_I2CCLKGS_CLK_EN                                        0x00000001
#define PRCM_I2CCLKGS_CLK_EN_MASK                                   0x00000001
#define PRCM_I2CCLKGS_CLK_EN_SHIFT                                           0

/******************************************************************************
 *
 * Register: PRCM_I2CCLKGDS
 *
 ******************************************************************************
 * Field:     [0] CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by I2CCLKGR.AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_I2CCLKGDS_CLK_EN                                       0x00000001
#define PRCM_I2CCLKGDS_CLK_EN_MASK                                  0x00000001
#define PRCM_I2CCLKGDS_CLK_EN_SHIFT                                          0

/******************************************************************************
 *
 * Register: PRCM_UARTCLKGR
 *
 ******************************************************************************
 * Field:   [9:8] AM_CLK_EN
 *
 *
 * 0: No force
 * 1: Force clock on for all modes (Run, Sleep and Deep Sleep)
 *
 * Overrides CLK_EN,  UARTCLKGS.CLK_EN and  UARTCLKGDS.CLK_EN when enabled.
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 * ENUMs:
 * AM_UART1                 Enable clock for UART1
 * AM_UART0                 Enable clock for UART0
 */

#define PRCM_UARTCLKGR_AM_CLK_EN_MASK                               0x00000300
#define PRCM_UARTCLKGR_AM_CLK_EN_SHIFT                                       8
#define PRCM_UARTCLKGR_AM_CLK_EN_AM_UART1                           0x00000200
#define PRCM_UARTCLKGR_AM_CLK_EN_AM_UART0                           0x00000100

/* Field:   [1:0] CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 * ENUMs:
 * UART1                    Enable clock for UART1
 * UART0                    Enable clock for UART0
 */

#define PRCM_UARTCLKGR_CLK_EN_MASK                                  0x00000003
#define PRCM_UARTCLKGR_CLK_EN_SHIFT                                          0
#define PRCM_UARTCLKGR_CLK_EN_UART1                                 0x00000002
#define PRCM_UARTCLKGR_CLK_EN_UART0                                 0x00000001

/******************************************************************************
 *
 * Register: PRCM_UARTCLKGS
 *
 ******************************************************************************
 * Field:   [1:0] CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by UARTCLKGR.AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 * ENUMs:
 * AM_UART1                 Enable clock for UART1
 * AM_UART0                 Enable clock for UART0
 */

#define PRCM_UARTCLKGS_CLK_EN_MASK                                  0x00000003
#define PRCM_UARTCLKGS_CLK_EN_SHIFT                                          0
#define PRCM_UARTCLKGS_CLK_EN_AM_UART1                              0x00000002
#define PRCM_UARTCLKGS_CLK_EN_AM_UART0                              0x00000001

/******************************************************************************
 *
 * Register: PRCM_UARTCLKGDS
 *
 ******************************************************************************
 * Field:   [1:0] CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by UARTCLKGR.AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 * ENUMs:
 * AM_UART1                 Enable clock for UART1
 * AM_UART0                 Enable clock for UART0
 */

#define PRCM_UARTCLKGDS_CLK_EN_MASK                                 0x00000003
#define PRCM_UARTCLKGDS_CLK_EN_SHIFT                                         0
#define PRCM_UARTCLKGDS_CLK_EN_AM_UART1                             0x00000002
#define PRCM_UARTCLKGDS_CLK_EN_AM_UART0                             0x00000001

/******************************************************************************
 *
 * Register: PRCM_SSICLKGR
 *
 ******************************************************************************
 * Field:   [9:8] AM_CLK_EN
 *
 *
 * 0: No force
 * 1: Force clock on for all modes (Run, Sleep and Deep Sleep)
 *
 * Overrides CLK_EN,  SSICLKGS.CLK_EN and  SSICLKGDS.CLK_EN when enabled.
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 * ENUMs:
 * SSI1                     Enable clock for SSI1
 * SSI0                     Enable clock for SSI0
 */

#define PRCM_SSICLKGR_AM_CLK_EN_MASK                                0x00000300
#define PRCM_SSICLKGR_AM_CLK_EN_SHIFT                                        8
#define PRCM_SSICLKGR_AM_CLK_EN_SSI1                                0x00000200
#define PRCM_SSICLKGR_AM_CLK_EN_SSI0                                0x00000100

/* Field:   [1:0] CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 * ENUMs:
 * SSI1                     Enable clock for SSI1
 * SSI0                     Enable clock for SSI0
 */

#define PRCM_SSICLKGR_CLK_EN_MASK                                   0x00000003
#define PRCM_SSICLKGR_CLK_EN_SHIFT                                           0
#define PRCM_SSICLKGR_CLK_EN_SSI1                                   0x00000002
#define PRCM_SSICLKGR_CLK_EN_SSI0                                   0x00000001

/******************************************************************************
 *
 * Register: PRCM_SSICLKGS
 *
 ******************************************************************************
 * Field:   [1:0] CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by SSICLKGR.AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 * ENUMs:
 * SSI1                     Enable clock for SSI1
 * SSI0                     Enable clock for SSI0
 */

#define PRCM_SSICLKGS_CLK_EN_MASK                                   0x00000003
#define PRCM_SSICLKGS_CLK_EN_SHIFT                                           0
#define PRCM_SSICLKGS_CLK_EN_SSI1                                   0x00000002
#define PRCM_SSICLKGS_CLK_EN_SSI0                                   0x00000001

/******************************************************************************
 *
 * Register: PRCM_SSICLKGDS
 *
 ******************************************************************************
 * Field:   [1:0] CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by SSICLKGR.AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 * ENUMs:
 * SSI1                     Enable clock for SSI1
 * SSI0                     Enable clock for SSI0
 */

#define PRCM_SSICLKGDS_CLK_EN_MASK                                  0x00000003
#define PRCM_SSICLKGDS_CLK_EN_SHIFT                                          0
#define PRCM_SSICLKGDS_CLK_EN_SSI1                                  0x00000002
#define PRCM_SSICLKGDS_CLK_EN_SSI0                                  0x00000001

/******************************************************************************
 *
 * Register: PRCM_I2SCLKGR
 *
 ******************************************************************************
 * Field:     [8] AM_CLK_EN
 *
 *
 * 0: No force
 * 1: Force clock on for all modes (Run, Sleep and Deep Sleep)
 *
 * Overrides CLK_EN,  I2SCLKGS.CLK_EN and  I2SCLKGDS.CLK_EN when enabled.
 * SYSBUS clock will always run when enabled
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_I2SCLKGR_AM_CLK_EN                                     0x00000100
#define PRCM_I2SCLKGR_AM_CLK_EN_MASK                                0x00000100
#define PRCM_I2SCLKGR_AM_CLK_EN_SHIFT                                        8

/* Field:     [0] CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_I2SCLKGR_CLK_EN                                        0x00000001
#define PRCM_I2SCLKGR_CLK_EN_MASK                                   0x00000001
#define PRCM_I2SCLKGR_CLK_EN_SHIFT                                           0

/******************************************************************************
 *
 * Register: PRCM_I2SCLKGS
 *
 ******************************************************************************
 * Field:     [0] CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * Can be forced on by I2SCLKGR.AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_I2SCLKGS_CLK_EN                                        0x00000001
#define PRCM_I2SCLKGS_CLK_EN_MASK                                   0x00000001
#define PRCM_I2SCLKGS_CLK_EN_SHIFT                                           0

/******************************************************************************
 *
 * Register: PRCM_I2SCLKGDS
 *
 ******************************************************************************
 * Field:     [0] CLK_EN
 *
 *
 * 0: Disable clock
 * 1: Enable clock
 *
 * SYSBUS clock will always run when enabled
 *
 * Can be forced on by I2SCLKGR.AM_CLK_EN
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_I2SCLKGDS_CLK_EN                                       0x00000001
#define PRCM_I2SCLKGDS_CLK_EN_MASK                                  0x00000001
#define PRCM_I2SCLKGDS_CLK_EN_SHIFT                                          0

/******************************************************************************
 *
 * Register: PRCM_SYSBUSCLKDIV
 *
 ******************************************************************************
 * Field:   [2:0] RATIO
 *
 * Internal. Only to be used through TI provided API.
 * ENUMs:
 * DIV2                     Internal. Only to be used through TI provided API.
 * DIV1                     Internal. Only to be used through TI provided API.
 */

#define PRCM_SYSBUSCLKDIV_RATIO_MASK                                0x00000007
#define PRCM_SYSBUSCLKDIV_RATIO_SHIFT                                        0
#define PRCM_SYSBUSCLKDIV_RATIO_DIV2                                0x00000001
#define PRCM_SYSBUSCLKDIV_RATIO_DIV1                                0x00000000

/******************************************************************************
 *
 * Register: PRCM_CPUCLKDIV
 *
 ******************************************************************************
 * Field:     [0] RATIO
 *
 * Internal. Only to be used through TI provided API.
 * ENUMs:
 * DIV2                     Internal. Only to be used through TI provided API.
 * DIV1                     Internal. Only to be used through TI provided API.
 */

#define PRCM_CPUCLKDIV_RATIO                                        0x00000001
#define PRCM_CPUCLKDIV_RATIO_MASK                                   0x00000001
#define PRCM_CPUCLKDIV_RATIO_SHIFT                                           0
#define PRCM_CPUCLKDIV_RATIO_DIV2                                   0x00000001
#define PRCM_CPUCLKDIV_RATIO_DIV1                                   0x00000000

/******************************************************************************
 *
 * Register: PRCM_PERBUSCPUCLKDIV
 *
 ******************************************************************************
 * Field:   [3:0] RATIO
 *
 * Internal. Only to be used through TI provided API.
 * ENUMs:
 * DIV256                   Internal. Only to be used through TI provided API.
 * DIV128                   Internal. Only to be used through TI provided API.
 * DIV64                    Internal. Only to be used through TI provided API.
 * DIV32                    Internal. Only to be used through TI provided API.
 * DIV16                    Internal. Only to be used through TI provided API.
 * DIV8                     Internal. Only to be used through TI provided API.
 * DIV4                     Internal. Only to be used through TI provided API.
 * DIV2                     Internal. Only to be used through TI provided API.
 * DIV1                     Internal. Only to be used through TI provided API.
 */

#define PRCM_PERBUSCPUCLKDIV_RATIO_MASK                             0x0000000f
#define PRCM_PERBUSCPUCLKDIV_RATIO_SHIFT                                     0
#define PRCM_PERBUSCPUCLKDIV_RATIO_DIV256                           0x00000008
#define PRCM_PERBUSCPUCLKDIV_RATIO_DIV128                           0x00000007
#define PRCM_PERBUSCPUCLKDIV_RATIO_DIV64                            0x00000006
#define PRCM_PERBUSCPUCLKDIV_RATIO_DIV32                            0x00000005
#define PRCM_PERBUSCPUCLKDIV_RATIO_DIV16                            0x00000004
#define PRCM_PERBUSCPUCLKDIV_RATIO_DIV8                             0x00000003
#define PRCM_PERBUSCPUCLKDIV_RATIO_DIV4                             0x00000002
#define PRCM_PERBUSCPUCLKDIV_RATIO_DIV2                             0x00000001
#define PRCM_PERBUSCPUCLKDIV_RATIO_DIV1                             0x00000000

/******************************************************************************
 *
 * Register: PRCM_PERDMACLKDIV
 *
 ******************************************************************************
 * Field:   [3:0] RATIO
 *
 * Internal. Only to be used through TI provided API.
 * ENUMs:
 * DIV256                   Internal. Only to be used through TI provided API.
 * DIV128                   Internal. Only to be used through TI provided API.
 * DIV64                    Internal. Only to be used through TI provided API.
 * DIV32                    Internal. Only to be used through TI provided API.
 * DIV16                    Internal. Only to be used through TI provided API.
 * DIV8                     Internal. Only to be used through TI provided API.
 * DIV4                     Internal. Only to be used through TI provided API.
 * DIV2                     Internal. Only to be used through TI provided API.
 * DIV1                     Internal. Only to be used through TI provided API.
 */

#define PRCM_PERDMACLKDIV_RATIO_MASK                                0x0000000f
#define PRCM_PERDMACLKDIV_RATIO_SHIFT                                        0
#define PRCM_PERDMACLKDIV_RATIO_DIV256                              0x00000008
#define PRCM_PERDMACLKDIV_RATIO_DIV128                              0x00000007
#define PRCM_PERDMACLKDIV_RATIO_DIV64                               0x00000006
#define PRCM_PERDMACLKDIV_RATIO_DIV32                               0x00000005
#define PRCM_PERDMACLKDIV_RATIO_DIV16                               0x00000004
#define PRCM_PERDMACLKDIV_RATIO_DIV8                                0x00000003
#define PRCM_PERDMACLKDIV_RATIO_DIV4                                0x00000002
#define PRCM_PERDMACLKDIV_RATIO_DIV2                                0x00000001
#define PRCM_PERDMACLKDIV_RATIO_DIV1                                0x00000000

/******************************************************************************
 *
 * Register: PRCM_I2SBCLKSEL
 *
 ******************************************************************************
 * Field:     [0] SRC
 *
 * BCLK source selector
 *
 * 0: Use external BCLK
 * 1: Use internally generated clock
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_I2SBCLKSEL_SRC                                         0x00000001
#define PRCM_I2SBCLKSEL_SRC_MASK                                    0x00000001
#define PRCM_I2SBCLKSEL_SRC_SHIFT                                            0

/******************************************************************************
 *
 * Register: PRCM_GPTCLKDIV
 *
 ******************************************************************************
 * Field:   [3:0] RATIO
 *
 * Scalar used for GPTs. The division rate will be constant and ungated for Run
 * / Sleep / DeepSleep mode.   For changes to take effect, CLKLOADCTL.LOAD
 * needs to be written Other values are not supported.
 * ENUMs:
 * DIV256                   Divide by 256
 * DIV128                   Divide by 128
 * DIV64                    Divide by 64
 * DIV32                    Divide by 32
 * DIV16                    Divide by 16
 * DIV8                     Divide by 8
 * DIV4                     Divide by 4
 * DIV2                     Divide by 2
 * DIV1                     Divide by 1
 */

#define PRCM_GPTCLKDIV_RATIO_MASK                                   0x0000000f
#define PRCM_GPTCLKDIV_RATIO_SHIFT                                           0
#define PRCM_GPTCLKDIV_RATIO_DIV256                                 0x00000008
#define PRCM_GPTCLKDIV_RATIO_DIV128                                 0x00000007
#define PRCM_GPTCLKDIV_RATIO_DIV64                                  0x00000006
#define PRCM_GPTCLKDIV_RATIO_DIV32                                  0x00000005
#define PRCM_GPTCLKDIV_RATIO_DIV16                                  0x00000004
#define PRCM_GPTCLKDIV_RATIO_DIV8                                   0x00000003
#define PRCM_GPTCLKDIV_RATIO_DIV4                                   0x00000002
#define PRCM_GPTCLKDIV_RATIO_DIV2                                   0x00000001
#define PRCM_GPTCLKDIV_RATIO_DIV1                                   0x00000000

/******************************************************************************
 *
 * Register: PRCM_I2SCLKCTL
 *
 ******************************************************************************
 * Field:     [3] SMPL_ON_POSEDGE
 *
 * On the I2S serial interface, data and WCLK is sampled and clocked out on
 * opposite edges of BCLK.
 *
 * 0 - data and WCLK are sampled on the negative edge and clocked out on the
 * positive edge.
 * 1 - data and WCLK are sampled on the positive edge and clocked out on the
 * negative edge.
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_I2SCLKCTL_SMPL_ON_POSEDGE                              0x00000008
#define PRCM_I2SCLKCTL_SMPL_ON_POSEDGE_MASK                         0x00000008
#define PRCM_I2SCLKCTL_SMPL_ON_POSEDGE_SHIFT                                 3

/* Field:   [2:1] WCLK_PHASE
 *
 * Decides how the WCLK division ratio is calculated and used to generate
 * different duty cycles (See I2SWCLKDIV.WDIV).
 *
 * 0: Single phase
 * 1: Dual phase
 * 2: User Defined
 * 3: Reserved/Undefined
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_I2SCLKCTL_WCLK_PHASE_MASK                              0x00000006
#define PRCM_I2SCLKCTL_WCLK_PHASE_SHIFT                                      1

/* Field:     [0] EN
 *
 *
 * 0: MCLK, BCLK and WCLK will be static low
 * 1: Enables the generation of  MCLK, BCLK and WCLK
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_I2SCLKCTL_EN                                           0x00000001
#define PRCM_I2SCLKCTL_EN_MASK                                      0x00000001
#define PRCM_I2SCLKCTL_EN_SHIFT                                              0

/******************************************************************************
 *
 * Register: PRCM_I2SMCLKDIV
 *
 ******************************************************************************
 * Field:   [9:0] MDIV
 *
 * An unsigned factor of the division ratio used to generate MCLK [2-1024]:
 *
 * MCLK = MCUCLK/MDIV[Hz]
 * MCUCLK is 48MHz.
 *
 * A value of 0 is interpreted as 1024.
 * A value of 1 is invalid.
 * If MDIV is odd the low phase of the clock is one MCUCLK period longer than
 * the high phase.
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_I2SMCLKDIV_MDIV_MASK                                   0x000003ff
#define PRCM_I2SMCLKDIV_MDIV_SHIFT                                           0

/******************************************************************************
 *
 * Register: PRCM_I2SBCLKDIV
 *
 ******************************************************************************
 * Field:   [9:0] BDIV
 *
 * An unsigned factor of the division ratio used to generate I2S BCLK [2-1024]:
 *
 * BCLK = MCUCLK/BDIV[Hz]
 * MCUCLK is 48MHz.
 *
 * A value of 0 is interpreted as 1024.
 * A value of 1 is invalid.
 * If BDIV is odd and I2SCLKCTL.SMPL_ON_POSEDGE = 0, the low phase of the clock
 * is one MCUCLK period longer than the high phase.
 * If BDIV is odd and I2SCLKCTL.SMPL_ON_POSEDGE = 1 , the high phase of the
 * clock is one MCUCLK period longer than the low phase.
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_I2SBCLKDIV_BDIV_MASK                                   0x000003ff
#define PRCM_I2SBCLKDIV_BDIV_SHIFT                                           0

/******************************************************************************
 *
 * Register: PRCM_I2SWCLKDIV
 *
 ******************************************************************************
 * Field:  [15:0] WDIV
 *
 * If I2SCLKCTL.WCLK_PHASE = 0, Single phase.
 * WCLK is high one BCLK period and low WDIV[9:0] (unsigned, [1-1023]) BCLK
 * periods.
 *
 * WCLK = MCUCLK / BDIV*(WDIV[9:0] + 1) [Hz]
 * MCUCLK is 48MHz.
 *
 * If I2SCLKCTL.WCLK_PHASE = 1, Dual phase.
 * Each phase on WCLK (50% duty cycle) is WDIV[9:0] (unsigned, [1-1023]) BCLK
 * periods.
 *
 * WCLK = MCUCLK / BDIV*(2*WDIV[9:0]) [Hz]
 *
 * If I2SCLKCTL.WCLK_PHASE = 2, User defined.
 * WCLK is high WDIV[7:0] (unsigned, [1-255]) BCLK periods and low WDIV[15:8]
 * (unsigned, [1-255]) BCLK periods.
 *
 * WCLK = MCUCLK / (BDIV*(WDIV[7:0] + WDIV[15:8]) [Hz]
 *
 * For changes to take effect, CLKLOADCTL.LOAD needs to be written
 */

#define PRCM_I2SWCLKDIV_WDIV_MASK                                   0x0000ffff
#define PRCM_I2SWCLKDIV_WDIV_SHIFT                                           0

/******************************************************************************
 *
 * Register: PRCM_RESETSECDMA
 *
 ******************************************************************************
 * Field:     [8] DMA
 *
 * Write 1 to reset. HW cleared.
 * Acess will only have effect when PERIPH power domain is on,
 * PDSTAT0.PERIPH_ON = 1
 * Before writing set FLASH:CFG.DIS_READACCESS = 1 to ensure the reset is not
 * activated while executing from flash. This means one cannot execute from
 * flash when using the SW reset.
 */

#define PRCM_RESETSECDMA_DMA                                        0x00000100
#define PRCM_RESETSECDMA_DMA_MASK                                   0x00000100
#define PRCM_RESETSECDMA_DMA_SHIFT                                           8

/* Field:     [2] PKA
 *
 * Write 1 to reset. HW cleared.
 * Acess will only have effect when PERIPH power domain is on,
 * PDSTAT0.PERIPH_ON = 1
 * Before writing set FLASH:CFG.DIS_READACCESS = 1 to ensure the reset is not
 * activated while executing from flash. This means one cannot execute from
 * flash when using the SW reset.
 */

#define PRCM_RESETSECDMA_PKA                                        0x00000004
#define PRCM_RESETSECDMA_PKA_MASK                                   0x00000004
#define PRCM_RESETSECDMA_PKA_SHIFT                                           2

/* Field:     [1] TRNG
 *
 * Write 1 to reset. HW cleared.
 * Acess will only have effect when PERIPH power domain is on,
 * PDSTAT0.PERIPH_ON = 1
 * Before writing set FLASH:CFG.DIS_READACCESS = 1 to ensure the reset is not
 * activated while executing from flash. This means one cannot execute from
 * flash when using the SW reset.
 */

#define PRCM_RESETSECDMA_TRNG                                       0x00000002
#define PRCM_RESETSECDMA_TRNG_MASK                                  0x00000002
#define PRCM_RESETSECDMA_TRNG_SHIFT                                          1

/* Field:     [0] CRYPTO
 *
 * Write 1 to reset. HW cleared.
 * Acess will only have effect when PERIPH power domain is on,
 * PDSTAT0.PERIPH_ON = 1
 * Before writing set FLASH:CFG.DIS_READACCESS = 1 to ensure the reset is not
 * activated while executing from flash. This means one cannot execute from
 * flash when using the SW reset.
 */

#define PRCM_RESETSECDMA_CRYPTO                                     0x00000001
#define PRCM_RESETSECDMA_CRYPTO_MASK                                0x00000001
#define PRCM_RESETSECDMA_CRYPTO_SHIFT                                        0

/******************************************************************************
 *
 * Register: PRCM_RESETGPIO
 *
 ******************************************************************************
 * Field:     [0] GPIO
 *
 *
 * 0: No action
 * 1: Reset GPIO. HW cleared.
 *
 * Acess will only have effect when PERIPH power domain is on,
 * PDSTAT0.PERIPH_ON = 1
 * Before writing set FLASH:CFG.DIS_READACCESS = 1 to ensure the reset is not
 * activated while executing from flash. This means one cannot execute from
 * flash when using the SW reset.
 */

#define PRCM_RESETGPIO_GPIO                                         0x00000001
#define PRCM_RESETGPIO_GPIO_MASK                                    0x00000001
#define PRCM_RESETGPIO_GPIO_SHIFT                                            0

/******************************************************************************
 *
 * Register: PRCM_RESETGPT
 *
 ******************************************************************************
 * Field:     [0] GPT
 *
 *
 * 0: No action
 * 1: Reset all GPTs. HW cleared.
 *
 * Acess will only have effect when PERIPH power domain is on,
 * PDSTAT0.PERIPH_ON = 1
 * Before writing set FLASH:CFG.DIS_READACCESS = 1 to ensure the reset is not
 * activated while executing from flash. This means one cannot execute from
 * flash when using the SW reset.
 */

#define PRCM_RESETGPT_GPT                                           0x00000001
#define PRCM_RESETGPT_GPT_MASK                                      0x00000001
#define PRCM_RESETGPT_GPT_SHIFT                                              0

/******************************************************************************
 *
 * Register: PRCM_RESETI2C
 *
 ******************************************************************************
 * Field:     [0] I2C
 *
 *
 * 0: No action
 * 1: Reset I2C. HW cleared.
 *
 * Acess will only have effect when SERIAL power domain is on,
 * PDSTAT0.SERIAL_ON = 1
 * Before writing set FLASH:CFG.DIS_READACCESS = 1 to ensure the reset is not
 * activated while executing from flash. This means one cannot execute from
 * flash when using the SW reset.
 */

#define PRCM_RESETI2C_I2C                                           0x00000001
#define PRCM_RESETI2C_I2C_MASK                                      0x00000001
#define PRCM_RESETI2C_I2C_SHIFT                                              0

/******************************************************************************
 *
 * Register: PRCM_RESETUART
 *
 ******************************************************************************
 * Field:     [1] UART1
 *
 *
 * 0: No action
 * 1: Reset UART1. HW cleared.
 *
 * Acess will only have effect when PERIPH power domain is on,
 * PDSTAT0.PERIPH_ON = 1
 * Before writing set FLASH:CFG.DIS_READACCESS = 1 to ensure the reset is not
 * activated while executing from flash. This means one cannot execute from
 * flash when using the SW reset.
 */

#define PRCM_RESETUART_UART1                                        0x00000002
#define PRCM_RESETUART_UART1_MASK                                   0x00000002
#define PRCM_RESETUART_UART1_SHIFT                                           1

/* Field:     [0] UART0
 *
 *
 * 0: No action
 * 1: Reset UART0. HW cleared.
 *
 * Acess will only have effect when SERIAL power domain is on,
 * PDSTAT0.SERIAL_ON = 1
 * Before writing set FLASH:CFG.DIS_READACCESS = 1 to ensure the reset is not
 * activated while executing from flash. This means one cannot execute from
 * flash when using the SW reset.
 */

#define PRCM_RESETUART_UART0                                        0x00000001
#define PRCM_RESETUART_UART0_MASK                                   0x00000001
#define PRCM_RESETUART_UART0_SHIFT                                           0

/******************************************************************************
 *
 * Register: PRCM_RESETSSI
 *
 ******************************************************************************
 * Field:   [1:0] SSI
 *
 * SSI 0:
 *
 * 0: No action
 * 1: Reset SSI. HW cleared.
 *
 * Acess will only have effect when SERIAL power domain is on,
 * PDSTAT0.SERIAL_ON = 1
 * Before writing set FLASH:CFG.DIS_READACCESS = 1 to ensure the reset is not
 * activated while executing from flash. This means one cannot execute from
 * flash when using the SW reset.
 *
 * SSI 1:
 *
 * 0: No action
 * 1: Reset SSI. HW cleared.
 *
 * Acess will only have effect when PERIPH power domain is on,
 * PDSTAT0.PERIPH_ON = 1
 * Before writing set FLASH:CFG.DIS_READACCESS = 1 to ensure the reset is not
 * activated while executing from flash. This means one cannot execute from
 * flash when using the SW reset.
 */

#define PRCM_RESETSSI_SSI_MASK                                      0x00000003
#define PRCM_RESETSSI_SSI_SHIFT                                              0

/******************************************************************************
 *
 * Register: PRCM_RESETI2S
 *
 ******************************************************************************
 * Field:     [0] I2S
 *
 *
 * 0: No action
 * 1: Reset module. HW cleared.
 *
 * Acess will only have effect when PERIPH power domain is on,
 * PDSTAT0.PERIPH_ON = 1
 * Before writing set FLASH:CFG.DIS_READACCESS = 1 to ensure the reset is not
 * activated while executing from flash. This means one cannot execute from
 * flash when using the SW reset.
 */

#define PRCM_RESETI2S_I2S                                           0x00000001
#define PRCM_RESETI2S_I2S_MASK                                      0x00000001
#define PRCM_RESETI2S_I2S_SHIFT                                              0

/******************************************************************************
 *
 * Register: PRCM_PDCTL0
 *
 ******************************************************************************
 * Field:     [2] PERIPH_ON
 *
 * PERIPH Power domain.
 *
 * 0: PERIPH power domain is powered down
 * 1: PERIPH power domain is powered up
 */

#define PRCM_PDCTL0_PERIPH_ON                                       0x00000004
#define PRCM_PDCTL0_PERIPH_ON_MASK                                  0x00000004
#define PRCM_PDCTL0_PERIPH_ON_SHIFT                                          2

/* Field:     [1] SERIAL_ON
 *
 * SERIAL Power domain.
 *
 * 0: SERIAL power domain is powered down
 * 1: SERIAL power domain is powered up
 */

#define PRCM_PDCTL0_SERIAL_ON                                       0x00000002
#define PRCM_PDCTL0_SERIAL_ON_MASK                                  0x00000002
#define PRCM_PDCTL0_SERIAL_ON_SHIFT                                          1

/* Field:     [0] RFC_ON
 *
 *
 * 0: RFC power domain powered off if also PDCTL1.RFC_ON = 0
 * 1: RFC power domain powered on
 */

#define PRCM_PDCTL0_RFC_ON                                          0x00000001
#define PRCM_PDCTL0_RFC_ON_MASK                                     0x00000001
#define PRCM_PDCTL0_RFC_ON_SHIFT                                             0

/******************************************************************************
 *
 * Register: PRCM_PDCTL0RFC
 *
 ******************************************************************************
 * Field:     [0] ON
 *
 * Alias for PDCTL0.RFC_ON
 */

#define PRCM_PDCTL0RFC_ON                                           0x00000001
#define PRCM_PDCTL0RFC_ON_MASK                                      0x00000001
#define PRCM_PDCTL0RFC_ON_SHIFT                                              0

/******************************************************************************
 *
 * Register: PRCM_PDCTL0SERIAL
 *
 ******************************************************************************
 * Field:     [0] ON
 *
 * Alias for PDCTL0.SERIAL_ON
 */

#define PRCM_PDCTL0SERIAL_ON                                        0x00000001
#define PRCM_PDCTL0SERIAL_ON_MASK                                   0x00000001
#define PRCM_PDCTL0SERIAL_ON_SHIFT                                           0

/******************************************************************************
 *
 * Register: PRCM_PDCTL0PERIPH
 *
 ******************************************************************************
 * Field:     [0] ON
 *
 * Alias for PDCTL0.PERIPH_ON
 */

#define PRCM_PDCTL0PERIPH_ON                                        0x00000001
#define PRCM_PDCTL0PERIPH_ON_MASK                                   0x00000001
#define PRCM_PDCTL0PERIPH_ON_SHIFT                                           0

/******************************************************************************
 *
 * Register: PRCM_PDSTAT0
 *
 ******************************************************************************
 * Field:     [2] PERIPH_ON
 *
 * PERIPH Power domain.
 *
 * 0: Domain may be powered down
 * 1: Domain powered up (guaranteed)
 */

#define PRCM_PDSTAT0_PERIPH_ON                                      0x00000004
#define PRCM_PDSTAT0_PERIPH_ON_MASK                                 0x00000004
#define PRCM_PDSTAT0_PERIPH_ON_SHIFT                                         2

/* Field:     [1] SERIAL_ON
 *
 * SERIAL Power domain.
 *
 * 0: Domain may be powered down
 * 1: Domain powered up (guaranteed)
 */

#define PRCM_PDSTAT0_SERIAL_ON                                      0x00000002
#define PRCM_PDSTAT0_SERIAL_ON_MASK                                 0x00000002
#define PRCM_PDSTAT0_SERIAL_ON_SHIFT                                         1

/* Field:     [0] RFC_ON
 *
 * RFC Power domain
 *
 * 0: Domain may be powered down
 * 1: Domain powered up (guaranteed)
 */

#define PRCM_PDSTAT0_RFC_ON                                         0x00000001
#define PRCM_PDSTAT0_RFC_ON_MASK                                    0x00000001
#define PRCM_PDSTAT0_RFC_ON_SHIFT                                            0

/******************************************************************************
 *
 * Register: PRCM_PDSTAT0RFC
 *
 ******************************************************************************
 * Field:     [0] ON
 *
 * Alias for PDSTAT0.RFC_ON
 */

#define PRCM_PDSTAT0RFC_ON                                          0x00000001
#define PRCM_PDSTAT0RFC_ON_MASK                                     0x00000001
#define PRCM_PDSTAT0RFC_ON_SHIFT                                             0

/******************************************************************************
 *
 * Register: PRCM_PDSTAT0SERIAL
 *
 ******************************************************************************
 * Field:     [0] ON
 *
 * Alias for PDSTAT0.SERIAL_ON
 */

#define PRCM_PDSTAT0SERIAL_ON                                       0x00000001
#define PRCM_PDSTAT0SERIAL_ON_MASK                                  0x00000001
#define PRCM_PDSTAT0SERIAL_ON_SHIFT                                          0

/******************************************************************************
 *
 * Register: PRCM_PDSTAT0PERIPH
 *
 ******************************************************************************
 * Field:     [0] ON
 *
 * Alias for PDSTAT0.PERIPH_ON
 */

#define PRCM_PDSTAT0PERIPH_ON                                       0x00000001
#define PRCM_PDSTAT0PERIPH_ON_MASK                                  0x00000001
#define PRCM_PDSTAT0PERIPH_ON_SHIFT                                          0

/******************************************************************************
 *
 * Register: PRCM_PDCTL1
 *
 ******************************************************************************
 * Field:   [4:3] VIMS_MODE
 *
 *
 * 00: VIMS power domain is only powered when CPU power domain is powered.
 * 01: VIMS power domain is powered whenever the BUS power domain is powered.
 * 1X: Block power up of VIMS power domain at next wake up. This mode only has
 * effect when VIMS power domain is not powered. Used for Autonomous RF Core.
 */

#define PRCM_PDCTL1_VIMS_MODE_MASK                                  0x00000018
#define PRCM_PDCTL1_VIMS_MODE_SHIFT                                          3

/* Field:     [2] RFC_ON
 *
 *  0: RFC power domain powered off if also PDCTL0.RFC_ON = 0 1: RFC power
 * domain powered on  Bit shall be used by RFC in autonomous mode but there is
 * no HW restrictions fom system CPU to access the bit.
 */

#define PRCM_PDCTL1_RFC_ON                                          0x00000004
#define PRCM_PDCTL1_RFC_ON_MASK                                     0x00000004
#define PRCM_PDCTL1_RFC_ON_SHIFT                                             2

/* Field:     [1] CPU_ON
 *
 *
 * 0: Causes a power down of the CPU power domain when system CPU indicates it
 * is idle.
 * 1: Initiates power-on of the CPU power domain.
 *
 * This bit is automatically set by a WIC power-on event.
 */

#define PRCM_PDCTL1_CPU_ON                                          0x00000002
#define PRCM_PDCTL1_CPU_ON_MASK                                     0x00000002
#define PRCM_PDCTL1_CPU_ON_SHIFT                                             1

/******************************************************************************
 *
 * Register: PRCM_PDCTL1CPU
 *
 ******************************************************************************
 * Field:     [0] ON
 *
 * This is an alias for PDCTL1.CPU_ON
 */

#define PRCM_PDCTL1CPU_ON                                           0x00000001
#define PRCM_PDCTL1CPU_ON_MASK                                      0x00000001
#define PRCM_PDCTL1CPU_ON_SHIFT                                              0

/******************************************************************************
 *
 * Register: PRCM_PDCTL1RFC
 *
 ******************************************************************************
 * Field:     [0] ON
 *
 * This is an alias for PDCTL1.RFC_ON
 */

#define PRCM_PDCTL1RFC_ON                                           0x00000001
#define PRCM_PDCTL1RFC_ON_MASK                                      0x00000001
#define PRCM_PDCTL1RFC_ON_SHIFT                                              0

/******************************************************************************
 *
 * Register: PRCM_PDCTL1VIMS
 *
 ******************************************************************************
 * Field:   [1:0] MODE
 *
 * This is an alias for PDCTL1.VIMS_MODE
 */

#define PRCM_PDCTL1VIMS_MODE_MASK                                   0x00000003
#define PRCM_PDCTL1VIMS_MODE_SHIFT                                           0

/******************************************************************************
 *
 * Register: PRCM_PDSTAT1
 *
 ******************************************************************************
 * Field:     [4] BUS_ON
 *
 *
 * 0: BUS domain not accessible
 * 1: BUS domain is currently accessible
 */

#define PRCM_PDSTAT1_BUS_ON                                         0x00000010
#define PRCM_PDSTAT1_BUS_ON_MASK                                    0x00000010
#define PRCM_PDSTAT1_BUS_ON_SHIFT                                            4

/* Field:     [3] VIMS_ON
 *
 *
 * 0: VIMS domain not accessible
 * 1: VIMS domain is currently accessible
 */

#define PRCM_PDSTAT1_VIMS_ON                                        0x00000008
#define PRCM_PDSTAT1_VIMS_ON_MASK                                   0x00000008
#define PRCM_PDSTAT1_VIMS_ON_SHIFT                                           3

/* Field:     [2] RFC_ON
 *
 *
 * 0: RFC domain not accessible
 * 1: RFC domain is currently accessible
 */

#define PRCM_PDSTAT1_RFC_ON                                         0x00000004
#define PRCM_PDSTAT1_RFC_ON_MASK                                    0x00000004
#define PRCM_PDSTAT1_RFC_ON_SHIFT                                            2

/* Field:     [1] CPU_ON
 *
 *
 * 0: CPU and BUS domain not accessible
 * 1: CPU and BUS domains are both currently accessible
 */

#define PRCM_PDSTAT1_CPU_ON                                         0x00000002
#define PRCM_PDSTAT1_CPU_ON_MASK                                    0x00000002
#define PRCM_PDSTAT1_CPU_ON_SHIFT                                            1

/******************************************************************************
 *
 * Register: PRCM_PDSTAT1BUS
 *
 ******************************************************************************
 * Field:     [0] ON
 *
 * This is an alias for PDSTAT1.BUS_ON
 */

#define PRCM_PDSTAT1BUS_ON                                          0x00000001
#define PRCM_PDSTAT1BUS_ON_MASK                                     0x00000001
#define PRCM_PDSTAT1BUS_ON_SHIFT                                             0

/******************************************************************************
 *
 * Register: PRCM_PDSTAT1RFC
 *
 ******************************************************************************
 * Field:     [0] ON
 *
 * This is an alias for PDSTAT1.RFC_ON
 */

#define PRCM_PDSTAT1RFC_ON                                          0x00000001
#define PRCM_PDSTAT1RFC_ON_MASK                                     0x00000001
#define PRCM_PDSTAT1RFC_ON_SHIFT                                             0

/******************************************************************************
 *
 * Register: PRCM_PDSTAT1CPU
 *
 ******************************************************************************
 * Field:     [0] ON
 *
 * This is an alias for PDSTAT1.CPU_ON
 */

#define PRCM_PDSTAT1CPU_ON                                          0x00000001
#define PRCM_PDSTAT1CPU_ON_MASK                                     0x00000001
#define PRCM_PDSTAT1CPU_ON_SHIFT                                             0

/******************************************************************************
 *
 * Register: PRCM_PDSTAT1VIMS
 *
 ******************************************************************************
 * Field:     [0] ON
 *
 * This is an alias for PDSTAT1.VIMS_ON
 */

#define PRCM_PDSTAT1VIMS_ON                                         0x00000001
#define PRCM_PDSTAT1VIMS_ON_MASK                                    0x00000001
#define PRCM_PDSTAT1VIMS_ON_SHIFT                                            0

/******************************************************************************
 *
 * Register: PRCM_RFCBITS
 *
 ******************************************************************************
 * Field:  [31:0] READ
 *
 * Control bits for RFC. The RF core CPE processor will automatically check
 * this register when it boots, and it can be used to immediately instruct CPE
 * to perform some tasks at its start-up. The supported functionality is
 * ROM-defined and may vary. See the technical reference manual for more
 * details.
 */

#define PRCM_RFCBITS_READ_MASK                                      0xffffffff
#define PRCM_RFCBITS_READ_SHIFT                                              0

/******************************************************************************
 *
 * Register: PRCM_RFCMODESEL
 *
 ******************************************************************************
 * Field:   [2:0] CURR
 *
 * Selects the set of commands that the RFC will accept. Only modes permitted
 * by RFCMODEHWOPT.AVAIL are writeable. See the technical reference manual for
 * details.
 * ENUMs:
 * MODE7                    Select Mode 7
 * MODE6                    Select Mode 6
 * MODE5                    Select Mode 5
 * MODE4                    Select Mode 4
 * MODE3                    Select Mode 3
 * MODE2                    Select Mode 2
 * MODE1                    Select Mode 1
 * MODE0                    Select Mode 0
 */

#define PRCM_RFCMODESEL_CURR_MASK                                   0x00000007
#define PRCM_RFCMODESEL_CURR_SHIFT                                           0
#define PRCM_RFCMODESEL_CURR_MODE7                                  0x00000007
#define PRCM_RFCMODESEL_CURR_MODE6                                  0x00000006
#define PRCM_RFCMODESEL_CURR_MODE5                                  0x00000005
#define PRCM_RFCMODESEL_CURR_MODE4                                  0x00000004
#define PRCM_RFCMODESEL_CURR_MODE3                                  0x00000003
#define PRCM_RFCMODESEL_CURR_MODE2                                  0x00000002
#define PRCM_RFCMODESEL_CURR_MODE1                                  0x00000001
#define PRCM_RFCMODESEL_CURR_MODE0                                  0x00000000

/******************************************************************************
 *
 * Register: PRCM_RFCMODEHWOPT
 *
 ******************************************************************************
 * Field:   [7:0] AVAIL
 *
 * Permitted RFC modes. More than one mode can be permitted.
 * ENUMs:
 * MODE7                    Mode 7 permitted
 * MODE6                    Mode 6 permitted
 * MODE5                    Mode 5 permitted
 * MODE4                    Mode 4 permitted
 * MODE3                    Mode 3 permitted
 * MODE2                    Mode 2 permitted
 * MODE1                    Mode 1 permitted
 * MODE0                    Mode 0 permitted
 */

#define PRCM_RFCMODEHWOPT_AVAIL_MASK                                0x000000ff
#define PRCM_RFCMODEHWOPT_AVAIL_SHIFT                                        0
#define PRCM_RFCMODEHWOPT_AVAIL_MODE7                               0x00000080
#define PRCM_RFCMODEHWOPT_AVAIL_MODE6                               0x00000040
#define PRCM_RFCMODEHWOPT_AVAIL_MODE5                               0x00000020
#define PRCM_RFCMODEHWOPT_AVAIL_MODE4                               0x00000010
#define PRCM_RFCMODEHWOPT_AVAIL_MODE3                               0x00000008
#define PRCM_RFCMODEHWOPT_AVAIL_MODE2                               0x00000004
#define PRCM_RFCMODEHWOPT_AVAIL_MODE1                               0x00000002
#define PRCM_RFCMODEHWOPT_AVAIL_MODE0                               0x00000001

/******************************************************************************
 *
 * Register: PRCM_PWRPROFSTAT
 *
 ******************************************************************************
 * Field:   [7:0] VALUE
 *
 * SW can use these bits to timestamp the application. These bits are also
 * available through the testtap and can thus be used by the emulator to
 * profile in real time.
 */

#define PRCM_PWRPROFSTAT_VALUE_MASK                                 0x000000ff
#define PRCM_PWRPROFSTAT_VALUE_SHIFT                                         0

/******************************************************************************
 *
 * Register: PRCM_MCUSRAMCFG
 *
 ******************************************************************************
 * Field:     [5] BM_OFF
 *
 * Burst Mode disable
 *
 * 0: Burst Mode enabled.
 * 1: Burst Mode off.
 */

#define PRCM_MCUSRAMCFG_BM_OFF                                      0x00000020
#define PRCM_MCUSRAMCFG_BM_OFF_MASK                                 0x00000020
#define PRCM_MCUSRAMCFG_BM_OFF_SHIFT                                         5

/* Field:     [4] PAGE
 *
 * Page Mode select
 *
 * 0: Page Mode disabled. Memory works in standard mode
 * 1: Page Mode enabled. Only one  half of butterfly array selected. Page Mode
 * will select either LSB half or MSB half of the word based on PGS setting.
 *
 * This mode can be used for additional power saving
 */

#define PRCM_MCUSRAMCFG_PAGE                                        0x00000010
#define PRCM_MCUSRAMCFG_PAGE_MASK                                   0x00000010
#define PRCM_MCUSRAMCFG_PAGE_SHIFT                                           4

/* Field:     [3] PGS
 *
 * 0: Select LSB half of word during Page Mode, PAGE = 1
 * 1: Select MSB half of word during Page Mode, PAGE = 1
 */

#define PRCM_MCUSRAMCFG_PGS                                         0x00000008
#define PRCM_MCUSRAMCFG_PGS_MASK                                    0x00000008
#define PRCM_MCUSRAMCFG_PGS_SHIFT                                            3

/* Field:     [2] BM
 *
 * Burst Mode Enable
 *
 * 0: Burst Mode Disable. Memory works in standard mode.
 * 1: Burst Mode Enable
 *
 * When in Burst Mode bitline precharge and wordline firing depends on PCH_F
 * and PCH_L.
 * Burst Mode results in reduction in active power.
 */

#define PRCM_MCUSRAMCFG_BM                                          0x00000004
#define PRCM_MCUSRAMCFG_BM_MASK                                     0x00000004
#define PRCM_MCUSRAMCFG_BM_SHIFT                                             2

/* Field:     [1] PCH_F
 *
 * 0: No bitline precharge in second half of cycle
 * 1: Bitline precharge in second half of cycle when in Burst Mode, BM = 1
 */

#define PRCM_MCUSRAMCFG_PCH_F                                       0x00000002
#define PRCM_MCUSRAMCFG_PCH_F_MASK                                  0x00000002
#define PRCM_MCUSRAMCFG_PCH_F_SHIFT                                          1

/* Field:     [0] PCH_L
 *
 * 0: No bitline precharge in first half of cycle
 * 1: Bitline precharge in first half of cycle when in Burst Mode, BM = 1
 */

#define PRCM_MCUSRAMCFG_PCH_L                                       0x00000001
#define PRCM_MCUSRAMCFG_PCH_L_MASK                                  0x00000001
#define PRCM_MCUSRAMCFG_PCH_L_SHIFT                                          0

/******************************************************************************
 *
 * Register: PRCM_RAMRETEN
 *
 ******************************************************************************
 * Field:     [3] RFCULL
 *
 * 0: Retention for RFC ULL SRAM disabled
 * 1: Retention for RFC ULL SRAM enabled
 *
 * Memories controlled:
 * CPEULLRAM
 */

#define PRCM_RAMRETEN_RFCULL                                        0x00000008
#define PRCM_RAMRETEN_RFCULL_MASK                                   0x00000008
#define PRCM_RAMRETEN_RFCULL_SHIFT                                           3

/* Field:     [2] RFC
 *
 * 0: Retention for RFC SRAM disabled
 * 1: Retention for RFC SRAM enabled
 *
 * Memories controlled: CPERAM  MCERAM  RFERAM  DSBRAM
 */

#define PRCM_RAMRETEN_RFC                                           0x00000004
#define PRCM_RAMRETEN_RFC_MASK                                      0x00000004
#define PRCM_RAMRETEN_RFC_SHIFT                                              2

/* Field:   [1:0] VIMS
 *
 *
 * 0: Memory retention disabled
 * 1: Memory retention enabled
 *
 * Bit 0: VIMS_TRAM
 * Bit 1: VIMS_CRAM
 *
 * Legal modes depend on settings in VIMS:CTL.MODE
 *
 * 00: VIMS:CTL.MODE must be OFF before DEEPSLEEP is asserted - must be set to
 * CACHE or SPLIT mode after waking up again
 * 01: VIMS:CTL.MODE must be GPRAM before DEEPSLEEP is asserted. Must remain in
 * GPRAM mode after wake up, alternatively select OFF mode first and then CACHE
 * or SPILT mode.
 * 10: Illegal mode
 * 11: No restrictions
 */

#define PRCM_RAMRETEN_VIMS_MASK                                     0x00000003
#define PRCM_RAMRETEN_VIMS_SHIFT                                             0

/******************************************************************************
 *
 * Register: PRCM_OSCIMSC
 *
 ******************************************************************************
 * Field:     [7] HFSRCPENDIM
 *
 * 0: Disable interrupt generation when HFSRCPEND is qualified
 * 1: Enable interrupt generation when HFSRCPEND is qualified
 */

#define PRCM_OSCIMSC_HFSRCPENDIM                                    0x00000080
#define PRCM_OSCIMSC_HFSRCPENDIM_MASK                               0x00000080
#define PRCM_OSCIMSC_HFSRCPENDIM_SHIFT                                       7

/* Field:     [6] LFSRCDONEIM
 *
 * 0: Disable interrupt generation when LFSRCDONE is qualified
 * 1: Enable interrupt generation when LFSRCDONE is qualified
 */

#define PRCM_OSCIMSC_LFSRCDONEIM                                    0x00000040
#define PRCM_OSCIMSC_LFSRCDONEIM_MASK                               0x00000040
#define PRCM_OSCIMSC_LFSRCDONEIM_SHIFT                                       6

/* Field:     [5] XOSCDLFIM
 *
 * 0: Disable interrupt generation when XOSCDLF is qualified
 * 1: Enable interrupt generation when XOSCDLF is qualified
 */

#define PRCM_OSCIMSC_XOSCDLFIM                                      0x00000020
#define PRCM_OSCIMSC_XOSCDLFIM_MASK                                 0x00000020
#define PRCM_OSCIMSC_XOSCDLFIM_SHIFT                                         5

/* Field:     [4] XOSCLFIM
 *
 * 0: Disable interrupt generation when XOSCLF is qualified
 * 1: Enable interrupt generation when XOSCLF is qualified
 */

#define PRCM_OSCIMSC_XOSCLFIM                                       0x00000010
#define PRCM_OSCIMSC_XOSCLFIM_MASK                                  0x00000010
#define PRCM_OSCIMSC_XOSCLFIM_SHIFT                                          4

/* Field:     [3] RCOSCDLFIM
 *
 * 0: Disable interrupt generation when RCOSCDLF is qualified
 * 1: Enable interrupt generation when RCOSCDLF is qualified
 */

#define PRCM_OSCIMSC_RCOSCDLFIM                                     0x00000008
#define PRCM_OSCIMSC_RCOSCDLFIM_MASK                                0x00000008
#define PRCM_OSCIMSC_RCOSCDLFIM_SHIFT                                        3

/* Field:     [2] RCOSCLFIM
 *
 * 0: Disable interrupt generation when RCOSCLF is qualified
 * 1: Enable interrupt generation when RCOSCLF is qualified
 */

#define PRCM_OSCIMSC_RCOSCLFIM                                      0x00000004
#define PRCM_OSCIMSC_RCOSCLFIM_MASK                                 0x00000004
#define PRCM_OSCIMSC_RCOSCLFIM_SHIFT                                         2

/* Field:     [1] XOSCHFIM
 *
 * 0: Disable interrupt generation when XOSCHF is qualified
 * 1: Enable interrupt generation when XOSCHF is qualified
 */

#define PRCM_OSCIMSC_XOSCHFIM                                       0x00000002
#define PRCM_OSCIMSC_XOSCHFIM_MASK                                  0x00000002
#define PRCM_OSCIMSC_XOSCHFIM_SHIFT                                          1

/* Field:     [0] RCOSCHFIM
 *
 * 0: Disable interrupt generation when RCOSCHF is qualified
 * 1: Enable interrupt generation when RCOSCHF is qualified
 */

#define PRCM_OSCIMSC_RCOSCHFIM                                      0x00000001
#define PRCM_OSCIMSC_RCOSCHFIM_MASK                                 0x00000001
#define PRCM_OSCIMSC_RCOSCHFIM_SHIFT                                         0

/******************************************************************************
 *
 * Register: PRCM_OSCRIS
 *
 ******************************************************************************
 * Field:     [7] HFSRCPENDRIS
 *
 * 0: HFSRCPEND has not been qualified
 * 1: HFSRCPEND has been qualified since last clear
 *
 * Interrupt is qualified regardless of OSCIMSC.HFSRCPENDIM setting. The order
 * of qualifying raw interrupt and enable of interrupt mask is indifferent for
 * generating an OSC Interrupt.
 *
 * Set by HW. Cleared by writing to OSCICR.HFSRCPENDC
 */

#define PRCM_OSCRIS_HFSRCPENDRIS                                    0x00000080
#define PRCM_OSCRIS_HFSRCPENDRIS_MASK                               0x00000080
#define PRCM_OSCRIS_HFSRCPENDRIS_SHIFT                                       7

/* Field:     [6] LFSRCDONERIS
 *
 * 0: LFSRCDONE has not been qualified
 * 1: LFSRCDONE has been qualified since last clear
 *
 * Interrupt is qualified regardless of OSCIMSC.LFSRCDONEIM setting. The order
 * of qualifying raw interrupt and enable of interrupt mask is indifferent for
 * generating an OSC Interrupt.
 *
 * Set by HW. Cleared by writing to OSCICR.LFSRCDONEC
 */

#define PRCM_OSCRIS_LFSRCDONERIS                                    0x00000040
#define PRCM_OSCRIS_LFSRCDONERIS_MASK                               0x00000040
#define PRCM_OSCRIS_LFSRCDONERIS_SHIFT                                       6

/* Field:     [5] XOSCDLFRIS
 *
 * 0: XOSCDLF has not been qualified
 * 1: XOSCDLF has been qualified since last clear.
 *
 * Interrupt is qualified regardless of OSCIMSC.XOSCDLFIM setting. The order of
 * qualifying raw interrupt and enable of interrupt mask is indifferent for
 * generating an OSC Interrupt.
 *
 * Set by HW. Cleared by writing to OSCICR.XOSCDLFC
 */

#define PRCM_OSCRIS_XOSCDLFRIS                                      0x00000020
#define PRCM_OSCRIS_XOSCDLFRIS_MASK                                 0x00000020
#define PRCM_OSCRIS_XOSCDLFRIS_SHIFT                                         5

/* Field:     [4] XOSCLFRIS
 *
 * 0: XOSCLF has not been qualified
 * 1: XOSCLF has been qualified since last clear.
 *
 * Interrupt is qualified regardless of OSCIMSC.XOSCLFIM setting. The order of
 * qualifying raw interrupt and enable of interrupt mask is indifferent for
 * generating an OSC Interrupt.
 *
 * Set by HW. Cleared by writing to OSCICR.XOSCLFC
 */

#define PRCM_OSCRIS_XOSCLFRIS                                       0x00000010
#define PRCM_OSCRIS_XOSCLFRIS_MASK                                  0x00000010
#define PRCM_OSCRIS_XOSCLFRIS_SHIFT                                          4

/* Field:     [3] RCOSCDLFRIS
 *
 * 0: RCOSCDLF has not been qualified
 * 1: RCOSCDLF has been qualified since last clear.
 *
 * Interrupt is qualified regardless of OSCIMSC.RCOSCDLFIM setting. The order
 * of qualifying raw interrupt and enable of interrupt mask is indifferent for
 * generating an OSC Interrupt.
 *
 * Set by HW. Cleared by writing to OSCICR.RCOSCDLFC
 */

#define PRCM_OSCRIS_RCOSCDLFRIS                                     0x00000008
#define PRCM_OSCRIS_RCOSCDLFRIS_MASK                                0x00000008
#define PRCM_OSCRIS_RCOSCDLFRIS_SHIFT                                        3

/* Field:     [2] RCOSCLFRIS
 *
 * 0: RCOSCLF has not been qualified
 * 1: RCOSCLF has been qualified since last clear.
 *
 * Interrupt is qualified regardless of OSCIMSC.RCOSCLFIM setting. The order of
 * qualifying raw interrupt and enable of interrupt mask is indifferent for
 * generating an OSC Interrupt.
 *
 * Set by HW. Cleared by writing to OSCICR.RCOSCLFC
 */

#define PRCM_OSCRIS_RCOSCLFRIS                                      0x00000004
#define PRCM_OSCRIS_RCOSCLFRIS_MASK                                 0x00000004
#define PRCM_OSCRIS_RCOSCLFRIS_SHIFT                                         2

/* Field:     [1] XOSCHFRIS
 *
 * 0: XOSCHF has not been qualified
 * 1: XOSCHF has been qualified since last clear.
 *
 * Interrupt is qualified regardless of OSCIMSC.XOSCHFIM setting. The order of
 * qualifying raw interrupt and enable of interrupt mask is indifferent for
 * generating an OSC Interrupt.
 *
 * Set by HW. Cleared by writing to OSCICR.XOSCHFC
 */

#define PRCM_OSCRIS_XOSCHFRIS                                       0x00000002
#define PRCM_OSCRIS_XOSCHFRIS_MASK                                  0x00000002
#define PRCM_OSCRIS_XOSCHFRIS_SHIFT                                          1

/* Field:     [0] RCOSCHFRIS
 *
 * 0: RCOSCHF has not been qualified
 * 1: RCOSCHF has been qualified since last clear.
 *
 * Interrupt is qualified regardless of OSCIMSC.RCOSCHFIM setting. The order of
 * qualifying raw interrupt and enable of interrupt mask is indifferent for
 * generating an OSC Interrupt.
 *
 * Set by HW. Cleared by writing to OSCICR.RCOSCHFC
 */

#define PRCM_OSCRIS_RCOSCHFRIS                                      0x00000001
#define PRCM_OSCRIS_RCOSCHFRIS_MASK                                 0x00000001
#define PRCM_OSCRIS_RCOSCHFRIS_SHIFT                                         0

/******************************************************************************
 *
 * Register: PRCM_OSCICR
 *
 ******************************************************************************
 * Field:     [7] HFSRCPENDC
 *
 * Writing 1 to this field clears the HFSRCPEND raw interrupt status. Writing 0
 * has no effect.
 */

#define PRCM_OSCICR_HFSRCPENDC                                      0x00000080
#define PRCM_OSCICR_HFSRCPENDC_MASK                                 0x00000080
#define PRCM_OSCICR_HFSRCPENDC_SHIFT                                         7

/* Field:     [6] LFSRCDONEC
 *
 * Writing 1 to this field clears the LFSRCDONE raw interrupt status. Writing 0
 * has no effect.
 */

#define PRCM_OSCICR_LFSRCDONEC                                      0x00000040
#define PRCM_OSCICR_LFSRCDONEC_MASK                                 0x00000040
#define PRCM_OSCICR_LFSRCDONEC_SHIFT                                         6

/* Field:     [5] XOSCDLFC
 *
 * Writing 1 to this field clears the XOSCDLF raw interrupt status. Writing 0
 * has no effect.
 */

#define PRCM_OSCICR_XOSCDLFC                                        0x00000020
#define PRCM_OSCICR_XOSCDLFC_MASK                                   0x00000020
#define PRCM_OSCICR_XOSCDLFC_SHIFT                                           5

/* Field:     [4] XOSCLFC
 *
 * Writing 1 to this field clears the XOSCLF raw interrupt status. Writing 0
 * has no effect.
 */

#define PRCM_OSCICR_XOSCLFC                                         0x00000010
#define PRCM_OSCICR_XOSCLFC_MASK                                    0x00000010
#define PRCM_OSCICR_XOSCLFC_SHIFT                                            4

/* Field:     [3] RCOSCDLFC
 *
 * Writing 1 to this field clears the RCOSCDLF raw interrupt status. Writing 0
 * has no effect.
 */

#define PRCM_OSCICR_RCOSCDLFC                                       0x00000008
#define PRCM_OSCICR_RCOSCDLFC_MASK                                  0x00000008
#define PRCM_OSCICR_RCOSCDLFC_SHIFT                                          3

/* Field:     [2] RCOSCLFC
 *
 * Writing 1 to this field clears the RCOSCLF raw interrupt status. Writing 0
 * has no effect.
 */

#define PRCM_OSCICR_RCOSCLFC                                        0x00000004
#define PRCM_OSCICR_RCOSCLFC_MASK                                   0x00000004
#define PRCM_OSCICR_RCOSCLFC_SHIFT                                           2

/* Field:     [1] XOSCHFC
 *
 * Writing 1 to this field clears the XOSCHF raw interrupt status. Writing 0
 * has no effect.
 */

#define PRCM_OSCICR_XOSCHFC                                         0x00000002
#define PRCM_OSCICR_XOSCHFC_MASK                                    0x00000002
#define PRCM_OSCICR_XOSCHFC_SHIFT                                            1

/* Field:     [0] RCOSCHFC
 *
 * Writing 1 to this field clears the RCOSCHF raw interrupt status. Writing 0
 * has no effect.
 */

#define PRCM_OSCICR_RCOSCHFC                                        0x00000001
#define PRCM_OSCICR_RCOSCHFC_MASK                                   0x00000001
#define PRCM_OSCICR_RCOSCHFC_SHIFT                                           0

#endif /* __ARCH_ARM_SRC_TIVA_HARDWARE_CC13X2_CC26X2_V2_PRCM_H */
