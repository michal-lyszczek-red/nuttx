/****************************************************************************
 * arch/arm/src/stm32/stm32_hciuart.c
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
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <nuttx/semaphore.h>
#include <nuttx/wireless/bt_uart.h>
#include <nuttx/power/pm.h>

#include <arch/board/board.h>

#include "chip.h"
#include "stm32_uart.h"
#include "stm32_dma.h"
#include "stm32_rcc.h"
#include "up_arch.h"
#include "up_internal.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Some sanity checks *******************************************************/
/* DMA configuration */

/* If DMA is enabled on any USART, then very that other pre-requisites
 * have also been selected.
 */

#ifdef SERIAL_HAVE_DMA

#  if defined(CONFIG_STM32_STM32F20XX) || defined(CONFIG_STM32_STM32F4XXX)
/* Verify that DMA has been enabled and the DMA channel has been defined.
 */

#    if defined(CONFIG_USART1_RXDMA) || defined(CONFIG_USART6_RXDMA)
#      ifndef CONFIG_STM32_DMA2
#        error STM32 USART1/6 receive DMA requires CONFIG_STM32_DMA2
#      endif
#    endif

#    if defined(CONFIG_USART2_RXDMA) || defined(CONFIG_USART3_RXDMA) || \
        defined(CONFIG_UART7_RXDMA) || defined(CONFIG_UART8_RXDMA)
#      ifndef CONFIG_STM32_DMA1
#        error STM32 USART2/3/4/5/7/8 receive DMA requires CONFIG_STM32_DMA1
#      endif
#    endif

/* For the F4, there are alternate DMA channels for USART1 and 6.
 * Logic in the board.h file make the DMA channel selection by defining
 * the following in the board.h file.
 */

#    if defined(CONFIG_USART1_RXDMA) && !defined(DMAMAP_USART1_RX)
#      error "USART1 DMA channel not defined (DMAMAP_USART1_RX)"
#    endif

#    if defined(CONFIG_USART2_RXDMA) && !defined(DMAMAP_USART2_RX)
#      error "USART2 DMA channel not defined (DMAMAP_USART2_RX)"
#    endif

#    if defined(CONFIG_USART3_RXDMA) && !defined(DMAMAP_USART3_RX)
#      error "USART3 DMA channel not defined (DMAMAP_USART3_RX)"
#    endif

#    if defined(CONFIG_USART6_RXDMA) && !defined(DMAMAP_USART6_RX)
#      error "USART6 DMA channel not defined (DMAMAP_USART6_RX)"
#    endif

#    if defined(CONFIG_UART7_RXDMA) && !defined(DMAMAP_UART7_RX)
#      error "UART7 DMA channel not defined (DMAMAP_UART7_RX)"
#    endif

#    if defined(CONFIG_UART8_RXDMA) && !defined(DMAMAP_UART8_RX)
#      error "UART8 DMA channel not defined (DMAMAP_UART8_RX)"
#    endif

#  elif defined(CONFIG_STM32_STM32L15XX) || defined(CONFIG_STM32_STM32F10XX) || \
        defined(CONFIG_STM32_STM32F30XX) || defined(CONFIG_STM32_STM32F33XX) || \
        defined(CONFIG_STM32_STM32F37XX)

#    if defined(CONFIG_USART1_RXDMA) || defined(CONFIG_USART2_RXDMA) || \
      defined(CONFIG_USART3_RXDMA)
#      ifndef CONFIG_STM32_DMA1
#        error STM32 USART1/2/3 receive DMA requires CONFIG_STM32_DMA1
#      endif
#    endif

/* There are no optional DMA channel assignments for the F1 */

#    define DMAMAP_USART1_RX  DMACHAN_USART1_RX
#    define DMAMAP_USART2_RX  DMACHAN_USART2_RX
#    define DMAMAP_USART3_RX  DMACHAN_USART3_RX

#  endif

/* The DMA buffer size when using RX DMA to emulate a FIFO.
 *
 * When streaming data, the generic serial layer will be called
 * every time the FIFO receives half this number of bytes.
 */
#  if !defined(CONFIG_STM32_SERIAL_RXDMA_BUFFER_SIZE)
#    define CONFIG_STM32_SERIAL_RXDMA_BUFFER_SIZE 32
#  endif
#  define RXDMA_MUTIPLE  4
#  define RXDMA_MUTIPLE_MASK  (RXDMA_MUTIPLE -1)
#  define RXDMA_BUFFER_SIZE   ((CONFIG_STM32_SERIAL_RXDMA_BUFFER_SIZE \
                                + RXDMA_MUTIPLE_MASK) \
                                & ~RXDMA_MUTIPLE_MASK)

/* DMA priority */

#  ifndef CONFIG_USART_DMAPRIO
#    if defined(CONFIG_STM32_STM32L15XX) || defined(CONFIG_STM32_STM32F10XX) || \
        defined(CONFIG_STM32_STM32F30XX) || defined(CONFIG_STM32_STM32F33XX) || \
        defined(CONFIG_STM32_STM32F37XX)
#      define CONFIG_USART_DMAPRIO  DMA_CCR_PRIMED
#    elif defined(CONFIG_STM32_STM32F20XX) || defined(CONFIG_STM32_STM32F4XXX)
#      define CONFIG_USART_DMAPRIO  DMA_SCR_PRIMED
#    else
#      error "Unknown STM32 DMA"
#    endif
#  endif
#    if defined(CONFIG_STM32_STM32L15XX) || defined(CONFIG_STM32_STM32F10XX) || \
        defined(CONFIG_STM32_STM32F30XX) || defined(CONFIG_STM32_STM32F33XX) || \
        defined(CONFIG_STM32_STM32F37XX)
#    if (CONFIG_USART_DMAPRIO & ~DMA_CCR_PL_MASK) != 0
#      error "Illegal value for CONFIG_USART_DMAPRIO"
#    endif
#  elif defined(CONFIG_STM32_STM32F20XX) || defined(CONFIG_STM32_STM32F4XXX)
#    if (CONFIG_USART_DMAPRIO & ~DMA_SCR_PL_MASK) != 0
#      error "Illegal value for CONFIG_USART_DMAPRIO"
#    endif
#  else
#    error "Unknown STM32 DMA"
#  endif

/* DMA control word */

#  if defined(CONFIG_STM32_STM32F20XX) || defined(CONFIG_STM32_STM32F4XXX)
#    define SERIAL_DMA_CONTROL_WORD      \
                (DMA_SCR_DIR_P2M       | \
                 DMA_SCR_CIRC          | \
                 DMA_SCR_MINC          | \
                 DMA_SCR_PSIZE_8BITS   | \
                 DMA_SCR_MSIZE_8BITS   | \
                 CONFIG_USART_DMAPRIO  | \
                 DMA_SCR_PBURST_SINGLE | \
                 DMA_SCR_MBURST_SINGLE)
#  else
#    define SERIAL_DMA_CONTROL_WORD      \
                (DMA_CCR_CIRC          | \
                 DMA_CCR_MINC          | \
                 DMA_CCR_PSIZE_8BITS   | \
                 DMA_CCR_MSIZE_8BITS   | \
                 CONFIG_USART_DMAPRIO)
# endif
#endif

/* All interrupts */

#define HCIUART_ALLINTS (USART_CR1_USED_INTS | USART_CR3_EIE)

/* Power management definitions */

#if defined(CONFIG_PM) && !defined(CONFIG_PM_SERIAL_ACTIVITY)
#  define CONFIG_PM_SERIAL_ACTIVITY 10
#endif
#if defined(CONFIG_PM)
#  define PM_IDLE_DOMAIN             0 /* Revisit */
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* This structure is the variable state of the HCI UART */

struct hciuart_state_s
{
  /* Registered Rx callback */

  btuart_rxcallback_t callback;      /* Rx callback function */
  void *arg;                         /* Rx callback argument */

  /* Rx/Tx circular buffer management */

  sem_t rxwait;                      /* Supports wait for more Rx data */
  sem_t txwait;                      /* Supports wait for space in Tx buffer */

  volatile uint16_t rxhead;          /* Head and tail index of the Rx buffer */
  uint16_t rxtail;
  uint16_t txhead;                   /* Head and tail index of the Tx buffer */
  volatile uint16_t txtail;
  volatile bool rxwaiting;           /* A thread is waiting for more Rx data */
  volatile bool txwaiting;           /* A thread is waiting for space in the Tx buffer */

  /* RX DMA state */

#ifdef SERIAL_HAVE_DMA
  uint16_t dmatail;                  /* Tail index of the Rx DMA buffer */
  bool rxenable;                     /* DMA-based reception en/disable */
  DMA_HANDLE rxdmastream;            /* currently-open receive DMA stream */
#endif
};

/* This structure is the constant configuration of the HCI UART */

struct hciuart_config_s
{
  struct btuart_lowerhalf_s lower;   /* Generic HCI-UART lower half */
  struct hciuart_state_s *state;     /* Reference to variable state */

  uint8_t *rxbuffer;                 /* Rx buffer start */
  uint8_t *txbuffer;                 /* Tx buffer start */
#ifdef SERIAL_HAVE_DMA
  uint8_t *rxdmabuffer;              /* Rx DMA buffer start */
#endif
  uint16_t rxbufsize;                /* Size of the Rx buffer */
  uint16_t txbufsize;                /* Size of the tx buffer */
#ifdef SERIAL_HAVE_DMA
  uint8_t rxdmachan;                 /* Rx DMA channel */
#endif

