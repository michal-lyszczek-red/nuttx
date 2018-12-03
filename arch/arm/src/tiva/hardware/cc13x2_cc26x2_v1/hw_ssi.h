/******************************************************************************
 *  Filename:       hw_ssi_h
 *  Revised:        2017-01-10 11:54:43 +0100 (Tue, 10 Jan 2017)
 *  Revision:       48190
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

#ifndef __ARCH_ARM_SRC_TIVA_HARDWARE_CC13X2_CC26X2_V1_SSI_H
#define __ARCH_ARM_SRC_TIVA_HARDWARE_CC13X2_CC26X2_V1_SSI_H

/******************************************************************************
 * Pre-processor Definitions
 ******************************************************************************/

/******************************************************************************
 *
 * This section defines the register offsets of
 * SSI component
 *
 ******************************************************************************
 * Control 0
 */

#define SSI_CR0_OFFSET                                              0x00000000

/* Control 1 */

#define SSI_CR1_OFFSET                                              0x00000004

/* Data */

#define SSI_DR_OFFSET                                               0x00000008

/* Status */

#define SSI_SR_OFFSET                                               0x0000000c

/* Clock Prescale */

#define SSI_CPSR_OFFSET                                             0x00000010

/* Interrupt Mask Set and Clear */

#define SSI_IMSC_OFFSET                                             0x00000014

/* Raw Interrupt Status */

#define SSI_RIS_OFFSET                                              0x00000018

/* Masked Interrupt Status */

#define SSI_MIS_OFFSET                                              0x0000001c

/* Interrupt Clear */

#define SSI_ICR_OFFSET                                              0x00000020

/* DMA Control */

#define SSI_DMACR_OFFSET                                            0x00000024

/******************************************************************************
 *
 * Register: SSI_CR0
 *
 ******************************************************************************
 * Field:  [15:8] SCR
 *
 * Serial clock rate:
 * This is used to generate the transmit and receive bit rate of the SSI. The
 * bit rate is
 * (SSI's clock frequency)/((SCR+1)*CPSR.CPSDVSR).
 * SCR is a value from 0-255.
 */

#define SSI_CR0_SCR_MASK                                            0x0000ff00
#define SSI_CR0_SCR_SHIFT                                                    8

/* Field:     [7] SPH
 *
 * CLKOUT phase (Motorola SPI frame format only)
 * This bit selects the clock edge that captures data and enables it to change
 * state. It
 * has the most impact on the first bit transmitted by either permitting or not
 * permitting a clock transition before the first data capture edge.
 * ENUMs:
 * 2ND_CLK_EDGE             Data is captured on the second clock edge
 *                          transition.
 * 1ST_CLK_EDGE             Data is captured on the first clock edge
 *                          transition.
 */

#define SSI_CR0_SPH                                                 0x00000080
#define SSI_CR0_SPH_MASK                                            0x00000080
#define SSI_CR0_SPH_SHIFT                                                    7
#define SSI_CR0_SPH_2ND_CLK_EDGE                                    0x00000080
#define SSI_CR0_SPH_1ST_CLK_EDGE                                    0x00000000

/* Field:     [6] SPO
 *
 * CLKOUT polarity (Motorola SPI frame format only)
 * ENUMs:
 * HIGH                     SSI produces a steady state HIGH value on the
 *                          CLKOUT pin when data is not being transferred.
 * LOW                      SSI produces a steady state LOW value on the
 *                          CLKOUT pin when data is
 *                          not being transferred.
 */

#define SSI_CR0_SPO                                                 0x00000040
#define SSI_CR0_SPO_MASK                                            0x00000040
#define SSI_CR0_SPO_SHIFT                                                    6
#define SSI_CR0_SPO_HIGH                                            0x00000040
#define SSI_CR0_SPO_LOW                                             0x00000000

/* Field:   [5:4] FRF
 *
 * Frame format.
 * The supported frame formats are Motorola SPI, TI synchronous serial and
 * National Microwire.
 * Value 0'b11 is reserved and shall not be used.
 * ENUMs:
 * NATIONAL_MICROWIRE       National Microwire frame format
 * TI_SYNC_SERIAL           TI synchronous serial frame format
 * MOTOROLA_SPI             Motorola SPI frame format
 */

