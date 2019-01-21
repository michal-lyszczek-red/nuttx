/************************************************************************************
 * arch/arm/src/tiva/cc13xx/cc13x2_cc26xx2_v1_rom.h
 *
 *   Copyright (C) 2019 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * This is a port of TI's rom.h file which has a fully compatible BSD license:
 *
 *    Copyright (c) 2015-2017, Texas Instruments Incorporated
 *    All rights reserved.
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
 ************************************************************************************/

#ifndef __ARCH_ARM_SRC_TIVA_CC13XX_CC13X2_CC26X2_V2_ROM_H
#define __ARCH_ARM_SRC_TIVA_CC13XX_CC13X2_CC26X2_V2_ROM_H

/************************************************************************************
 * Pre-processor Definitions
 ************************************************************************************/

/* Start address of the ROM hard API access table (located after the ROM FW rev field) */

#define ROM_HAPI_TABLE_ADDR 0x10000048

/* Pointer to the ROM HAPI table */

#define P_HARD_API                     ((hard_api_t *)ROM_HAPI_TABLE_ADDR)

#define hapi_crc32(a,b,c)              P_HARD_API->crc32(a,b,c)
#define hapi_get_flashsize()           P_HARD_API->get_flashsize()
#define hapi_get_chipid()              P_HARD_API->get_chipid()
#define hapi_reset_device()            P_HARD_API->reset_device()
#define hapi_fletcher32(a,b,c)         P_HARD_API->fletcher32(a,b,c)
#define hapi_min(a,b)                  P_HARD_API->min(a,b)
#define hapi_max(a,b)                  P_HARD_API->max(a,b)
#define hapi_mean(a,b)                 P_HARD_API->mean(a,b)
#define hapi_standard_devation(a,b)    P_HARD_API->standard_devation(a,b)
#define hapi_source_safeswitch()       P_HARD_API->hf_source_safeswitch()
#define hapi_select_compa_input(a)     P_HARD_API->select_compa_input(a)
#define hapi_select_compa_ref(a)       P_HARD_API->select_compa_ref(a)
#define hapi_select_adc_compb_input(a) P_HARD_API->select_adc_compb_input(a)
#define hapi_select_dac_vref(a)        P_HARD_API->select_dac_vref(a)

/* Defines for input parameter to the hapi_select_compa_input function.
 * The define values can not be changed!
 */

#define COMPA_IN_NC            0x00
#define COMPA_IN_AUXIO7        0x09
#define COMPA_IN_AUXIO6        0x0a
#define COMPA_IN_AUXIO5        0x0b
#define COMPA_IN_AUXIO4        0x0c
#define COMPA_IN_AUXIO3        0x0d
#define COMPA_IN_AUXIO2        0x0e
#define COMPA_IN_AUXIO1        0x0f
#define COMPA_IN_AUXIO0        0x10

/* Defines for input parameter to the hapi_select_compa_ref function.
 * The define values can not be changed!
 */

#define COMPA_REF_NC           0x00
#define COMPA_REF_DCOUPL       0x01
#define COMPA_REF_VSS          0x02
#define COMPA_REF_VDDS         0x03
#define COMPA_REF_ADCVREFP     0x04
#define COMPA_REF_AUXIO7       0x09
#define COMPA_REF_AUXIO6       0x0a
#define COMPA_REF_AUXIO5       0x0b
#define COMPA_REF_AUXIO4       0x0c
#define COMPA_REF_AUXIO3       0x0d
#define COMPA_REF_AUXIO2       0x0e
#define COMPA_REF_AUXIO1       0x0f
#define COMPA_REF_AUXIO0       0x10

/* Defines for input parameter to the hapi_select_adc_compb_input function.
 * The define values can not be changed!
 */

#define ADC_COMPB_IN_NC        0x00
#define ADC_COMPB_IN_DCOUPL    0x03
#define ADC_COMPB_IN_VSS       0x04
#define ADC_COMPB_IN_VDDS      0x05
#define ADC_COMPB_IN_AUXIO7    0x09
#define ADC_COMPB_IN_AUXIO6    0x0a
#define ADC_COMPB_IN_AUXIO5    0x0b
#define ADC_COMPB_IN_AUXIO4    0x0c
#define ADC_COMPB_IN_AUXIO3    0x0d
#define ADC_COMPB_IN_AUXIO2    0x0e
#define ADC_COMPB_IN_AUXIO1    0x0f
#define ADC_COMPB_IN_AUXIO0    0x10

/* Defines for input parameter to the hapi_select_dac_vref function.
 * The define values can not be changed!
 */

#define DAC_REF_NC             0x00
#define DAC_REF_DCOUPL         0x01
#define DAC_REF_VSS            0x02
#define DAC_REF_VDDS           0x03

/* Pointers to the main API tables */

#define ROM_API_TABLE           ((uint32_t *) 0x10000180)
#define ROM_VERSION             (ROM_API_TABLE[0])