  uint8_t irq;                       /* IRQ associated with this USART */
  uint32_t baud;                     /* Configured baud */
  uint32_t apbclock;                 /* PCLK 1 or 2 frequency */
  uint32_t usartbase;                /* Base address of USART registers */
  uint32_t tx_gpio;                  /* U[S]ART TX GPIO pin configuration */
  uint32_t rx_gpio;                  /* U[S]ART RX GPIO pin configuration */
  uint32_t cts_gpio;                 /* U[S]ART CTS GPIO pin configuration */
  uint32_t rts_gpio;                 /* U[S]ART RTS GPIO pin configuration */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static inline uint32_t hciuart_getreg32(const struct hciuart_config_s *config,
              unsigned int offset);
static inline void hciuart_putreg32(const struct hciuart_config_s *config,
              unsigned int offset, uint32_t value);
static void hciuart_enableints(const struct hciuart_config_s *config,
              uint32_t intset);
static void hciuart_disableints(const struct hciuart_config_s *config,
              uint32_t intset);
static bool hciuart_isenabled(const struct hciuart_config_s *config,
              uint32_t intset);
static inline bool hciuart_rxenabled(const struct hciuart_config_s *config);
#ifdef SERIAL_HAVE_DMA
static int  hciuart_dma_nextrx(const struct hciuart_config_s *config);
#endif
static ssize_t hciuart_copytorxbuffer(const struct hciuart_config_s *config);
static bool hciuart_rxflowcontrol(struct btuart_lowerhalf_s *lower,
              unsigned int nbuffered, bool upper);
static ssize_t hciuart_copyfromrxbuffer(const struct hciuart_config_s *config,
              uint8_t *dest, size_t destlen);
static ssize_t hciuart_copytotxfifo(const struct hciuart_config_s *config);
static void hciuart_line_configure(const struct hciuart_config_s *config);
static void hciuart_apbclock_enable(const struct hciuart_config_s *config);
static int  hciuart_configure(const struct hciuart_config_s *config);
static int  hciuart_interrupt(int irq, void *context, void *arg);

/* HCI-UART Lower-Half Methods */

static void hciuart_rxattach(const struct btuart_lowerhalf_s *lower,
              btuart_rxcallback_t callback, void *arg);
static void hciuart_rxenable(const struct btuart_lowerhalf_s *lower,
              bool enable);
static ssize_t hciuart_read(const struct btuart_lowerhalf_s *lower,
              void *buffer, size_t buflen);
static ssize_t hciuart_write(const struct btuart_lowerhalf_s *lower,
              const void *buffer, size_t buflen);
static ssize_t hciuart_rxdrain(const struct btuart_lowerhalf_s *lower);

#ifdef SERIAL_HAVE_DMA
static void hciuart_dma_rxcallback(DMA_HANDLE handle, uint8_t status,
              void *arg);
#endif

#ifdef CONFIG_PM
static void hciuart_pm_notify(struct pm_callback_s *cb, int dowmin,
              enum pm_state_e pmstate);
static int  hciuart_pm_prepare(struct pm_callback_s *cb, int domain,
              enum pm_state_e pmstate);
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* This describes the state of the STM32 USART1 ports. */

#ifdef CONFIG_STM32_USART1_HCIUART
/* I/O buffers */

static uint8_t g_usart1_rxbuffer[CONFIG_USART1_RXBUFSIZE];
static uint8_t g_usart1_txbuffer[CONFIG_USART1_TXBUFSIZE];
# ifdef CONFIG_USART1_RXDMA
static uint8_t g_usart1_rxdmabuffer[RXDMA_BUFFER_SIZE];
# endif

/* HCI USART1 variable state information */

static struct hciuart_state_s g_hciusart1_state;

/* HCI USART1 constant configuration information */

static const struct hciuart_config_s g_hciusart1_config =
{
  .lower         =
    {
      .rxattach  = hciuart_rxattach,
      .rxenable  = hciuart_rxenable,
      .read      = hciuart_read,
      .write     = hciuart_write,
      .rxdrain   = hciuart_rxdrain,
    },
  .state         = &g_hciusart1_state,

  .rxbuffer      = g_usart1_rxbuffer,
  .txbuffer      = g_usart1_txbuffer,
#ifdef CONFIG_USART1_RXDMA
  .rxdmabuffer   = ,
#endif
  .rxbufsize     = CONFIG_USART1_RXBUFSIZE,
  .txbufsize     = CONFIG_USART1_TXBUFSIZE,
#ifdef SERIAL_HAVE_DMA
  .rxdmachan     = DMAMAP_USART1_RX,
#endif

  .irq           = STM32_IRQ_USART1,
  .baud          = CONFIG_USART1_BAUD,
#if defined(CONFIG_STM32_STM32F33XX)
  .apbclock      = STM32_PCLK1_FREQUENCY, /* Errata 2.5.1 */
#else
  .apbclock      = STM32_PCLK2_FREQUENCY,
#endif
  .usartbase     = STM32_USART1_BASE,
  .tx_gpio       = GPIO_USART1_TX,
  .rx_gpio       = GPIO_USART1_RX,
  .cts_gpio      = GPIO_USART1_CTS,
  .rts_gpio      = GPIO_USART1_RTS,
};
#endif

/* This describes the state of the STM32 USART2 port. */

#ifdef CONFIG_STM32_USART2_HCIUART
/* I/O buffers */

static uint8_t g_usart2_rxbuffer[CONFIG_USART2_RXBUFSIZE];
static uint8_t g_usart2_txbuffer[CONFIG_USART2_TXBUFSIZE];
# ifdef CONFIG_USART2_RXDMA
static uint8_t g_usart2_rxdmabuffer[RXDMA_BUFFER_SIZE];
# endif

/* HCI USART2 variable state information */

static struct hciuart_state_s g_hciusart2_state;

/* HCI USART2 constant configuration information */

static const struct hciuart_config_s g_hciusart2_config =
{
  .lower         =
    {
      .rxattach  = hciuart_rxattach,
      .rxenable  = hciuart_rxenable,
      .read      = hciuart_read,
      .write     = hciuart_write,
      .rxdrain   = hciuart_rxdrain,
    },
  .state         = &g_hciusart2_state,

  .rxbuffer      = g_usart2_rxbuffer,
  .txbuffer      = g_usart2_txbuffer,
#ifdef CONFIG_USART2_RXDMA
  .rxdmabuffer   = g_usart2_rxdmabuffer,
#endif
  .rxbufsize     = CONFIG_USART2_RXBUFSIZE,
  .txbufsize     = CONFIG_USART2_TXBUFSIZE,
#ifdef SERIAL_HAVE_DMA
  .rxdmachan     = DMAMAP_USART2_RX,
#endif

  .irq           = STM32_IRQ_USART2,
  .baud          = CONFIG_USART2_BAUD,
  .apbclock      = STM32_PCLK1_FREQUENCY,
  .usartbase     = STM32_USART2_BASE,
  .tx_gpio       = GPIO_USART2_TX,
  .rx_gpio       = GPIO_USART2_RX,
  .cts_gpio      = GPIO_USART2_CTS,
  .rts_gpio      = GPIO_USART2_RTS,
};
#endif

/* This describes the state of the STM32 USART3 port. */

#ifdef CONFIG_STM32_USART3_HCIUART
/* I/O buffers */

static uint8_t g_usart3_rxbuffer[CONFIG_USART3_RXBUFSIZE];
static uint8_t g_usart3_txbuffer[CONFIG_USART3_TXBUFSIZE];
#ifdef CONFIG_USART3_RXDMA
static uint8_t g_usart3_rxdmabuffer[RXDMA_BUFFER_SIZE];
#endif

/* HCI USART3 variable state information */

static struct hciuart_state_s g_hciusart3_state;

/* HCI USART3 constant configuration information */

static const struct hciuart_config_s g_hciusart3_config =
{
  .lower         =
    {
      .rxattach  = hciuart_rxattach,
      .rxenable  = hciuart_rxenable,
      .read      = hciuart_read,
      .write     = hciuart_write,
      .rxdrain   = hciuart_rxdrain,
    },
  .state         = &g_hciusart3_state,

  .rxbuffer      = g_usart3_rxbuffer,
  .txbuffer      = g_usart3_txbuffer,
#ifdef CONFIG_USART3_RXDMA
  .rxdmabuffer   = g_usart3_rxdmabuffer,
#endif
  .rxbufsize     = CONFIG_USART3_RXBUFSIZE,
  .txbufsize     = CONFIG_USART3_TXBUFSIZE,
#ifdef SERIAL_HAVE_DMA
  .rxdmachan     = DMAMAP_USART3_RX,
#endif

  .irq           = STM32_IRQ_USART3,
  .baud          = CONFIG_USART3_BAUD,
  .apbclock      = STM32_PCLK1_FREQUENCY,
  .usartbase     = STM32_USART3_BASE,
  .tx_gpio       = GPIO_USART3_TX,
  .rx_gpio       = GPIO_USART3_RX,
  .cts_gpio      = GPIO_USART3_CTS,
  .rts_gpio      = GPIO_USART3_RTS,
};
#endif

/* This describes the state of the STM32 USART6 port. */

#ifdef CONFIG_STM32_USART6_HCIUART
/* I/O buffers */

#ifdef CONFIG_STM32_USART6_HCIUART
static uint8_t g_usart6_rxbuffer[CONFIG_USART6_RXBUFSIZE];
static uint8_t g_usart6_txbuffer[CONFIG_USART6_TXBUFSIZE];
# ifdef CONFIG_USART6_RXDMA
static uint8_t g_usart6_rxdmabuffer[RXDMA_BUFFER_SIZE];
# endif
#endif

/* HCI USART6 variable state information */

static struct hciuart_state_s g_hciusart6_state;

/* HCI USART6 constant configuration information */

static const struct hciuart_config_s g_hciusart6_config =
{
  .lower         =
    {
      .rxattach  = hciuart_rxattach,
      .rxenable  = hciuart_rxenable,
      .read      = hciuart_read,
      .write     = hciuart_write,
      .rxdrain   = hciuart_rxdrain,
    },
  .state         = &g_hciusart6_state,

  .rxbuffer      = g_usart6_rxbuffer,
  .txbuffer      = g_usart6_txbuffer,
#ifdef CONFIG_USART6_RXDMA
  .rxdmabuffer   = g_usart6_rxdmabuffer,
#endif
  .rxbufsize     = CONFIG_USART6_RXBUFSIZE,
  .txbufsize     = CONFIG_USART6_TXBUFSIZE,
#ifdef SERIAL_HAVE_DMA
  .rxdmachan     = DMAMAP_USART6_RX,
#endif

  .irq           = STM32_IRQ_USART6,
  .baud          = CONFIG_USART6_BAUD,
  .apbclock      = STM32_PCLK2_FREQUENCY,
  .usartbase     = STM32_USART6_BASE,
  .tx_gpio       = GPIO_USART6_TX,
  .rx_gpio       = GPIO_USART6_RX,
  .cts_gpio      = GPIO_USART6_CTS,
  .rts_gpio      = GPIO_USART6_RTS,
};
#endif

/* This describes the state of the STM32 UART7 port. */

#ifdef CONFIG_STM32_UART7_HCIUART
/* I/O buffers */

static uint8_t g_uart7_rxbuffer[CONFIG_UART7_RXBUFSIZE];
static uint8_t g_uart7_txbuffer[CONFIG_UART7_TXBUFSIZE];
#ifdef CONFIG_UART7_RXDMA
static uint8_t g_uart7_rxdmabuffer[RXDMA_BUFFER_SIZE];
#endif

/* HCI UART7 variable state information */

static struct hciuart_state_s g_hciuart7_state;

/* HCI UART7 constant configuration information */

static const struct hciuart_config_s g_hciuart7_config =
{
  .lower         =
    {
      .rxattach  = hciuart_rxattach,
      .rxenable  = hciuart_rxenable,
      .read      = hciuart_read,
      .write     = hciuart_write,
      .rxdrain   = hciuart_rxdrain,
    },
  .state         = &g_hciuart7_state,

  .rxbuffer      = g_uart7_rxbuffer,
  .txbuffer      = g_uart7_txbuffer,
#ifdef CONFIG_UART7_RXDMA
  .rxdmabuffer   = g_uart7_rxdmabuffer,
#endif
  .rxbufsize     = CONFIG_UART7_RXBUFSIZE,
  .txbufsize     = CONFIG_UART7_TXBUFSIZE,
#ifdef SERIAL_HAVE_DMA
  .rxdmachan     = DMAMAP_UART7_RX,
#endif

  .irq           = STM32_IRQ_UART7,
  .baud          = CONFIG_UART7_BAUD,
  .apbclock      = STM32_PCLK1_FREQUENCY,
  .usartbase     = STM32_UART7_BASE,
  .tx_gpio       = GPIO_UART7_TX,
  .rx_gpio       = GPIO_UART7_RX,
  .cts_gpio      = GPIO_UART7_CTS,
  .rts_gpio      = GPIO_UART7_RTS,
};
#endif

/* This describes the state of the STM32 UART8 port. */

#ifdef CONFIG_STM32_UART8_HCIUART
/* I/O buffers */

static uint8_t g_uart8_rxbuffer[CONFIG_UART8_RXBUFSIZE];
static uint8_t g_uart8_txbuffer[CONFIG_UART8_TXBUFSIZE];
#ifdef CONFIG_UART8_RXDMA
static uint8_t g_uart8_rxdmabuffer[RXDMA_BUFFER_SIZE];
#endif

/* HCI UART8 variable state information */

static struct hciuart_state_s g_hciuart8_state;

/* HCI UART8 constant configuration information */

static const struct hciuart_config_s g_hciuart8_config =
{
  .lower         =
    {
      .rxattach  = hciuart_rxattach,
      .rxenable  = hciuart_rxenable,
      .read      = hciuart_read,
      .write     = hciuart_write,
      .rxdrain   = hciuart_rxdrain,
    },
  .state         = &g_hciuart8_state,

  .rxbuffer      = g_uart8_rxbuffer,
  .txbuffer      = g_uart8_txbuffer,
#ifdef CONFIG_UART8_RXDMA
  .rxdmabuffer   = g_uart8_rxdmabuffer,
#endif
  .rxbufsize     = CONFIG_UART8_RXBUFSIZE,
  .txbufsize     = CONFIG_UART8_TXBUFSIZE,
#ifdef SERIAL_HAVE_DMA
  .rxdmachan     = DMAMAP_UART8_RX,
#endif

  .irq           = STM32_IRQ_UART8,
  .baud          = CONFIG_UART8_BAUD,
  .apbclock      = STM32_PCLK1_FREQUENCY,
  .usartbase     = STM32_UART8_BASE,
  .tx_gpio       = GPIO_UART8_TX,
  .rx_gpio       = GPIO_UART8_RX,
  .cts_gpio      = GPIO_UART8_CTS,
  .rts_gpio      = GPIO_UART8_RTS,
};
#endif

/* This table lets us iterate over the configured USARTs */

static const struct hciuart_config_s * const g_hciuarts[STM32_NUSART] =
{
#ifdef CONFIG_STM32_USART1_HCIUART
  [0] = &g_hciusart1_config,
#endif
#ifdef CONFIG_STM32_USART2_HCIUART
  [1] = &g_hciusart2_config,
#endif
#ifdef CONFIG_STM32_USART3_HCIUART
  [2] = &g_hciusart3_config,
#endif
#ifdef CONFIG_STM32_USART6_HCIUART
  [4] = &g_hciusart6_config,
#endif
#ifdef CONFIG_STM32_UART7_HCIUART
  [5] = &g_hciuart7_config,
#endif
#ifdef CONFIG_STM32_UART8_HCIUART
  [6] = &g_hciuart8_config,
#endif
};

#ifdef CONFIG_PM
static  struct pm_callback_s g_serialcb =
{
  .notify  = hciuart_pm_notify,
  .prepare = hciuart_pm_prepare,
};
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: hciuart_getreg32
 ****************************************************************************/

static inline uint32_t hciuart_getreg32(const struct hciuart_config_s *config,
                                        unsigned int offset)
{
  return getreg32(config->usartbase + offset);
}

/****************************************************************************
 * Name: hciuart_putreg32
 ****************************************************************************/

static inline void hciuart_putreg32(const struct hciuart_config_s *config,
                                    unsigned int offset, uint32_t value)
{
  putreg32(value, config->usartbase + offset);
}

/****************************************************************************
 * Name: hciuart_enableints
 *
 * Description:
 *   Enable interrupts as specified by bits in the 'intset' argument
 *
 *   NOTE: This operation is not atomic.  This function should be called
 *   only from within a critical section.
 *
 ****************************************************************************/

static void hciuart_enableints(const struct hciuart_config_s *config,
                               uint32_t intset)
{
  uint32_t regval;

  /* And restore the interrupt state (see the interrupt enable/usage table
   * above)
   */

  regval  = hciuart_getreg32(config, STM32_USART_CR1_OFFSET);
  regval |= (intset & USART_CR1_USED_INTS);
  hciuart_putreg32(config, STM32_USART_CR1_OFFSET, regval);

  regval  = hciuart_getreg32(config, STM32_USART_CR3_OFFSET);
  regval |= (intset & USART_CR3_EIE);
  hciuart_putreg32(config, STM32_USART_CR3_OFFSET, regval);
}

/****************************************************************************
 * Name: hciuart_disableints
 *
 * Description:
 *   Disable interrupts as specified by bits in the 'intset' argument
 *
 *   NOTE: This operation is not atomic.  This function should be called
 *   only from within a critical section.
 *
 ****************************************************************************/

static void hciuart_disableints(const struct hciuart_config_s *config,
                                uint32_t intset)
{
  uint32_t regval;

  /* And restore the interrupt state (see the interrupt enable/usage table
   * above)
   */

  regval  = hciuart_getreg32(config, STM32_USART_CR1_OFFSET);
  regval &= ~(intset & USART_CR1_USED_INTS);
  hciuart_putreg32(config, STM32_USART_CR1_OFFSET, regval);

  regval  = hciuart_getreg32(config, STM32_USART_CR3_OFFSET);
  regval &= ~(intset & USART_CR3_EIE);
  hciuart_putreg32(config, STM32_USART_CR3_OFFSET, regval);
}

/****************************************************************************
 * Name: hciuart_isenabled
 *
 * Description:
 *   Return true if any any of the interrupts specified in the 'intset'
 *   argument are enabled.
 *
 ****************************************************************************/

static bool hciuart_isenabled(const struct hciuart_config_s *config,
                              uint32_t intset)
{
  uint32_t regval;

  /* And restore the interrupt state (see the interrupt enable/usage table
   * above)
   */

  regval  = hciuart_getreg32(config, STM32_USART_CR1_OFFSET);
  if ((regval & USART_CR1_USED_INTS) != 0)
    {
      return true;
    }

  regval  = hciuart_getreg32(config, STM32_USART_CR3_OFFSET);
  if ((regval & USART_CR3_EIE) != 0)
    {
      return true;
    }

  return false;
}

/****************************************************************************
 * Name: hciuart_rxenabled
 *
 * Description:
 *   Check if Rx interrupts are enabled.
 *
 ****************************************************************************/

static inline bool hciuart_rxenabled(const struct hciuart_config_s *config)
{
#ifdef SERIAL_HAVE_DMA
  const struct hciuart_config_s *state = config->state;
  return state->rxenabled;
#else
  return hciuart_isenabled(config, USART_CR1_RXNEIE);
#endif
}

/****************************************************************************
 * Name: hciuart_dma_nextrx
 *
 * Description:
 *   Returns the index into the RX FIFO where the DMA will place the next
 *   byte that it receives.
 *
 ****************************************************************************/

#ifdef SERIAL_HAVE_DMA
static int hciuart_dma_nextrx(const struct hciuart_config_s *config)
{
  struct hciuart_state_s *state = config->state;
  size_t dmaresidual;

  dmaresidual = stm32_dmaresidual(state->rxdmastream);

  return (RXDMA_BUFFER_SIZE - (int)dmaresidual);
}
#endif

/****************************************************************************
 * Name: hciuart_copytorxbuffer
 *
 * Description:
 *   Copy data to the driver Rx buffer.  The source is either the U[S]ART
 *   Rx FIFO or the Rx DMA buffer, depending upon the configuration.
 *
 ****************************************************************************/

static ssize_t hciuart_copytorxbuffer(const struct hciuart_config_s *config)
{
  struct hciuart_state_s *state;
  ssize_t nbytes = 0;
  uint16_t rxhead;
  uint16_t rxtail;
  uint16_t rxnext;
#ifdef SERIAL_HAVE_DMA
  uint16_t dmatail;
#endif
  uint8_t rxbyte;

  /* Get a copy of the rxhead and rxtail indices of the Rx buffer */

  state  = config->state;
  rxhead = state->rxhead;
  rxtail = state->rxtail;

#ifdef SERIAL_HAVE_DMA
  /* Get a copy of the dmatail index of the Rx DMA buffer */

  dmatail = state->dmatail;

  /* Compare dmatail to the current DMA pointer, if they do notmatch, then
   * there is new Rx data available in the Rx DMA buffer.
   */

  while ((hciuart_dma_nextrx(config) != dmatail))
    {
      /* Compare the Rx buffer head and tail indices.  If the incremented
       * tail index would make the Rx buffer appear empty, then we must
       * stop the copy.  If there is data pending in the Rx DMA buffer,
       * this could be very bad because a data overrun condition is likely
       * to occur.
       */

      rxnext = rxtail + 1;
      if (rxnext >= config->rxbufsize)
        {
          rxnext = 0
        }

      /* Would this make the Rx buffer appear full? */

      if (rxnext == rxhead)
        {
          /* Yes, stop the copy and update the indices */

          break;
        }

      /* Get a byte from the Rx DMA buffer */

      rxbyte = config->rxdmabuffer[dmatail];

      if (++dmatail >= RXDMA_BUFFER_SIZE)
        {
          dmatail = 0;
        }

      /* And add it to the tail of the Rx buffer */

      config->rxbuffer[rxtail] = rxbyte;
      rxtail = rxnext;
      nbytes++;
    }

  state->dmatail = dmatail;
#else
  /* Is there data available in the Rx FIFO? */

  while ((hciuart_getreg32(config, STM32_USART_SR_OFFSET) & USART_SR_RXNE) != 0)
    {
      /* Compare the Rx buffer head and tail indices.  If the incremented
       * tail index would make the Rx buffer appear empty, then we must
       * stop the copy.  If there is data pending in the Rx FIFO, this
       * could be very bad because a data overrun condition is likely to
       * occur.
       */

      rxnext = rxtail + 1;
      if (rxnext >= config->rxbufsize)
        {
          rxnext = 0;
        }

      /* Would this make the Rx buffer appear full? */

      if (rxnext == rxhead)
        {
          /* Yes, stop the copy and update the indices */

          break;
        }

      /* Get a byte from the Rx FIFO buffer */

      rxbyte = hciuart_getreg32(config, STM32_USART_RDR_OFFSET) & 0xff;

      /* And add it to the tail of the Rx buffer */

      config->rxbuffer[rxtail] = rxbyte;
      rxtail = rxnext;
      nbytes++;
    }
#endif

  /* Save the updated Rx buffer tail index */

  state->rxtail = rxtail;

  /* Notify any waiting threads that new Rx data is available */

  if (nbytes > 0 && state->rxwaiting)
    {
      state->rxwaiting = false;
      nxsem_post(&state->rxwait);
    }

  return nbytes;
}

/****************************************************************************
 * Name: hciuart_rxflowcontrol
 *
 * Description:
 *   Called when Rx buffer is full (or exceeds configured watermark levels
 *   if CONFIG_SERIAL_IFLOWCONTROL_WATERMARKS is defined).
 *   Return true if UART activated RX flow control to block more incoming
 *   data
 *
 * Input Parameters:
 *   lower     - UART device instance
 *   nbuffered - the number of characters currently buffered
 *               (if CONFIG_SERIAL_IFLOWCONTROL_WATERMARKS is
 *               not defined the value will be 0 for an empty buffer or the
 *               defined buffer size for a full buffer)
 *   upper     - true indicates the upper watermark was crossed where
 *               false indicates the lower watermark has been crossed
 *
 * Returned Value:
 *   true if RX flow control activated.
 *
 ****************************************************************************/

/* REVISIT:  Needs to be integrated into hciuart_copyfromrxbuffer() */
#warning Missing logic

static bool hciuart_rxflowcontrol(struct btuart_lowerhalf_s *lower,
                                  unsigned int nbuffered, bool upper)
{
  const struct hciuart_config_s *config =
    (const struct hciuart_config_s *)lower;

#if defined(CONFIG_STM32_FLOWCONTROL_BROKEN)
  /* Assert/de-assert nRTS set it high resume/stop sending */

  stm32_gpiowrite(config->rts_gpio, upper);

  if (upper)
    {
      /* With heavy Rx traffic, RXNE might be set and data pending.
       * Returning 'true' in such case would cause RXNE left unhandled
       * and causing interrupt storm. Sending end might be also be slow
       * to react on nRTS, and returning 'true' here would prevent
       * processing that data.
       *
       * Therefore, return 'false' so input data is still being processed
       * until sending end reacts on nRTS signal and stops sending more.
       */

      return false;
    }

  return upper;

#else
  /* Is the RX buffer full? */

  if (upper)
    {
      /* Disable Rx interrupt to prevent more data being from
       * peripheral.  When hardware RTS is enabled, this will
       * prevent more data from coming in.
       *
       * This function is only called when UART recv buffer is full,
       * that is: "lower->recv.head + 1 == lower->recv.tail".
       *
       * Logic in "uart_read" will automatically toggle Rx interrupts
       * when buffer is read empty and thus we do not have to re-
       * enable Rx interrupts.
       */

      hciuart_disableints(config, USART_CR1_RXNEIE);
      return true;
    }

  /* No.. The RX buffer is empty */

  else
    {
      /* We might leave Rx interrupt disabled if full recv buffer was
       * read empty.  Enable Rx interrupt to make sure that more input is
       * received.
       */

      hciuart_enableints(config, USART_CR1_RXNEIE);
    }

  return false;
#endif
}

/****************************************************************************
 * Name: hciuart_copyfromrxbuffer
 *
 * Description:
 *   Copy data from the driver Rx buffer to the caller provided destination
 *   buffer.
 *
 ****************************************************************************/

static ssize_t hciuart_copyfromrxbuffer(const struct hciuart_config_s *config,
                                        uint8_t *dest, size_t destlen)
{
  struct hciuart_state_s *state;
  ssize_t nbytes = 0;
  uint16_t rxhead;
  uint16_t rxtail;
  uint8_t rxbyte;

  /* Get a copy of the rxhead and rxtail indices of the Rx buffer */

  state  = config->state;
  rxhead = state->rxhead;
  rxtail = state->rxtail;

  /* Is there data available in the Rx FIFO? */

  while ((hciuart_getreg32(config, STM32_USART_SR_OFFSET) & USART_SR_RXNE) != 0)
    {
      /* Is the Rx buffer empty? Is there space in the user buffer? */

      if (rxhead == rxtail && nbytes < destlen)
        {
          /* Yes, stop the copy and update the indices */

          break;
        }

      /* Get a byte from the head of the Rx buffer */

      rxbyte = config->rxbuffer[rxhead];

      /* And add it to the caller's buffer buffer */

      dest[nbytes] = rxbyte;
      nbytes++;

      if (++rxhead >= config->rxbufsize)
        {
          rxhead = 0;
        }

      /* Check if we need to enable Rx flow control */
#warning Missing logic
    }

  /* Save the updated Rx buffer head index */

  state->rxhead = rxhead;
  return nbytes;
}

/****************************************************************************
 * Name: hciuart_copytotxfifo
 *
 * Description:
 *   Copy data from the Tx buffer to the Tx FIFO
 *
 ****************************************************************************/

static ssize_t hciuart_copytotxfifo(const struct hciuart_config_s *config)
{
  struct hciuart_state_s *state;
  ssize_t nbytes = 0;
  uint16_t txhead;
  uint16_t txtail;
  uint8_t txbyte;

  /* Get a copy of the txhead and txtail indices of the Rx buffer */

  state  = config->state;
  txhead = state->txhead;
  txtail = state->txtail;

  /* Compare the Tx buffer head and tail indices.  If the Tx buffer is
   * empty, then we finished with the copy.
   */

  while (txhead != txtail)
    {
      /* Is the transmit data register empty?
       *
       * TXE: Transmit data register empty
       *   This bit is set by hardware when the content of the TDR register has
       *   been transferred into the shift register.
       */

      if ((hciuart_getreg32(config, STM32_USART_SR_OFFSET) & USART_SR_TXE) == 0)
        {
          break;
        }

      /* Get a byte from the head of the Tx buffer */

      txbyte = config->txbuffer[txhead];
      if (++txhead >= config->txbufsize)
        {
          txhead = 0;
        }

      /* And add it to the of the Tx FIFO */

      hciuart_putreg32(config, STM32_USART_TDR_OFFSET, (uint32_t)txbyte);
      nbytes++;
    }

  return nbytes;
}

/****************************************************************************
 * Name: hciuart_line_configure
 *
 * Description:
 *   Set the serial line format and speed.
 *
 *   Per "Specification of the Bluetooth System, Wireless connections made
 *   easy, Host Controller Interface [Transport Layer]", Volume 4, Revision
 *   1.2 or later, 1 January 2006, HCI UART tranport uses these settings:
 *
 *     8 data bits, no parity, 1 stop, RTS/CTS flow control
 *
 *   BAUD and flow control response time are manufacturer specific.
 *
 ****************************************************************************/

static void hciuart_line_configure(const struct hciuart_config_s *config)
{
#if defined(CONFIG_STM32_STM32F30XX) || defined(CONFIG_STM32_STM32F33XX) || \
    defined(CONFIG_STM32_STM32F37XX)
  uint32_t usartdiv8;
#else
  uint32_t usartdiv32;
  uint32_t mantissa;
  uint32_t fraction;
#endif
  uint32_t regval;
  uint32_t brr;

  /* Load CR1 */

  regval = hciuart_getreg32(config, STM32_USART_CR1_OFFSET);

#if defined(CONFIG_STM32_STM32F30XX) || defined(CONFIG_STM32_STM32F33XX)|| \
    defined(CONFIG_STM32_STM32F37XX)
  /* This first implementation is for U[S]ARTs that support oversampling
   * by 8 in additional to the standard oversampling by 16.
   * With baud rate of fCK / Divider for oversampling by 16.
   * and baud rate of  2 * fCK / Divider for oversampling by 8
   *
   * In case of oversampling by 8, the equation is:
   *
   *   baud      = 2 * fCK / usartdiv8
   *   usartdiv8 = 2 * fCK / baud
   */

   usartdiv8 = ((config->apbclock << 1) + (config->baud >> 1)) / config->baud;

  /* Baud rate for standard USART (SPI mode included):
   *
   * In case of oversampling by 16, the equation is:
   *   baud       = fCK / usartdiv16
   *   usartdiv16 = fCK / baud
   *              = 2 * usartdiv8
   *
   * Use oversamply by 8 only if the divisor is small.  But what is small?
   */

  if (usartdiv8 > 100)
    {
      /* Use usartdiv16 */

      brr  = (usartdiv8 + 1) >> 1;

      /* Clear oversampling by 8 to enable oversampling by 16 */

      regval &= ~USART_CR1_OVER8;
    }
  else
    {
      DEBUGASSERT(usartdiv8 >= 8);

      /* Perform mysterious operations on bits 0-3 */

      brr  = ((usartdiv8 & 0xfff0) | ((usartdiv8 & 0x000f) >> 1));

      /* Set oversampling by 8 */

      regval |= USART_CR1_OVER8;
    }

#else
  /* This second implementation is for U[S]ARTs that support fractional
   * dividers.
   *
   * Configure the USART Baud Rate.  The baud rate for the receiver and
   * transmitter (Rx and Tx) are both set to the same value as programmed
   * in the Mantissa and Fraction values of USARTDIV.
   *
   *   baud     = fCK / (16 * usartdiv)
   *   usartdiv = fCK / (16 * baud)
   *
   * Where fCK is the input clock to the peripheral (PCLK1 for USART2, 3, 4, 5
   * or PCLK2 for USART1)
   *
   * First calculate (NOTE: all stand baud values are even so dividing by two
   * does not lose precision):
   *
   *   usartdiv32 = 32 * usartdiv = fCK / (baud/2)
   */

  usartdiv32 = config->apbclock / (config->baud >> 1);

  /* The mantissa part is then */

  mantissa   = usartdiv32 >> 5;

  /* The fractional remainder (with rounding) */

  fraction   = (usartdiv32 - (mantissa << 5) + 1) >> 1;

#if defined(CONFIG_STM32_STM32F4XXX)
  /* The F4 supports 8 X in oversampling additional to the
   * standard oversampling by 16.
   *
   * With baud rate of fCK / (16 * Divider) for oversampling by 16.
   * and baud rate of  fCK /  (8 * Divider) for oversampling by 8
   */

  /* Check if 8x oversampling is necessary */

  if (mantissa == 0)
    {
      regval |= USART_CR1_OVER8;

      /* Rescale the mantissa */

      mantissa = usartdiv32 >> 4;

      /* The fractional remainder (with rounding) */

      fraction = (usartdiv32 - (mantissa << 4) + 1) >> 1;
    }
  else
    {
      /* Use 16x Oversampling */

      regval &= ~USART_CR1_OVER8;
    }
#endif

  brr  = mantissa << USART_BRR_MANT_SHIFT;
  brr |= fraction << USART_BRR_FRAC_SHIFT;
#endif

  hciuart_putreg32(config, STM32_USART_CR1_OFFSET, regval);
  hciuart_putreg32(config, STM32_USART_BRR_OFFSET, brr);

  /* Configure parity mode
   *
   * HCI UART spec requires:  No parity
   */

  regval &= ~(USART_CR1_PCE | USART_CR1_PS | USART_CR1_M);

  /* Configure word length
   *
   * HCI UART spec requires:  8 data bits
   */

  regval |= USART_CR1_M;
  hciuart_putreg32(config, STM32_USART_CR1_OFFSET, regval);

  /* Configure STOP bits
   *
   * HCI UART spec requires:  1 stop bit
   */

  regval = hciuart_getreg32(config, STM32_USART_CR2_OFFSET);
  regval &= ~(USART_CR2_STOP_MASK);
  hciuart_putreg32(config, STM32_USART_CR2_OFFSET, regval);

  /* Configure hardware flow control */

  regval  = hciuart_getreg32(config, STM32_USART_CR3_OFFSET);
  regval &= ~(USART_CR3_CTSE | USART_CR3_RTSE);

  /* Use software controlled RTS flow control. Because STM current STM32
   * have broken HW based RTS behavior (they assert nRTS after every byte
   * received)  Enable this setting workaround this issue by using software
   * based management of RTS.
   */

#if !defined(CONFIG_STM32_FLOWCONTROL_BROKEN)
  regval |= USART_CR3_RTSE;
#endif
  regval |= USART_CR3_CTSE;

  hciuart_putreg32(config, STM32_USART_CR3_OFFSET, regval);
}

/****************************************************************************
 * Name: hciuart_apbclock_enable
 *
 * Description:
 *   Enable or disable APB clock for the USART peripheral
 *
 * Input Parameters:
 *   lower - A reference to the UART driver state structure
 *   on    - Enable clock if 'on' is 'true' and disable if 'false'
 *
 ****************************************************************************/

static void hciuart_apbclock_enable(const struct hciuart_config_s *config)
{
  uint32_t rcc_en;
  uint32_t regaddr;

  /* Determine which USART to configure */

  switch (config->usartbase)
    {
#ifdef CONFIG_STM32_USART1_HCIUART
    case STM32_USART1_BASE:
      rcc_en = RCC_APB2ENR_USART1EN;
      regaddr = STM32_RCC_APB2ENR;
      break;
#endif

#ifdef CONFIG_STM32_USART2_HCIUART
    case STM32_USART2_BASE:
      rcc_en = RCC_APB1ENR_USART2EN;
      regaddr = STM32_RCC_APB1ENR;
      break;
#endif

#ifdef CONFIG_STM32_USART3_HCIUART
    case STM32_USART3_BASE:
      rcc_en = RCC_APB1ENR_USART3EN;
      regaddr = STM32_RCC_APB1ENR;
      break;
#endif

#ifdef CONFIG_STM32_USART6_HCIUART
    case STM32_USART6_BASE:
      rcc_en = RCC_APB2ENR_USART6EN;
      regaddr = STM32_RCC_APB2ENR;
      break;
#endif

#ifdef CONFIG_STM32_UART7_HCIUART
    case STM32_UART7_BASE:
      rcc_en = RCC_APB1ENR_UART7EN;
      regaddr = STM32_RCC_APB1ENR;
      break;
#endif

#ifdef CONFIG_STM32_UART8_HCIUART
    case STM32_UART8_BASE:
      rcc_en = RCC_APB1ENR_UART8EN;
      regaddr = STM32_RCC_APB1ENR;
      break;
#endif

    default:
      return;
    }

  /* Enable/disable APB 1/2 clock for USART */

  modifyreg32(regaddr, 0, rcc_en);
}

/****************************************************************************
 * Name: hciuart_configure
 *
 * Description:
 *   Configure the USART clocking, GPIO pins, baud, bits, parity, etc.
 *
 *   Per "Specification of the Bluetooth System, Wireless connections made
 *   easy, Host Controller Interface [Transport Layer]", Volume 4, Revision
 *   1.2 or later, 1 January 2006, HCI UART tranport uses these settings:
 *
 *     8 data bits, no parity, 1 stop, RTS/CTS flow control
 *
 *   BAUD and flow control response time are manufacturer specific.
 *
 ****************************************************************************/

static int hciuart_configure(const struct hciuart_config_s *config)
{
  uint32_t regval;
  uint32_t pinset;

  /* Note: The logic here depends on the fact that that the USART module
   * was enabled in stm32_lowsetup().
   */

  /* Enable USART APB1/2 clock */

  hciuart_apbclock_enable(config);

  /* Configure pins for USART use */

  stm32_configgpio(config->tx_gpio);
  stm32_configgpio(config->rx_gpio);
  stm32_configgpio(config->cts_gpio);

  pinset = config->rts_gpio;

#ifdef CONFIG_STM32_FLOWCONTROL_BROKEN
  /* Use software controlled RTS flow control. Because STM current STM32
   * have broken HW based RTS behavior (they assert nRTS after every byte
   * received)  Enable this setting workaround this issue by using software
   * based management of RTS.
   */

  pinset = (config & ~GPIO_MODE_MASK) | GPIO_OUTPUT;
#endif
  stm32_configgpio(pinset);

  /* Configure CR2 */
  /* Clear STOP, CLKEN, CPOL, CPHA, LBCL, and interrupt enable bits */
  /* HCI UART spec:  1 stop bit */

  regval  = hciuart_getreg32(config, STM32_USART_CR2_OFFSET);
  regval &= ~(USART_CR2_STOP_MASK | USART_CR2_CLKEN | USART_CR2_CPOL |
              USART_CR2_CPHA | USART_CR2_LBCL | USART_CR2_LBDIE);
  hciuart_putreg32(config, STM32_USART_CR2_OFFSET, regval);

  /* Configure CR1 */
  /* Clear TE, REm and all interrupt enable bits */

  regval  = hciuart_getreg32(config, STM32_USART_CR1_OFFSET);
  regval &= ~(USART_CR1_TE | USART_CR1_RE | USART_CR1_ALLINTS);

  hciuart_putreg32(config, STM32_USART_CR1_OFFSET, regval);

  /* Configure CR3 */
  /* Clear CTSE, RTSE, and all interrupt enable bits */

  regval  = hciuart_getreg32(config, STM32_USART_CR3_OFFSET);
  regval &= ~(USART_CR3_CTSIE | USART_CR3_CTSE | USART_CR3_RTSE | USART_CR3_EIE);

  hciuart_putreg32(config, STM32_USART_CR3_OFFSET, regval);

  /* Configure the USART line format and speed. */

  hciuart_line_configure(config);

  /* Enable Rx, Tx, and the USART */

  regval      = hciuart_getreg32(config, STM32_USART_CR1_OFFSET);
  regval     |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);
  hciuart_putreg32(config, STM32_USART_CR1_OFFSET, regval);

#ifdef SERIAL_HAVE_DMA
  /* Acquire the DMA channel.  This should always succeed. */

