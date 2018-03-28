/****************************************************************************
 * wireless/bluetooth/bt_core.h
 * Bluetooth subsystem core APIs.
 *
 *   Copyright (C) 2018 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Ported from the Intel/Zephyr arduino101_firmware_source-v1.tar package
 * where the code was released with a compatible 3-clause BSD license:
 *
 *   Copyright (c) 2016, Intel Corporation
 *   All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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
 ****************************************************************************/

#ifndef __INCLUDE_NUTTX_WIRELESS_BT_CORE_H
#define __INCLUDE_NUTTX_WIRELESS_BT_CORE_H 1

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <string.h>

#include <nuttx/wireless/bt_buf.h>
#include <nuttx/wireless/bt_hci.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* BT_ADDR_STR_LEN
 * Recommended length of user string buffer for Bluetooth address
 *
 * The recommended length guarantee the output of address conversion will not
 * lose valuable information about address being processed.
 */

#define BT_ADDR_STR_LEN 18

/* BT_ADDR_LE_STR_LEN
 * Recommended length of user string buffer for Bluetooth LE address
 *
 * The recommended length guarantee the output of address conversion will not
 * lose valuable information about address being processed.
 */

#define BT_ADDR_LE_STR_LEN 27

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Advertising API */

begin_packed_struct struct bt_eir_s
{
  uint8_t len;
  uint8_t type;
  uint8_t data[29];
} end_packed_struct;

/* Security level */

enum bt_security_e
{
  BT_SECURITY_LOW,    /* No encryption and no authentication. */
  BT_SECURITY_MEDIUM, /* encryption and no authentication (no MITM). */
  BT_SECURITY_HIGH,   /* encryption and authentication (MITM). */
  BT_SECURITY_FIPS,   /* Authenticated LE Secure Connections and
                       * encryption. */
};

/****************************************************************************
 * Name: bt_le_scan_cb_t
 *
 * Description:
 *   A function of this type will be called back when user application
 *   triggers active LE scan. The caller will populate all needed
 *   parameters based on data coming from scan result.
 *   Such function can be set by user when LE active scan API is used.
 *
 * Input Parameters:
 *  addr     - Advertiser LE address and type.
 *  rssi     - Strength of advertiser signal.
 *  adv_type - Type of advertising response from advertiser.
 *  adv_data - Address of buffer containing advertiser data.
 *  len      - Length of advertiser data contained in buffer.
 *
 ****************************************************************************/

typedef CODE void bt_le_scan_cb_t(FAR const bt_addr_le_t *addr, int8_t rssi,
                                  uint8_t adv_type,
                                  FAR const uint8_t *adv_data, uint8_t len);

/****************************************************************************
 * Inline Functions
 ****************************************************************************/

/****************************************************************************
 * Name: bt_addr_to_str
 *
 * Description:
 *   Converts binary Bluetooth address to string.
 *
 * Input Parameters:
 *   addr - Address of buffer containing binary Bluetooth address.
 *   str  - Address of user buffer with enough room to store formatted
 *          string containing binary address.
 *   len  - Length of data to be copied to user string buffer. Refer to
 *          BT_ADDR_STR_LEN about recommended value.
 *
 * Returned Value:
 *   Number of successfully formatted bytes from binary address.
 *
 ****************************************************************************/

static inline int bt_addr_to_str(FAR const bt_addr_t *addr, FAR char *str,
                                 size_t len)
{
  return snprintf(str, len, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
                  addr->val[5], addr->val[4], addr->val[3],
                  addr->val[2], addr->val[1], addr->val[0]);
}

/****************************************************************************
 * Name: bt_addr_le_to_str
 *
 * Description:
 *   Converts binary LE Bluetooth address to string.
 *
 * Input Parameters:
 *   addr     - Address of buffer containing binary LE Bluetooth address.
 *   user_buf - Address of user buffer with enough room to store
 *              formatted string containing binary LE address.
 *   len      - Length of data to be copied to user string buffer. Refer to
 *              BT_ADDR_LE_STR_LEN about recommended value.
 *
 * Returned Value:
 *   Number of successfully formatted bytes from binary address.
 *
 ****************************************************************************/

static inline int bt_addr_le_to_str(const bt_addr_le_t *addr, char *str,
            size_t len)
{
  char type[7];

  switch (addr->type)
  {
    case BT_ADDR_LE_PUBLIC:
      strcpy(type, "public");
      break;

    case BT_ADDR_LE_RANDOM:
      strcpy(type, "random");
      break;

    default:
      sprintf(type, "0x%02x", addr->type);
      break;
  }

  return snprintf(str, len, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X (%s)",
                  addr->val[5], addr->val[4], addr->val[3],
                  addr->val[2], addr->val[1], addr->val[0], type);
}

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: bt_init
 *
 * Description:
 *   Initialize Bluetooth. Must be the called before anything else.
 *   Caller shall be either task or a fiber.
 *
 * Returned Value:
 *    Zero on success or (negative) error code otherwise.
 *
 ****************************************************************************/

int bt_init(void);

/****************************************************************************
 * Name: bt_start_advertising
 *
 * Description:
 *   Set advertisement data, scan response data, advertisement parameters
 *   and start advertising.
 *
 * Input Parameters:
 *   type - Advertising type.
 *   ad   - Data to be used in advertisement packets.
 *   sd   - Data to be used in scan response packets.
 *
 * Returned Value:
 *   Zero on success or (negative) error code otherwise.
 *
 ****************************************************************************/

int bt_start_advertising(uint8_t type, FAR const struct bt_eir_s *ad,
                         FAR const struct bt_eir_s *sd);

/****************************************************************************
 * Name: bt_stop_advertising
 *
 * Description:
 *   Stops ongoing advertising.
 *
 * Returned Value:
 *   Zero on success or (negative) error code otherwise.
 *
 ****************************************************************************/

int bt_stop_advertising(void);

/****************************************************************************
 * Name: bt_start_scanning
 *
 * Description:
 *   Start LE scanning with and provide results through the specified
 *   callback.
 *
 * Input Parameters:
 *   filter_dups - Enable duplicate filtering (or not).
 *   cb          - Callback to notify scan results.
 *
 * Returned Value:
 *   Zero on success or error code otherwise, positive in case
 *   of protocol error or negative (POSIX) in case of stack internal error
 *
 ****************************************************************************/

int bt_start_scanning(uint8_t filter_dups, bt_le_scan_cb_t cb);

/****************************************************************************
 * Name: bt_stop_scanning
 *
 * Description:
 *   Stops ongoing LE scanning.
 *
 * Returned Value:
 *   Zero on success or error code otherwise, positive in case
 *   of protocol error or negative (POSIX) in case of stack internal error
 *
 ****************************************************************************/

int bt_stop_scanning(void);

#endif /* __INCLUDE_NUTTX_WIRELESS_BT_CORE_H */