#define SSI_CR0_FRF_MASK                                            0x00000030
#define SSI_CR0_FRF_SHIFT                                                    4
#define SSI_CR0_FRF_NATIONAL_MICROWIRE                              0x00000020
#define SSI_CR0_FRF_TI_SYNC_SERIAL                                  0x00000010
#define SSI_CR0_FRF_MOTOROLA_SPI                                    0x00000000

/* Field:   [3:0] DSS
 *
 * Data Size Select.
 * Values 0b0000, 0b0001, 0b0010 are reserved and shall not be used.
 * ENUMs:
 * 16_BIT                   16-bit data
 * 15_BIT                   15-bit data
 * 14_BIT                   14-bit data
 * 13_BIT                   13-bit data
 * 12_BIT                   12-bit data
 * 11_BIT                   11-bit data
 * 10_BIT                   10-bit data
 * 9_BIT                    9-bit data
 * 8_BIT                    8-bit data
 * 7_BIT                    7-bit data
 * 6_BIT                    6-bit data
 * 5_BIT                    5-bit data
 * 4_BIT                    4-bit data
 */

#define SSI_CR0_DSS_MASK                                            0x0000000f
#define SSI_CR0_DSS_SHIFT                                                    0
#define SSI_CR0_DSS_16_BIT                                          0x0000000f
#define SSI_CR0_DSS_15_BIT                                          0x0000000e
#define SSI_CR0_DSS_14_BIT                                          0x0000000d
#define SSI_CR0_DSS_13_BIT                                          0x0000000c
#define SSI_CR0_DSS_12_BIT                                          0x0000000b
#define SSI_CR0_DSS_11_BIT                                          0x0000000a
#define SSI_CR0_DSS_10_BIT                                          0x00000009
#define SSI_CR0_DSS_9_BIT                                           0x00000008
#define SSI_CR0_DSS_8_BIT                                           0x00000007
#define SSI_CR0_DSS_7_BIT                                           0x00000006
#define SSI_CR0_DSS_6_BIT                                           0x00000005
#define SSI_CR0_DSS_5_BIT                                           0x00000004
#define SSI_CR0_DSS_4_BIT                                           0x00000003

/******************************************************************************
 *
 * Register: SSI_CR1
 *
 ******************************************************************************
 * Field:     [3] SOD
 *
 * Slave-mode output disabled
 * This bit is relevant only in the slave mode, MS=1. In multiple-slave
 * systems, it is possible for an SSI master to broadcast a message to all
 * slaves in the system while ensuring that only one slave drives data onto its
 * serial output line. In such systems the RXD lines from multiple slaves could
 * be tied together. To operate in such systems, this bitfield can be set if
 * the SSI slave is not supposed to drive the TXD line:
 *
 * 0: SSI can drive the TXD output in slave mode.
 * 1: SSI cannot drive the TXD output in slave mode.
 */

#define SSI_CR1_SOD                                                 0x00000008
#define SSI_CR1_SOD_MASK                                            0x00000008
#define SSI_CR1_SOD_SHIFT                                                    3

/* Field:     [2] MS
 *
 * Master or slave mode select. This bit can be modified only when SSI is
 * disabled, SSE=0.
 * ENUMs:
 * SLAVE                    Device configured as slave
 * MASTER                   Device configured as master
 */

#define SSI_CR1_MS                                                  0x00000004
#define SSI_CR1_MS_MASK                                             0x00000004
#define SSI_CR1_MS_SHIFT                                                     2
#define SSI_CR1_MS_SLAVE                                            0x00000004
#define SSI_CR1_MS_MASTER                                           0x00000000

/* Field:     [1] SSE
 *
 * Synchronous serial interface enable.
 * ENUMs:
 * SSI_ENABLED              Operation enabled
 * SSI_DISABLED             Operation disabled
 */

#define SSI_CR1_SSE                                                 0x00000002
#define SSI_CR1_SSE_MASK                                            0x00000002
#define SSI_CR1_SSE_SHIFT                                                    1
#define SSI_CR1_SSE_SSI_ENABLED                                     0x00000002
#define SSI_CR1_SSE_SSI_DISABLED                                    0x00000000