  state->rxdmastream = stm32_dmachannel(config->rxdmachan);

  /* Configure for circular DMA reception into the RX fifo */

  stm32_dmasetup(state->rxdmastream,
                 config->usartbase + STM32_USART_RDR_OFFSET,
                 (uint32_t)config->rxdmabuffer,
                 RXDMA_BUFFER_SIZE,
                 SERIAL_DMA_CONTROL_WORD);

  /* Reset our DMA shadow pointer to match the address just
   * programmed above.
   */

  state->dmatail = 0;

  /* Enable receive DMA for the UART */

  regval  = hciuart_getreg32(config, STM32_USART_CR3_OFFSET);
  regval |= USART_CR3_DMAR;
  hciuart_putreg32(config, STM32_USART_CR3_OFFSET, regval);

  /* Start the DMA channel, and arrange for callbacks at the half and
   * full points in the FIFO.  This ensures that we have half a FIFO
   * worth of time to claim bytes before they are overwritten.
   */

  stm32_dmastart(state->rxdmastream, hciuart_dma_rxcallback,
                 (void *)config, true);
#endif

  return OK;
}

/****************************************************************************
 * Name: hciuart_interrupt
 *
 * Description:
 *   This is the USART interrupt callback.  It will be invoked when an
 *   interrupt received on the 'irq'  It should call uart_transmitchars or
 *   uart_receivechar to perform the appropriate data transfers.  The
 *   interrupt handling logic must be able to map the 'irq' number into the
 *   appropriate btuart_lowerhalf_s structure in order to call these functions.
 *
 ****************************************************************************/

