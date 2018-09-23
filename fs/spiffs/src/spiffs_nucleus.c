/****************************************************************************
 * fs/spiffs.h/spiffs_nucleus.c
 *
 *   Copyright (C) 2018 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * This is a port of version 0.3.7 of SPIFFS by Peter Andersion.  That
 * version was originally released under the MIT license but is here re-
 * released under the NuttX BSD license.
 *
 *   Copyright (c) 2013-2017 Peter Andersson (pelleplutt1976@gmail.com)
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

#include <nuttx/kmalloc.h>

#include "spiffs.h"
#include "spiffs_nucleus.h"

/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef struct
{
  FAR struct spiffs_file_s *fobj;
  uint32_t remaining_objndx_pages_to_visit;
  int16_t map_objndx_start_spndx;
  int16_t map_objndx_end_spndx;
} spiffs_ix_map_populate_state;

typedef struct
{
  int16_t min_obj_id;
  int16_t max_obj_id;
  uint32_t compaction;
  const uint8_t *conflicting_name;
} spiffs_free_obj_id_state;


/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int32_t spiffs_page_data_check(FAR struct spiffs_s *fs,
                                      FAR struct spiffs_file_s *fobj,
                                      int16_t pgndx, int16_t spndx)
{
  int32_t ret = OK;

  if (pgndx == (int16_t) - 1)
    {
      /* referring to page 0xffff...., bad object index */

      return SPIFFS_ERR_INDEX_REF_FREE;
    }

  if (pgndx % SPIFFS_PAGES_PER_BLOCK(fs) < SPIFFS_OBJ_LOOKUP_PAGES(fs))
    {
      /* referring to an object lookup page, bad object index */

      return SPIFFS_ERR_INDEX_REF_LU;
    }

  if (pgndx > SPIFFS_MAX_PAGES(fs))
    {
      /* referring to a bad page */

      return SPIFFS_ERR_INDEX_REF_INVALID;
    }

#if SPIFFS_PAGE_CHECK
  struct spiffs_page_header_s ph;
  ret = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_READ,
                   fobj->objid, SPIFFS_PAGE_TO_PADDR(fs, pgndx),
                   sizeof(struct spiffs_page_header_s), (uint8_t *) & ph);
  SPIFFS_CHECK_RES(ret);
  SPIFFS_VALIDATE_DATA(ph, fobj->objid & ~SPIFFS_OBJ_ID_IX_FLAG, spndx);
#endif
  return ret;
}

static int32_t spiffs_page_index_check(FAR struct spiffs_s *fs, FAR struct spiffs_file_s *fobj,
                                       int16_t pgndx, int16_t spndx)
{
  int32_t ret = OK;
  if (pgndx == (int16_t) - 1)
    {
      /* referring to page 0xffff...., bad object index */

      return SPIFFS_ERR_INDEX_FREE;
    }

  if (pgndx % SPIFFS_PAGES_PER_BLOCK(fs) < SPIFFS_OBJ_LOOKUP_PAGES(fs))
    {
      /* referring to an object lookup page, bad object index */

      return SPIFFS_ERR_INDEX_LU;
    }

  if (pgndx > SPIFFS_MAX_PAGES(fs))
    {
      /* referring to a bad page */

      return SPIFFS_ERR_INDEX_INVALID;
    }

#if SPIFFS_PAGE_CHECK
  struct spiffs_page_header_s ph;
  ret = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_IX | SPIFFS_OP_C_READ,
                   fobj->objid, SPIFFS_PAGE_TO_PADDR(fs, pgndx),
                   sizeof(struct spiffs_page_header_s), (uint8_t *) & ph);
  SPIFFS_CHECK_RES(ret);
  SPIFFS_VALIDATE_OBJIX(ph, fobj->objid, spndx);
#endif

  return ret;
}

int32_t spiffs_phys_cpy(FAR struct spiffs_s *fs,
                      int16_t objid, uint32_t dst, uint32_t src, uint32_t len)
{
  int32_t ret;
  uint8_t b[SPIFFS_COPY_BUFFER_STACK];

  while (len > 0)
    {
      uint32_t chunk_size = MIN(SPIFFS_COPY_BUFFER_STACK, len);
      ret =
        _spiffs_rd(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_MOVS, objid, src,
                   chunk_size, b);
      SPIFFS_CHECK_RES(ret);
      ret =
        _spiffs_wr(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_MOVD, objid, dst,
                   chunk_size, b);
      SPIFFS_CHECK_RES(ret);
      len -= chunk_size;
      src += chunk_size;
      dst += chunk_size;
    }

  return OK;
}

/* Find object lookup entry containing given objid with visitor.
 * Iterate over object lookup pages in each block until a given object objid entry is found.
 * When found, the visitor function is called with block index, entry index and user data.
 * If visitor returns SPIFFS_VIS_CONTINUE, the search goes on. Otherwise, the search will be
 * ended and visitor's return code is returned to caller.
 * If no visitor is given (0) the search returns on first entry with matching object objid.
 * If no match is found in all look up, SPIFFS_VIS_END is returned.
 * @param fs                    the file system
 * @param starting_block        the starting block to start search in
 * @param starting_lu_entry     the look up index entry to start search in
 * @param flags                 ored combination of SPIFFS_VIS_CHECK_ID, SPIFFS_VIS_CHECK_PH,
 *                              SPIFFS_VIS_NO_WRAP
 * @param objid                 argument object ID
 * @param v                     visitor callback function
 * @param user_const_p          any const pointer, passed to the callback visitor function
 * @param user_var_p            any pointer, passed to the callback visitor function
 * @param block_ix              reported block index where match was found
 * @param lu_entry              reported look up index where match was found
 */

int32_t spiffs_foreach_objlu(FAR struct spiffs_s *fs,
                                         int16_t starting_block,
                                         int starting_lu_entry,
                                         uint8_t flags,
                                         int16_t objid,
                                         spiffs_visitor_f v,
                                         const void *user_const_p,
                                         void *user_var_p,
                                         int16_t *block_ix,
                                         int *lu_entry)
{
  int32_t ret = OK;
  int32_t entry_count = fs->geo.neraseblocks * SPIFFS_OBJ_LOOKUP_MAX_ENTRIES(fs);
  int16_t cur_block = starting_block;
  uint32_t cur_block_addr = starting_block * SPIFFS_CFG_LOG_BLOCK_SZ(fs);
  int16_t *obj_lu_buf = (int16_t *) fs->lu_work;
  int cur_entry = starting_lu_entry;
  int entries_per_page = (SPIFFS_CFG_LOG_PAGE_SZ(fs) / sizeof(int16_t));

  /* wrap initial */

  if (cur_entry > (int)SPIFFS_OBJ_LOOKUP_MAX_ENTRIES(fs) - 1)
    {
      cur_entry = 0;
      cur_block++;
      cur_block_addr = cur_block * SPIFFS_CFG_LOG_BLOCK_SZ(fs);
      if (cur_block >= fs->geo.neraseblocks)
        {
          if (flags & SPIFFS_VIS_NO_WRAP)
            {
              return SPIFFS_VIS_END;
            }
          else
            {
              /* block wrap */

              cur_block = 0;
              cur_block_addr = 0;
            }
        }
    }

  /* check each block */

  while (ret == OK && entry_count > 0)
    {
      int obj_lookup_page = cur_entry / entries_per_page;

      /* check each object lookup page */

      while (ret == OK &&
             obj_lookup_page < (int)SPIFFS_OBJ_LOOKUP_PAGES(fs))
        {
          int entry_offset = obj_lookup_page * entries_per_page;
          ret = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU | SPIFFS_OP_C_READ,
                           0, cur_block_addr + SPIFFS_PAGE_TO_PADDR(fs,
                                                                    obj_lookup_page),
                           SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->lu_work);

          /* check each entry */

          while (ret == OK && cur_entry - entry_offset < entries_per_page &&     /* for 
                                                                                  * non-last 
                                                                                  * obj 
                                                                                  * lookup 
                                                                                  * pages */
                 cur_entry < (int)SPIFFS_OBJ_LOOKUP_MAX_ENTRIES(fs))    /* for
                                                                         * last 
                                                                         * obj
                                                                         * lookup 
                                                                         * page */
            {
              if ((flags & SPIFFS_VIS_CHECK_ID) == 0 ||
                  obj_lu_buf[cur_entry - entry_offset] == objid)
                {
                  if (block_ix)
                    {
                      *block_ix = cur_block;
                    }

                  if (lu_entry)
                    {
                      *lu_entry = cur_entry;
                    }

                  if (v)
                    {
                      ret = v(fs,
                              (flags & SPIFFS_VIS_CHECK_PH) ? objid :
                              obj_lu_buf[cur_entry - entry_offset], cur_block,
                              cur_entry, user_const_p, user_var_p);
                      if (ret == SPIFFS_VIS_COUNTINUE ||
                          ret == SPIFFS_VIS_COUNTINUE_RELOAD)
                        {
                          if (ret == SPIFFS_VIS_COUNTINUE_RELOAD)
                            {
                              ret =
                                _spiffs_rd(fs,
                                           SPIFFS_OP_T_OBJ_LU |
                                           SPIFFS_OP_C_READ, 0,
                                           cur_block_addr +
                                           SPIFFS_PAGE_TO_PADDR(fs,
                                                                obj_lookup_page),
                                           SPIFFS_CFG_LOG_PAGE_SZ(fs),
                                           fs->lu_work);
                              SPIFFS_CHECK_RES(ret);
                            }

                          ret = OK;
                          cur_entry++;
                          entry_count--;
                          continue;
                        }
                      else
                        {
                          return ret;
                        }
                    }
                  else
                    {
                      return OK;
                    }
                }

              entry_count--;
              cur_entry++;
            }

          obj_lookup_page++;
        }

      cur_entry = 0;
      cur_block++;
      cur_block_addr += SPIFFS_CFG_LOG_BLOCK_SZ(fs);

      if (cur_block >= fs->geo.neraseblocks)
        {
          if (flags & SPIFFS_VIS_NO_WRAP)
            {
              return SPIFFS_VIS_END;
            }
          else
            {
              /* block wrap */

              cur_block = 0;
              cur_block_addr = 0;
            }
        }
    }

  SPIFFS_CHECK_RES(ret);
  return SPIFFS_VIS_END;
}

int32_t spiffs_erase_block(FAR struct spiffs_s *fs, int16_t blkndx)
{
  int32_t ret;
  uint32_t addr = SPIFFS_BLOCK_TO_PADDR(fs, blkndx);
  int32_t size = SPIFFS_CFG_LOG_BLOCK_SZ(fs);

  /* here we ignore ret, just try erasing the block */

  while (size > 0)
    {
      finfo("erase " _SPIPRIad ":" _SPIPRIi "\n", addr,
                 SPIFFS_CFG_PHYS_ERASE_SZ(fs));
      SPIFFS_HAL_ERASE(fs, addr, SPIFFS_CFG_PHYS_ERASE_SZ(fs));

      addr += SPIFFS_CFG_PHYS_ERASE_SZ(fs);
      size -= SPIFFS_CFG_PHYS_ERASE_SZ(fs);
    }

  fs->free_blocks++;

  /* register erase count for this block */

  ret = _spiffs_wr(fs, SPIFFS_OP_C_WRTHRU | SPIFFS_OP_T_OBJ_LU2, 0,
                   SPIFFS_ERASE_COUNT_PADDR(fs, blkndx),
                   sizeof(int16_t), (uint8_t *) & fs->max_erase_count);
  SPIFFS_CHECK_RES(ret);

#if SPIFFS_USE_MAGIC
  /* finally, write magic */

  int16_t magic = SPIFFS_MAGIC(fs, blkndx);
  ret = _spiffs_wr(fs, SPIFFS_OP_C_WRTHRU | SPIFFS_OP_T_OBJ_LU2, 0,
                   SPIFFS_MAGIC_PADDR(fs, blkndx),
                   sizeof(int16_t), (uint8_t *) & magic);
  SPIFFS_CHECK_RES(ret);
#endif

  fs->max_erase_count++;
  if (fs->max_erase_count == SPIFFS_OBJ_ID_IX_FLAG)
    {
      fs->max_erase_count = 0;
    }

  return ret;
}