/* Field:     [0] LBM
 *
 * Loop back mode:
 *
 * 0: Normal serial port operation enabled.
 * 1: Output of transmit serial shifter is connected to input of receive serial
 * shifter internally.
 */

#define SSI_CR1_LBM                                                 0x00000001
#define SSI_CR1_LBM_MASK                                            0x00000001
#define SSI_CR1_LBM_SHIFT                                                    0

/******************************************************************************
 *
 * Register: SSI_DR
 *
 ******************************************************************************
 * Field:  [15:0] DATA
 *
 * Transmit/receive data
 * The values read from this field or written to this field must be
 * right-justified when SSI is programmed for a data size that is less than 16
 * bits (CR0.DSS != 0b1111). Unused bits at the top are ignored by transmit
 * logic. The receive logic automatically right-justifies.
 */

#define SSI_DR_DATA_MASK                                            0x0000ffff
#define SSI_DR_DATA_SHIFT                                                    0

/******************************************************************************
 *
 * Register: SSI_SR
 *
 ******************************************************************************
 * Field:     [4] BSY
 *
 * Serial interface busy:
 *
 * 0: SSI is idle
 * 1: SSI is currently transmitting and/or receiving a frame or the transmit
 * FIFO is not empty.
 */

#define SSI_SR_BSY                                                  0x00000010
#define SSI_SR_BSY_MASK                                             0x00000010
#define SSI_SR_BSY_SHIFT                                                     4

/* Field:     [3] RFF
 *
 * Receive FIFO full:
 *
 * 0: Receive FIFO is not full.
 * 1: Receive FIFO is full.
 */

#define SSI_SR_RFF                                                  0x00000008
#define SSI_SR_RFF_MASK                                             0x00000008
#define SSI_SR_RFF_SHIFT                                                     3

/* Field:     [2] RNE
 *
 * Receive FIFO not empty
 *
 * 0: Receive FIFO is empty.
 * 1: Receive FIFO is not empty.
 */

#define SSI_SR_RNE                                                  0x00000004
#define SSI_SR_RNE_MASK                                             0x00000004
#define SSI_SR_RNE_SHIFT                                                     2

/* Field:     [1] TNF
 *
 * Transmit FIFO not full:
 *
 * 0: Transmit FIFO is full.
 * 1: Transmit FIFO is not full.
 */

#define SSI_SR_TNF                                                  0x00000002
#define SSI_SR_TNF_MASK                                             0x00000002
#define SSI_SR_TNF_SHIFT                                                     1

/* Field:     [0] TFE
 *
 * Transmit FIFO empty:
 *
 * 0: Transmit FIFO is not empty.
 * 1: Transmit FIFO is empty.
 */

#define SSI_SR_TFE                                                  0x00000001
#define SSI_SR_TFE_MASK                                             0x00000001
#define SSI_SR_TFE_SHIFT                                                     0

/******************************************************************************
 *
 * Register: SSI_CPSR
 *
 ******************************************************************************
 * Field:   [7:0] CPSDVSR
 *
 * Clock prescale divisor:
 * This field specifies the division factor by which the input system clock to
 * SSI must be internally divided before further use.
 * The value programmed into this field must be an even non-zero number
 * (2-254). The least significant bit of the programmed number is hard-coded to
 * zero. If an odd number is written to this register, data read back from
 * this register has the least significant bit as zero.
 */

#define SSI_CPSR_CPSDVSR_MASK                                       0x000000ff
#define SSI_CPSR_CPSDVSR_SHIFT                                               0

/******************************************************************************
 *
 * Register: SSI_IMSC
 *
 ******************************************************************************
 * Field:     [3] TXIM
 *
 * Transmit FIFO interrupt mask:
 * A read returns the current mask for transmit FIFO interrupt. On a write of
 * 1, the mask for transmit FIFO interrupt is set which means the interrupt
 * state will be reflected in MIS.TXMIS. A write of 0 clears the mask which
 * means MIS.TXMIS will not reflect the interrupt.
 */