static int hciuart_interrupt(int irq, void *context, void *arg)
{
  const struct hciuart_config_s *config =
    (const struct hciuart_config_s *)arg;
  struct hciuart_state_s *state;
  uint32_t status;
  int  passes;
  bool handled;

  DEBUGASSERT(config != NULL && config->state != NULL);
  state = config->state;

  /* Report serial activity to the power management logic */

#if defined(CONFIG_PM) && CONFIG_PM_SERIAL_ACTIVITY > 0
  pm_activity(PM_IDLE_DOMAIN, CONFIG_PM_SERIAL_ACTIVITY);
#endif

  /* Loop until there are no characters to be transferred or,
   * until we have been looping for a long time.
   */

  handled = true;
  for (passes = 0; passes < 256 && handled; passes++)
    {
      handled = false;

      /* Get the masked USART status word. */

      status = hciuart_getreg32(config, STM32_USART_SR_OFFSET);

      /* USART interrupts:
       *
       * Enable             Status          Meaning                         Usage
       * ------------------ --------------- ------------------------------- ----------
       * USART_CR1_IDLEIE   USART_SR_IDLE   Idle Line Detected              (not used)
       * USART_CR1_RXNEIE   USART_SR_RXNE   Received Data Ready to be Read
       * "              "   USART_SR_ORE    Overrun Error Detected
       * USART_CR1_TCIE     USART_SR_TC     Transmission Complete           (used only for RS-485)
       * USART_CR1_TXEIE    USART_SR_TXE    Transmit Data Register Empty
       * USART_CR1_PEIE     USART_SR_PE     Parity Error
       *
       * USART_CR2_LBDIE    USART_SR_LBD    Break Flag                      (not used)
       * USART_CR3_EIE      USART_SR_FE     Framing Error
       * "           "      USART_SR_NE     Noise Error
       * "           "      USART_SR_ORE    Overrun Error Detected
       * USART_CR3_CTSIE    USART_SR_CTS    CTS flag                        (not used)
       *
       * NOTE: Some of these status bits must be cleared by explicitly writing zero
       * to the SR register: USART_SR_CTS, USART_SR_LBD. Note of those are currently
       * being used.
       */

#ifndef SERIAL_HAVE_DMA
      /* There should never be an Rx FIFO interrupt in this configuration */

      DEBUGASSERT((status & USART_SR_RXNE) == 0 || !hciuart_rxenabled(config))
#else
      /* Handle incoming, receive bytes. */

      if ((status & USART_SR_RXNE) != 0 && hciuart_rxenabled(config))
        {
          ssize_t nbytes;

          /* Received data ready... copy data from the Rx FIFO to the Rx
           * buffer.
           */

          nbytes = hciuart_copytorxbuffer(config);
          UNUSED(nbytes);

          /* Is there anything in the Rx buffer?  Has the user registered an
           * Rx callback function?
           */

          if (state->rxhead != state->rxtail && state->callback != NULL)
            {
              state->callback(config->lower, state->arg);
              handled = true;
            }
        }
      else
#endif

      /* We may still have to read from the DR register to clear any pending
       * error conditions.
       */

      if ((status & (USART_SR_ORE | USART_SR_NE | USART_SR_FE)) != 0)
        {
#if defined(CONFIG_STM32_STM32F30XX) || defined(CONFIG_STM32_STM32F33XX) || \
    defined(CONFIG_STM32_STM32F37XX)
          /* These errors are cleared by writing the corresponding bit to the
           * interrupt clear register (ICR).
           */

          hciuart_putreg32(config, STM32_USART_ICR_OFFSET,
                      (USART_ICR_NCF | USART_ICR_ORECF | USART_ICR_FECF));
#else
          /* If an error occurs, read from DR to clear the error (data has
           * been lost).  If ORE is set along with RXNE then it tells you
           * that the byte *after* the one in the data register has been
           * lost, but the data register value is correct.  That case will
           * be handled above if interrupts are enabled. Otherwise, that
           * good byte will be lost.
           */

          (void)hciuart_getreg32(config, STM32_USART_RDR_OFFSET);
#endif
        }

      /* Handle outgoing, transmit bytes
       *
       * TXE: Transmit data register empty
       *   This bit is set by hardware when the content of the TDR register has
       *   been transferred into the shift register.
       */

      if ((status & USART_SR_TXE) != 0 &&
          hciuart_isenabled(config, USART_CR1_TXEIE))
        {
          ssize_t nbytes;

          /* Transmit data register empty ... copy data from the Tx buffer
           * to the Tx FIFO.
           */

          nbytes = hciuart_copytotxfifo(config);
          UNUSED(nbytes);

          /* This copy will free up space in the Tx FIFO.  Wake up any
           * threads that may have been waiting for space in the Tx
           * buffer.
           */

          if (state->txwaiting)
            {
              state->txwaiting = false;
              nxsem_post(&state->txwait);
            }

          handled = true;
        }
    }

  return OK;
}