#if SPIFFS_USE_MAGIC && SPIFFS_USE_MAGIC_LENGTH
int32_t spiffs_probe(FAR struct spiffs_config_s *cfg)
{
  int32_t ret;
  uint32_t paddr;
  spiffs dummy_fs;              /* create a dummy fs struct just to be able to
                                 * use macros */
  memcpy(&dummy_fs.cfg, cfg, sizeof(struct spiffs_config_s));
  dummy_fs.geo.neraseblocks = 0;

  /* Read three magics, as one block may be in an aborted erase state.
   * At least two of these must contain magic and be in decreasing order.
   */

  int16_t magic[3];
  int16_t blkndx_count[3];

  int16_t blkndx;
  for (blkndx = 0; blkndx < 3; blkndx++)
    {
      paddr = SPIFFS_MAGIC_PADDR(&dummy_fs, blkndx);
      cfg->hal_read_f(paddr, sizeof(int16_t), (uint8_t *) & magic[blkndx]);
      blkndx_count[blkndx] = magic[blkndx] ^ SPIFFS_MAGIC(&dummy_fs, 0);
      SPIFFS_CHECK_RES(ret);
    }

  /* check that we have sane number of blocks */

  if (blkndx_count[0] < 3)
    {
      return SPIFFS_ERR_PROBE_TOO_FEW_BLOCKS;
    }

  /* check that the order is correct, take aborted erases in calculation
   * first block aborted erase
   */

  if (magic[0] == (int16_t) (-1) && blkndx_count[1] - blkndx_count[2] == 1)
    {
      return (blkndx_count[1] + 1) * cfg->log_block_size;
    }

  /* second block aborted erase */

  if (magic[1] == (int16_t) (-1) && blkndx_count[0] - blkndx_count[2] == 2)
    {
      return blkndx_count[0] * cfg->log_block_size;
    }

  /* third block aborted erase */

  if (magic[2] == (int16_t) (-1) && blkndx_count[0] - blkndx_count[1] == 1)
    {
      return blkndx_count[0] * cfg->log_block_size;
    }

  /* no block has aborted erase */

  if (blkndx_count[0] - blkndx_count[1] == 1 && blkndx_count[1] - blkndx_count[2] == 1)
    {
      return blkndx_count[0] * cfg->log_block_size;
    }

  return SPIFFS_ERR_PROBE_NOT_A_FS;
}
#endif /* SPIFFS_USE_MAGIC && SPIFFS_USE_MAGIC_LENGTH */

static int32_t spiffs_obj_lu_scan_v(FAR struct spiffs_s *fs,
                                    int16_t objid,
                                    int16_t blkndx,
                                    int ix_entry,
                                    const void *user_const_p, void *user_var_p)
{
  if (objid == SPIFFS_OBJ_ID_FREE)
    {
      if (ix_entry == 0)
        {
          /* todo optimize further, return SPIFFS_NEXT_BLOCK */

          fs->free_blocks++;
        }
    }
  else if (objid == SPIFFS_OBJ_ID_DELETED)
    {
      fs->stats_p_deleted++;
    }
  else
    {
      fs->stats_p_allocated++;
    }

  return SPIFFS_VIS_COUNTINUE;
}

/* Scans thru all obj lu and counts free, deleted and used pages
 * Find the maximum block erase count
 * Checks magic if enabled
 */

int32_t spiffs_obj_lu_scan(FAR struct spiffs_s *fs)
{
  int32_t ret;
  int16_t blkndx;
  int entry;
#if SPIFFS_USE_MAGIC
  int16_t unerased_blkndx = (int16_t) - 1;
#endif

  /* find out erase count.  if enabled, check magic */

  blkndx = 0;
  int16_t erase_count_final;
  int16_t erase_count_min = SPIFFS_OBJ_ID_FREE;
  int16_t erase_count_max = 0;

  while (blkndx < fs->geo.neraseblocks)
    {
#if SPIFFS_USE_MAGIC
      int16_t magic;
      ret = _spiffs_rd(fs,
                       SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ,
                       0, SPIFFS_MAGIC_PADDR(fs, blkndx),
                       sizeof(int16_t), (uint8_t *) & magic);

      SPIFFS_CHECK_RES(ret);
      if (magic != SPIFFS_MAGIC(fs, blkndx))
        {
          if (unerased_blkndx == (int16_t) - 1)
            {
              /* allow one unerased block as it might be powered down during an 
               * erase
               */

              unerased_blkndx = blkndx;
            }
          else
            {
              /* more than one unerased block, bail out */

              SPIFFS_CHECK_RES(SPIFFS_ERR_NOT_A_FS);
            }
        }
#endif
      int16_t erase_count;
      ret = _spiffs_rd(fs,
                       SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ,
                       0, SPIFFS_ERASE_COUNT_PADDR(fs, blkndx),
                       sizeof(int16_t), (uint8_t *) & erase_count);
      SPIFFS_CHECK_RES(ret);
      if (erase_count != SPIFFS_OBJ_ID_FREE)
        {
          erase_count_min = MIN(erase_count_min, erase_count);
          erase_count_max = MAX(erase_count_max, erase_count);
        }

      blkndx++;
    }

  if (erase_count_min == 0 && erase_count_max == SPIFFS_OBJ_ID_FREE)
    {
      /* clean system, set counter to zero */

      erase_count_final = 0;
    }
  else if (erase_count_max - erase_count_min > (SPIFFS_OBJ_ID_FREE) / 2)
    {
      /* wrap, take min */

      erase_count_final = erase_count_min + 1;
    }
  else
    {
      erase_count_final = erase_count_max + 1;
    }

  fs->max_erase_count = erase_count_final;

#if SPIFFS_USE_MAGIC
  if (unerased_blkndx != (int16_t) - 1)
    {
      /* found one unerased block, remedy */

      finfo("mount: erase block " _SPIPRIbl "\n", blkndx);
      ret = spiffs_erase_block(fs, unerased_blkndx);
      SPIFFS_CHECK_RES(ret);
    }
#endif

  /* count blocks */

  fs->free_blocks = 0;
  fs->stats_p_allocated = 0;
  fs->stats_p_deleted = 0;

  ret = spiffs_foreach_objlu(fs,
                                         0,
                                         0,
                                         0,
                                         0,
                                         spiffs_obj_lu_scan_v,
                                         0, 0, &blkndx, &entry);

  if (ret == SPIFFS_VIS_END)
    {
      ret = OK;
    }

  SPIFFS_CHECK_RES(ret);
  return ret;
}

/* Find free object lookup entry.  Iterate over object lookup pages in each
 * block until a free object ID entry is found
 */

int32_t spiffs_obj_lu_find_free(FAR struct spiffs_s *fs,
                                int16_t starting_block,
                                int starting_lu_entry,
                                int16_t *block_ix, int *lu_entry)
{
  int32_t ret;

  if (!fs->cleaning && fs->free_blocks < 2)
    {
      ret = spiffs_gc_quick(fs, 0);
      if (ret == SPIFFS_ERR_NO_DELETED_BLOCKS)
        {
          ret = OK;
        }

      SPIFFS_CHECK_RES(ret);

      if (fs->free_blocks < 2)
        {
          return -ENOSPC;
        }
    }

  ret = spiffs_obj_lu_find_id(fs, starting_block, starting_lu_entry,
                              SPIFFS_OBJ_ID_FREE, block_ix, lu_entry);
  if (ret == OK)
    {
      fs->free_blkndx = *block_ix;
      fs->free_entry = (*lu_entry) + 1;
      if (*lu_entry == 0)
        {
          fs->free_blocks--;
        }
    }

  if (ret == -ENOSPC)
    {
      finfo("fs full\n");
    }

  return ret;
}

/* Find object lookup entry containing given objid.  Iterate over object lookup
 * pages in each block until a given object objid entry is found
 */

int32_t spiffs_obj_lu_find_id(FAR struct spiffs_s *fs,
                              int16_t starting_block,
                              int starting_lu_entry,
                              int16_t objid,
                              int16_t *block_ix, int *lu_entry)
{
  int32_t ret =
    spiffs_foreach_objlu(fs, starting_block, starting_lu_entry,
                                     SPIFFS_VIS_CHECK_ID, objid, 0, 0, 0,
                                     block_ix, lu_entry);
  if (ret == SPIFFS_VIS_END)
    {
      ret = -ENOENT;
    }

  return ret;
}

static int32_t spiffs_obj_lu_find_id_and_span_v(FAR struct spiffs_s *fs,
                                                int16_t objid,
                                                int16_t blkndx,
                                                int ix_entry,
                                                const void *user_const_p,
                                                void *user_var_p)
{
  int32_t ret;
  struct spiffs_page_header_s ph;
  int16_t pgndx = SPIFFS_OBJ_LOOKUP_ENTRY_TO_PIX(fs, blkndx, ix_entry);
  ret = _spiffs_rd(fs, 0, SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ,
                   SPIFFS_PAGE_TO_PADDR(fs, pgndx), sizeof(struct spiffs_page_header_s),
                   (uint8_t *) & ph);

  SPIFFS_CHECK_RES(ret);

  if (ph.objid == objid &&
      ph.span_ix == *((int16_t *) user_var_p) &&
      (ph.
       flags & (SPIFFS_PH_FLAG_FINAL | SPIFFS_PH_FLAG_DELET |
                SPIFFS_PH_FLAG_USED)) == SPIFFS_PH_FLAG_DELET &&
      !((objid & SPIFFS_OBJ_ID_IX_FLAG) &&
        (ph.flags & SPIFFS_PH_FLAG_IXDELE) == 0 && ph.span_ix == 0) &&
      (user_const_p == 0 || *((const int16_t *)user_const_p) != pgndx))
    {
      return OK;
    }
  else
    {
      return SPIFFS_VIS_COUNTINUE;
    }
}

/* Find object lookup entry containing given objid and span index.  Iterate over
 * object lookup pages in each block until a given object objid entry is found
 */

int32_t spiffs_obj_lu_find_id_and_span(FAR struct spiffs_s *fs,
                                       int16_t objid,
                                       int16_t spndx,
                                       int16_t exclusion_pgndx,
                                       int16_t *pgndx)
{
  int32_t ret;
  int16_t blkndx;
  int entry;

  ret = spiffs_foreach_objlu(fs,
                                         fs->lu_blkndx,
                                         fs->lu_entry,
                                         SPIFFS_VIS_CHECK_ID,
                                         objid,
                                         spiffs_obj_lu_find_id_and_span_v,
                                         exclusion_pgndx ? &exclusion_pgndx : 0,
                                         &spndx, &blkndx, &entry);

  if (ret == SPIFFS_VIS_END)
    {
      ret = -ENOENT;
    }

  SPIFFS_CHECK_RES(ret);

  if (pgndx)
    {
      *pgndx = SPIFFS_OBJ_LOOKUP_ENTRY_TO_PIX(fs, blkndx, entry);
    }

  fs->lu_blkndx = blkndx;
  fs->lu_entry = entry;

  return ret;
}

/* Find object lookup entry containing given objid and span index in page headers only
 * Iterate over object lookup pages in each block until a given object objid entry is found
 */

int32_t spiffs_obj_lu_find_id_and_span_byphdr(FAR struct spiffs_s *fs,
                                             int16_t objid,
                                             int16_t spndx,
                                             int16_t exclusion_pgndx,
                                             int16_t *pgndx)
{
  int32_t ret;
  int16_t blkndx;
  int entry;

  ret = spiffs_foreach_objlu(fs,
                                         fs->lu_blkndx,
                                         fs->lu_entry,
                                         SPIFFS_VIS_CHECK_PH,
                                         objid,
                                         spiffs_obj_lu_find_id_and_span_v,
                                         exclusion_pgndx ? &exclusion_pgndx : 0,
                                         &spndx, &blkndx, &entry);

  if (ret == SPIFFS_VIS_END)
    {
      ret = -ENOENT;
    }

  if (ret < 0)
    {
      return ret;
    }

  if (pgndx)
    {
      *pgndx = SPIFFS_OBJ_LOOKUP_ENTRY_TO_PIX(fs, blkndx, entry);
    }

  fs->lu_blkndx = blkndx;
  fs->lu_entry = entry;
  return ret;
}

/* Allocates a free defined page with given objid
 * Occupies object lookup entry and page
 * data may be NULL; where only page header is stored, len and page_offs is ignored
 */

int32_t spiffs_page_allocate_data(FAR struct spiffs_s *fs,
                                int16_t objid,
                                struct spiffs_page_header_s *ph,
                                uint8_t *data,
                                uint32_t len,
                                uint32_t page_offs,
                                uint8_t finalize, int16_t *pgndx)
{
  int32_t ret = OK;
  int16_t blkndx;
  int entry;

  /* find free entry */

  ret =
    spiffs_obj_lu_find_free(fs, fs->free_blkndx,
                            fs->free_entry, &blkndx, &entry);
  SPIFFS_CHECK_RES(ret);

  /* occupy page in object lookup */

  ret = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_LU | SPIFFS_OP_C_UPDT,
                   0, SPIFFS_BLOCK_TO_PADDR(fs, blkndx) +
                   entry * sizeof(int16_t), sizeof(int16_t),
                   (uint8_t *) & objid);
  SPIFFS_CHECK_RES(ret);

  fs->stats_p_allocated++;

  /* write page header */

  ph->flags &= ~SPIFFS_PH_FLAG_USED;
  ret = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_UPDT,
                   0, SPIFFS_OBJ_LOOKUP_ENTRY_TO_PADDR(fs, blkndx, entry),
                   sizeof(struct spiffs_page_header_s), (uint8_t *) ph);
  SPIFFS_CHECK_RES(ret);

  /* write page data */

  if (data)
    {
      ret = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_UPDT,
                       0, SPIFFS_OBJ_LOOKUP_ENTRY_TO_PADDR(fs, blkndx,
                                                           entry) +
                       sizeof(struct spiffs_page_header_s) + page_offs, len, data);
      SPIFFS_CHECK_RES(ret);
    }

  /* finalize header if necessary */

  if (finalize && (ph->flags & SPIFFS_PH_FLAG_FINAL))
    {
      ph->flags &= ~SPIFFS_PH_FLAG_FINAL;
      ret = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_UPDT,
                       0, SPIFFS_OBJ_LOOKUP_ENTRY_TO_PADDR(fs, blkndx,
                                                           entry) +
                       offsetof(struct spiffs_page_header_s, flags), sizeof(uint8_t),
                       (uint8_t *) & ph->flags);
      SPIFFS_CHECK_RES(ret);
    }

  /* return written page */

  if (pgndx)
    {
      *pgndx = SPIFFS_OBJ_LOOKUP_ENTRY_TO_PIX(fs, blkndx, entry);
    }

  return ret;
}

