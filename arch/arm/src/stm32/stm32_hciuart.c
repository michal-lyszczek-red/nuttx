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

  btuart_rxcallback_t *callback;     /* Rx callback function */
  FAR void *arg;                     /* Rx callback argument */

  /* RX DMA state */

#ifdef SERIAL_HAVE_DMA
  DMA_HANDLE rxdmastream;            /* currently-open receive DMA stream */
  bool rxenable;                     /* DMA-based reception en/disable */
  uint32_t rxdmanext;                /* Next byte in DMA buffer to be read */
#endif
};

/* This structure is the constant configuration of the HCI UART */

struct hciuart_config_s
{
  struct btuart_lowerhalf_s lower;   /* Generic HCI-UART lower half */
  FAR struct hciuart_state_s *state; /* Reference to variable state */

  FAR uint8_t *rxbuffer;             /* Rx buffer start */
  FAR uint8_t *txbuffer;             /* Tx buffer start */
#ifdef SERIAL_HAVE_DMA
  FAR uint8_t *rxdmabuffer;          /* Rx DMA buffer start */
#endif
  uint16_t rxbufsize;                /* Size of the Rx buffer */
  uint16_t txbufsize;                /* Size of the tx buffer */
#ifdef SERIAL_HAVE_DMA
  uint8_t rxdmachan;                 /* Rx DMA channel */
#endif

  uint8_t parity;                    /* 0=none, 1=odd, 2=even */
  uint8_t bits;                      /* Number of bits (7 or 8) */
  bool stopbits2;                    /* True: Configure with 2 vs. 1 stop */
  uint32_t baud;                     /* Configured baud */