/****************************************************************************
 * Name: hciuart_rxattach
 *
 * Description:
 *   Attach/detach the upper half Rx callback.
 *
 *   rxattach() allows the upper half logic to attach a callback function
 *   that will be used to inform the upper half that an Rx frame is
 *   available.  This callback will, most likely, be invoked in the
 *   context of an interrupt callback.  The receive() method should then
 *   be invoked in order to receive the obtain the Rx frame data.
 *
 ****************************************************************************/

static void hciuart_rxattach(const struct btuart_lowerhalf_s *lower,
                             btuart_rxcallback_t callback, void *arg)
{
  const struct hciuart_config_s *config =
    (const struct hciuart_config_s *)lower;
  struct hciuart_state_s *state;
  irqstate_t flags;

  DEBUGASSERT(config != NULL && config->state != NULL);
  state = config->state;

  /* If the callback is NULL, then we are detaching */

  flags = enter_critical_section();
  if (callback == NULL)
    {
      uint32_t intset;

      /* Disable Rx callbacks and detach the Rx callback */

      intset = USART_CR1_RXNEIE | USART_CR1_PEIE | USART_CR3_EIE;
      hciuart_disableints(config, intset);

      state->callback = NULL;
      state->arg      = NULL;
    }

  /* Otherwise, we are attaching */

  else
    {
      state->callback = NULL;
      state->arg      = arg;
      state->callback = callback;
    }

  leave_critical_section(flags);
}