/* Moves a page from src to a free page and finalizes it. Updates page index.
 * Page data is given in param page. If page data is null, provided header
 * is used for meta-info and page data is physically copied.
 */

int32_t spiffs_page_move(FAR struct spiffs_s *fs,
                         int16_t objid, FAR uint8_t *page_data, int16_t ix,
                         FAR struct spiffs_page_header_s *page_hdr,
                         int16_t src_pgndx, FAR int16_t *dst_pgndx)
{
  int32_t ret;
  uint8_t was_final = 0;
  struct spiffs_page_header_s *p_hdr;
  int16_t blkndx;
  int entry;
  int16_t free_pgndx;

  /* find free entry */

  ret = spiffs_obj_lu_find_free(fs, fs->free_blkndx,
                                fs->free_entry, &blkndx, &entry);
  SPIFFS_CHECK_RES(ret);
  free_pgndx = SPIFFS_OBJ_LOOKUP_ENTRY_TO_PIX(fs, blkndx, entry);

  if (dst_pgndx)
    {
      *dst_pgndx = free_pgndx;
    }

  p_hdr = page_data ? (struct spiffs_page_header_s *) page_data : page_hdr;
  if (page_data)
    {
      /* got page data */

      was_final = (p_hdr->flags & SPIFFS_PH_FLAG_FINAL) == 0;

      /* write unfinalized page */

      p_hdr->flags |= SPIFFS_PH_FLAG_FINAL;
      p_hdr->flags &= ~SPIFFS_PH_FLAG_USED;
      ret = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_UPDT,
                       0, SPIFFS_PAGE_TO_PADDR(fs, free_pgndx),
                       SPIFFS_CFG_LOG_PAGE_SZ(fs), page_data);
    }
  else
    {
      /* copy page data */

      ret = spiffs_phys_cpy(fs, objid, SPIFFS_PAGE_TO_PADDR(fs, free_pgndx),
                            SPIFFS_PAGE_TO_PADDR(fs, src_pgndx),
                            SPIFFS_CFG_LOG_PAGE_SZ(fs));
    }

  SPIFFS_CHECK_RES(ret);

  /* mark entry in destination object lookup */

  ret = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_LU | SPIFFS_OP_C_UPDT, 0,
                   SPIFFS_BLOCK_TO_PADDR(fs, SPIFFS_BLOCK_FOR_PAGE(fs, free_pgndx)) +
                   SPIFFS_OBJ_LOOKUP_ENTRY_FOR_PAGE(fs, free_pgndx) *
                   sizeof(int16_t), sizeof(int16_t),
                   (FAR uint8_t *)&ix);
  SPIFFS_CHECK_RES(ret);

  fs->stats_p_allocated++;

  if (was_final)
    {
      /* mark finalized in destination page */

      p_hdr->flags &= ~(SPIFFS_PH_FLAG_FINAL | SPIFFS_PH_FLAG_USED);
      ret = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_UPDT, objid,
                       SPIFFS_PAGE_TO_PADDR(fs, free_pgndx) +
                       offsetof(struct spiffs_page_header_s, flags), sizeof(uint8_t),
                       (uint8_t *) & p_hdr->flags);
      SPIFFS_CHECK_RES(ret);
    }

  /* mark source deleted */

  ret = spiffs_page_delete(fs, src_pgndx);
  return ret;
}

/* Deletes a page and removes it from object lookup. */

int32_t spiffs_page_delete(FAR struct spiffs_s *fs, int16_t pgndx)
{
  int32_t ret;

  /* mark deleted entry in source object lookup */

  int16_t d_obj_id = SPIFFS_OBJ_ID_DELETED;
  ret = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_LU | SPIFFS_OP_C_DELE, 0,
                   SPIFFS_BLOCK_TO_PADDR(fs, SPIFFS_BLOCK_FOR_PAGE(fs, pgndx)) +
                   SPIFFS_OBJ_LOOKUP_ENTRY_FOR_PAGE(fs, pgndx) *
                   sizeof(int16_t), sizeof(int16_t),
                   (uint8_t *) & d_obj_id);
  SPIFFS_CHECK_RES(ret);

  fs->stats_p_deleted++;
  fs->stats_p_allocated--;

  /* mark deleted in source page */

  uint8_t flags = 0xff;
#if SPIFFS_NO_BLIND_WRITES
  ret = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_READ,
                   0, SPIFFS_PAGE_TO_PADDR(fs,
                                           pgndx) + offsetof(struct spiffs_page_header_s,
                                                           flags),
                   sizeof(flags), &flags);
  SPIFFS_CHECK_RES(ret);
#endif
  flags &= ~(SPIFFS_PH_FLAG_DELET | SPIFFS_PH_FLAG_USED);
  ret = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_DELE, 0,
                   SPIFFS_PAGE_TO_PADDR(fs, pgndx) + offsetof(struct spiffs_page_header_s, flags),
                   sizeof(flags), &flags);

  return ret;
}

/* Create an object index header page with empty index and undefined length */

int spiffs_object_create(FAR struct spiffs_s *fs,
                         int16_t objid, const uint8_t name[],
                         uint8_t type, FAR int16_t *objhdr_pgndx)
{
  struct spiffs_pgobj_ixheader_s objndx_hdr;
  int16_t blkndx;
  int entry;
  int ret = OK;

  ret = spiffs_gc_check(fs, SPIFFS_DATA_PAGE_SIZE(fs));
  SPIFFS_CHECK_RES(ret);

  objid |= SPIFFS_OBJ_ID_IX_FLAG;

  /* find free entry */

  ret =
    spiffs_obj_lu_find_free(fs, fs->free_blkndx,
                            fs->free_entry, &blkndx, &entry);
  SPIFFS_CHECK_RES(ret);
  finfo("create: found free page @ " _SPIPRIpg " blkndx:" _SPIPRIbl " entry:"
             _SPIPRIsp "\n", (int16_t) SPIFFS_OBJ_LOOKUP_ENTRY_TO_PIX(fs,
                                                                             blkndx,
                                                                             entry),
             blkndx, entry);

  /* occupy page in object lookup */

  ret = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_LU | SPIFFS_OP_C_UPDT,
                   0, SPIFFS_BLOCK_TO_PADDR(fs,
                                            blkndx) +
                   entry * sizeof(int16_t), sizeof(int16_t),
                   (uint8_t *) & objid);
  SPIFFS_CHECK_RES(ret);

  fs->stats_p_allocated++;

  /* write empty object index page */

  objndx_hdr.p_hdr.objid = objid;
  objndx_hdr.p_hdr.span_ix = 0;
  objndx_hdr.p_hdr.flags =
    0xff & ~(SPIFFS_PH_FLAG_FINAL | SPIFFS_PH_FLAG_INDEX | SPIFFS_PH_FLAG_USED);
  objndx_hdr.type = type;
  objndx_hdr.size = SPIFFS_UNDEFINED_LEN;  /* keep ones so we can update later
                                         * without wasting this page */
  strncpy((char *)objndx_hdr.name, (const char *)name, SPIFFS_NAME_MAX);

  /* update page */

  ret = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_UPDT,
                   0, SPIFFS_OBJ_LOOKUP_ENTRY_TO_PADDR(fs, blkndx, entry),
                   sizeof(struct spiffs_pgobj_ixheader_s), (uint8_t *) & objndx_hdr);

  SPIFFS_CHECK_RES(ret);
  spiffs_cb_object_event(fs, (spiffs_page_object_ix *) & objndx_hdr,
                         SPIFFS_EV_IX_NEW, objid, 0,
                         SPIFFS_OBJ_LOOKUP_ENTRY_TO_PIX(fs, blkndx, entry),
                         SPIFFS_UNDEFINED_LEN);

  if (objhdr_pgndx)
    {
      *objhdr_pgndx = SPIFFS_OBJ_LOOKUP_ENTRY_TO_PIX(fs, blkndx, entry);
    }

  return ret;
}

/* update object index header with any combination of name/size/index
 * new_objhdr_data may be null, if so the object index header page is loaded
 * name may be null, if so name is not changed
 * size may be null, if so size is not changed
 */

int spiffs_object_update_index_hdr(FAR struct spiffs_s *fs,
                                   FAR struct spiffs_file_s *fobj,
                                   int16_t objid, int16_t objhdr_pgndx,
                                   FAR uint8_t *new_objhdr_data,
                                   const uint8_t name[],
                                   uint32_t size, FAR int16_t *new_pgndx)
{
  int32_t ret = OK;
  FAR struct spiffs_pgobj_ixheader_s *objhdr;
  int16_t new_objhdr_pgndx;

  objid |= SPIFFS_OBJ_ID_IX_FLAG;

  if (new_objhdr_data)
    {
      /* object index header page already given to us, no need to load it */

      objhdr = (struct spiffs_pgobj_ixheader_s *) new_objhdr_data;
    }
  else
    {
      /* read object index header page */

      ret = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_IX | SPIFFS_OP_C_READ,
                       fobj->objid, SPIFFS_PAGE_TO_PADDR(fs, objhdr_pgndx),
                       SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->work);
      SPIFFS_CHECK_RES(ret);
      objhdr = (struct spiffs_pgobj_ixheader_s *) fs->work;
    }

  SPIFFS_VALIDATE_OBJIX(objhdr->p_hdr, objid, 0);

  /* change name */

  if (name)
    {
      strncpy((char *)objhdr->name, (const char *)name, SPIFFS_NAME_MAX);
    }

  if (size)
    {
      objhdr->size = size;
    }

  /* move and update page */

  ret = spiffs_page_move(fs, fobj == NULL ? 0 : fobj->objid,
                         (FAR uint8_t *)objhdr, objid,
                         0, objhdr_pgndx, &new_objhdr_pgndx);

  if (ret == OK)
    {
      if (new_pgndx)
        {
          *new_pgndx = new_objhdr_pgndx;
        }

      /* callback on object index update */

      spiffs_cb_object_event(fs, (spiffs_page_object_ix *) objhdr,
                             new_objhdr_data ? SPIFFS_EV_IX_UPD :
                             SPIFFS_EV_IX_UPD_HDR, objid,
                             objhdr->p_hdr.span_ix, new_objhdr_pgndx,
                             objhdr->size);
      if (fobj)
        {
          fobj->objhdr_pgndx = new_objhdr_pgndx;  /* if this is not in the
                                                   * registered cluster */
        }
    }

  return ret;
}

void spiffs_cb_object_event(FAR struct spiffs_s *fs,
                            FAR spiffs_page_object_ix *objndx,
                            int ev,
                            int16_t obj_id_raw,
                            int16_t spndx,
                            int16_t new_pgndx, uint32_t new_size)
{
  FAR struct spiffs_file_s *fobj;
  FAR struct spiffs_file_s *next;
  int16_t objid = obj_id_raw & ~SPIFFS_OBJ_ID_IX_FLAG;

  finfo("       CALLBACK  %s objid:" _SPIPRIid " spndx:" _SPIPRIsp " npgndx:"
             _SPIPRIpg " nsz:" _SPIPRIi "\n", (const char *[])
             {
             "UPD", "NEW", "DEL", "MOV", "HUP", "???"}[MIN(ev, 5)],
             obj_id_raw, spndx, new_pgndx, new_size);

  /* Update index caches in all file descriptors */

  for (fobj  = (FAR struct spiffs_file_s *)dq_peek(&fs->objq);
       fobj != NULL;
       fobj  = next)
    {
      /* Set up for the next time through the loop (in case fobj is deleted) */

      next = (FAR struct spiffs_file_s *)dq_next((FAR dq_entry_t *)fobj);

      /* Is this the object we are looking for? */

      if ((fobj->objid & ~SPIFFS_OBJ_ID_IX_FLAG) != objid)
        {
          continue;             /* Object not related to updated file */
        }

      if (spndx == 0)
        {
          /* object index header update */

          if (ev != SPIFFS_EV_IX_DEL)
            {
              finfo("Setting fobj %d (offset=%d) objhdr_pgndx to %04x size=%d\n",
                    fobj->objid, fobj->offset, new_pgndx, new_size);

              fobj->objhdr_pgndx = new_pgndx;
              if (new_size != 0)
                {
                  /* update size and offsets for fobj to this file */

                  fobj->size = new_size;
                  uint32_t act_new_size =
                    new_size == SPIFFS_UNDEFINED_LEN ? 0 : new_size;
                  if (act_new_size > 0 && fobj->cache_page)
                    {
                      act_new_size =
                        MAX(act_new_size,
                            fobj->cache_page->offset +
                            fobj->cache_page->size);
                    }

                  if (fobj->offset > act_new_size)
                    {
                      fobj->offset = act_new_size;
                    }

                  if (fs->f_pos > act_new_size)
                    {
                      fs->f_pos = act_new_size;
                    }

                  if (fobj->cache_page &&
                      fobj->cache_page->offset > act_new_size + 1)
                    {
                      spiffs_cacheinfo
                        ("CACHE_DROP: file trunced, dropping cache page "
                         _SPIPRIi ", no writeback\n", fobj->cache_page->ix);
                      spiffs_cache_fd_release(fs, fobj->cache_page);
                    }
                }
            }
          else
            {
              /* Removing file */

              if (fobj->cache_page)
                {
                  spiffs_cacheinfo
                    ("CACHE_DROP: file deleted, dropping cache page " _SPIPRIi
                     ", no writeback\n", fobj->cache_page->ix);
                  spiffs_cache_fd_release(fs, fobj->cache_page);
                }

              finfo("       callback: release fobj " _SPIPRIfd
                         " span:" _SPIPRIsp " objndx_pgndx to " _SPIPRIpg "\n",
                         fobj->objid, spndx, new_pgndx);

              /* Remove the file object */

              spiffs_file_free(fs, fobj);
            }
        }

      if (fobj->objndx_spndx == spndx)
        {
          if (ev != SPIFFS_EV_IX_DEL)
            {
              finfo("       callback: setting fobj " _SPIPRIfd
                         " span:" _SPIPRIsp " objndx_pgndx to " _SPIPRIpg "\n",
                         fobj->objid, spndx, new_pgndx);
              fobj->objndx_pgndx = new_pgndx;
            }
          else
            {
              fobj->objndx_pgndx = 0;
            }
        }
    }
}