#define SSI_IMSC_TXIM                                               0x00000008
#define SSI_IMSC_TXIM_MASK                                          0x00000008
#define SSI_IMSC_TXIM_SHIFT                                                  3

/* Field:     [2] RXIM
 *
 * Receive FIFO interrupt mask:
 * A read returns the current mask for receive FIFO interrupt. On a write of 1,
 * the mask for receive FIFO interrupt is set which means the interrupt state
 * will be reflected in MIS.RXMIS. A write of 0 clears the mask which means
 * MIS.RXMIS will not reflect the interrupt.
 */

#define SSI_IMSC_RXIM                                               0x00000004
#define SSI_IMSC_RXIM_MASK                                          0x00000004
#define SSI_IMSC_RXIM_SHIFT                                                  2

/* Field:     [1] RTIM
 *
 * Receive timeout interrupt mask:
 * A read returns the current mask for receive timeout interrupt. On a write of
 * 1, the mask for receive timeout interrupt is set which means the interrupt
 * state will be reflected in MIS.RTMIS. A write of 0 clears the mask which
 * means MIS.RTMIS will not reflect the interrupt.
 */

#define SSI_IMSC_RTIM                                               0x00000002
#define SSI_IMSC_RTIM_MASK                                          0x00000002
#define SSI_IMSC_RTIM_SHIFT                                                  1

/* Field:     [0] RORIM
 *
 * Receive overrun interrupt mask:
 * A read returns the current mask for receive overrun interrupt. On a write of
 * 1, the mask for receive overrun interrupt is set which means the interrupt
 * state will be reflected in MIS.RORMIS. A write of 0 clears the mask which
 * means MIS.RORMIS will not reflect the interrupt.
 */

#define SSI_IMSC_RORIM                                              0x00000001
#define SSI_IMSC_RORIM_MASK                                         0x00000001
#define SSI_IMSC_RORIM_SHIFT                                                 0

/******************************************************************************
 *
 * Register: SSI_RIS
 *
 ******************************************************************************
 * Field:     [3] TXRIS
 *
 * Raw transmit FIFO interrupt status:
 * The transmit interrupt is asserted when there are four or fewer valid
 * entries in the transmit FIFO. The transmit interrupt is not qualified with
 * the SSI enable signal. Therefore one of the following ways can be used:
 *  - data can be written to the transmit FIFO prior to enabling the SSI and
 * the
 * interrupts.
 *  - SSI and interrupts can be enabled so that data can be written to the
 * transmit FIFO by an interrupt service routine.
 */

#define SSI_RIS_TXRIS                                               0x00000008
#define SSI_RIS_TXRIS_MASK                                          0x00000008
#define SSI_RIS_TXRIS_SHIFT                                                  3

/* Field:     [2] RXRIS
 *
 * Raw interrupt state of receive FIFO interrupt:
 * The receive interrupt is asserted when there are four or more valid entries
 * in the receive FIFO.
 */

#define SSI_RIS_RXRIS                                               0x00000004
#define SSI_RIS_RXRIS_MASK                                          0x00000004
#define SSI_RIS_RXRIS_SHIFT                                                  2

/* Field:     [1] RTRIS
 *
 * Raw interrupt state of receive timeout interrupt:
 * The receive timeout interrupt is asserted when the receive FIFO is not empty
 * and SSI has remained idle for a fixed 32 bit period. This mechanism can be
 * used to notify the user that data is still present in the receive FIFO and
 * requires servicing. This interrupt is deasserted if the receive FIFO becomes
 * empty by subsequent reads, or if new data is received on RXD.
 * It can also be cleared by writing to ICR.RTIC.
 */

#define SSI_RIS_RTRIS                                               0x00000002
#define SSI_RIS_RTRIS_MASK                                          0x00000002
#define SSI_RIS_RTRIS_SHIFT                                                  1

/* Field:     [0] RORRIS
 *
 * Raw interrupt state of receive overrun interrupt:
 * The receive overrun interrupt is asserted when the FIFO is already full and
 * an additional data frame is received, causing an overrun of the FIFO. Data
 * is over-written in the
 * receive shift register, but not the FIFO so the FIFO contents stay valid.
 * It can also be cleared by writing to ICR.RORIC.
 */