/****************************************************************************
 * Name: hciuart_rxenable
 *
 * Description:
 *   Enable/disable RX callbacks from the HCI UART.
 *
 *   hciuart_rxenable() may be used to enable or disable callback events.
 *   This probably translates to enabling and disabled Rx interrupts at
 *   the UART.  NOTE:  Rx event notification should be done sparingly:
 *   Rx data overrun may occur when Rx events are disabled!
 *
 ****************************************************************************/

static void hciuart_rxenable(const struct btuart_lowerhalf_s *lower,
                             bool enable)
{
  const struct hciuart_config_s *config =
    (const struct hciuart_config_s *)lower;

#ifdef SERIAL_HAVE_DMA
  struct hciuart_state_s *state = config->state;

  /* En/disable DMA reception.
   *
   * Note that it is not safe to check for available bytes and immediately
   * pass them to uart_recvchars as that could potentially recurse back
   * to us again.  Instead, bytes must wait until the next up_dma_poll or
   * DMA event.
   */

  state->rxenable = enable;
#else
  uint32_t intset;
  irqstate_t flags;

  /* USART receive interrupts:
   *
   * Enable             Status          Meaning                         Usage
   * ------------------ --------------- ------------------------------- ----------
   * USART_CR1_IDLEIE   USART_SR_IDLE   Idle Line Detected              (not used)
   * USART_CR1_RXNEIE   USART_SR_RXNE   Received Data Ready to be Read
   * "              "   USART_SR_ORE    Overrun Error Detected
   * USART_CR1_PEIE     USART_SR_PE     Parity Error
   *
   * USART_CR2_LBDIE    USART_SR_LBD    Break Flag                      (not used)
   * USART_CR3_EIE      USART_SR_FE     Framing Error
   * "           "      USART_SR_NE     Noise Error
   * "           "      USART_SR_ORE    Overrun Error Detected
   */

  flags = enter_critical_section();
  if (enable)
    {
      /* Receive an interrupt when their is anything in the Rx data register (or an Rx
       * timeout occurs).
       */

#ifdef CONFIG_USART_ERRINTS
      intset = USART_CR1_RXNEIE | USART_CR1_PEIE | USART_CR3_EIE;
#else
      intset = USART_CR1_RXNEIE;
#endif
      hciuart_enableints(config, intset);
    }
  else
    {
      intset = USART_CR1_RXNEIE | USART_CR1_PEIE | USART_CR3_EIE;
      hciuart_disableints(config, intset);
    }

  leave_critical_section(flags);
#endif
}