/* Open object by objid */

int32_t spiffs_object_open_byid(FAR struct spiffs_s *fs,
                                 int16_t objid,
                                 FAR struct spiffs_file_s *fobj,
                                 uint16_t flags, uint16_t mode)
{
  int32_t ret = OK;
  int16_t pgndx;

  ret =
    spiffs_obj_lu_find_id_and_span(fs, objid | SPIFFS_OBJ_ID_IX_FLAG, 0, 0,
                                   &pgndx);
  SPIFFS_CHECK_RES(ret);

  ret = spiffs_object_open_bypage(fs, pgndx, fobj, flags, mode);

  return ret;
}

/* Open object by page index */

int spiffs_object_open_bypage(FAR struct spiffs_s *fs, int16_t pgndx,
                               FAR struct spiffs_file_s *fobj, uint16_t flags,
                               uint16_t mode)
{
  struct spiffs_pgobj_ixheader_s objndx_hdr;
  int16_t objid;
  int16_t blkndx;
  int entry;
  int ret = OK;

  ret = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_IX | SPIFFS_OP_C_READ,
                   fobj->objid, SPIFFS_PAGE_TO_PADDR(fs, pgndx),
                   sizeof(struct spiffs_pgobj_ixheader_s),
                   (FAR uint8_t *)&objndx_hdr);
  SPIFFS_CHECK_RES(ret);

  blkndx = SPIFFS_BLOCK_FOR_PAGE(fs, pgndx);
  entry  = SPIFFS_OBJ_LOOKUP_ENTRY_FOR_PAGE(fs, pgndx);

  ret = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU | SPIFFS_OP_C_READ,
                   0, SPIFFS_BLOCK_TO_PADDR(fs, blkndx) +
                   entry * sizeof(int16_t), sizeof(int16_t),
                   (FAR uint8_t *)&objid);

  fobj->objhdr_pgndx = pgndx;
  fobj->size         = objndx_hdr.size;
  fobj->offset       = 0;
  fobj->objndx_pgndx = pgndx;
  fobj->objndx_spndx = 0;
  fobj->objid        = objid;
  fobj->flags        = flags;

  SPIFFS_VALIDATE_OBJIX(objndx_hdr.p_hdr, fobj->objid, 0);

  finfo("open: fobj " _SPIPRIfd "\n", fobj->objid);
  return ret;
}

/* Append to object.  Deep current object index (header) page in fs->work buffer */

int spiffs_object_append(FAR struct spiffs_s *fs,
                         FAR struct spiffs_file_s *fobj, off_t offset,
                         FAR uint8_t *data, size_t len)
{
  size_t written = 0;
  int ret = OK;

  finfo("append: " _SPIPRIi " bytes @ offs " _SPIPRIi " of size " _SPIPRIi
             "\n", len, offset, fobj->size);

  if (offset > fobj->size)
    {
      finfo("append: offset reversed to size\n");
      offset = fobj->size;
    }

  ret = spiffs_gc_check(fs, len + SPIFFS_DATA_PAGE_SIZE(fs));   /* add an extra 
                                                                 * page of data 
                                                                 * worth for meta */
  if (ret != OK)
    {
      finfo("append: gc check fail " _SPIPRIi "\n", ret);
    }

  SPIFFS_CHECK_RES(ret);

  FAR struct spiffs_pgobj_ixheader_s *objhdr =
    (FAR struct spiffs_pgobj_ixheader_s *) fs->work;
  spiffs_page_object_ix *objndx = (spiffs_page_object_ix *) fs->work;
  struct spiffs_page_header_s p_hdr;

  int16_t cur_objndx_spndx = 0;
  int16_t prev_objndx_spndx = (int16_t) - 1;
  int16_t cur_objndx_pgndx = fobj->objhdr_pgndx;
  int16_t new_objhdr_page;

  int16_t data_spndx = offset / SPIFFS_DATA_PAGE_SIZE(fs);
  int16_t data_page;
  uint32_t page_offs = offset % SPIFFS_DATA_PAGE_SIZE(fs);

  /* write all data */

  while (ret == OK && written < len)
    {
      /* calculate object index page span index */

      cur_objndx_spndx = SPIFFS_OBJ_IX_ENTRY_SPAN_IX(fs, data_spndx);

      /* handle storing and loading of object indices */

      if (cur_objndx_spndx != prev_objndx_spndx)
        {
          /* new object index page.  Within this clause we return directly
           * if something fails, object index mess-up
           */

          if (written > 0)
            {
              /* store previous object index page, unless first pass */

              finfo("append: " _SPIPRIid " store objndx " _SPIPRIpg ":"
                         _SPIPRIsp ", written " _SPIPRIi "\n", fobj->objid,
                         cur_objndx_pgndx, prev_objndx_spndx, written);

              if (prev_objndx_spndx == 0)
                {
                  /* this is an update to object index header page */

                  objhdr->size = offset + written;
                  if (offset == 0)
                    {
                      /* was an empty object, update same page (size was
                       * 0xffffffff)
                       */

                      ret = spiffs_page_index_check(fs, fobj, cur_objndx_pgndx, 0);
                      SPIFFS_CHECK_RES(ret);
                      ret =
                        _spiffs_wr(fs, SPIFFS_OP_T_OBJ_IX | SPIFFS_OP_C_UPDT,
                                   fobj->objid, SPIFFS_PAGE_TO_PADDR(fs, cur_objndx_pgndx),
                                   SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->work);
                      SPIFFS_CHECK_RES(ret);
                    }
                  else
                    {
                      /* was a nonempty object, update to new page */

                      ret = spiffs_object_update_index_hdr(fs, fobj, fobj->objid,
                                                           fobj->objhdr_pgndx,
                                                           fs->work, 0,
                                                           offset + written,
                                                           &new_objhdr_page);
                      SPIFFS_CHECK_RES(ret);
                      finfo("append: " _SPIPRIid " store new objhdr, "
                                 _SPIPRIpg ":" _SPIPRIsp ", written " _SPIPRIi
                                 "\n", fobj->objid, new_objhdr_page, 0,
                                 written);
                    }
                }
              else
                {
                  /* this is an update to an object index page */

                  ret =
                    spiffs_page_index_check(fs, fobj, cur_objndx_pgndx,
                                            prev_objndx_spndx);
                  SPIFFS_CHECK_RES(ret);

                  ret = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_IX | SPIFFS_OP_C_UPDT,
                                   fobj->objid, SPIFFS_PAGE_TO_PADDR(fs,
                                                                      cur_objndx_pgndx),
                                   SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->work);
                  SPIFFS_CHECK_RES(ret);
                  spiffs_cb_object_event(fs, (spiffs_page_object_ix *) fs->work,
                                         SPIFFS_EV_IX_UPD, fobj->objid,
                                         objndx->p_hdr.span_ix, cur_objndx_pgndx,
                                         0);

                  /* update length in object index header page */

                  ret = spiffs_object_update_index_hdr(fs, fobj, fobj->objid,
                                                       fobj->objhdr_pgndx, 0, 0,
                                                       offset + written,
                                                       &new_objhdr_page);
                  SPIFFS_CHECK_RES(ret);
                  finfo("append: " _SPIPRIid " store new size I " _SPIPRIi
                             " in objhdr, " _SPIPRIpg ":" _SPIPRIsp
                             ", written " _SPIPRIi "\n", fobj->objid,
                             offset + written, new_objhdr_page, 0, written);
                }

              fobj->size = offset + written;
              fobj->offset = offset + written;
            }

          /* create or load new object index page */

          if (cur_objndx_spndx == 0)
            {
              /* load object index header page, must always exist */

              finfo("append: " _SPIPRIid " load objndxhdr page " _SPIPRIpg
                         ":" _SPIPRIsp "\n", fobj->objid, cur_objndx_pgndx,
                         cur_objndx_spndx);
              ret =
                _spiffs_rd(fs, SPIFFS_OP_T_OBJ_IX | SPIFFS_OP_C_READ,
                           fobj->objid, SPIFFS_PAGE_TO_PADDR(fs,
                                                              cur_objndx_pgndx),
                           SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->work);
              SPIFFS_CHECK_RES(ret);
              SPIFFS_VALIDATE_OBJIX(objhdr->p_hdr, fobj->objid,
                                    cur_objndx_spndx);
            }
          else
            {
              int16_t len_objndx_spndx =
                SPIFFS_OBJ_IX_ENTRY_SPAN_IX(fs,
                                            (fobj->size -
                                             1) / SPIFFS_DATA_PAGE_SIZE(fs));

              /* on subsequent passes, create a new object index page */

              if (written > 0 || cur_objndx_spndx > len_objndx_spndx)
                {
                  p_hdr.objid = fobj->objid | SPIFFS_OBJ_ID_IX_FLAG;
                  p_hdr.span_ix = cur_objndx_spndx;
                  p_hdr.flags =
                    0xff & ~(SPIFFS_PH_FLAG_FINAL | SPIFFS_PH_FLAG_INDEX);
                  ret =
                    spiffs_page_allocate_data(fs,
                                              fobj->objid | SPIFFS_OBJ_ID_IX_FLAG,
                                              &p_hdr, 0, 0, 0, 1,
                                              &cur_objndx_pgndx);
                  SPIFFS_CHECK_RES(ret);

                  /* quick "load" of new object index page */

                  memset(fs->work, 0xff, SPIFFS_CFG_LOG_PAGE_SZ(fs));
                  memcpy(fs->work, &p_hdr, sizeof(struct spiffs_page_header_s));
                  spiffs_cb_object_event(fs, (spiffs_page_object_ix *) fs->work,
                                         SPIFFS_EV_IX_NEW, fobj->objid,
                                         cur_objndx_spndx, cur_objndx_pgndx, 0);
                  finfo("append: " _SPIPRIid " create objndx page, "
                             _SPIPRIpg ":" _SPIPRIsp ", written " _SPIPRIi "\n",
                             fobj->objid, cur_objndx_pgndx, cur_objndx_spndx,
                             written);
                }
              else
                {
                  /* on first pass, we load existing object index page */

                  int16_t pgndx;
                  finfo("append: " _SPIPRIid " find objndx span_ix:"
                             _SPIPRIsp "\n", fobj->objid, cur_objndx_spndx);
                  if (fobj->objndx_spndx == cur_objndx_spndx)
                    {
                      pgndx = fobj->objndx_pgndx;
                    }
                  else
                    {
                      ret =
                        spiffs_obj_lu_find_id_and_span(fs,
                                                       fobj->objid | SPIFFS_OBJ_ID_IX_FLAG,
                                                       cur_objndx_spndx, 0, &pgndx);
                      SPIFFS_CHECK_RES(ret);
                    }
                  finfo("append: " _SPIPRIid " found object index at page "
                             _SPIPRIpg " [fobj size " _SPIPRIi "]\n", fobj->objid,
                             pgndx, fobj->size);
                  ret =
                    _spiffs_rd(fs, SPIFFS_OP_T_OBJ_IX | SPIFFS_OP_C_READ,
                               fobj->objid, SPIFFS_PAGE_TO_PADDR(fs, pgndx),
                               SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->work);
                  SPIFFS_CHECK_RES(ret);
                  SPIFFS_VALIDATE_OBJIX(objhdr->p_hdr, fobj->objid,
                                        cur_objndx_spndx);
                  cur_objndx_pgndx = pgndx;
                }

              fobj->objndx_pgndx = cur_objndx_pgndx;
              fobj->objndx_spndx = cur_objndx_spndx;
              fobj->offset = offset + written;
              fobj->size = offset + written;
            }

          prev_objndx_spndx = cur_objndx_spndx;
        }

      /* write data */

      uint32_t to_write =
        MIN(len - written, SPIFFS_DATA_PAGE_SIZE(fs) - page_offs);
      if (page_offs == 0)
        {
          /* at beginning of a page, allocate and write a new page of data */

          p_hdr.objid = fobj->objid & ~SPIFFS_OBJ_ID_IX_FLAG;
          p_hdr.span_ix = data_spndx;
          p_hdr.flags = 0xff & ~(SPIFFS_PH_FLAG_FINAL); /* finalize immediately */
          ret =
            spiffs_page_allocate_data(fs, fobj->objid & ~SPIFFS_OBJ_ID_IX_FLAG,
                                      &p_hdr, &data[written], to_write,
                                      page_offs, 1, &data_page);
          finfo("append: " _SPIPRIid " store new data page, " _SPIPRIpg ":"
                     _SPIPRIsp " offset:" _SPIPRIi ", len " _SPIPRIi
                     ", written " _SPIPRIi "\n", fobj->objid, data_page,
                     data_spndx, page_offs, to_write, written);
        }
      else
        {
          /* append to existing page, fill out free data in existing page */

          if (cur_objndx_spndx == 0)
            {
              /* get data page from object index header page */

              data_page =
                ((int16_t *) ((uint8_t *) objhdr +
                                     sizeof(struct spiffs_pgobj_ixheader_s)))
                [data_spndx];
            }
          else
            {
              /* get data page from object index page */

              data_page =
                ((int16_t *) ((uint8_t *) objndx +
                                     sizeof(spiffs_page_object_ix)))
                [SPIFFS_OBJ_IX_ENTRY(fs, data_spndx)];
            }

          ret = spiffs_page_data_check(fs, fobj, data_page, data_spndx);
          SPIFFS_CHECK_RES(ret);

          ret = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_UPDT,
                           fobj->objid, SPIFFS_PAGE_TO_PADDR(fs,
                                                              data_page) +
                           sizeof(struct spiffs_page_header_s) + page_offs, to_write,
                           &data[written]);
          finfo("append: " _SPIPRIid " store to existing data page, "
                     _SPIPRIpg ":" _SPIPRIsp " offset:" _SPIPRIi ", len "
                     _SPIPRIi ", written " _SPIPRIi "\n", fobj->objid, data_page,
                     data_spndx, page_offs, to_write, written);
        }

      if (ret != OK)
        {
          break;
        }

      /* update memory representation of object index page with new data page */

      if (cur_objndx_spndx == 0)
        {
          /* update object index header page */

          ((int16_t *) ((uint8_t *) objhdr +
                               sizeof(struct spiffs_pgobj_ixheader_s)))[data_spndx]
            = data_page;

          finfo("append: " _SPIPRIid " wrote page " _SPIPRIpg
                     " to objhdr entry " _SPIPRIsp " in mem\n", fobj->objid,
                     data_page, data_spndx);

          objhdr->size = offset + written;
        }
      else
        {
          /* update object index page */

          ((int16_t *) ((uint8_t *) objndx +
                               sizeof(spiffs_page_object_ix)))
            [SPIFFS_OBJ_IX_ENTRY(fs, data_spndx)] = data_page;

          finfo("append: " _SPIPRIid " wrote page " _SPIPRIpg
                     " to objndx entry " _SPIPRIsp " in mem\n", fobj->objid,
                     data_page, (int16_t) SPIFFS_OBJ_IX_ENTRY(fs,
                                                                     data_spndx));
        }

      /* update internals */

      page_offs = 0;
      data_spndx++;
      written += to_write;
    }

  fobj->size = offset + written;
  fobj->offset = offset + written;
  fobj->objndx_pgndx = cur_objndx_pgndx;
  fobj->objndx_spndx = cur_objndx_spndx;

  /* finalize updated object indices */

  int32_t res2 = OK;
  if (cur_objndx_spndx != 0)
    {
      /* wrote beyond object index header page.  Write last modified object
       * index page, unless object header index page
       */

      finfo("append: " _SPIPRIid " store objndx page, " _SPIPRIpg ":"
                 _SPIPRIsp ", written " _SPIPRIi "\n", fobj->objid,
                 cur_objndx_pgndx, cur_objndx_spndx, written);

      res2 = spiffs_page_index_check(fs, fobj, cur_objndx_pgndx, cur_objndx_spndx);
      SPIFFS_CHECK_RES(res2);

      res2 = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_IX | SPIFFS_OP_C_UPDT,
                        fobj->objid, SPIFFS_PAGE_TO_PADDR(fs, cur_objndx_pgndx),
                        SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->work);
      SPIFFS_CHECK_RES(res2);
      spiffs_cb_object_event(fs, (spiffs_page_object_ix *) fs->work,
                             SPIFFS_EV_IX_UPD, fobj->objid, objndx->p_hdr.span_ix,
                             cur_objndx_pgndx, 0);

      /* update size in object header index page */

      res2 = spiffs_object_update_index_hdr(fs, fobj, fobj->objid,
                                            fobj->objhdr_pgndx, 0, 0,
                                            offset + written,
                                            &new_objhdr_page);
      finfo("append: " _SPIPRIid " store new size II " _SPIPRIi
                 " in objhdr, " _SPIPRIpg ":" _SPIPRIsp ", written " _SPIPRIi
                 ", ret " _SPIPRIi "\n", fobj->objid, offset + written,
                 new_objhdr_page, 0, written, res2);
      SPIFFS_CHECK_RES(res2);
    }
  else
    {
      /* wrote within object index header page */

      if (offset == 0)
        {
          /* wrote to empty object - simply update size and write whole page */

          objhdr->size = offset + written;
          finfo("append: " _SPIPRIid " store fresh objhdr page, "
                     _SPIPRIpg ":" _SPIPRIsp ", written " _SPIPRIi "\n",
                     fobj->objid, cur_objndx_pgndx, cur_objndx_spndx, written);

          res2 = spiffs_page_index_check(fs, fobj, cur_objndx_pgndx, cur_objndx_spndx);
          SPIFFS_CHECK_RES(res2);

          res2 = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_IX | SPIFFS_OP_C_UPDT,
                            fobj->objid, SPIFFS_PAGE_TO_PADDR(fs, cur_objndx_pgndx),
                            SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->work);
          SPIFFS_CHECK_RES(res2);

          /* callback on object index update */

          spiffs_cb_object_event(fs, (spiffs_page_object_ix *) fs->work,
                                 SPIFFS_EV_IX_UPD_HDR, fobj->objid,
                                 objhdr->p_hdr.span_ix, cur_objndx_pgndx,
                                 objhdr->size);
        }
      else
        {
          /* modifying object index header page, update size and make new copy */

          res2 = spiffs_object_update_index_hdr(fs, fobj, fobj->objid,
                                                fobj->objhdr_pgndx, fs->work, 0,
                                                offset + written,
                                                &new_objhdr_page);
          finfo("append: " _SPIPRIid " store modified objhdr page, "
                     _SPIPRIpg ":" _SPIPRIsp ", written " _SPIPRIi "\n",
                     fobj->objid, new_objhdr_page, 0, written);
          SPIFFS_CHECK_RES(res2);
        }
    }

  return ret;
}