#define ROM_API_AON_EVENT_TABLE  ((uint32_t *)(ROM_API_TABLE[1]))
#define ROM_API_AON_IOC_TABLE    ((uint32_t *)(ROM_API_TABLE[2]))
#define ROM_API_AON_RTC_TABLE    ((uint32_t *)(ROM_API_TABLE[3]))
#define ROM_API_AUX_CTRL_TABLE   ((uint32_t *)(ROM_API_TABLE[5]))
#define ROM_API_AUX_TDC_TABLE    ((uint32_t *)(ROM_API_TABLE[6]))
#define ROM_API_DDI_TABLE        ((uint32_t *)(ROM_API_TABLE[9]))
#define ROM_API_FLASH_TABLE      ((uint32_t *)(ROM_API_TABLE[10]))
#define ROM_API_I2C_TABLE        ((uint32_t *)(ROM_API_TABLE[11]))
#define ROM_API_INTERRUPT_TABLE  ((uint32_t *)(ROM_API_TABLE[12]))
#define ROM_API_IOC_TABLE        ((uint32_t *)(ROM_API_TABLE[13]))
#define ROM_API_PRCM_TABLE       ((uint32_t *)(ROM_API_TABLE[14]))
#define ROM_API_SMPH_TABLE       ((uint32_t *)(ROM_API_TABLE[15]))
#define ROM_API_SSI_TABLE        ((uint32_t *)(ROM_API_TABLE[17]))
#define ROM_API_TIMER_TABLE      ((uint32_t *)(ROM_API_TABLE[18]))
#define ROM_API_TRNG_TABLE       ((uint32_t *)(ROM_API_TABLE[19]))
#define ROM_API_UART_TABLE       ((uint32_t *)(ROM_API_TABLE[20]))
#define ROM_API_UDMA_TABLE       ((uint32_t *)(ROM_API_TABLE[21]))
#define ROM_API_VIMS_TABLE       ((uint32_t *)(ROM_API_TABLE[22]))
#define ROM_API_CRYPTO_TABLE     ((uint32_t *)(ROM_API_TABLE[23]))
#define ROM_API_OSC_TABLE        ((uint32_t *)(ROM_API_TABLE[24]))
#define ROM_API_AUX_ADC_TABLE    ((uint32_t *)(ROM_API_TABLE[25]))
#define ROM_API_SYS_CTRL_TABLE   ((uint32_t *)(ROM_API_TABLE[26]))
#define ROM_API_AON_BATMON_TABLE ((uint32_t *)(ROM_API_TABLE[27]))
#define ROM_API_SETUP_ROM_TABLE  ((uint32_t *)(ROM_API_TABLE[28]))
#define ROM_API_I2S_TABLE        ((uint32_t *)(ROM_API_TABLE[29]))
#define ROM_API_PWR_CTRL_TABLE   ((uint32_t *)(ROM_API_TABLE[30]))
#define ROM_API_AES_TABLE        ((uint32_t *)(ROM_API_TABLE[31]))
#define ROM_API_PKA_TABLE        ((uint32_t *)(ROM_API_TABLE[32]))
#define ROM_API_SHA2_TABLE       ((uint32_t *)(ROM_API_TABLE[33]))

/* AON_EVENT FUNCTIONS */

#define AONEventMcuWakeUpSet \
    ((void (*)(uint32_t ui32MCUWUEvent, uint32_t ui32EventSrc)) \
    ROM_API_AON_EVENT_TABLE[0])

#define AONEventMcuWakeUpGet \
    ((uint32_t (*)(uint32_t ui32MCUWUEvent)) \
    ROM_API_AON_EVENT_TABLE[1])

#define AONEventMcuSet \
    ((void (*)(uint32_t ui32MCUEvent, uint32_t ui32EventSrc)) \
    ROM_API_AON_EVENT_TABLE[4])

#define AONEventMcuGet \
    ((uint32_t (*)(uint32_t ui32MCUEvent)) \
    ROM_API_AON_EVENT_TABLE[5])

/* AON_RTC FUNCTIONS */

#define AONRTCCurrent64BitValueGet \
    ((uint64_t (*)(void)) \
    ROM_API_AON_RTC_TABLE[12])

/* AUX_TDC FUNCTIONS */

#define AUXTDCConfigSet \
    ((void (*)(uint32_t ui32Base, uint32_t ui32StartCondition, uint32_t ui32StopCondition)) \
    ROM_API_AUX_TDC_TABLE[0])

#define AUXTDCMeasurementDone \
    ((uint32_t (*)(uint32_t ui32Base)) \
    ROM_API_AUX_TDC_TABLE[1])

/* DDI FUNCTIONS */

#define DDI16BitWrite \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Reg, uint32_t ui32Mask, uint32_t ui32WrData)) \
    ROM_API_DDI_TABLE[0])

#define DDI16BitfieldWrite \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Reg, uint32_t ui32Mask, uint32_t ui32Shift, uint16_t ui32Data)) \
    ROM_API_DDI_TABLE[1])

#define DDI16BitRead \
    ((uint16_t (*)(uint32_t ui32Base, uint32_t ui32Reg, uint32_t ui32Mask)) \
    ROM_API_DDI_TABLE[2])

#define DDI16BitfieldRead \
    ((uint16_t (*)(uint32_t ui32Base, uint32_t ui32Reg, uint32_t ui32Mask, uint32_t ui32Shift)) \
    ROM_API_DDI_TABLE[3])

#define DDI32RegWrite \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Reg, uint32_t ui32Val)) \
    ROM_API_DDI_TABLE[4])

/* FLASH FUNCTIONS */

#define FlashPowerModeSet \
    ((void (*)(uint32_t ui32PowerMode, uint32_t ui32BankGracePeriod, uint32_t ui32PumpGracePeriod)) \
    ROM_API_FLASH_TABLE[0])

#define FlashPowerModeGet \
    ((uint32_t (*)(void)) \
    ROM_API_FLASH_TABLE[1])

#define FlashProtectionSet \
    ((void (*)(uint32_t ui32SectorAddress, uint32_t ui32ProtectMode)) \
    ROM_API_FLASH_TABLE[2])

#define FlashProtectionGet \
    ((uint32_t (*)(uint32_t ui32SectorAddress)) \
    ROM_API_FLASH_TABLE[3])

#define FlashProtectionSave \
    ((uint32_t (*)(uint32_t ui32SectorAddress)) \
    ROM_API_FLASH_TABLE[4])

#define FlashEfuseReadRow \
    ((bool (*)(uint32_t *pui32EfuseData, uint32_t ui32RowAddress)) \
    ROM_API_FLASH_TABLE[8])