  uint8_t irq;                       /* IRQ associated with this USART */
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
static void hciuart_isenabled(const struct hciuart_config_s *config,
              uint32_t intset);
#ifdef SERIAL_HAVE_DMA
static int  hciuart_dma_nextrx(const struct hciuart_config_s *config);
#endif
static void hciuart_setup(const struct hciuart_config_s *config);
#ifdef SERIAL_HAVE_DMA
static int  hciuart_dma_setup(struct btuart_lowerhalf_s *lower)
#endif
static void hciuart_apbclock(const struct hciuart_config_s *config, bool on);
static void hciuart_configure(const struct hciuart_config_s *config);
static int  hciuart_interrupt(int irq, void *context, void *arg);

/* HCI-UART Lower-Half Methods */

static void hciuart_rxattach(FAR const struct btuart_lowerhalf_s *lower,
              btuart_rxcallback_t callback, FAR void *arg);
static void hciuart_rxenable(FAR const struct btuart_lowerhalf_s *lower,
              bool enable);
static void hciuart_txenable(FAR const struct btuart_lowerhalf_s *lower,
              bool enable);
static ssize_t hciuart_read(FAR const struct btuart_lowerhalf_s *lower,
              FAR void *buffer, size_t buflen);
static ssize_t hciuart_write(FAR const struct btuart_lowerhalf_s *lower,
              FAR const void *buffer, size_t buflen);
static ssize_t hciuart_rxdrain(FAR const struct btuart_lowerhalf_s *lower);

#ifdef SERIAL_HAVE_DMA
### REVISIT:  These are DMA replaces for the old uart operations
static void up_dma_rxint(struct btuart_lowerhalf_s *lower, bool enable);
static void up_dma_rxcallback(DMA_HANDLE handle, uint8_t status, void *arg);
#endif

#ifdef CONFIG_PM
static void up_pm_notify(struct pm_callback_s *cb, int dowmin,
              enum pm_state_e pmstate);
static int  up_pm_prepare(struct pm_callback_s *cb, int domain,
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

  .parity        = CONFIG_USART1_PARITY,
  .bits          = CONFIG_USART1_BITS,
  .stopbits2     = CONFIG_USART1_2STOP,
  .baud          = CONFIG_USART1_BAUD,

  uint8_t irq;                       /* IRQ associated with this USART */
  uint32_t apbclock;                 /* PCLK 1 or 2 frequency */
  uint32_t usartbase;                /* Base address of USART registers */
  uint32_t tx_gpio;                  /* U[S]ART TX GPIO pin configuration */
  uint32_t rx_gpio;                  /* U[S]ART RX GPIO pin configuration */
  uint32_t cts_gpio;                 /* U[S]ART CTS GPIO pin configuration */
  uint32_t rts_gpio;                 /* U[S]ART RTS GPIO pin configuration */

  .irq           = STM32_IRQ_USART1,
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
#ifdef CONFIG_USART1_RXDMA
  .rxdmachan = ,
  .rxdmabuffer  = g_usart1_rxdmabuffer,
#endif
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
#ifdef CONFIG_USART1_RXDMA
  .rxdmabuffer   = g_usart2_rxdmabuffer,
#endif
  .rxbufsize     = CONFIG_USART2_RXBUFSIZE,
  .txbufsize     = CONFIG_USART2_TXBUFSIZE,
#ifdef SERIAL_HAVE_DMA
  .rxdmachan     = DMAMAP_USART2_RX,
#endif

  .parity        = CONFIG_USART2_PARITY,
  .bits          = CONFIG_USART2_BITS,
  .stopbits2     = CONFIG_USART2_2STOP,
  .baud          = CONFIG_USART2_BAUD,

  .irq           = STM32_IRQ_USART2,
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
#ifdef CONFIG_USART1_RXDMA
  .rxdmabuffer   = g_usart3_rxdmabuffer,
#endif
  .rxbufsize     = CONFIG_USART3_RXBUFSIZE,
  .txbufsize     = CONFIG_USART3_TXBUFSIZE,
#ifdef SERIAL_HAVE_DMA
  .rxdmachan     = DMAMAP_USART3_RX,
#endif

  .parity        = CONFIG_USART3_PARITY,
  .bits          = CONFIG_USART3_BITS,
  .stopbits2     = CONFIG_USART3_2STOP,
  .baud          = CONFIG_USART3_BAUD,

  .irq           = STM32_IRQ_USART3,
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
#ifdef CONFIG_USART1_RXDMA
  .rxdmabuffer   = g_usart6_rxdmabuffer,
#endif
  .rxbufsize     = CONFIG_USART6_RXBUFSIZE,
  .txbufsize     = CONFIG_USART6_TXBUFSIZE,
#ifdef SERIAL_HAVE_DMA
  .rxdmachan     = DMAMAP_USART6_RX,
#endif

  .parity        = CONFIG_USART6_PARITY,
  .bits          = CONFIG_USART6_BITS,
  .stopbits2     = CONFIG_USART6_2STOP,
  .baud          = CONFIG_USART6_BAUD,

  .irq           = STM32_IRQ_USART6,
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
#ifdef CONFIG_USART1_RXDMA
  .rxdmabuffer   = g_uart7_rxdmabuffer,
#endif
  .rxbufsize     = CONFIG_UART7_RXBUFSIZE,
  .txbufsize     = CONFIG_UART7_TXBUFSIZE,
#ifdef SERIAL_HAVE_DMA
  .rxdmachan     = DMAMAP_UART7_RX,
#endif

  .parity        = CONFIG_UART7_PARITY,
  .bits          = CONFIG_UART7_BITS,
  .stopbits2     = CONFIG_UART7_2STOP,
  .baud          = CONFIG_UART7_BAUD,

  .irq           = STM32_IRQ_UART7,
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
#ifdef CONFIG_USART1_RXDMA
  .rxdmabuffer   = g_uart8_rxdmabuffer,
#endif
  .rxbufsize     = CONFIG_UART8_RXBUFSIZE,
  .txbufsize     = CONFIG_UART8_TXBUFSIZE,
#ifdef SERIAL_HAVE_DMA
  .rxdmachan     = DMAMAP_UART8_RX,
#endif

  .parity        = CONFIG_UART8_PARITY,
  .bits          = CONFIG_UART8_BITS,
  .stopbits2     = CONFIG_UART8_2STOP,
  .baud          = CONFIG_UART8_BAUD,

  .irq           = STM32_IRQ_UART8,
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
  .notify  = up_pm_notify,
  .prepare = up_pm_prepare,
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

static void hciuart_isenabled(const struct hciuart_config_s *config,
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
 * Name: hciuart_setup
 *
 * Description:
 *   Set the serial line format and speed.
 *
 ****************************************************************************/

static void hciuart_setup(const struct hciuart_config_s *config)
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

  /* Configure parity mode */

  regval &= ~(USART_CR1_PCE | USART_CR1_PS | USART_CR1_M);

  if (config->parity == 1)       /* Odd parity */
    {
      regval |= (USART_CR1_PCE | USART_CR1_PS);
    }
  else if (config->parity == 2)  /* Even parity */
    {
      regval |= USART_CR1_PCE;
    }

  /* Configure word length (parity uses one of configured bits)
   *
   * Default: 1 start, 8 data (no parity), n stop, OR
   *          1 start, 7 data + parity, n stop
   */

  if (config->bits == 9 || (config->bits == 8 && config->parity != 0))
    {
      /* Select: 1 start, 8 data + parity, n stop, OR
       *         1 start, 9 data (no parity), n stop.
       */

      regval |= USART_CR1_M;
    }

  hciuart_putreg32(config, STM32_USART_CR1_OFFSET, regval);

  /* Configure STOP bits */

  regval = hciuart_getreg32(config, STM32_USART_CR2_OFFSET);
  regval &= ~(USART_CR2_STOP_MASK);

  if (config->stopbits2)
    {
      regval |= USART_CR2_STOP2;
    }

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
 * Name: hciuart_dma_setup
 *
 * Description:
 *   Configure the USART baud, bits, parity, etc. This method is called the
 *   first time that the serial port is opened.
 *
 ****************************************************************************/

#ifdef SERIAL_HAVE_DMA
static int hciuart_dma_setup(const struct btuart_lowerhalf_s *lower)
{
  const struct hciuart_config_s *config =
    (const struct hciuart_config_s *)lower;
  struct hciuart_state_s *state = config->state;
  int result;
  uint32_t regval;

  /* Do the basic UART setup  */

  result = hciuart_configure(config);
  if (result < 0)
    {
      return result;
    }

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

  state->rxdmanext = 0;

  /* Enable receive DMA for the UART */

  regval  = hciuart_getreg32(config, STM32_USART_CR3_OFFSET);
  regval |= USART_CR3_DMAR;
  hciuart_putreg32(config, STM32_USART_CR3_OFFSET, regval);

  /* Start the DMA channel, and arrange for callbacks at the half and
   * full points in the FIFO.  This ensures that we have half a FIFO
   * worth of time to claim bytes before they are overwritten.
   */

  stm32_dmastart(state->rxdmastream, up_dma_rxcallback, (void *)config, true);

  return OK;
}
#endif

/****************************************************************************
 * Name: hciuart_apbclock
 *
 * Description:
 *   Enable or disable APB clock for the USART peripheral
 *
 * Input Parameters:
 *   lower - A reference to the UART driver state structure
 *   on    - Enable clock if 'on' is 'true' and disable if 'false'
 *
 ****************************************************************************/

static void hciuart_apbclock(const struct hciuart_config_s *config, bool on)
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

  if (on)
    {
      modifyreg32(regaddr, 0, rcc_en);
    }
  else
    {
      modifyreg32(regaddr, rcc_en, 0);
    }
}

/****************************************************************************
 * Name: hciuart_configure
 *
 * Description:
 *   Configure the USART baud, bits, parity, etc. This method is called the
 *   first time that the serial port is opened.
 *
 ****************************************************************************/

static int hciuart_configure(const struct hciuart_config_s *config)
{
  uint32_t regval;
  uint32_t config;

  /* Note: The logic here depends on the fact that that the USART module
   * was enabled in stm32_lowsetup().
   */

  /* Enable USART APB1/2 clock */

  hciuart_apbclock(config, true);

  /* Configure pins for USART use */

  stm32_configgpio(config->tx_gpio);
  stm32_configgpio(config->rx_gpio);
  stm32_configgpio(config->cts_gpio);

  config = config->rts_gpio;

#ifdef CONFIG_STM32_FLOWCONTROL_BROKEN
  /* Use software controlled RTS flow control. Because STM current STM32
   * have broken HW based RTS behavior (they assert nRTS after every byte
   * received)  Enable this setting workaround this issue by using software
   * based management of RTS.
   */

  config = (config & ~GPIO_MODE_MASK) | GPIO_OUTPUT;
#endif
  stm32_configgpio(config);

  /* Configure CR2 */
  /* Clear STOP, CLKEN, CPOL, CPHA, LBCL, and interrupt enable bits */

  regval  = hciuart_getreg32(config, STM32_USART_CR2_OFFSET);
  regval &= ~(USART_CR2_STOP_MASK | USART_CR2_CLKEN | USART_CR2_CPOL |
              USART_CR2_CPHA | USART_CR2_LBCL | USART_CR2_LBDIE);

  /* Configure STOP bits */

  if (config->stopbits2)
    {
      regval |= USART_CR2_STOP2;
    }

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

  hciuart_setup(config);

  /* Enable Rx, Tx, and the USART */

  regval      = hciuart_getreg32(config, STM32_USART_CR1_OFFSET);
  regval     |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);
  hciuart_putreg32(config, STM32_USART_CR1_OFFSET, regval);

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
    (const struct hciuart_config_s *)lower;
  struct hciuart_state_s *state = config->state;
  uint32_t status
  int  passes;
  bool handled;

  DEBUGASSERT(config != NULL);

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

      /* Handle incoming, receive bytes. */

      if ((status & USART_SR_RXNE) != 0 &&
          (hciuart_isenabled(config, USART_CR1_RXNEIE))
        {
          uint32_t rdr;

          /* Received data ready... process incoming bytes. */

#ifdef SERIAL_HAVE_DMA
          /* Compare our receive pointer to the current DMA pointer, if they
           * do not match, then there are bytes to be received.
           */

          while ((hciuart_dma_nextrx(config) != state->rxdmanext))
            {
              int c = config->rxdmabuffer[state->rxdmanext];

              state->rxdmanext++;
              if (state->rxdmanext == RXDMA_BUFFER_SIZE)
                {
                  state->rxdmanext = 0;
                }

#warning Missing logic
            }
#else
          /* Copy data from Rx FIFO */

          while ((hciuart_getreg32(config, STM32_USART_SR_OFFSET) & USART_SR_RXNE) != 0);
            {
              rdr = hciuart_getreg32(config, STM32_USART_RDR_OFFSET) & 0xff;
#warning Missing logic
            }
#endif
          /* Copy data from the DMA buffer */

          handled = true;
        }

      /* We may still have to read from the DR register to clear any pending
       * error conditions.
       */

      else if ((status & (USART_SR_ORE | USART_SR_NE | USART_SR_FE)) != 0)
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

      /* Handle outgoing, transmit bytes */

      if ((status & USART_SR_TXE) != 0 &&
          hciuart_isenabled(config, USART_CR1_TXEIE))
        {
          /* Transmit data register empty ... process outgoing bytes */

          uart_xmitchars(&config->lower);
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

static void hciuart_rxattach(FAR const struct btuart_lowerhalf_s *lower,
                             btuart_rxcallback_t callback, FAR void *arg)
{
  const struct hciuart_config_s *config =
    (const struct hciuart_config_s *)lower;

  /* If the callback is NULL, then we are detaching */

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

  return OK;
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

static void hciuart_rxenable(FAR const struct btuart_lowerhalf_s *lower,
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
  uint32_t inset;
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

static ssize_t hciuart_read(FAR const struct btuart_lowerhalf_s *lower,
                            FAR void *buffer, size_t buflen)
{
  /* Flush data from the Rx FIFO (or DMA buffer */)
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

static ssize_t hciuart_write(FAR const struct btuart_lowerhalf_s *lower,
                             FAR const void *buffer, size_t buflen)
{
  const struct hciuart_config_s *config =
    (const struct hciuart_config_s *)lower;
  irqstate_t flags;

  /* Disable Tx interrupts while we operate on the Tx buffer */

  /* USART transmit interrupts:
   *
   * Enable             Status          Meaning                      Usage
   * ------------------ --------------- ---------------------------- ----------
   * USART_CR1_TCIE     USART_SR_TC     Transmission Complete        (used only for RS-485)
   * USART_CR1_TXEIE    USART_SR_TXE    Transmit Data Register Empty
   * USART_CR3_CTSIE    USART_SR_CTS    CTS flag                     (not used)
   */

  flags = enter_critical_section();
  hciuart_disableints(config, USART_CR1_TXEIE);
  if (enable)
    {

      /* Fake a TX interrupt here by just calling uart_xmitchars() with
       * interrupts disabled (note this may recurse).
       */

      uart_xmitchars(lower);
    }
  else
    {
      /* Disable the TX interrupt */

    }

  leave_critical_section(flags);
  /* Copy data to the Tx buffer */

  /* Wait for space to become available in the Tx buffer */

      /* Set to receive an interrupt when the TX data register is empty */

      hciuart_enableints(config, USART_CR1_TXEIE);

}

/****************************************************************************
 * Name: hciuart_rxdrain
 *
 * Description:
 *   Flush/drain all buffered RX data
 *
 ****************************************************************************/

static ssize_t hciuart_rxdrain(FAR const struct btuart_lowerhalf_s *lower)
{
  /* Stop Rx DMA */

  /* Mark the Rx buffer as empty */

  /* Reset the Rx DMA buffer */

  /* Flush data in the Rx FIFO */
}

/****************************************************************************
 * Name: up_rxflowcontrol
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

static bool up_rxflowcontrol(struct btuart_lowerhalf_s *lower,
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
  uint16_t ie;

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

      up_rxint(lower, true);
    }

  return false;
#endif
}

/****************************************************************************
 * Name: up_send
 *
 * Description:
 *   This method will send one byte on the USART
 *
 ****************************************************************************/

static void up_send(struct btuart_lowerhalf_s *lower, int ch)
{
  const struct hciuart_config_s *config =
    (const struct hciuart_config_s *)lower;
  hciuart_putreg32(config, STM32_USART_TDR_OFFSET, (uint32_t)ch);
}

/****************************************************************************
 * Name: up_txready
 *
 * Description:
 *   Return true if the transmit data register is empty
 *
 ****************************************************************************/

static bool up_txready(struct btuart_lowerhalf_s *lower)
{
  const struct hciuart_config_s *config =
    (const struct hciuart_config_s *)lower;

  return ((hciuart_getreg32(config, STM32_USART_SR_OFFSET) & USART_SR_TXE) != 0);
}

/****************************************************************************
 * Name: up_dma_rxcallback
 *
 * Description:
 *   This function checks the current DMA state and calls the generic
 *   serial stack when bytes appear to be available.
 *
 ****************************************************************************/

#ifdef SERIAL_HAVE_DMA
static void up_dma_rxcallback(DMA_HANDLE handle, uint8_t status, void *arg)
{
  const struct hciuart_config_s *config =
    (const struct hciuart_config_s *)lower;
  struct hciuart_state_s *state = config->state;

  /* Compare our receive pointer to the current DMA pointer, if they
   * do not match, then there are bytes to be received.
   */

  if (state->rxenable && hciuart_dma_nextrx(config) != state->rxdmanext)
    {
      /* Copy data from the Rx DMA buffer */
#warning Missing logic
    }
}
#endif

/****************************************************************************
 * Name: up_pm_notify
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
static void up_pm_notify(struct pm_callback_s *cb, int domain,
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
 * Name: up_pm_prepare
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
static int up_pm_prepare(struct pm_callback_s *cb, int domain,
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
 * Name: stm32_serial_get_uart
 *
 * Description:
 *   Get serial driver structure for STM32 USART
 *
 ****************************************************************************/

FAR uart_dev_t *stm32_serial_get_uart(int uart_num)
{
  int uart_idx = uart_num - 1;

  if (uart_idx < 0 || uart_idx >= STM32_NUSART || !g_hciuarts[uart_idx])
    {
      return NULL;
    }

  return &g_hciuarts[uart_idx]->lower;
}

/****************************************************************************
 * Name: hciuart_instantiate
 *
 * Description:
 *   Register serial console and serial ports.  This assumes that
 *   hciuart_initialize was called previously.
 *
 ****************************************************************************/

FAR struct btuart_lowerhalf_s *hciuart_instantiate(int uart)
{
  char devname[16];
  unsigned i;
  unsigned minor = 0;
#ifdef CONFIG_PM
  int ret;
#endif

  /* Register to receive power management callbacks */

#ifdef CONFIG_PM
  ret = pm_register(&g_serialcb);
  DEBUGASSERT(ret == OK);
  UNUSED(ret);
#endif

  /* Register all HCI-UARTs */

  strcpy(devname, "/dev/ttySx");

  for (i = 0; i < STM32_NUSART; i++)
    {
      /* Don't create a device for non-configured ports. */

      if (g_hciuarts[i] == 0)
        {
          continue;
        }

      /* Configure the UART */

      hciuart_configure(&g_hciuarts[i]);

      /* Register USARTs as devices in increasing order */

      devname[9] = '0' + minor++;
      (void)uart_register(devname, &g_hciuarts[i]->lower);
    }
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
  unsigned i;

  /* Configure all USART interrupts */

  for (i = 0; i < STM32_NUSART; i++)
    {
      if (g_hciuarts[i])
        {
          hciuart_disableints(config, HCIUART_ALLINTS);
        }

      /* Attach and enable the HCI UART IRQ */

      ret = irq_attach(config->irq, hciuart_interrupt, config);
      if (ret == OK)
        {
          /* Enable the interrupt (RX and TX interrupts are still disabled
           * in the USART)
           */

          up_enable_irq(config->irq);
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
      up_dma_rxcallback(g_hciusart1_config.state->rxdmastream, 0,
                        &g_hciusart1_config);
    }
#endif

#ifdef CONFIG_USART2_RXDMA
  if (g_hciusart2_config.state->rxdmastream != NULL)
    {
      up_dma_rxcallback(g_hciusart2_config.state->rxdmastream, 0,
                        &g_hciusart2_config);
    }
#endif

#ifdef CONFIG_USART3_RXDMA
  if (g_hciusart3_config.state->rxdmastream != NULL)
    {
      up_dma_rxcallback(g_hciusart3_config.state->rxdmastream, 0,
                        &g_hciusart3_config);
    }
#endif

#ifdef CONFIG_USART6_RXDMA
  if (g_hciusart6_config.state->rxdmastream != NULL)
    {
      up_dma_rxcallback(g_hciusart6_config.state->rxdmastream, 0,
                        &g_hciusart6_config);
    }
#endif

#ifdef CONFIG_UART7_RXDMA
  if (g_hciuart7_config.state->rxdmastream != NULL)
    {
      up_dma_rxcallback(g_hciuart7_config.state->rxdmastream, 0, &g_hciuart7_config);
    }
#endif

#ifdef CONFIG_UART8_RXDMA
  if (g_hciuart8_config.state->rxdmastream != NULL)
    {
      up_dma_rxcallback(g_hciuart8.state->rxdmastream, 0,
                        &g_hciuart8_config);
    }
#endif

  leave_critical_section(flags);
}