/* Modify object.  Keep current object index (header) page in fs->work buffer */

int spiffs_object_modify(FAR struct spiffs_s *fs,
                         FAR struct spiffs_file_s *fobj, off_t offset,
                         FAR uint8_t *data, size_t len)
{
  size_t written = 0;
  int ret = OK;

  ret = spiffs_gc_check(fs, len + SPIFFS_DATA_PAGE_SIZE(fs));
  SPIFFS_CHECK_RES(ret);

  FAR struct spiffs_pgobj_ixheader_s *objhdr =
    (FAR struct spiffs_pgobj_ixheader_s *) fs->work;
  spiffs_page_object_ix *objndx = (spiffs_page_object_ix *) fs->work;
  struct spiffs_page_header_s p_hdr;

  int16_t cur_objndx_spndx = 0;
  int16_t prev_objndx_spndx = (int16_t) - 1;
  int16_t cur_objndx_pgndx = fobj->objhdr_pgndx;
  int16_t new_objhdr_pgndx;

  int16_t data_spndx = offset / SPIFFS_DATA_PAGE_SIZE(fs);
  int16_t data_pgndx;
  uint32_t page_offs = offset % SPIFFS_DATA_PAGE_SIZE(fs);

  /* write all data */

  while (ret == OK && written < len)
    {
      /* calculate object index page span index */

      cur_objndx_spndx = SPIFFS_OBJ_IX_ENTRY_SPAN_IX(fs, data_spndx);

      /* handle storing and loading of object indices */

      if (cur_objndx_spndx != prev_objndx_spndx)
        {
          /* new object index page.  Within this clause we return directly
           * if something fails, object index mess-up
           */

          if (written > 0)
            {
              /* store previous object index (header) page, unless first pass */

              if (prev_objndx_spndx == 0)
                {
                  /* store previous object index header page */

                  ret = spiffs_object_update_index_hdr(fs, fobj, fobj->objid,
                                                       fobj->objhdr_pgndx,
                                                       fs->work, 0, 0,
                                                       &new_objhdr_pgndx);
                  finfo("modify: store modified objhdr page, " _SPIPRIpg
                             ":" _SPIPRIsp ", written " _SPIPRIi "\n",
                             new_objhdr_pgndx, 0, written);
                  SPIFFS_CHECK_RES(ret);
                }
              else
                {
                  /* store new version of previous object index page */

                  int16_t new_objndx_pgndx;

                  ret =
                    spiffs_page_index_check(fs, fobj, cur_objndx_pgndx,
                                            prev_objndx_spndx);
                  SPIFFS_CHECK_RES(ret);

                  ret = spiffs_page_move(fs, fobj->objid, (FAR uint8_t *)objndx,
                                         fobj->objid, 0, cur_objndx_pgndx,
                                         &new_objndx_pgndx);
                  finfo("modify: store previous modified objndx page, "
                             _SPIPRIid ":" _SPIPRIsp ", written " _SPIPRIi "\n",
                             new_objndx_pgndx, objndx->p_hdr.span_ix, written);
                  SPIFFS_CHECK_RES(ret);
                  spiffs_cb_object_event(fs, (spiffs_page_object_ix *) objndx,
                                         SPIFFS_EV_IX_UPD, fobj->objid,
                                         objndx->p_hdr.span_ix, new_objndx_pgndx,
                                         0);
                }
            }

          /* load next object index page */

          if (cur_objndx_spndx == 0)
            {
              /* load object index header page, must exist */

              finfo("modify: load objndxhdr page " _SPIPRIpg ":" _SPIPRIsp
                         "\n", cur_objndx_pgndx, cur_objndx_spndx);
              ret =
                _spiffs_rd(fs, SPIFFS_OP_T_OBJ_IX | SPIFFS_OP_C_READ,
                           fobj->objid, SPIFFS_PAGE_TO_PADDR(fs,
                                                              cur_objndx_pgndx),
                           SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->work);
              SPIFFS_CHECK_RES(ret);
              SPIFFS_VALIDATE_OBJIX(objhdr->p_hdr, fobj->objid,
                                    cur_objndx_spndx);
            }
          else
            {
              /* load existing object index page on first pass */

              int16_t pgndx;
              finfo("modify: find objndx span_ix:" _SPIPRIsp "\n",
                         cur_objndx_spndx);
              if (fobj->objndx_spndx == cur_objndx_spndx)
                {
                  pgndx = fobj->objndx_pgndx;
                }
              else
                {
                  ret =
                    spiffs_obj_lu_find_id_and_span(fs,
                                                   fobj->objid | SPIFFS_OBJ_ID_IX_FLAG,
                                                   cur_objndx_spndx, 0, &pgndx);
                  SPIFFS_CHECK_RES(ret);
                }
              finfo("modify: found object index at page " _SPIPRIpg "\n",
                         pgndx);
              ret =
                _spiffs_rd(fs, SPIFFS_OP_T_OBJ_IX | SPIFFS_OP_C_READ,
                           fobj->objid, SPIFFS_PAGE_TO_PADDR(fs, pgndx),
                           SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->work);
              SPIFFS_CHECK_RES(ret);
              SPIFFS_VALIDATE_OBJIX(objhdr->p_hdr, fobj->objid,
                                    cur_objndx_spndx);
              cur_objndx_pgndx = pgndx;
            }

          fobj->objndx_pgndx = cur_objndx_pgndx;
          fobj->objndx_spndx = cur_objndx_spndx;
          fobj->offset = offset + written;
          prev_objndx_spndx = cur_objndx_spndx;
        }

      /* write partial data */

      uint32_t to_write =
        MIN(len - written, SPIFFS_DATA_PAGE_SIZE(fs) - page_offs);
      int16_t orig_data_pgndx;

      if (cur_objndx_spndx == 0)
        {
          /* get data page from object index header page */

          orig_data_pgndx =
            ((int16_t *) ((uint8_t *) objhdr +
                                 sizeof(struct spiffs_pgobj_ixheader_s)))
            [data_spndx];
        }
      else
        {
          /* get data page from object index page */

          orig_data_pgndx =
            ((int16_t *) ((uint8_t *) objndx +
                                 sizeof(spiffs_page_object_ix)))
            [SPIFFS_OBJ_IX_ENTRY(fs, data_spndx)];
        }

      p_hdr.objid = fobj->objid & ~SPIFFS_OBJ_ID_IX_FLAG;
      p_hdr.span_ix = data_spndx;
      p_hdr.flags = 0xff;

      if (page_offs == 0 && to_write == SPIFFS_DATA_PAGE_SIZE(fs))
        {
          /* a full page, allocate and write a new page of data */

          ret =
            spiffs_page_allocate_data(fs, fobj->objid & ~SPIFFS_OBJ_ID_IX_FLAG,
                                      &p_hdr, &data[written], to_write,
                                      page_offs, 1, &data_pgndx);
          finfo("modify: store new data page, " _SPIPRIpg ":" _SPIPRIsp
                     " offset:" _SPIPRIi ", len " _SPIPRIi ", written " _SPIPRIi
                     "\n", data_pgndx, data_spndx, page_offs, to_write, written);
        }
      else
        {
          /* write to existing page, allocate new and copy unmodified data */

          ret = spiffs_page_data_check(fs, fobj, orig_data_pgndx, data_spndx);
          SPIFFS_CHECK_RES(ret);

          ret =
            spiffs_page_allocate_data(fs, fobj->objid & ~SPIFFS_OBJ_ID_IX_FLAG,
                                      &p_hdr, 0, 0, 0, 0, &data_pgndx);
          if (ret != OK)
            break;

          /* copy unmodified data */

          if (page_offs > 0)
            {
              /* before modification */

              ret = spiffs_phys_cpy(fs, fobj->objid,
                                    SPIFFS_PAGE_TO_PADDR(fs, data_pgndx) +
                                    sizeof(struct spiffs_page_header_s),
                                    SPIFFS_PAGE_TO_PADDR(fs, orig_data_pgndx) +
                                    sizeof(struct spiffs_page_header_s), page_offs);
              if (ret != OK)
                break;
            }
          if (page_offs + to_write < SPIFFS_DATA_PAGE_SIZE(fs))
            {
              /* after modification */

              ret = spiffs_phys_cpy(fs, fobj->objid,
                                    SPIFFS_PAGE_TO_PADDR(fs,
                                                         data_pgndx) +
                                    sizeof(struct spiffs_page_header_s) + page_offs +
                                    to_write, SPIFFS_PAGE_TO_PADDR(fs,
                                                                   orig_data_pgndx)
                                    + sizeof(struct spiffs_page_header_s) + page_offs +
                                    to_write,
                                    SPIFFS_DATA_PAGE_SIZE(fs) - (page_offs +
                                                                 to_write));
              if (ret != OK)
                {
                  break;
                }
            }

          ret = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_UPDT,
                           fobj->objid, SPIFFS_PAGE_TO_PADDR(fs, data_pgndx) +
                           sizeof(struct spiffs_page_header_s) + page_offs, to_write,
                           &data[written]);
          if (ret != OK)
            {
              break;
            }

          p_hdr.flags &= ~SPIFFS_PH_FLAG_FINAL;
          ret = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_UPDT,
                           fobj->objid, SPIFFS_PAGE_TO_PADDR(fs, data_pgndx) +
                           offsetof(struct spiffs_page_header_s, flags), sizeof(uint8_t),
                           (uint8_t *) & p_hdr.flags);
          if (ret != OK)
            {
              break;
            }

          finfo("modify: store to existing data page, src:" _SPIPRIpg
                     ", dst:" _SPIPRIpg ":" _SPIPRIsp " offset:" _SPIPRIi
                     ", len " _SPIPRIi ", written " _SPIPRIi "\n",
                     orig_data_pgndx, data_pgndx, data_spndx, page_offs, to_write,
                     written);
        }

      /* delete original data page */

      ret = spiffs_page_delete(fs, orig_data_pgndx);
      if (ret != OK)
        {
          break;
        }

      /* update memory representation of object index page with new data page */

      if (cur_objndx_spndx == 0)
        {
          /* update object index header page */

          ((int16_t *) ((uint8_t *) objhdr +
                               sizeof(struct spiffs_pgobj_ixheader_s)))[data_spndx]
            = data_pgndx;
          finfo("modify: wrote page " _SPIPRIpg " to objhdr entry "
                     _SPIPRIsp " in mem\n", data_pgndx, data_spndx);
        }
      else
        {
          /* update object index page */

          ((int16_t *) ((uint8_t *) objndx +
                               sizeof(spiffs_page_object_ix)))
            [SPIFFS_OBJ_IX_ENTRY(fs, data_spndx)] = data_pgndx;
          finfo("modify: wrote page " _SPIPRIpg " to objndx entry "
                     _SPIPRIsp " in mem\n", data_pgndx,
                     (int16_t) SPIFFS_OBJ_IX_ENTRY(fs, data_spndx));
        }

      /* update internals */

      page_offs = 0;
      data_spndx++;
      written += to_write;
    }

  fobj->offset = offset + written;
  fobj->objndx_pgndx = cur_objndx_pgndx;
  fobj->objndx_spndx = cur_objndx_spndx;

  /* finalize updated object indices */

  int32_t res2 = OK;
  if (cur_objndx_spndx != 0)
    {
      /* wrote beyond object index header page.  Write last modified object
       * index page.  Move and update page
       */

      int16_t new_objndx_pgndx;

      res2 = spiffs_page_index_check(fs, fobj, cur_objndx_pgndx, cur_objndx_spndx);
      SPIFFS_CHECK_RES(res2);

      res2 = spiffs_page_move(fs, fobj->objid, (uint8_t *) objndx, fobj->objid, 0,
                              cur_objndx_pgndx, &new_objndx_pgndx);
      finfo("modify: store modified objndx page, " _SPIPRIpg ":" _SPIPRIsp
                 ", written " _SPIPRIi "\n", new_objndx_pgndx, cur_objndx_spndx,
                 written);
      fobj->objndx_pgndx = new_objndx_pgndx;
      fobj->objndx_spndx = cur_objndx_spndx;
      SPIFFS_CHECK_RES(res2);
      spiffs_cb_object_event(fs, (spiffs_page_object_ix *) objndx,
                             SPIFFS_EV_IX_UPD, fobj->objid, objndx->p_hdr.span_ix,
                             new_objndx_pgndx, 0);

    }
  else
    {
      /* wrote within object index header page */

      res2 = spiffs_object_update_index_hdr(fs, fobj, fobj->objid,
                                            fobj->objhdr_pgndx, fs->work, 0,
                                            0, &new_objhdr_pgndx);
      finfo("modify: store modified objhdr page, " _SPIPRIpg ":"
                 _SPIPRIsp ", written " _SPIPRIi "\n", new_objhdr_pgndx, 0,
                 written);
      SPIFFS_CHECK_RES(res2);
    }

  return ret;
}