#define FlashDisableSectorsForWrite \
    ((void (*)(void)) \
    ROM_API_FLASH_TABLE[9])

/* I2C FUNCTIONS */

#define I2CMasterInitExpClk \
    ((void (*)(uint32_t ui32Base, uint32_t ui32I2CClk, bool bFast)) \
    ROM_API_I2C_TABLE[0])

#define I2CMasterErr \
    ((uint32_t (*)(uint32_t ui32Base)) \
    ROM_API_I2C_TABLE[1])

/* INTERRUPT FUNCTIONS */

#define IntPriorityGroupingSet \
    ((void (*)(uint32_t ui32Bits)) \
    ROM_API_INTERRUPT_TABLE[0])

#define IntPriorityGroupingGet \
    ((uint32_t (*)(void)) \
    ROM_API_INTERRUPT_TABLE[1])

#define IntPrioritySet \
    ((void (*)(uint32_t ui32Interrupt, uint8_t ui8Priority)) \
    ROM_API_INTERRUPT_TABLE[2])

#define IntPriorityGet \
    ((int32_t (*)(uint32_t ui32Interrupt)) \
    ROM_API_INTERRUPT_TABLE[3])

#define IntEnable \
    ((void (*)(uint32_t ui32Interrupt)) \
    ROM_API_INTERRUPT_TABLE[4])

#define IntDisable \
    ((void (*)(uint32_t ui32Interrupt)) \
    ROM_API_INTERRUPT_TABLE[5])

#define IntPendSet \
    ((void (*)(uint32_t ui32Interrupt)) \
    ROM_API_INTERRUPT_TABLE[6])

#define IntPendGet \
    ((bool (*)(uint32_t ui32Interrupt)) \
    ROM_API_INTERRUPT_TABLE[7])

#define IntPendClear \
    ((void (*)(uint32_t ui32Interrupt)) \
    ROM_API_INTERRUPT_TABLE[8])

/* IOC FUNCTIONS */

#define IOCPortConfigureSet \
    ((void (*)(uint32_t ui32IOId, uint32_t ui32PortId, uint32_t ui32IOConfig)) \
    ROM_API_IOC_TABLE[0])

#define IOCPortConfigureGet \
    ((uint32_t (*)(uint32_t ui32IOId)) \
    ROM_API_IOC_TABLE[1])

#define IOCIOShutdownSet \
    ((void (*)(uint32_t ui32IOId, uint32_t ui32IOShutdown)) \
    ROM_API_IOC_TABLE[2])

#define IOCIOModeSet \
    ((void (*)(uint32_t ui32IOId, uint32_t ui32IOMode)) \
    ROM_API_IOC_TABLE[4])

#define IOCIOIntSet \
    ((void (*)(uint32_t ui32IOId, uint32_t ui32Int, uint32_t ui32EdgeDet)) \
    ROM_API_IOC_TABLE[5])

#define IOCIOPortPullSet \
    ((void (*)(uint32_t ui32IOId, uint32_t ui32Pull)) \
    ROM_API_IOC_TABLE[6])

#define IOCIOHystSet \
    ((void (*)(uint32_t ui32IOId, uint32_t ui32Hysteresis)) \
    ROM_API_IOC_TABLE[7])

#define IOCIOInputSet \
    ((void (*)(uint32_t ui32IOId, uint32_t ui32Input)) \
    ROM_API_IOC_TABLE[8])

#define IOCIOSlewCtrlSet \
    ((void (*)(uint32_t ui32IOId, uint32_t ui32SlewEnable)) \
    ROM_API_IOC_TABLE[9])

#define IOCIODrvStrengthSet \
    ((void (*)(uint32_t ui32IOId, uint32_t ui32IOCurrent, uint32_t ui32DrvStrength)) \
    ROM_API_IOC_TABLE[10])

#define IOCIOPortIdSet \
    ((void (*)(uint32_t ui32IOId, uint32_t ui32PortId)) \
    ROM_API_IOC_TABLE[11])

#define IOCIntEnable \
    ((void (*)(uint32_t ui32IOId)) \
    ROM_API_IOC_TABLE[12])

#define IOCIntDisable \
    ((void (*)(uint32_t ui32IOId)) \
    ROM_API_IOC_TABLE[13])

#define IOCPinTypeGpioInput \
    ((void (*)(uint32_t ui32IOId)) \
    ROM_API_IOC_TABLE[14])

#define IOCPinTypeGpioOutput \
    ((void (*)(uint32_t ui32IOId)) \
    ROM_API_IOC_TABLE[15])

#define IOCPinTypeUart \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Rx, uint32_t ui32Tx, uint32_t ui32Cts, uint32_t ui32Rts)) \
    ROM_API_IOC_TABLE[16])

#define IOCPinTypeSsiMaster \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Rx, uint32_t ui32Tx, uint32_t ui32Fss, uint32_t ui32Clk)) \
    ROM_API_IOC_TABLE[17])

#define IOCPinTypeSsiSlave \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Rx, uint32_t ui32Tx, uint32_t ui32Fss, uint32_t ui32Clk)) \
    ROM_API_IOC_TABLE[18])

#define IOCPinTypeI2c \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Data, uint32_t ui32Clk)) \
    ROM_API_IOC_TABLE[19])

#define IOCPinTypeAux \
    ((void (*)(uint32_t ui32IOId)) \
    ROM_API_IOC_TABLE[21])

/* PRCM FUNCTIONS */

#define PRCMInfClockConfigureSet \
    ((void (*)(uint32_t ui32ClkDiv, uint32_t ui32PowerMode)) \
    ROM_API_PRCM_TABLE[0])