/****************************************************************************
 * Name: hciuart_read
 *
 * Description:
 *   Read UART data.
 *
 *   hciuart_read() after receipt of a callback notifying the upper half of
 *   the availability of Rx frame, the upper half may call the receive()
 *   method in order to obtain the buffered Rx frame data.
 *
 ****************************************************************************/

static ssize_t hciuart_read(const struct btuart_lowerhalf_s *lower,
                            void *buffer, size_t buflen)
{
  const struct hciuart_config_s *config =
    (const struct hciuart_config_s *)lower;
  struct hciuart_state_s *state = config->state;
  uint8_t *dest;
  size_t remaining;
  size_t ntotal;
  ssize_t nbytes;
  bool rxenable;

  /* NOTE:  This assumes that the caller has exclusive access to the Rx
   * buffer, i.e., one lower half instance can server only one upper half!
   */

  DEBUGASSERT(config != NULL && config->state != NULL);
  state = config->state;

  /* Read any pending data to the Rx buffer */

  nbytes = hciuart_copytorxbuffer(config);
  UNUSED(nbytes);

  /* Loop copying data to the user buffer while the Rx buffer is not empty
   * and the callers buffer is not full.
   */

  dest      = (uint8_t *)buffer;
  remaining = buflen;
  ntotal    = 0;

  rxenable  = hciuart_rxenabled(config);
  hciuart_rxenable(lower, false);

  while (state->rxtail != state->rxhead && ntotal < buflen)
    {
      nbytes = hciuart_copyfromrxbuffer(config, dest, remaining);
      if (nbytes <= 0)
        {
          DEBUGASSERT(nbytes == 0);

          /* If no data has been received, then we must wait for the arrival
           * of new Rx data and try again.
           */

          if (ntotal == 0)
            {
              DEBUGASSERT(!state->rxwaiting);
              state->rxwaiting = true;
              do
                {
                  int ret = nxsem_wait(&state->rxwait);
                  DEBUGASSERT(ret == 0 || ret == -EINTR);
                  UNUSED(ret);
                }
              while (state->rxwaiting);
            }

          /* Otherwise, this must be the end of the packet.  Just break out
           * and return what we have.
           */

          else
            {
              break;
            }
        }
      else
        {
          /* More data has been copied.  Update pointers, counts, and
           * indices.
           */

          ntotal    += nbytes;
          dest      += nbytes;
          remaining -= nbytes;

          /* Read any additional pending data into the Rx buffer that may
           * have accumulated while we were copying.
           */

          nbytes = hciuart_copytorxbuffer(config);
          if (nbytes < 0)
            {
              /* An error occurred.. this should not really happen */

              return nbytes;
            }

          /* Otherwise, continue looping */
        }
    }

  hciuart_rxenable(lower, rxenable);
  return ntotal;
}

/****************************************************************************
 * Name: hciuart_write
 *
 * Description:
 *   Write UART data.
 *
 *   hciuart_write() will add the outgoing frame to the Tx buffer and will
 *   return immediately.  This function may block only in the event that
 *   there is insufficient buffer space to hold the Tx frame data.  In that
 *   case the lower half will block until there is sufficient to buffer
 *   the entire outgoing packet.
 *
 ****************************************************************************/

static ssize_t hciuart_write(const struct btuart_lowerhalf_s *lower,
                             const void *buffer, size_t buflen)
{
  const struct hciuart_config_s *config =
    (const struct hciuart_config_s *)lower;
  struct hciuart_state_s *state;
  const uint8_t *src;
  ssize_t nbytes = 0;
  uint16_t txhead;
  uint16_t txtail;
  uint16_t txnext;
  size_t remaining;
  irqstate_t flags;

  DEBUGASSERT(config != NULL && config->state != NULL);
  state = config->state;

  /* NOTE:  This assumes that the caller has exclusive access to the Tx
   * buffer, i.e., one lower half instance can server only one upper half!
   */

  /* Make sure that the Tx Interrupts are disabled.
   * USART transmit interrupts:
   *
   * Enable             Status          Meaning                      Usage
   * ------------------ --------------- ---------------------------- ----------
   * USART_CR1_TCIE     USART_SR_TC     Transmission Complete        (used only for RS-485)
   * USART_CR1_TXEIE    USART_SR_TXE    Transmit Data Register Empty
   * USART_CR3_CTSIE    USART_SR_CTS    CTS flag                     (not used)
   */

  flags = enter_critical_section();
  hciuart_disableints(config, USART_CR1_TXEIE);
  leave_critical_section(flags);

   /* Loop until all of the user data have been moved to the Tx buffer */

   src       = buffer;
   remaining = buflen;

  while (remaining > 0)
    {
      /* Copy bytes to the tail of the Tx buffer */
      /* Get a copy of the rxhead and rxtail indices of the Tx buffer */

      txhead = state->txhead;
      txtail = state->txtail;

      txnext = txtail + 1;
      if (txnext >= config->txbufsize)
        {
          txnext = 0;
        }

      /* Is there space available in the Tx buffer? Do have more bytes to
       * copy?
       */

      while (txhead != txnext && remaining > 0)
        {
          /* Yes.. copy one byte to the Tx buffer */

          config->txbuffer[txtail] = *src++;

          txtail = txnext;
          if (++txnext >= config->txbufsize)
            {
              txnext = 0;
            }

          remaining--;
        }

      /* Save the updated Tx buffer tail index */

      state->txtail = txtail;

      /* Copy bytes from the Tx buffer to the Tx FIFO */

      nbytes = hciuart_copytotxfifo(config);

      /* If nothing could be copied to the Tx FIFO and we still have user
       * data that we have not added to the Tx buffer, then we must wait for
       * space in the Tx* buffer then try again.
       */

      if (nbytes <= 0 && remaining > 0)
        {
          DEBUGASSERT(nbytes == 0);

          /* Enable the Tx interrupt and wait for space open up in the Tx
           * buffer.
           */

          flags = enter_critical_section();
          hciuart_enableints(config, USART_CR1_TXEIE);

          DEBUGASSERT(!state->rxwaiting);
          state->rxwaiting = true;
          do
            {
              int ret = nxsem_wait(&state->rxwait);
              DEBUGASSERT(ret == 0 || ret == -EINTR);
              UNUSED(ret);
            }
          while (state->rxwaiting);

          /* Disable Tx interrupts again */

          hciuart_disableints(config, USART_CR1_TXEIE);
          leave_critical_section(flags);
        }
    }

  /* If the Tx buffer is not empty, then exit with the Tx interrupts enabled. */

  if (state->txhead != state->txtail)
    {
      flags = enter_critical_section();
      hciuart_disableints(config, USART_CR1_TXEIE);
      leave_critical_section(flags);
    }

  return buflen;
}