static int32_t spiffs_find_objhdr_pgndx_callback(FAR struct spiffs_s *fs, int16_t objid,
                                                 int16_t blkndx, int ix_entry,
                                                 FAR const void *user_const_p,
                                                 FAR void *user_var_p)
{
  int32_t ret;
  struct spiffs_pgobj_ixheader_s objhdr;
  int16_t pgndx = SPIFFS_OBJ_LOOKUP_ENTRY_TO_PIX(fs, blkndx, ix_entry);

  if (objid == SPIFFS_OBJ_ID_FREE || objid == SPIFFS_OBJ_ID_DELETED ||
      (objid & SPIFFS_OBJ_ID_IX_FLAG) == 0)
    {
      return SPIFFS_VIS_COUNTINUE;
    }

  ret = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ,
                   0, SPIFFS_PAGE_TO_PADDR(fs, pgndx),
                   sizeof(struct spiffs_pgobj_ixheader_s), (uint8_t *) & objhdr);
  SPIFFS_CHECK_RES(ret);
  if (objhdr.p_hdr.span_ix == 0 &&
      (objhdr.p_hdr.
       flags & (SPIFFS_PH_FLAG_DELET | SPIFFS_PH_FLAG_FINAL |
                SPIFFS_PH_FLAG_IXDELE)) ==
      (SPIFFS_PH_FLAG_DELET | SPIFFS_PH_FLAG_IXDELE))
    {
      if (strcmp((const char *)user_const_p, (char *)objhdr.name) == 0)
        {
          return OK;
        }
    }

  return SPIFFS_VIS_COUNTINUE;
}

/* Finds object index header page by name */

int32_t spiffs_find_objhdr_pgndx(FAR struct spiffs_s *fs,
                                 const uint8_t name[SPIFFS_NAME_MAX],
                                 FAR int16_t *pgndx)
{
  int32_t ret;
  int16_t blkndx;
  int entry;

  ret = spiffs_foreach_objlu(fs, fs->lu_blkndx, fs->lu_entry,
                             0, 0, spiffs_find_objhdr_pgndx_callback,
                             name, 0, &blkndx, &entry);

  if (ret == SPIFFS_VIS_END)
    {
      ret = -ENOENT;
    }

  SPIFFS_CHECK_RES(ret);

  if (pgndx)
    {
      *pgndx = SPIFFS_OBJ_LOOKUP_ENTRY_TO_PIX(fs, blkndx, entry);
    }

  fs->lu_blkndx = blkndx;
  fs->lu_entry = entry;

  return ret;
}

/* Truncates object to new size. If new size is null, object may be removed totally */