#define PRCMInfClockConfigureGet \
    ((uint32_t (*)(uint32_t ui32PowerMode)) \
    ROM_API_PRCM_TABLE[1])

#define PRCMAudioClockConfigSet \
    ((void (*)(uint32_t ui32ClkConfig, uint32_t ui32SampleRate)) \
    ROM_API_PRCM_TABLE[4])

#define PRCMPowerDomainOn \
    ((void (*)(uint32_t ui32Domains)) \
    ROM_API_PRCM_TABLE[5])

#define PRCMPowerDomainOff \
    ((void (*)(uint32_t ui32Domains)) \
    ROM_API_PRCM_TABLE[6])

#define PRCMPeripheralRunEnable \
    ((void (*)(uint32_t ui32Peripheral)) \
    ROM_API_PRCM_TABLE[7])

#define PRCMPeripheralRunDisable \
    ((void (*)(uint32_t ui32Peripheral)) \
    ROM_API_PRCM_TABLE[8])

#define PRCMPeripheralSleepEnable \
    ((void (*)(uint32_t ui32Peripheral)) \
    ROM_API_PRCM_TABLE[9])

#define PRCMPeripheralSleepDisable \
    ((void (*)(uint32_t ui32Peripheral)) \
    ROM_API_PRCM_TABLE[10])

#define PRCMPeripheralDeepSleepEnable \
    ((void (*)(uint32_t ui32Peripheral)) \
    ROM_API_PRCM_TABLE[11])

#define PRCMPeripheralDeepSleepDisable \
    ((void (*)(uint32_t ui32Peripheral)) \
    ROM_API_PRCM_TABLE[12])

#define PRCMPowerDomainStatus \
    ((uint32_t (*)(uint32_t ui32Domains)) \
    ROM_API_PRCM_TABLE[13])

#define PRCMDeepSleep \
    ((void (*)(void)) \
    ROM_API_PRCM_TABLE[14])

#define PRCMAudioClockConfigSetOverride \
    ((void (*)(uint32_t ui32ClkConfig, uint32_t ui32MstDiv, uint32_t ui32BitDiv, uint32_t ui32WordDiv)) \
    ROM_API_PRCM_TABLE[17])

/* SMPH FUNCTIONS */

#define SMPHAcquire \
    ((void (*)(uint32_t ui32Semaphore)) \
    ROM_API_SMPH_TABLE[0])

/* SSI FUNCTIONS */

#define SSIConfigSetExpClk \
    ((void (*)(uint32_t ui32Base, uint32_t ui32SSIClk, uint32_t ui32Protocol, uint32_t ui32Mode, uint32_t ui32BitRate, uint32_t ui32DataWidth)) \
    ROM_API_SSI_TABLE[0])

#define SSIDataPut \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Data)) \
    ROM_API_SSI_TABLE[1])

#define SSIDataPutNonBlocking \
    ((int32_t (*)(uint32_t ui32Base, uint32_t ui32Data)) \
    ROM_API_SSI_TABLE[2])

#define SSIDataGet \
    ((void (*)(uint32_t ui32Base, uint32_t *pui32Data)) \
    ROM_API_SSI_TABLE[3])

#define SSIDataGetNonBlocking \
    ((int32_t (*)(uint32_t ui32Base, uint32_t *pui32Data)) \
    ROM_API_SSI_TABLE[4])

/* TIMER FUNCTIONS */

#define TimerConfigure \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Config)) \
    ROM_API_TIMER_TABLE[0])

#define TimerLevelControl \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Timer, bool bInvert)) \
    ROM_API_TIMER_TABLE[1])

#define TimerStallControl \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Timer, bool bStall)) \
    ROM_API_TIMER_TABLE[3])

#define TimerWaitOnTriggerControl \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Timer, bool bWait)) \
    ROM_API_TIMER_TABLE[4])

#define TimerIntervalLoadMode \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Timer, uint32_t ui32Mode)) \
    ROM_API_TIMER_TABLE[5])

#define TimerMatchUpdateMode \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Timer, uint32_t ui32Mode)) \
    ROM_API_TIMER_TABLE[6])

/* TRNG FUNCTIONS */

#define TRNGConfigure \
    ((void (*)(uint32_t ui32MinSamplesPerCycle, uint32_t ui32MaxSamplesPerCycle, uint32_t ui32ClocksPerSample)) \
    ROM_API_TRNG_TABLE[0])

#define TRNGNumberGet \
    ((uint32_t (*)(uint32_t ui32Word)) \
    ROM_API_TRNG_TABLE[1])

/* UART FUNCTIONS */

#define UARTFIFOLevelGet \
    ((void (*)(uint32_t ui32Base, uint32_t *pui32TxLevel, uint32_t *pui32RxLevel)) \
    ROM_API_UART_TABLE[0])

#define UARTConfigSetExpClk \
    ((void (*)(uint32_t ui32Base, uint32_t ui32UARTClk, uint32_t ui32Baud, uint32_t ui32Config)) \
    ROM_API_UART_TABLE[1])

#define UARTConfigGetExpClk \
    ((void (*)(uint32_t ui32Base, uint32_t ui32UARTClk, uint32_t *pui32Baud, uint32_t *pui32Config)) \
    ROM_API_UART_TABLE[2])

#define UARTDisable \
    ((void (*)(uint32_t ui32Base)) \
    ROM_API_UART_TABLE[3])

#define UARTCharGetNonBlocking \
    ((int32_t (*)(uint32_t ui32Base)) \
    ROM_API_UART_TABLE[4])

#define UARTCharGet \
    ((int32_t (*)(uint32_t ui32Base)) \
    ROM_API_UART_TABLE[5])

#define UARTCharPutNonBlocking \
    ((bool (*)(uint32_t ui32Base, uint8_t ui8Data)) \
    ROM_API_UART_TABLE[6])