#define SSI_RIS_RORRIS                                              0x00000001
#define SSI_RIS_RORRIS_MASK                                         0x00000001
#define SSI_RIS_RORRIS_SHIFT                                                 0

/******************************************************************************
 *
 * Register: SSI_MIS
 *
 ******************************************************************************
 * Field:     [3] TXMIS
 *
 * Masked interrupt state of transmit FIFO interrupt:
 * This field returns the masked interrupt state of transmit FIFO interrupt
 * which is the AND product of raw interrupt state RIS.TXRIS and the mask
 * setting IMSC.TXIM.
 */

#define SSI_MIS_TXMIS                                               0x00000008
#define SSI_MIS_TXMIS_MASK                                          0x00000008
#define SSI_MIS_TXMIS_SHIFT                                                  3

/* Field:     [2] RXMIS
 *
 * Masked interrupt state of receive FIFO interrupt:
 * This field returns the masked interrupt state of receive FIFO interrupt
 * which is the AND product of raw interrupt state RIS.RXRIS and the mask
 * setting IMSC.RXIM.
 */

#define SSI_MIS_RXMIS                                               0x00000004
#define SSI_MIS_RXMIS_MASK                                          0x00000004
#define SSI_MIS_RXMIS_SHIFT                                                  2

/* Field:     [1] RTMIS
 *
 * Masked interrupt state of receive timeout interrupt:
 * This field returns the masked interrupt state of receive timeout interrupt
 * which is the AND product of raw interrupt state RIS.RTRIS and the mask
 * setting IMSC.RTIM.
 */

#define SSI_MIS_RTMIS                                               0x00000002
#define SSI_MIS_RTMIS_MASK                                          0x00000002
#define SSI_MIS_RTMIS_SHIFT                                                  1

/* Field:     [0] RORMIS
 *
 * Masked interrupt state of receive overrun interrupt:
 * This field returns the masked interrupt state of receive overrun interrupt
 * which is the AND product of raw interrupt state RIS.RORRIS and the mask
 * setting IMSC.RORIM.
 */

#define SSI_MIS_RORMIS                                              0x00000001
#define SSI_MIS_RORMIS_MASK                                         0x00000001
#define SSI_MIS_RORMIS_SHIFT                                                 0

/******************************************************************************
 *
 * Register: SSI_ICR
 *
 ******************************************************************************
 * Field:     [1] RTIC
 *
 * Clear the receive timeout interrupt:
 * Writing 1 to this field clears the timeout interrupt (RIS.RTRIS). Writing 0
 * has no effect.
 */

#define SSI_ICR_RTIC                                                0x00000002
#define SSI_ICR_RTIC_MASK                                           0x00000002
#define SSI_ICR_RTIC_SHIFT                                                   1

/* Field:     [0] RORIC
 *
 * Clear the receive overrun interrupt:
 * Writing 1 to this field clears the overrun error interrupt (RIS.RORRIS).
 * Writing 0 has no effect.
 */

#define SSI_ICR_RORIC                                               0x00000001
#define SSI_ICR_RORIC_MASK                                          0x00000001
#define SSI_ICR_RORIC_SHIFT                                                  0

/******************************************************************************
 *
 * Register: SSI_DMACR
 *
 ******************************************************************************
 * Field:     [1] TXDMAE
 *
 * Transmit DMA enable. If this bit is set to 1, DMA for the transmit FIFO is
 * enabled.
 */

#define SSI_DMACR_TXDMAE                                            0x00000002
#define SSI_DMACR_TXDMAE_MASK                                       0x00000002
#define SSI_DMACR_TXDMAE_SHIFT                                               1

/* Field:     [0] RXDMAE
 *
 * Receive DMA enable. If this bit is set to 1, DMA for the receive FIFO is
 * enabled.
 */

#define SSI_DMACR_RXDMAE                                            0x00000001
#define SSI_DMACR_RXDMAE_MASK                                       0x00000001
#define SSI_DMACR_RXDMAE_SHIFT                                               0

#endif /* __ARCH_ARM_SRC_TIVA_HARDWARE_CC13X2_CC26X2_V1_SSI_H */