int spiffs_object_truncate(FAR struct spiffs_s *fs,
                           FAR struct spiffs_file_s *fobj, off_t new_size,
                           bool remove_full)
{
  int ret = OK;

  if ((fobj->size == SPIFFS_UNDEFINED_LEN || fobj->size == 0) && !remove_full)
    {
      /* no op */

      return ret;
    }

  /* need 2 pages if not removing: object index page + possibly chopped data
   * page
   */

  if (!remove_full)
    {
      ret = spiffs_gc_check(fs, SPIFFS_DATA_PAGE_SIZE(fs) * 2);
      SPIFFS_CHECK_RES(ret);
    }

  int16_t objndx_pgndx = fobj->objhdr_pgndx;
  int16_t data_spndx =
    (fobj->size > 0 ? fobj->size - 1 : 0) / SPIFFS_DATA_PAGE_SIZE(fs);
  uint32_t cur_size = fobj->size == (uint32_t) SPIFFS_UNDEFINED_LEN ? 0 : fobj->size;
  int16_t cur_objndx_spndx = 0;
  int16_t prev_objndx_spndx = (int16_t) - 1;
  FAR struct spiffs_pgobj_ixheader_s *objhdr =
    (FAR struct spiffs_pgobj_ixheader_s *) fs->work;
  spiffs_page_object_ix *objndx = (spiffs_page_object_ix *) fs->work;
  int16_t data_pgndx;
  int16_t new_objhdr_pgndx;

  /* before truncating, check if object is to be fully removed and mark this */

  if (remove_full && new_size == 0)
    {
      uint8_t flags =
        ~(SPIFFS_PH_FLAG_USED | SPIFFS_PH_FLAG_INDEX | SPIFFS_PH_FLAG_FINAL |
          SPIFFS_PH_FLAG_IXDELE);
      ret =
        _spiffs_wr(fs, SPIFFS_OP_T_OBJ_IX | SPIFFS_OP_C_UPDT, fobj->objid,
                   SPIFFS_PAGE_TO_PADDR(fs,
                                        fobj->objhdr_pgndx) +
                   offsetof(struct spiffs_page_header_s, flags), sizeof(uint8_t),
                   (uint8_t *) & flags);
      SPIFFS_CHECK_RES(ret);
    }

  /* delete from end of object until desired len is reached */

  while (cur_size > new_size)
    {
      cur_objndx_spndx = SPIFFS_OBJ_IX_ENTRY_SPAN_IX(fs, data_spndx);

      /* put object index for current data span index in work buffer */

      if (prev_objndx_spndx != cur_objndx_spndx)
        {
          if (prev_objndx_spndx != (int16_t) - 1)
            {
              /* remove previous object index page */

              finfo("truncate: delete objndx page " _SPIPRIpg ":" _SPIPRIsp
                         "\n", objndx_pgndx, prev_objndx_spndx);

              ret = spiffs_page_index_check(fs, fobj, objndx_pgndx, prev_objndx_spndx);
              SPIFFS_CHECK_RES(ret);

              ret = spiffs_page_delete(fs, objndx_pgndx);
              SPIFFS_CHECK_RES(ret);
              spiffs_cb_object_event(fs, (spiffs_page_object_ix *) 0,
                                     SPIFFS_EV_IX_DEL, fobj->objid,
                                     objndx->p_hdr.span_ix, objndx_pgndx, 0);
              if (prev_objndx_spndx > 0)
                {
                  /* Update object index header page, unless we totally want to 
                   * remove the file.
                   * If fully removing, we're not keeping consistency as good
                   * as when storing the header between chunks,
                   * would we be aborted. But when removing full files, a
                   * crammed system may otherwise
                   * report ERR_FULL a la windows. We cannot have that.
                   * Hence, take the risk - if aborted, a file check would free 
                   * the lost pages and mend things
                   * as the file is marked as fully deleted in the beginning.
                   */

                  if (!remove_full)
                    {
                      finfo("truncate: update objndx hdr page " _SPIPRIpg
                                 ":" _SPIPRIsp " to size " _SPIPRIi "\n",
                                 fobj->objhdr_pgndx, prev_objndx_spndx, cur_size);
                      ret =
                        spiffs_object_update_index_hdr(fs, fobj, fobj->objid,
                                                       fobj->objhdr_pgndx, 0, 0,
                                                       cur_size, &new_objhdr_pgndx);
                      SPIFFS_CHECK_RES(ret);
                    }
                  fobj->size = cur_size;
                }
            }

          /* load current object index (header) page */

          if (cur_objndx_spndx == 0)
            {
              objndx_pgndx = fobj->objhdr_pgndx;
            }
          else
            {
              ret =
                spiffs_obj_lu_find_id_and_span(fs,
                                               fobj->objid | SPIFFS_OBJ_ID_IX_FLAG,
                                               cur_objndx_spndx, 0, &objndx_pgndx);
              SPIFFS_CHECK_RES(ret);
            }

          finfo("truncate: load objndx page " _SPIPRIpg ":" _SPIPRIsp
                     " for data spndx:" _SPIPRIsp "\n", objndx_pgndx,
                     cur_objndx_spndx, data_spndx);
          ret =
            _spiffs_rd(fs, SPIFFS_OP_T_OBJ_IX | SPIFFS_OP_C_READ, fobj->objid,
                       SPIFFS_PAGE_TO_PADDR(fs, objndx_pgndx),
                       SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->work);
          SPIFFS_CHECK_RES(ret);
          SPIFFS_VALIDATE_OBJIX(objhdr->p_hdr, fobj->objid, cur_objndx_spndx);
          fobj->objndx_pgndx = objndx_pgndx;
          fobj->objndx_spndx = cur_objndx_spndx;
          fobj->offset = cur_size;

          prev_objndx_spndx = cur_objndx_spndx;
        }

      if (cur_objndx_spndx == 0)
        {
          /* get data page from object index header page */

          data_pgndx =
            ((int16_t *) ((uint8_t *) objhdr +
                                 sizeof(struct spiffs_pgobj_ixheader_s)))
            [data_spndx];
          ((int16_t *) ((uint8_t *) objhdr +
                               sizeof(struct spiffs_pgobj_ixheader_s)))[data_spndx]
            = SPIFFS_OBJ_ID_FREE;
        }
      else
        {
          /* get data page from object index page */

          data_pgndx =
            ((int16_t *) ((uint8_t *) objndx +
                                 sizeof(spiffs_page_object_ix)))
            [SPIFFS_OBJ_IX_ENTRY(fs, data_spndx)];
          ((int16_t *) ((uint8_t *) objndx +
                               sizeof(spiffs_page_object_ix)))
            [SPIFFS_OBJ_IX_ENTRY(fs, data_spndx)] = SPIFFS_OBJ_ID_FREE;
        }

      finfo("truncate: got data pgndx " _SPIPRIpg "\n", data_pgndx);

      if (new_size == 0 || remove_full ||
          cur_size - new_size >= SPIFFS_DATA_PAGE_SIZE(fs))
        {
          /* delete full data page */

          ret = spiffs_page_data_check(fs, fobj, data_pgndx, data_spndx);
          if (ret != SPIFFS_ERR_DELETED && ret != OK &&
              ret != SPIFFS_ERR_INDEX_REF_FREE)
            {
              finfo("truncate: err validating data pgndx " _SPIPRIi "\n",
                         ret);
              break;
            }

          if (ret == OK)
            {
              ret = spiffs_page_delete(fs, data_pgndx);
              if (ret != OK)
                {
                  finfo("truncate: err deleting data pgndx " _SPIPRIi "\n",
                             ret);
                  break;
                }
            }
          else if (ret == SPIFFS_ERR_DELETED ||
                   ret == SPIFFS_ERR_INDEX_REF_FREE)
            {
              ret = OK;
            }

          /* update current size */

          if (cur_size % SPIFFS_DATA_PAGE_SIZE(fs) == 0)
            {
              cur_size -= SPIFFS_DATA_PAGE_SIZE(fs);
            }
          else
            {
              cur_size -= cur_size % SPIFFS_DATA_PAGE_SIZE(fs);
            }

          fobj->size = cur_size;
          fobj->offset = cur_size;
          finfo("truncate: delete data page " _SPIPRIpg " for data spndx:"
                _SPIPRIsp ", cur_size:" _SPIPRIi "\n", data_pgndx, data_spndx,
                cur_size);
        }
      else
        {
          /* delete last page, partially */

          struct spiffs_page_header_s p_hdr;
          int16_t new_data_pgndx;
          uint32_t bytes_to_remove =
            SPIFFS_DATA_PAGE_SIZE(fs) - (new_size % SPIFFS_DATA_PAGE_SIZE(fs));
          finfo("truncate: delete " _SPIPRIi " bytes from data page "
                     _SPIPRIpg " for data spndx:" _SPIPRIsp ", cur_size:"
                     _SPIPRIi "\n", bytes_to_remove, data_pgndx, data_spndx,
                     cur_size);

          ret = spiffs_page_data_check(fs, fobj, data_pgndx, data_spndx);
          if (ret != OK)
            {
              break;
            }

          p_hdr.objid = fobj->objid & ~SPIFFS_OBJ_ID_IX_FLAG;
          p_hdr.span_ix = data_spndx;
          p_hdr.flags = 0xff;
 
          /* allocate new page and copy unmodified data */

          ret =
            spiffs_page_allocate_data(fs, fobj->objid & ~SPIFFS_OBJ_ID_IX_FLAG,
                                      &p_hdr, 0, 0, 0, 0, &new_data_pgndx);
          if (ret != OK)
            {
              break;
            }

          ret = spiffs_phys_cpy(fs, 0,
                                SPIFFS_PAGE_TO_PADDR(fs,
                                                     new_data_pgndx) +
                                sizeof(struct spiffs_page_header_s),
                                SPIFFS_PAGE_TO_PADDR(fs,
                                                     data_pgndx) +
                                sizeof(struct spiffs_page_header_s),
                                SPIFFS_DATA_PAGE_SIZE(fs) - bytes_to_remove);
          if (ret != OK)
            {
              break;
            }

          /* delete original data page */

          ret = spiffs_page_delete(fs, data_pgndx);
          if (ret != OK)
            {
              break;
            }

          p_hdr.flags &= ~SPIFFS_PH_FLAG_FINAL;
          ret = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_UPDT,
                           fobj->objid, SPIFFS_PAGE_TO_PADDR(fs, new_data_pgndx) +
                           offsetof(struct spiffs_page_header_s, flags), sizeof(uint8_t),
                           (uint8_t *) & p_hdr.flags);
          if (ret != OK)
            {
              break;
            }

          /* update memory representation of object index page with new data
           * page
           */

          if (cur_objndx_spndx == 0)
            {
              /* update object index header page */

              ((int16_t *) ((uint8_t *) objhdr +
                                   sizeof(struct spiffs_pgobj_ixheader_s)))
                [data_spndx] = new_data_pgndx;
              finfo("truncate: wrote page " _SPIPRIpg
                         " to objhdr entry " _SPIPRIsp " in mem\n",
                         new_data_pgndx, (int16_t) SPIFFS_OBJ_IX_ENTRY(fs,
                                                                            data_spndx));
            }
          else
            {
              /* update object index page */

              ((int16_t *) ((uint8_t *) objndx +
                                   sizeof(spiffs_page_object_ix)))
                [SPIFFS_OBJ_IX_ENTRY(fs, data_spndx)] = new_data_pgndx;
              finfo("truncate: wrote page " _SPIPRIpg " to objndx entry "
                         _SPIPRIsp " in mem\n", new_data_pgndx,
                         (int16_t) SPIFFS_OBJ_IX_ENTRY(fs, data_spndx));
            }

          cur_size = new_size;
          fobj->size = new_size;
          fobj->offset = cur_size;
          break;
        }

      data_spndx--;
    }

  /* update object indices */

  if (cur_objndx_spndx == 0)
    {
      /* update object index header page */

      if (cur_size == 0)
        {
          if (remove_full)
            {
              /* remove object altogether */

              finfo("truncate: remove object index header page " _SPIPRIpg
                         "\n", objndx_pgndx);

              ret = spiffs_page_index_check(fs, fobj, objndx_pgndx, 0);
              SPIFFS_CHECK_RES(ret);

              ret = spiffs_page_delete(fs, objndx_pgndx);
              SPIFFS_CHECK_RES(ret);
              spiffs_cb_object_event(fs, (spiffs_page_object_ix *) 0,
                                     SPIFFS_EV_IX_DEL, fobj->objid, 0, objndx_pgndx,
                                     0);
            }
          else
            {
              /* make uninitialized object */

              finfo("truncate: reset objhdr page " _SPIPRIpg "\n",
                         objndx_pgndx);
              memset(fs->work + sizeof(struct spiffs_pgobj_ixheader_s), 0xff,
                     SPIFFS_CFG_LOG_PAGE_SZ(fs) -
                     sizeof(struct spiffs_pgobj_ixheader_s));
              ret =
                spiffs_object_update_index_hdr(fs, fobj, fobj->objid, objndx_pgndx,
                                               fs->work, 0, SPIFFS_UNDEFINED_LEN,
                                               &new_objhdr_pgndx);
              SPIFFS_CHECK_RES(ret);
            }
        }
      else
        {
          /* update object index header page */

          finfo("truncate: update object index header page with indices and size\n");
          ret =
            spiffs_object_update_index_hdr(fs, fobj, fobj->objid, objndx_pgndx,
                                           fs->work, 0, cur_size,
                                           &new_objhdr_pgndx);
          SPIFFS_CHECK_RES(ret);
        }
    }
  else
    {
      /* update both current object index page and object index header page */

      int16_t new_objndx_pgndx;

      ret = spiffs_page_index_check(fs, fobj, objndx_pgndx, cur_objndx_spndx);
      SPIFFS_CHECK_RES(ret);

      /* move and update object index page */

      ret = spiffs_page_move(fs, fobj->objid, (uint8_t *) objhdr, fobj->objid, 0,
                             objndx_pgndx, &new_objndx_pgndx);
      SPIFFS_CHECK_RES(ret);
      spiffs_cb_object_event(fs, (spiffs_page_object_ix *) objhdr,
                             SPIFFS_EV_IX_UPD, fobj->objid, objndx->p_hdr.span_ix,
                             new_objndx_pgndx, 0);
      finfo("truncate: store modified objndx page, " _SPIPRIpg ":" _SPIPRIsp
                 "\n", new_objndx_pgndx, cur_objndx_spndx);
      fobj->objndx_pgndx = new_objndx_pgndx;
      fobj->objndx_spndx = cur_objndx_spndx;
      fobj->offset = cur_size;

      /* update object index header page with new size */

      ret = spiffs_object_update_index_hdr(fs, fobj, fobj->objid,
                                           fobj->objhdr_pgndx, 0, 0, cur_size,
                                           &new_objhdr_pgndx);
      SPIFFS_CHECK_RES(ret);
    }

  fobj->size = cur_size;
  return ret;
}

ssize_t spiffs_object_read(FAR struct spiffs_s *fs,
                           FAR struct spiffs_file_s *fobj, off_t offset,
                           size_t len, FAR uint8_t *dst)
{
  int32_t ret = OK;
  int16_t objndx_pgndx;
  int16_t data_pgndx;
  int16_t data_spndx = offset / SPIFFS_DATA_PAGE_SIZE(fs);
  uint32_t cur_offset = offset;
  int16_t cur_objndx_spndx;
  int16_t prev_objndx_spndx = (int16_t) - 1;
  FAR struct spiffs_pgobj_ixheader_s *objhdr =
    (FAR struct spiffs_pgobj_ixheader_s *) fs->work;
  spiffs_page_object_ix *objndx = (spiffs_page_object_ix *) fs->work;

  while (cur_offset < offset + len)
    {
      cur_objndx_spndx = SPIFFS_OBJ_IX_ENTRY_SPAN_IX(fs, data_spndx);
      if (prev_objndx_spndx != cur_objndx_spndx)
        {
          /* load current object index (header) page */

          if (cur_objndx_spndx == 0)
            {
              objndx_pgndx = fobj->objhdr_pgndx;
            }
          else
            {
              finfo("read: find objndx " _SPIPRIid ":" _SPIPRIsp "\n",
                     fobj->objid, cur_objndx_spndx);
              if (fobj->objndx_spndx == cur_objndx_spndx)
                {
                  objndx_pgndx = fobj->objndx_pgndx;
                }
              else
                {
                  ret = spiffs_obj_lu_find_id_and_span(fs,
                                                       fobj->objid | SPIFFS_OBJ_ID_IX_FLAG,
                                                       cur_objndx_spndx, 0,
                                                       &objndx_pgndx);
                  if (ret < 0)
                    {
                      return ret;
                    }
                }
            }

          finfo("read: load objndx page %d:%d for data spndx=%d\n",
                objndx_pgndx, cur_objndx_spndx, data_spndx);

          ret = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_IX | SPIFFS_OP_C_READ,
                           fobj->objid, SPIFFS_PAGE_TO_PADDR(fs, objndx_pgndx),
                           SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->work);
          if (ret < 0)
            {
              return ret;
            }

          SPIFFS_VALIDATE_OBJIX(objndx->p_hdr, fobj->objid, cur_objndx_spndx);

          fobj->offset       = cur_offset;
          fobj->objndx_pgndx = objndx_pgndx;
          fobj->objndx_spndx = cur_objndx_spndx;
          prev_objndx_spndx  = cur_objndx_spndx;
        }

      if (cur_objndx_spndx == 0)
        {
          /* Get data page from object index header page */

          data_pgndx = ((int16_t *)
            ((uint8_t *) objhdr +
             sizeof(struct spiffs_pgobj_ixheader_s)))[data_spndx];
        }
      else
        {
          /* Get data page from object index page */

          data_pgndx = ((int16_t *)
            ((uint8_t *) objndx +
             sizeof(spiffs_page_object_ix)))[SPIFFS_OBJ_IX_ENTRY(fs, data_spndx)];
        }

      /* All remaining data */

      uint32_t len_to_read = offset + len - cur_offset;

      /* Remaining data in page */

      len_to_read = MIN(len_to_read,
                        SPIFFS_DATA_PAGE_SIZE(fs) -
                        (cur_offset % SPIFFS_DATA_PAGE_SIZE(fs)));

      /* Remaining data in file */

      len_to_read = MIN(len_to_read, fobj->size);

      finfo("read: offset=%d rd=%d data spndx=%d is data_pgndx=%d addr=%p\n",
            cur_offset, len_to_read, data_spndx, data_pgndx,
            (SPIFFS_PAGE_TO_PADDR(fs, data_pgndx) +
             sizeof(struct spiffs_page_header_s) +
             (cur_offset % SPIFFS_DATA_PAGE_SIZE(fs))));

      if (len_to_read <= 0)
        {
          ret = SPIFFS_ERR_END_OF_OBJECT;
          break;
        }

      ret = spiffs_page_data_check(fs, fobj, data_pgndx, data_spndx);
      if (ret < 0)
        {
          return ret;
        }

      ret = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_READ,
                       fobj->objid, SPIFFS_PAGE_TO_PADDR(fs, data_pgndx) +
                       sizeof(struct spiffs_page_header_s) +
                       (cur_offset % SPIFFS_DATA_PAGE_SIZE(fs)), len_to_read,
                       dst);
      if (ret < 0)
        {
          return ret;
        }

      dst         += len_to_read;
      cur_offset  += len_to_read;
      fobj->offset = cur_offset;
      data_spndx++;
    }

  return ret;
}