#define UARTCharPut \
    ((void (*)(uint32_t ui32Base, uint8_t ui8Data)) \
    ROM_API_UART_TABLE[7])

/* UDMA FUNCTIONS */

#define uDMAChannelAttributeEnable \
    ((void (*)(uint32_t ui32Base, uint32_t ui32ChannelNum, uint32_t ui32Attr)) \
    ROM_API_UDMA_TABLE[0])

#define uDMAChannelAttributeDisable \
    ((void (*)(uint32_t ui32Base, uint32_t ui32ChannelNum, uint32_t ui32Attr)) \
    ROM_API_UDMA_TABLE[1])

#define uDMAChannelAttributeGet \
    ((uint32_t (*)(uint32_t ui32Base, uint32_t ui32ChannelNum)) \
    ROM_API_UDMA_TABLE[2])

#define uDMAChannelControlSet \
    ((void (*)(uint32_t ui32Base, uint32_t ui32ChannelStructIndex, uint32_t ui32Control)) \
    ROM_API_UDMA_TABLE[3])

#define uDMAChannelTransferSet \
    ((void (*)(uint32_t ui32Base, uint32_t ui32ChannelStructIndex, uint32_t ui32Mode, void *pvSrcAddr, void *pvDstAddr, uint32_t ui32TransferSize)) \
    ROM_API_UDMA_TABLE[4])

#define uDMAChannelScatterGatherSet \
    ((void (*)(uint32_t ui32Base, uint32_t ui32ChannelNum, uint32_t ui32TaskCount, void *pvTaskList, uint32_t ui32IsPeriphSG)) \
    ROM_API_UDMA_TABLE[5])

#define uDMAChannelSizeGet \
    ((uint32_t (*)(uint32_t ui32Base, uint32_t ui32ChannelStructIndex)) \
    ROM_API_UDMA_TABLE[6])

#define uDMAChannelModeGet \
    ((uint32_t (*)(uint32_t ui32Base, uint32_t ui32ChannelStructIndex)) \
    ROM_API_UDMA_TABLE[7])

/* VIMS FUNCTIONS */

#define VIMSConfigure \
    ((void (*)(uint32_t ui32Base, bool bRoundRobin, bool bPrefetch)) \
    ROM_API_VIMS_TABLE[0])

#define VIMSModeSet \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Mode)) \
    ROM_API_VIMS_TABLE[1])

#define VIMSModeGet \
    ((uint32_t (*)(uint32_t ui32Base)) \
    ROM_API_VIMS_TABLE[2])

#define VIMSModeSafeSet \
    ((void (*)(uint32_t ui32Base, uint32_t ui32NewMode, bool     blocking)) \
    ROM_API_VIMS_TABLE[3])

/* OSC FUNCTIONS */

#define OSCClockSourceGet \
    ((uint32_t (*)(uint32_t ui32SrcClk)) \
    ROM_API_OSC_TABLE[0])

#define OSCClockSourceSet \
    ((void (*)(uint32_t ui32SrcClk, uint32_t ui32Osc)) \
    ROM_API_OSC_TABLE[1])

#define OSC_HPOSCRelativeFrequencyOffsetGet \
    ((int32_t (*)(int32_t tempDegC)) \
    ROM_API_OSC_TABLE[2])

#define OSC_HPOSCRelativeFrequencyOffsetToRFCoreFormatConvert \
    ((int16_t (*)(int32_t HPOSC_RelFreqOffset)) \
    ROM_API_OSC_TABLE[3])

/* AUX_ADC FUNCTIONS */

#define AUXADCAdjustValueForGainAndOffset \
    ((int32_t (*)(int32_t adcValue, int32_t gain, int32_t offset)) \
    ROM_API_AUX_ADC_TABLE[0])

#define AUXADCDisable \
    ((void (*)(void)) \
    ROM_API_AUX_ADC_TABLE[1])

#define AUXADCDisableInputScaling \
    ((void (*)(void)) \
    ROM_API_AUX_ADC_TABLE[2])

#define AUXADCEnableAsync \
    ((void (*)(uint32_t refSource, uint32_t trigger)) \
    ROM_API_AUX_ADC_TABLE[3])

#define AUXADCEnableSync \
    ((void (*)(uint32_t refSource, uint32_t sampleTime, uint32_t trigger)) \
    ROM_API_AUX_ADC_TABLE[4])

#define AUXADCFlushFifo \
    ((void (*)(void)) \
    ROM_API_AUX_ADC_TABLE[5])

#define AUXADCGetAdjustmentGain \
    ((int32_t (*)(uint32_t refSource)) \
    ROM_API_AUX_ADC_TABLE[6])

#define AUXADCGetAdjustmentOffset \
    ((int32_t (*)(uint32_t refSource)) \
    ROM_API_AUX_ADC_TABLE[7])

#define AUXADCMicrovoltsToValue \
    ((int32_t (*)(int32_t fixedRefVoltage, int32_t microvolts)) \
    ROM_API_AUX_ADC_TABLE[8])

#define AUXADCPopFifo \
    ((uint32_t (*)(void)) \
    ROM_API_AUX_ADC_TABLE[9])

#define AUXADCReadFifo \
    ((uint32_t (*)(void)) \
    ROM_API_AUX_ADC_TABLE[10])

#define AUXADCUnadjustValueForGainAndOffset \
    ((int32_t (*)(int32_t adcValue, int32_t gain, int32_t offset)) \
    ROM_API_AUX_ADC_TABLE[11])

#define AUXADCValueToMicrovolts \
    ((int32_t (*)(int32_t fixedRefVoltage, int32_t adcValue)) \
    ROM_API_AUX_ADC_TABLE[12])