/****************************************************************************
 * Name: hciuart_rxdrain
 *
 * Description:
 *   Flush/drain all buffered RX data
 *
 ****************************************************************************/

static ssize_t hciuart_rxdrain(const struct btuart_lowerhalf_s *lower)
{
  const struct hciuart_config_s *config =
    (const struct hciuart_config_s *)lower;
  struct hciuart_state_s *state = config->state;
  size_t ntotal;
  ssize_t nbytes;
  bool rxenable;

  DEBUGASSERT(config != NULL && config->state != NULL);
  state = config->state;

  /* Read any pending data to the Rx buffer */

  nbytes = hciuart_copytorxbuffer(config);
  UNUSED(nbytes);

  /* Loop discarding in the Rx buffer until the Rx buffer is empty */

  ntotal    = 0;

  rxenable  = hciuart_rxenabled(config);
  hciuart_rxenable(lower, false);

  while (state->rxtail != state->rxhead)
    {
      size_t ndiscard;

      /* Keep track of how much is discarded */

      if (state->rxtail >= state->rxhead)
        {
          ndiscard = state->rxtail - state->rxhead;
        }
      else
        {
          ndiscard = (state->rxtail + config->rxbufsize) - state->rxhead;
        }

      ntotal += ndiscard;

      /* Discard the data in the Rx buffer */

      state->rxhead = 0;
      state->rxtail = 0;

      /* Read any additional pending data into the Rx buffer that may
       * have accumulated while we were discarding.
       */

      nbytes = hciuart_copytorxbuffer(config);
      UNUSED(nbytes);
    }

  hciuart_rxenable(lower, rxenable);
  return ntotal;
}

/****************************************************************************
 * Name: hciuart_dma_rxcallback
 *
 * Description:
 *   This function checks the current DMA state and calls the generic
 *   serial stack when bytes appear to be available.
 *
 ****************************************************************************/

#ifdef SERIAL_HAVE_DMA
static void hciuart_dma_rxcallback(DMA_HANDLE handle, uint8_t status,
                                   void *arg)
{
  const struct hciuart_config_s *config =
    (const struct hciuart_config_s *)lower;
  struct hciuart_state_s *state = config->state;
  ssize_t nbytes;

  DEBUGASSERT(config != NULL && config->state != NULL);
  state = config->state;

  /* Received data ready... copy and data from the Rx DMA buffer to the Rx
   * buffer.
   */

  nbytes = hciuart_copytorxbuffer(config);
  UNUSED(nbytes);

  /* Is there anything in the Rx buffer?  Has the user registered an Rx
   * callback function?
   */

  if (state->rxhead != state->rxtail && state->callback != NULL)
    {
      state->callback(config->lower, state->arg);
      handled = true;
    }
}
#endif

/****************************************************************************
 * Name: hciuart_pm_notify
 *
 * Description:
 *   Notify the driver of new power state. This callback is  called after
 *   all drivers have had the opportunity to prepare for the new power state.
 *
 * Input Parameters:
 *
 *    cb - Returned to the driver. The driver version of the callback
 *         structure may include additional, driver-specific state data at
 *         the end of the structure.
 *
 *    pmstate - Identifies the new PM state
 *
 * Returned Value:
 *   None - The driver already agreed to transition to the low power
 *   consumption state when when it returned OK to the prepare() call.
 *
 *
 ****************************************************************************/

#ifdef CONFIG_PM
static void hciuart_pm_notify(struct pm_callback_s *cb, int domain,
                              enum pm_state_e pmstate)
{
  switch (pmstate)
    {
      case(PM_NORMAL):
        {
          /* Logic for PM_NORMAL goes here */

        }
        break;

      case(PM_IDLE):
        {
          /* Logic for PM_IDLE goes here */

        }
        break;

      case(PM_STANDBY):
        {
          /* Logic for PM_STANDBY goes here */

        }
        break;

      case(PM_SLEEP):
        {
          /* Logic for PM_SLEEP goes here */

        }
        break;

      default:
        /* Should not get here */

        break;
    }
}
#endif

/****************************************************************************
 * Name: hciuart_pm_prepare
 *
 * Description:
 *   Request the driver to prepare for a new power state. This is a warning
 *   that the system is about to enter into a new power state. The driver
 *   should begin whatever operations that may be required to enter power
 *   state. The driver may abort the state change mode by returning a
 *   non-zero value from the callback function.
 *
 * Input Parameters:
 *
 *    cb - Returned to the driver. The driver version of the callback
 *         structure may include additional, driver-specific state data at
 *         the end of the structure.
 *
 *    pmstate - Identifies the new PM state
 *
 * Returned Value:
 *   Zero - (OK) means the event was successfully processed and that the
 *          driver is prepared for the PM state change.
 *
 *   Non-zero - means that the driver is not prepared to perform the tasks
 *              needed achieve this power setting and will cause the state
 *              change to be aborted. NOTE: The prepare() method will also
 *              be called when reverting from lower back to higher power
 *              consumption modes (say because another driver refused a
 *              lower power state change). Drivers are not permitted to
 *              return non-zero values when reverting back to higher power
 *              consumption modes!
 *
 *
 ****************************************************************************/

#ifdef CONFIG_PM
static int hciuart_pm_prepare(struct pm_callback_s *cb, int domain,
                              enum pm_state_e pmstate)
{
  /* Logic to prepare for a reduced power state goes here. */

  return OK;
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: hciuart_instantiate
 *
 * Description:
 *   Register serial console and serial ports.  This assumes that
 *   hciuart_initialize was called previously.
 *
 ****************************************************************************/

const struct btuart_lowerhalf_s *hciuart_instantiate(int uart)
{
  const struct hciuart_config_s *config;
#ifdef CONFIG_PM
  int ret;
#endif

  /* Check if this uart is available in the configuration */

  config = g_hciuarts[uart];
  if (config == NULL)
    {
      wlerr("ERROR: UART%d not configured\n", uart + 1);
      return NULL;
    }

  /* Register to receive power management callbacks */

#ifdef CONFIG_PM
  ret = pm_register(&g_serialcb);
  DEBUGASSERT(ret == OK);
  UNUSED(ret);
#endif

  /* Configure the UART */

  hciuart_configure(config);
  return &config->lower;
}

/****************************************************************************
 * Name: hciuart_initialize
 *
 * Description:
 *   Performs the low level USART initialization early in debug so that the
 *   serial console will be available during bootup.  This must be called
 *   after hciuart_instantiate.
 *
 ****************************************************************************/

void hciuart_initialize(void)
{
  const struct hciuart_config_s *config;
  struct hciuart_state_s *state;
  int ret;
  int i;

  /* Configure all USARTs */

  for (i = 0; i < STM32_NUSART; i++)
    {
      config = g_hciuarts[i];
      if (config != NULL)
        {
          state = config->state;

          /* Disable U[S]ART interrupts */

          hciuart_disableints(config, HCIUART_ALLINTS);

          /* Initialize signalling semaphores */

          nxsem_init(&state->rxwait, 0, 0);
          nxsem_setprotocol(&state->rxwait, SEM_PRIO_NONE);

          nxsem_init(&state->txwait, 0, 0);
          nxsem_setprotocol(&state->txwait, SEM_PRIO_NONE);

          /* Attach and enable the HCI UART IRQ */

          ret = irq_attach(config->irq, hciuart_interrupt, (void *)config);
          if (ret == OK)
            {
              /* Enable the interrupt (RX and TX interrupts are still disabled
               * in the USART)
               */

              up_enable_irq(config->irq);
            }
        }
    }
}

/****************************************************************************
 * Name: stm32_serial_dma_poll
 *
 * Description:
 *   Checks receive DMA buffers for received bytes that have not accumulated
 *   to the point where the DMA half/full interrupt has triggered.
 *
 *   This function should be called from a timer or other periodic context.
 *
 ****************************************************************************/

#ifdef SERIAL_HAVE_DMA
void stm32_serial_dma_poll(void)
{
  irqstate_t flags;

  flags = enter_critical_section();

#ifdef CONFIG_USART1_RXDMA
  if (g_hciusart1_config.state->rxdmastream != NULL)
    {
      hciuart_dma_rxcallback(g_hciusart1_config.state->rxdmastream, 0,
                        &g_hciusart1_config);
    }
#endif

#ifdef CONFIG_USART2_RXDMA
  if (g_hciusart2_config.state->rxdmastream != NULL)
    {
      hciuart_dma_rxcallback(g_hciusart2_config.state->rxdmastream, 0,
                        &g_hciusart2_config);
    }
#endif

#ifdef CONFIG_USART3_RXDMA
  if (g_hciusart3_config.state->rxdmastream != NULL)
    {
      hciuart_dma_rxcallback(g_hciusart3_config.state->rxdmastream, 0,
                        &g_hciusart3_config);
    }
#endif

#ifdef CONFIG_USART6_RXDMA
  if (g_hciusart6_config.state->rxdmastream != NULL)
    {
      hciuart_dma_rxcallback(g_hciusart6_config.state->rxdmastream, 0,
                        &g_hciusart6_config);
    }
#endif

#ifdef CONFIG_UART7_RXDMA
  if (g_hciuart7_config.state->rxdmastream != NULL)
    {
      hciuart_dma_rxcallback(g_hciuart7_config.state->rxdmastream, 0, &g_hciuart7_config);
    }
#endif

#ifdef CONFIG_UART8_RXDMA
  if (g_hciuart8_config.state->rxdmastream != NULL)
    {
      hciuart_dma_rxcallback(g_hciuart8.state->rxdmastream, 0,
                        &g_hciuart8_config);
    }
#endif

  leave_critical_section(flags);
}
#endif