static int32_t spiffs_obj_lu_find_free_obj_id_bitmap_v(FAR struct spiffs_s *fs,
                                                       int16_t objid,
                                                       int16_t blkndx,
                                                       int ix_entry,
                                                       const void *user_const_p,
                                                       void *user_var_p)
{
  if (objid != SPIFFS_OBJ_ID_FREE && objid != SPIFFS_OBJ_ID_DELETED)
    {
      int16_t min_obj_id = *((int16_t *) user_var_p);
      const uint8_t *conflicting_name = (const uint8_t *)user_const_p;

      /* if conflicting name parameter is given, also check if this name is
       * found in object index hdrs
       */

      if (conflicting_name && (objid & SPIFFS_OBJ_ID_IX_FLAG))
        {
          int16_t pgndx =
            SPIFFS_OBJ_LOOKUP_ENTRY_TO_PIX(fs, blkndx, ix_entry);
          int ret;
          struct spiffs_pgobj_ixheader_s objhdr;
          ret = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ,
                           0, SPIFFS_PAGE_TO_PADDR(fs, pgndx),
                           sizeof(struct spiffs_pgobj_ixheader_s),
                           (uint8_t *) & objhdr);
          SPIFFS_CHECK_RES(ret);
          if (objhdr.p_hdr.span_ix == 0 &&
              (objhdr.p_hdr.
               flags & (SPIFFS_PH_FLAG_DELET | SPIFFS_PH_FLAG_FINAL |
                        SPIFFS_PH_FLAG_IXDELE)) ==
              (SPIFFS_PH_FLAG_DELET | SPIFFS_PH_FLAG_IXDELE))
            {
              if (strcmp((const char *)user_const_p, (char *)objhdr.name) ==
                  0)
                {
                  return -EEXIST;
                }
            }
        }

      objid &= ~SPIFFS_OBJ_ID_IX_FLAG;
      uint32_t bit_ix = (objid - min_obj_id) & 7;
      int byte_ix = (objid - min_obj_id) >> 3;
      if (byte_ix >= 0 && (uint32_t) byte_ix < SPIFFS_CFG_LOG_PAGE_SZ(fs))
        {
          fs->work[byte_ix] |= (1 << bit_ix);
        }
    }

  return SPIFFS_VIS_COUNTINUE;
}

static int32_t spiffs_obj_lu_find_free_obj_id_compact_v(FAR struct spiffs_s *fs,
                                                        int16_t objid,
                                                        int16_t blkndx,
                                                        int ix_entry,
                                                        const void *user_const_p,
                                                        void *user_var_p)
{
  if (objid != SPIFFS_OBJ_ID_FREE && objid != SPIFFS_OBJ_ID_DELETED &&
      (objid & SPIFFS_OBJ_ID_IX_FLAG))
    {
      int32_t ret;
      const spiffs_free_obj_id_state *state =
        (const spiffs_free_obj_id_state *)user_const_p;
      struct spiffs_pgobj_ixheader_s objhdr;

      ret = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ,
                       0, SPIFFS_OBJ_LOOKUP_ENTRY_TO_PADDR(fs, blkndx, ix_entry),
                       sizeof(struct spiffs_pgobj_ixheader_s),
                       (uint8_t *) & objhdr);
      if (ret == OK && objhdr.p_hdr.span_ix == 0 &&
          ((objhdr.p_hdr.
            flags & (SPIFFS_PH_FLAG_INDEX | SPIFFS_PH_FLAG_FINAL |
                     SPIFFS_PH_FLAG_DELET)) == (SPIFFS_PH_FLAG_DELET)))
        {
          /* ok object look up entry */

          if (state->conflicting_name &&
              strcmp((const char *)state->conflicting_name, (char *)objhdr.name) == 0)
            {
              return -EEXIST;
            }

          objid &= ~SPIFFS_OBJ_ID_IX_FLAG;
          if (objid >= state->min_obj_id && objid <= state->max_obj_id)
            {
              uint8_t *map = (uint8_t *) fs->work;
              int ix = (objid - state->min_obj_id) / state->compaction;
              map[ix]++;
            }
        }
    }

  return SPIFFS_VIS_COUNTINUE;
}

/* Scans thru all object lookup for object index header pages. If total possible number of
 * object ids cannot fit into a work buffer, these are grouped. When a group containing free
 * object ids is found, the object lu is again scanned for object ids within group and bitmasked.
 * Finally, the bitmask is searched for a free objid
 */

int32_t spiffs_obj_lu_find_free_obj_id(FAR struct spiffs_s *fs, int16_t *objid,
                                       const uint8_t *conflicting_name)
{
  int32_t ret = OK;
  uint32_t max_objects = (fs->geo.neraseblocks * SPIFFS_OBJ_LOOKUP_MAX_ENTRIES(fs)) / 2;
  spiffs_free_obj_id_state state;
  int16_t free_obj_id = SPIFFS_OBJ_ID_FREE;

  state.min_obj_id = 1;
  state.max_obj_id = max_objects + 1;
  if (state.max_obj_id & SPIFFS_OBJ_ID_IX_FLAG)
    {
      state.max_obj_id = ((int16_t) - 1) & ~SPIFFS_OBJ_ID_IX_FLAG;
    }

  state.compaction = 0;
  state.conflicting_name = conflicting_name;
  while (ret == OK && free_obj_id == SPIFFS_OBJ_ID_FREE)
    {
      if (state.max_obj_id - state.min_obj_id <=
          (int16_t) SPIFFS_CFG_LOG_PAGE_SZ(fs) * 8)
        {
          /* possible to represent in bitmap */

          uint32_t i, j;
          finfo("free_obj_id: BITM min:" _SPIPRIid " max:" _SPIPRIid "\n",
                     state.min_obj_id, state.max_obj_id);

          memset(fs->work, 0, SPIFFS_CFG_LOG_PAGE_SZ(fs));
          ret =
            spiffs_foreach_objlu(fs, 0, 0, 0, 0,
                                             spiffs_obj_lu_find_free_obj_id_bitmap_v,
                                             conflicting_name,
                                             &state.min_obj_id, 0, 0);
          if (ret == SPIFFS_VIS_END)
            {
              ret = OK;
            }

          SPIFFS_CHECK_RES(ret);

          /* traverse bitmask until found free objid */

          for (i = 0; i < SPIFFS_CFG_LOG_PAGE_SZ(fs); i++)
            {
              uint8_t mask = fs->work[i];
              if (mask == 0xff)
                {
                  continue;
                }

              for (j = 0; j < 8; j++)
                {
                  if ((mask & (1 << j)) == 0)
                    {
                      *objid = (i << 3) + j + state.min_obj_id;
                      return OK;
                    }
                }
            }

          return -ENOSPC;
        }
      else
        {
          /* not possible to represent all ids in range in a bitmap, compact
           * and count
           */

          if (state.compaction != 0)
            {
              /* select element in compacted table, decrease range and
               * recompact
               */

              uint32_t i;
              uint32_t min_i = 0;
              uint8_t *map = (uint8_t *) fs->work;
              uint8_t min_count = 0xff;

              for (i = 0; i < SPIFFS_CFG_LOG_PAGE_SZ(fs) / sizeof(uint8_t); i++)
                {
                  if (map[i] < min_count)
                    {
                      min_count = map[i];
                      min_i = i;
                      if (min_count == 0)
                        {
                          break;
                        }
                    }
                }

              if (min_count == state.compaction)
                {
                  /* there are no free objids! */

                  finfo("free_obj_id: compacted table is full\n");
                  return -ENOSPC;
                }

              finfo("free_obj_id: COMP select index:" _SPIPRIi
                         " min_count:" _SPIPRIi " min:" _SPIPRIid " max:"
                         _SPIPRIid " compact:" _SPIPRIi "\n", min_i, min_count,
                         state.min_obj_id, state.max_obj_id, state.compaction);

              if (min_count == 0)
                {
                  /* no objid in this range, skip compacting and use directly */

                  *objid = min_i * state.compaction + state.min_obj_id;
                  return OK;
                }
              else
                {
                  finfo("free_obj_id: COMP SEL chunk:" _SPIPRIi " min:"
                             _SPIPRIid " -> " _SPIPRIid "\n", state.compaction,
                             state.min_obj_id,
                             state.min_obj_id + min_i * state.compaction);
                  state.min_obj_id += min_i * state.compaction;
                  state.max_obj_id = state.min_obj_id + state.compaction;

                  /* decrease compaction */
                }

              if ((state.max_obj_id - state.min_obj_id <=
                   (int16_t) SPIFFS_CFG_LOG_PAGE_SZ(fs) * 8))
                {
                  /* no need for compacting, use bitmap */

                  continue;
                }
            }

          /* in a work memory of log_page_size bytes, we may fit in
           * log_page_size ids
           * todo what if compaction is > 255 - then we cannot fit it in a byte
           */

          state.compaction =
            (state.max_obj_id -
             state.min_obj_id) / ((SPIFFS_CFG_LOG_PAGE_SZ(fs) / sizeof(uint8_t)));

          finfo("free_obj_id: COMP min:" _SPIPRIid " max:" _SPIPRIid
                     " compact:" _SPIPRIi "\n", state.min_obj_id,
                     state.max_obj_id, state.compaction);

          memset(fs->work, 0, SPIFFS_CFG_LOG_PAGE_SZ(fs));
          ret =
            spiffs_foreach_objlu(fs, 0, 0, 0, 0,
                                             spiffs_obj_lu_find_free_obj_id_compact_v,
                                             &state, 0, 0, 0);
          if (ret == SPIFFS_VIS_END)
            {
              ret = OK;
            }

          SPIFFS_CHECK_RES(ret);
          state.conflicting_name = 0;   /* searched for conflicting name once,
                                         * no need to do it again */
        }
    }

  return ret;
}

/* Given a object ID, find the corresponding file object instance */

int spiffs_find_fobj_byobjid(FAR struct spiffs_s *fs, int16_t objid,
                             FAR struct spiffs_file_s **ppfobj)
{
  FAR struct spiffs_file_s *fobj;
  int ret = -ENOENT;

  for (fobj  = (FAR struct spiffs_file_s *)dq_peek(&fs->objq);
       fobj != NULL;
       fobj  = (FAR struct spiffs_file_s *)dq_next((FAR dq_entry_t *)fobj))
    {
      if (fobj->objid == objid)
        {
          ret = OK;
          break;
        }
    }

  if (ppfobj != NULL)
    {
      *ppfobj = fobj;
    }

  return ret;
}

/* Given the page index of the object header, find the corresponding file
 * object instance.
 */

int spiffs_find_fobj_bypgndx(FAR struct spiffs_s *fs, int16_t pgndx,
                             FAR struct spiffs_file_s **ppfobj)
{
  FAR struct spiffs_file_s *fobj;
  int ret = -ENOENT;

  for (fobj  = (FAR struct spiffs_file_s *)dq_peek(&fs->objq);
       fobj != NULL;
       fobj  = (FAR struct spiffs_file_s *)dq_next((FAR dq_entry_t *)fobj))
    {
      if (fobj->objhdr_pgndx == pgndx)
        {
          ret = OK;
          break;
        }
    }

  if (ppfobj != NULL)
    {
      *ppfobj = fobj;
    }

  return ret;
}