/* SYS_CTRL FUNCTIONS */

#define SysCtrlResetSourceGet \
    ((uint32_t (*)(void)) \
    ROM_API_SYS_CTRL_TABLE[0])

#define SysCtrl_DCDC_VoltageConditionalControl \
    ((void (*)(void)) \
    ROM_API_SYS_CTRL_TABLE[1])

/* AON_BATMON FUNCTIONS */

#define AONBatMonTemperatureGetDegC \
    ((int32_t (*)(void)) \
    ROM_API_AON_BATMON_TABLE[0])

/* SETUP_ROM FUNCTIONS */

#define SetupAfterColdResetWakeupFromShutDownCfg1 \
    ((void (*)(uint32_t ccfg_ModeConfReg)) \
    ROM_API_SETUP_ROM_TABLE[0])

#define SetupAfterColdResetWakeupFromShutDownCfg2 \
    ((void (*)(uint32_t ui32Fcfg1Revision, uint32_t ccfg_ModeConfReg)) \
    ROM_API_SETUP_ROM_TABLE[1])

#define SetupAfterColdResetWakeupFromShutDownCfg3 \
    ((void (*)(uint32_t ccfg_ModeConfReg)) \
    ROM_API_SETUP_ROM_TABLE[2])

#define SetupGetTrimForAdcShModeEn \
    ((uint32_t (*)(uint32_t ui32Fcfg1Revision)) \
    ROM_API_SETUP_ROM_TABLE[3])

#define SetupGetTrimForAdcShVbufEn \
    ((uint32_t (*)(uint32_t ui32Fcfg1Revision)) \
    ROM_API_SETUP_ROM_TABLE[4])

#define SetupGetTrimForAmpcompCtrl \
    ((uint32_t (*)(uint32_t ui32Fcfg1Revision)) \
    ROM_API_SETUP_ROM_TABLE[5])

#define SetupGetTrimForAmpcompTh1 \
    ((uint32_t (*)(void)) \
    ROM_API_SETUP_ROM_TABLE[6])

#define SetupGetTrimForAmpcompTh2 \
    ((uint32_t (*)(void)) \
    ROM_API_SETUP_ROM_TABLE[7])

#define SetupGetTrimForAnabypassValue1 \
    ((uint32_t (*)(uint32_t ccfg_ModeConfReg)) \
    ROM_API_SETUP_ROM_TABLE[8])

#define SetupGetTrimForDblrLoopFilterResetVoltage \
    ((uint32_t (*)(uint32_t ui32Fcfg1Revision)) \
    ROM_API_SETUP_ROM_TABLE[9])

#define SetupGetTrimForRadcExtCfg \
    ((uint32_t (*)(uint32_t ui32Fcfg1Revision)) \
    ROM_API_SETUP_ROM_TABLE[10])

#define SetupGetTrimForRcOscLfIBiasTrim \
    ((uint32_t (*)(uint32_t ui32Fcfg1Revision)) \
    ROM_API_SETUP_ROM_TABLE[11])

#define SetupGetTrimForRcOscLfRtuneCtuneTrim \
    ((uint32_t (*)(void)) \
    ROM_API_SETUP_ROM_TABLE[12])

#define SetupGetTrimForXoscHfCtl \
    ((uint32_t (*)(uint32_t ui32Fcfg1Revision)) \
    ROM_API_SETUP_ROM_TABLE[13])

#define SetupGetTrimForXoscHfFastStart \
    ((uint32_t (*)(void)) \
    ROM_API_SETUP_ROM_TABLE[14])

#define SetupGetTrimForXoscHfIbiastherm \
    ((uint32_t (*)(void)) \
    ROM_API_SETUP_ROM_TABLE[15])

#define SetupGetTrimForXoscLfRegulatorAndCmirrwrRatio \
    ((uint32_t (*)(uint32_t ui32Fcfg1Revision)) \
    ROM_API_SETUP_ROM_TABLE[16])

#define SetupSetAonRtcSubSecInc \
    ((void (*)(uint32_t subSecInc)) \
    ROM_API_SETUP_ROM_TABLE[17])

#define SetupSetCacheModeAccordingToCcfgSetting \
    ((void (*)(void)) \
    ROM_API_SETUP_ROM_TABLE[18])

#define SetupStepVddrTrimTo \
    ((void (*)(uint32_t toCode)) \
    ROM_API_SETUP_ROM_TABLE[19])

/* I2S FUNCTIONS */

#define I2SPointerSet \
    ((void (*)(uint32_t ui32Base, bool bInput, void * pNextPointer)) \
    ROM_API_I2S_TABLE[0])

#define I2SSampleStampGet \
    ((uint32_t (*)(uint32_t ui32Base, uint32_t ui32Channel)) \
    ROM_API_I2S_TABLE[1])

/* PWR_CTRL FUNCTIONS */

#define PowerCtrlSourceSet \
    ((void (*)(uint32_t ui32PowerConfig)) \
    ROM_API_PWR_CTRL_TABLE[0])

/* AES FUNCTIONS */

#define AESConfigureCCMCtrl \
    ((void (*)(uint32_t nonceLength, uint32_t macLength, bool encrypt)) \
    ROM_API_AES_TABLE[0])

#define AESReadFromKeyStore \
    ((uint32_t (*)(uint32_t keyStoreArea)) \
    ROM_API_AES_TABLE[1])

#define AESReadTag \
    ((uint32_t (*)(uint8_t *tag, uint32_t tagLength)) \
    ROM_API_AES_TABLE[2])

#define AESSetInitializationVector \
    ((void (*)(const uint32_t *initializationVector)) \
    ROM_API_AES_TABLE[3])

#define AESStartDMAOperation \
    ((void (*)(const uint8_t *channel0Addr, uint32_t channel0Length, uint8_t *channel1Addr, uint32_t channel1Length)) \
    ROM_API_AES_TABLE[4])

#define AESVerifyTag \
    ((uint32_t (*)(const uint8_t *tag, uint32_t tagLength)) \
    ROM_API_AES_TABLE[5])

#define AESWaitForIRQFlags \
    ((uint32_t (*)(uint32_t irqFlags)) \
    ROM_API_AES_TABLE[6])

#define AESWriteCCMInitializationVector \
    ((void (*)(const uint8_t *nonce, uint32_t nonceLength)) \
    ROM_API_AES_TABLE[7])

#define AESWriteToKeyStore \
    ((uint32_t (*)(const uint8_t *aesKey, uint32_t aesKeyLength, uint32_t keyStoreArea)) \
    ROM_API_AES_TABLE[8])

/* PKA FUNCTIONS */

#define PKABigNumAddGetResult \
    ((uint32_t (*)(uint8_t *resultBuf, uint32_t *resultLength, uint32_t resultPKAMemAddr)) \
    ROM_API_PKA_TABLE[0])

#define PKABigNumCmpGetResult \
    ((uint32_t (*)(void)) \
    ROM_API_PKA_TABLE[1])

#define PKABigNumInvModGetResult \
    ((uint32_t (*)(uint8_t *resultBuf, uint32_t length, uint32_t resultPKAMemAddr)) \
    ROM_API_PKA_TABLE[2])

#define PKABigNumModGetResult \
    ((uint32_t (*)(uint8_t *resultBuf, uint32_t length, uint32_t resultPKAMemAddr)) \
    ROM_API_PKA_TABLE[3])

#define PKABigNumMultGetResult \
    ((uint32_t (*)(uint8_t *resultBuf, uint32_t *resultLength, uint32_t resultPKAMemAddr)) \
    ROM_API_PKA_TABLE[4])

#define PKAEccAddGetResult \
    ((uint32_t (*)(uint8_t *curvePointX, uint8_t *curvePointY, uint32_t resultPKAMemAddr, uint32_t length)) \
    ROM_API_PKA_TABLE[5])

#define PKAEccAddStart \
    ((uint32_t (*)(const uint8_t *curvePoint1X, const uint8_t *curvePoint1Y, const uint8_t *curvePoint2X, const uint8_t *curvePoint2Y, const uint8_t *prime, const uint8_t *a, uint32_t length, uint32_t *resultPKAMemAddr)) \
    ROM_API_PKA_TABLE[6])

#define PKAEccMultiplyGetResult \
    ((uint32_t (*)(uint8_t *curvePointX, uint8_t *curvePointY, uint32_t resultPKAMemAddr, uint32_t length)) \
    ROM_API_PKA_TABLE[7])

#define PKAEccMultiplyStart \
    ((uint32_t (*)(const uint8_t *scalar, const uint8_t *curvePointX, const uint8_t *curvePointY, const uint8_t *prime, const uint8_t *a, const uint8_t *b, uint32_t length, uint32_t *resultPKAMemAddr)) \
    ROM_API_PKA_TABLE[8])

#define PKAGetOpsStatus \
    ((uint32_t (*)(void)) \
    ROM_API_PKA_TABLE[9])

#define PKABigNumAddStart \
    ((uint32_t (*)(const uint8_t *bigNum1, uint32_t bigNum1Length, const uint8_t *bigNum2, uint32_t bigNum2Length, uint32_t *resultPKAMemAddr)) \
    ROM_API_PKA_TABLE[10])

#define PKABigNumCmpStart \
    ((uint32_t (*)(const uint8_t *bigNum1, const uint8_t *bigNum2, uint32_t length)) \
    ROM_API_PKA_TABLE[11])

#define PKABigNumInvModStart \
    ((uint32_t (*)(const uint8_t *bigNum, uint32_t bigNumLength, const uint8_t *modulus, uint32_t modulusLength, uint32_t *resultPKAMemAddr)) \
    ROM_API_PKA_TABLE[12])

#define PKABigNumModStart \
    ((uint32_t (*)(const uint8_t *bigNum, uint32_t bigNumLength, const uint8_t *modulus, uint32_t modulusLength, uint32_t *resultPKAMemAddr)) \
    ROM_API_PKA_TABLE[13])

#define PKABigNumMultiplyStart \
    ((uint32_t (*)(const uint8_t *multiplicand, uint32_t multiplicandLength, const uint8_t *multiplier, uint32_t multiplierLength, uint32_t *resultPKAMemAddr)) \
    ROM_API_PKA_TABLE[14])

#define PKABigNumSubGetResult \
    ((uint32_t (*)(uint8_t *resultBuf, uint32_t *resultLength, uint32_t resultPKAMemAddr)) \
    ROM_API_PKA_TABLE[15])

#define PKABigNumSubStart \
    ((uint32_t (*)(const uint8_t *minuend, uint32_t minuendLength, const uint8_t *subtrahend, uint32_t subtrahendLength, uint32_t *resultPKAMemAddr)) \
    ROM_API_PKA_TABLE[16])

#define PKAArrayAllZeros \
    ((bool (*)(const uint8_t *array, uint32_t arrayLength)) \
    ROM_API_PKA_TABLE[17])

#define PKABigNumDivideGetQuotient \
    ((uint32_t (*)(uint8_t *resultBuf, uint32_t *length, uint32_t resultQuotientMemAddr)) \
    ROM_API_PKA_TABLE[18])

#define PKABigNumDivideGetRemainder \
    ((uint32_t (*)(uint8_t *resultBuf, uint32_t *length, uint32_t resultRemainderMemAddr)) \
    ROM_API_PKA_TABLE[19])

#define PKABigNumDivideStart \
    ((uint32_t (*)(const uint8_t *dividend, uint32_t dividendLength, const uint8_t *divisor, uint32_t divisorLength, uint32_t *resultQuotientMemAddr, uint32_t *resultRemainderMemAddr)) \
    ROM_API_PKA_TABLE[20])

#define PKAEccVerifyPublicKeyWeierstrassStart \
    ((uint32_t (*)(const uint8_t *curvePointX, const uint8_t *curvePointY, const uint8_t *prime, const uint8_t *a, const uint8_t *b, const uint8_t *order, uint32_t length)) \
    ROM_API_PKA_TABLE[21])

#define PKAZeroOutArray \
    ((void (*)(const uint8_t *array, uint32_t arrayLength)) \
    ROM_API_PKA_TABLE[22])

#define PKAEccMontgomeryMultiplyStart \
    ((uint32_t (*)(const uint8_t *scalar, const uint8_t *curvePointX, const uint8_t *prime, const uint8_t *a, uint32_t length, uint32_t *resultPKAMemAddr)) \
    ROM_API_PKA_TABLE[23])

/* SHA2 FUNCTIONS */

#define SHA2ComputeFinalHash \
    ((uint32_t (*)(const uint8_t *message, uint8_t *resultDigest, uint32_t *intermediateDigest, uint32_t totalMsgLength, uint32_t messageLength, uint32_t hashAlgorithm)) \
    ROM_API_SHA2_TABLE[0])

#define SHA2ComputeHash \
    ((uint32_t (*)(const uint8_t *message, uint8_t *resultDigest, uint32_t totalMsgLength, uint32_t hashAlgorithm)) \
    ROM_API_SHA2_TABLE[1])

#define SHA2ComputeInitialHash \
    ((uint32_t (*)(const uint8_t *message, uint32_t *intermediateDigest, uint32_t hashAlgorithm, uint32_t initialMessageLength)) \
    ROM_API_SHA2_TABLE[2])

#define SHA2ComputeIntermediateHash \
    ((uint32_t (*)(const uint8_t *message, uint32_t *intermediateDigest, uint32_t hashAlgorithm, uint32_t intermediateMessageLength)) \
    ROM_API_SHA2_TABLE[3])

#define SHA2StartDMAOperation \
    ((void (*)(uint8_t *channel0Addr, uint32_t channel0Length, uint8_t *channel1Addr, uint32_t channel1Length)) \
    ROM_API_SHA2_TABLE[4])

#define SHA2WaitForIRQFlags \
    ((uint32_t (*)(uint32_t irqFlags)) \
    ROM_API_SHA2_TABLE[5])

/************************************************************************************
 * Public Types
 ************************************************************************************/

/* ROM Hard-API function interface types */

typedef uint32_t (*fptr_crc32_t)              (uint8_t *     /* data        */,\
                                               uint32_t      /* bytecount   */,\
                                               uint32_t      /* repeatcount */);

typedef uint32_t (*fptr_getflsize_t)          (void);

typedef uint32_t (*fptr_getchipid_t)          (void);

typedef uint32_t (*fptr_reserved1_t)          (uint32_t);

typedef uint32_t (*fptr_reserved2_t)          (void);

typedef uint32_t (*fptr_reserved3_t)          (uint8_t *,\
                                               uint32_t,\
                                               uint32_t);

typedef void     (*fptr_resetdev_t)           (void);

typedef uint32_t (*fptr_fletcher32_t)         (uint16_t *    /* data        */,\
                                               uint16_t      /* wordcount   */,\
                                               uint16_t      /* repeatcount */);

typedef uint32_t (*fptr_minval_t)             (uint32_t *    /* data   */,\
                                               uint32_t      /* count  */);

typedef uint32_t (*fptr_maxval_t)             (uint32_t *    /* buffer */,\
                                               uint32_t      /* count  */);

typedef uint32_t (*fptr_meanval_t)            (uint32_t *    /* buffer */,\
                                               uint32_t      /* count  */);

typedef uint32_t (*fptr_stddval_t)            (uint32_t *    /* buffer */,\
                                               uint32_t      /* count  */);

typedef void     (*fptr_hfsourcesafeswitch_t) (void);

typedef void     (*fptr_reserved4_t)          (uint32_t);

typedef void     (*fptr_reserved5_t)          (uint32_t);

typedef void     (*fptr_compain_t)            (uint8_t       /* signal */);

typedef void     (*fptr_comparef_t)           (uint8_t       /* signal */);

typedef void     (*fptr_adccompbin_t)         (uint8_t       /* signal */);

typedef void     (*fptr_dacvref_t)            (uint8_t       /* signal */);

/* ROM Hard-API access table type */

struct hard_api_s
{
  fptr_crc32_t               crc32;
  fptr_getflsize_t           get_flashsize;
  fptr_getchipid_t           get_chipid;
  fptr_reserved1_t           reserved1;
  fptr_reserved2_t           reserved2;
  fptr_reserved3_t           reserved3;
  fptr_resetdev_t            reset_device;
  fptr_fletcher32_t          fletcher32;
  fptr_minval_t              min;
  fptr_maxval_t              max;
  fptr_meanval_t             mean;
  fptr_stddval_t             standard_devation;
  fptr_reserved4_t           reserved4;
  fptr_reserved5_t           reserved5;
  fptr_hfsourcesafeswitch_t  hf_source_safeswitch;
  fptr_compain_t             select_compa_input;
  fptr_comparef_t            select_compa_ref;
  fptr_adccompbin_t          select_adc_compb_input;
  fptr_dacvref_t             select_dac_vref;
};

typedef struct hard_api_s hard_api_t;

#endif /* __ARCH_ARM_SRC_TIVA_CC13XX_CC13X2_CC26X2_V2_ROM_H */
