/****************************************************************************
 * fs/spiffs.h/spiffs_check.c
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

/* Contains functionality for checking file system consistency
 * and mending problems.
 * Three levels of consistency checks are implemented:
 *
 * Look up consistency
 *   Checks if indices in lookup pages are coherent with page headers
 * Object index consistency
 *   Checks if there are any orphaned object indices (missing object index
 *     headers).
 *   If an object index is found but not its header, the object index is
 *     deleted.
 *   This is critical for the following page consistency check.
 * Page consistency
 *   Checks for pages that ought to be indexed, ought not to be indexed, are
 *     multiple indexed
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include "spiffs.h"
#include "spiffs_nucleus.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define CHECK_CB(_fs, _type, _rep, _arg1, _arg2) \
   do \
     { \
       if ((_fs)->check_cb != NULL) \
         (_fs)->check_cb((_type), (_rep), (_arg1), (_arg2)); \
     } \
   while (0)

/* Look up consistency */

/* searches in the object indices and returns the referenced page index given
 * the object id and the data span index
 * destroys fs->lu_work
 */

static int32_t spiffs_object_get_data_page_index_reference(FAR struct spiffs_s *fs,
                                                           int16_t id,
                                                           int16_t
                                                           data_spix,
                                                           int16_t * pix,
                                                           int16_t *
                                                           objix_pix)
{
  uint32_t addr;
  int32_t res;

  /* calculate object index span index for given data page span index */

  int16_t objix_spix = SPIFFS_OBJ_IX_ENTRY_SPAN_IX(fs, data_spix);

  /* find obj index for obj id and span index */

  res = spiffs_obj_lu_find_id_and_span(fs, id | SPIFFS_OBJ_ID_IX_FLAG,
                                       objix_spix, 0, objix_pix);
  SPIFFS_CHECK_RES(res);

  /* load obj index entry */

  addr = SPIFFS_PAGE_TO_PADDR(fs, *objix_pix);
  if (objix_spix == 0)
    {
      /* get referenced page from object index header */

      addr += sizeof(struct spiffs_pgobj_ixheader_s) +
              data_spix * sizeof(int16_t);
    }
  else
    {
      /* get referenced page from object index */

      addr += sizeof(spiffs_page_object_ix) +
              SPIFFS_OBJ_IX_ENTRY(fs, data_spix) *
              sizeof(int16_t);
    }

  res = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ, 0, addr,
                   sizeof(int16_t), (uint8_t *) pix);

  return res;
}

/* copies page contents to a new page */

static int32_t spiffs_rewrite_page(FAR struct spiffs_s *fs, int16_t cur_pix,
                                   struct spiffs_page_header_s * p_hdr,
                                   int16_t * new_pix)
{
  int32_t res;

  res = spiffs_page_allocate_data(fs, p_hdr->id, p_hdr, 0, 0, 0, 0,
                                  new_pix);
  SPIFFS_CHECK_RES(res);
  res = spiffs_phys_cpy(fs, 0,
                        SPIFFS_PAGE_TO_PADDR(fs, *new_pix) +
                        sizeof(struct spiffs_page_header_s),
                        SPIFFS_PAGE_TO_PADDR(fs, cur_pix) +
                        sizeof(struct spiffs_page_header_s),
                        SPIFFS_DATA_PAGE_SIZE(fs));
  SPIFFS_CHECK_RES(res);
  return res;
}

/* rewrites the object index for given object id and replaces the
 * data page index to a new page index
 */

static int32_t spiffs_rewrite_index(FAR struct spiffs_s *fs, int16_t id,
                                   int16_t data_spix,
                                   int16_t new_data_pix,
                                   int16_t objix_pix)
{
  int32_t res;
  int16_t bix;
  int entry;
  int16_t free_pix;

  id |= SPIFFS_OBJ_ID_IX_FLAG;

  /* find free entry */

  res = spiffs_obj_lu_find_free(fs, fs->free_cursor_block_ix,
                                fs->free_cursor_obj_lu_entry, &bix, &entry);
  SPIFFS_CHECK_RES(res);
  free_pix = SPIFFS_OBJ_LOOKUP_ENTRY_TO_PIX(fs, bix, entry);

  /* calculate object index span index for given data page span index */

  int16_t objix_spix = SPIFFS_OBJ_IX_ENTRY_SPAN_IX(fs, data_spix);
  if (objix_spix == 0)
    {
      /* calc index in index header */

      entry = data_spix;
    }
  else
    {
      /* calc entry in index */

      entry = SPIFFS_OBJ_IX_ENTRY(fs, data_spix);
    }

  /* load index */

  res = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ,
                   0, SPIFFS_PAGE_TO_PADDR(fs, objix_pix),
                   SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->lu_work);
  SPIFFS_CHECK_RES(res);
  struct spiffs_page_header_s *objix_p_hdr = (struct spiffs_page_header_s *) fs->lu_work;

  /* be ultra safe, double check header against provided data */

  if (objix_p_hdr->id != id)
    {
      spiffs_page_delete(fs, free_pix);
      return SPIFFS_ERR_CHECK_OBJ_ID_MISM;
    }

  if (objix_p_hdr->span_ix != objix_spix)
    {
      spiffs_page_delete(fs, free_pix);
      return SPIFFS_ERR_CHECK_SPIX_MISM;
    }

  if ((objix_p_hdr->flags & (SPIFFS_PH_FLAG_USED | SPIFFS_PH_FLAG_IXDELE |
                             SPIFFS_PH_FLAG_INDEX | SPIFFS_PH_FLAG_FINAL |
                             SPIFFS_PH_FLAG_DELET)) !=
      (SPIFFS_PH_FLAG_IXDELE | SPIFFS_PH_FLAG_DELET))
    {
      spiffs_page_delete(fs, free_pix);
      return SPIFFS_ERR_CHECK_FLAGS_BAD;
    }

  /* rewrite in mem */

  if (objix_spix == 0)
    {
      ((int16_t *) ((uint8_t *) fs->lu_work +
                           sizeof(struct spiffs_pgobj_ixheader_s)))[data_spix] =
        new_data_pix;
    }
  else
    {
      ((int16_t *) ((uint8_t *) fs->lu_work +
                           sizeof(spiffs_page_object_ix)))[SPIFFS_OBJ_IX_ENTRY
                                                           (fs, data_spix)] =
        new_data_pix;
    }

  res = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_UPDT,
                   0, SPIFFS_PAGE_TO_PADDR(fs, free_pix),
                   SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->lu_work);
  SPIFFS_CHECK_RES(res);
  res = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_LU | SPIFFS_OP_C_UPDT,
                   0, SPIFFS_BLOCK_TO_PADDR(fs, SPIFFS_BLOCK_FOR_PAGE(fs,free_pix)) +
                   SPIFFS_OBJ_LOOKUP_ENTRY_FOR_PAGE(fs, free_pix) *
                   sizeof(int16_t), sizeof(int16_t),
                   (uint8_t *) & id);
  SPIFFS_CHECK_RES(res);
  res = spiffs_page_delete(fs, objix_pix);

  return res;
}

/* deletes an object just by marking object index header as deleted */

static int32_t spiffs_delete_obj_lazy(FAR struct spiffs_s *fs, int16_t id)
{
  int16_t objix_hdr_pix;
  uint8_t flags = 0xff;
  int32_t res;

  res = spiffs_obj_lu_find_id_and_span(fs, id, 0, 0, &objix_hdr_pix);
  if (res == SPIFFS_ERR_NOT_FOUND)
    {
      return OK;
    }

  SPIFFS_CHECK_RES(res);
#if SPIFFS_NO_BLIND_WRITES
  res = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU | SPIFFS_OP_C_READ, 0,
                   SPIFFS_PAGE_TO_PADDR(fs, objix_hdr_pix) +
                   offsetof(struct spiffs_page_header_s, flags), sizeof(flags),
                   &flags);
  SPIFFS_CHECK_RES(res);
#endif
  flags &= ~SPIFFS_PH_FLAG_IXDELE;
  res = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_LU | SPIFFS_OP_C_UPDT, 0,
                   SPIFFS_PAGE_TO_PADDR(fs, objix_hdr_pix) +
                   offsetof(struct spiffs_page_header_s, flags), sizeof(flags),
                   &flags);
  return res;
}

/* validates the given look up entry */

static int32_t spiffs_lookup_check_validate(FAR struct spiffs_s *fs, int16_t lu_obj_id,
                                            struct spiffs_page_header_s * p_hdr,
                                            int16_t cur_pix,
                                            int16_t cur_block,
                                            int cur_entry, int *reload_lu)
{
  uint8_t delete_page = 0;
  int32_t res = OK;
  int16_t objix_pix;
  int16_t ref_pix;

  (void)cur_block;
  (void)cur_entry;

  /* check validity, take actions */

  if (((lu_obj_id == SPIFFS_OBJ_ID_DELETED) &&
       (p_hdr->flags & SPIFFS_PH_FLAG_DELET)) ||
      ((lu_obj_id == SPIFFS_OBJ_ID_FREE) &&
       (p_hdr->flags & SPIFFS_PH_FLAG_USED) == 0))
    {
      /* look up entry deleted / free but used in page header */

      spiffs_checkinfo("LU: pix " _SPIPRIpg
                       " deleted/free in lu but not on page\n", cur_pix);
      *reload_lu = 1;
      delete_page = 1;
      if (p_hdr->flags & SPIFFS_PH_FLAG_INDEX)
        {
          /* header says data page
           * data page can be removed if not referenced by some object index
           */

          res = spiffs_object_get_data_page_index_reference(fs, p_hdr->id,
                                                            p_hdr->span_ix,
                                                            &ref_pix, &objix_pix);
          if (res == SPIFFS_ERR_NOT_FOUND)
            {
              /* no object with this id, so remove page safely */

              res = OK;
            }
          else
            {
              SPIFFS_CHECK_RES(res);
              if (ref_pix == cur_pix)
                {
                  /* data page referenced by object index but deleted in lu
                   * copy page to new place and re-write the object index to
                   * new place
                   */

                  int16_t new_pix;
                  res = spiffs_rewrite_page(fs, cur_pix, p_hdr, &new_pix);

                  spiffs_checkinfo
                    ("LU: FIXUP: data page not found elsewhere, rewriting "
                     _SPIPRIpg " to new page " _SPIPRIpg "\n", cur_pix,
                     new_pix);
                  SPIFFS_CHECK_RES(res);

                  *reload_lu = 1;

                  spiffs_checkinfo("LU: FIXUP: " _SPIPRIpg " rewritten to "
                                   _SPIPRIpg ", affected objix_pix " _SPIPRIpg
                                   "\n", cur_pix, new_pix, objix_pix);
                  res = spiffs_rewrite_index(fs, p_hdr->id, p_hdr->span_ix,
                                             new_pix, objix_pix);
                  if (res <= _SPIFFS_ERR_CHECK_FIRST &&
                      res > _SPIFFS_ERR_CHECK_LAST)
                    {
                      /* index bad also, cannot mend this file */

                      spiffs_checkinfo("LU: FIXUP: index bad " _SPIPRIi
                                       ", cannot mend!\n", res);
                      res = spiffs_page_delete(fs, new_pix);
                      SPIFFS_CHECK_RES(res);
                      res = spiffs_delete_obj_lazy(fs, p_hdr->id);
                      CHECK_CB(fs, SPIFFS_CHECK_LOOKUP,
                               SPIFFS_CHECK_DELETE_BAD_FILE, p_hdr->id, 0);
                    }
                  else
                    {
                      CHECK_CB(fs, SPIFFS_CHECK_LOOKUP, SPIFFS_CHECK_FIX_INDEX,
                               p_hdr->id, p_hdr->span_ix);
                    }

                  SPIFFS_CHECK_RES(res);
                }
            }
        }
      else
        {
          /* header says index page
           * index page can be removed if other index with same id and
           * span index is found
           */

          res = spiffs_obj_lu_find_id_and_span(fs,
                                               p_hdr->id | SPIFFS_OBJ_ID_IX_FLAG,
                                               p_hdr->span_ix, cur_pix, 0);
          if (res == SPIFFS_ERR_NOT_FOUND)
            {
              /* no such index page found, check for a data page amongst page
               * headers.  lu cannot be trusted
               */

              res = spiffs_obj_lu_find_id_and_span_by_phdr(fs,
                                                           p_hdr->id | SPIFFS_OBJ_ID_IX_FLAG,
                                                           0, 0, 0);
              if (res == OK)
                {
                  /* ignore other errors
                   * got a data page also, assume lu corruption only, rewrite
                   * to new page
                   */

                  int16_t new_pix;
                  res = spiffs_rewrite_page(fs, cur_pix, p_hdr, &new_pix);
                  spiffs_checkinfo
                    ("LU: FIXUP: ix page with data not found elsewhere, rewriting "
                     _SPIPRIpg " to new page " _SPIPRIpg "\n", cur_pix,
                     new_pix);
                  SPIFFS_CHECK_RES(res);
                  *reload_lu = 1;
                  CHECK_CB(fs, SPIFFS_CHECK_LOOKUP, SPIFFS_CHECK_FIX_LOOKUP,
                           p_hdr->id, p_hdr->span_ix);
                }
            }
          else
            {
              SPIFFS_CHECK_RES(res);
            }
        }
    }

  if (lu_obj_id != SPIFFS_OBJ_ID_FREE && lu_obj_id != SPIFFS_OBJ_ID_DELETED)
    {
      /* look up entry used */

      if ((p_hdr->id | SPIFFS_OBJ_ID_IX_FLAG) !=
          (lu_obj_id | SPIFFS_OBJ_ID_IX_FLAG))
        {
          spiffs_checkinfo("LU: pix " _SPIPRIpg " differ in id lu:"
                           _SPIPRIid " ph:" _SPIPRIid "\n", cur_pix, lu_obj_id,
                           p_hdr->id);
          delete_page = 1;
          if ((p_hdr->flags & SPIFFS_PH_FLAG_DELET) == 0 ||
              (p_hdr->flags & SPIFFS_PH_FLAG_FINAL) ||
              (p_hdr->flags & (SPIFFS_PH_FLAG_INDEX | SPIFFS_PH_FLAG_IXDELE)) ==
              0)
            {
              /* page deleted or not finalized, just remove it */
            }
          else
            {
              if (p_hdr->flags & SPIFFS_PH_FLAG_INDEX)
                {
                  /* if data page, check for reference to this page */

                  res = spiffs_object_get_data_page_index_reference(fs,
                                                                    p_hdr->id,
                                                                    p_hdr->span_ix,
                                                                    &ref_pix,
                                                                    &objix_pix);
                  if (res == SPIFFS_ERR_NOT_FOUND)
                    {
                      /* no object with this id, so remove page safely */

                      res = OK;
                    }
                  else
                    {
                      SPIFFS_CHECK_RES(res);

                      /* if found, rewrite page with object id, update index,
                       * and delete current
                       */

                      if (ref_pix == cur_pix)
                        {
                          int16_t new_pix;
                          res = spiffs_rewrite_page(fs, cur_pix, p_hdr, &new_pix);
                          SPIFFS_CHECK_RES(res);
                          res = spiffs_rewrite_index(fs, p_hdr->id,
                                                     p_hdr->span_ix, new_pix,
                                                     objix_pix);
                          if (res <= _SPIFFS_ERR_CHECK_FIRST &&
                              res > _SPIFFS_ERR_CHECK_LAST)
                            {
                              /* index bad also, cannot mend this file */

                              spiffs_checkinfo("LU: FIXUP: index bad " _SPIPRIi
                                               ", cannot mend!\n", res);
                              res = spiffs_page_delete(fs, new_pix);
                              SPIFFS_CHECK_RES(res);
                              res = spiffs_delete_obj_lazy(fs, p_hdr->id);
                              *reload_lu = 1;
                              CHECK_CB(fs, SPIFFS_CHECK_LOOKUP,
                                       SPIFFS_CHECK_DELETE_BAD_FILE,
                                       p_hdr->id, 0);
                            }

                          SPIFFS_CHECK_RES(res);
                        }
                    }
                }
              else
                {
                  /* else if index, check for other pages with both id's
                   * and span index
                   */

                  int16_t objix_pix_lu, objix_pix_ph;

                  /* see if other object index page exists for lookup obj id
                   * and span index
                   */

                  res = spiffs_obj_lu_find_id_and_span(fs,
                                                       lu_obj_id | SPIFFS_OBJ_ID_IX_FLAG,
                                                       p_hdr->span_ix, 0,
                                                       &objix_pix_lu);
                  if (res == SPIFFS_ERR_NOT_FOUND)
                    {
                      res = OK;
                      objix_pix_lu = 0;
                    }

                  SPIFFS_CHECK_RES(res);

                  /* see if other object index exists for page header obj id
                   * and span index
                   */

                  res = spiffs_obj_lu_find_id_and_span(fs,
                                                       p_hdr->id | SPIFFS_OBJ_ID_IX_FLAG,
                                                       p_hdr->span_ix, 0,
                                                       &objix_pix_ph);
                  if (res == SPIFFS_ERR_NOT_FOUND)
                    {
                      res = OK;
                      objix_pix_ph = 0;
                    }

                  SPIFFS_CHECK_RES(res);

                  /* if both id's found, just delete current */

                  if (objix_pix_ph == 0 || objix_pix_lu == 0)
                    {
                      /* otherwise try finding first corresponding data pages */

                      int16_t data_pix_lu, data_pix_ph;

                      /* see if other data page exists for look up obj id and
                       * span index
                       */

                      res = spiffs_obj_lu_find_id_and_span(fs,
                                                           lu_obj_id & ~SPIFFS_OBJ_ID_IX_FLAG,
                                                           0, 0, &data_pix_lu);
                      if (res == SPIFFS_ERR_NOT_FOUND)
                        {
                          res = OK;
                          objix_pix_lu = 0;
                        }

                      SPIFFS_CHECK_RES(res);

                      /* see if other data page exists for page header obj id
                       * and span index
                       */

                      res = spiffs_obj_lu_find_id_and_span(fs,
                                                           p_hdr->id & ~SPIFFS_OBJ_ID_IX_FLAG,
                                                           0, 0, &data_pix_ph);
                      if (res == SPIFFS_ERR_NOT_FOUND)
                        {
                          res = OK;
                          objix_pix_ph = 0;
                        }

                      SPIFFS_CHECK_RES(res);

                      struct spiffs_page_header_s new_ph;
                      new_ph.flags = 0xff & ~(SPIFFS_PH_FLAG_USED |
                                              SPIFFS_PH_FLAG_INDEX |
                                              SPIFFS_PH_FLAG_FINAL);
                      new_ph.span_ix = p_hdr->span_ix;
                      int16_t new_pix;

                      if ((objix_pix_lu && data_pix_lu && data_pix_ph && objix_pix_ph == 0) ||
                          (objix_pix_lu == 0 && data_pix_ph && objix_pix_ph == 0))
                        {
                          /* got a data page for page header obj id
                           * rewrite as obj_id_ph
                           */

                          new_ph.id = p_hdr->id | SPIFFS_OBJ_ID_IX_FLAG;
                          res = spiffs_rewrite_page(fs, cur_pix, &new_ph, &new_pix);
                          spiffs_checkinfo("LU: FIXUP: rewrite page " _SPIPRIpg
                                           " as " _SPIPRIid " to pix " _SPIPRIpg
                                           "\n", cur_pix, new_ph.id,
                                           new_pix);
                          CHECK_CB(fs, SPIFFS_CHECK_LOOKUP,
                                   SPIFFS_CHECK_FIX_LOOKUP, p_hdr->id,
                                   p_hdr->span_ix);
                          SPIFFS_CHECK_RES(res);
                          *reload_lu = 1;
                        }
                      else
                        if ((objix_pix_ph && data_pix_ph && data_pix_lu && objix_pix_lu == 0) ||
                            (objix_pix_ph == 0 && data_pix_lu && objix_pix_lu == 0))
                        {
                          /* got a data page for look up obj id
                           * rewrite as obj_id_lu
                           */

                          new_ph.id = lu_obj_id | SPIFFS_OBJ_ID_IX_FLAG;
                          spiffs_checkinfo("LU: FIXUP: rewrite page " _SPIPRIpg
                                           " as " _SPIPRIid "\n", cur_pix,
                                           new_ph.id);
                          CHECK_CB(fs, SPIFFS_CHECK_LOOKUP,
                                   SPIFFS_CHECK_FIX_LOOKUP, p_hdr->id,
                                   p_hdr->span_ix);
                          res = spiffs_rewrite_page(fs, cur_pix, &new_ph, &new_pix);
                          SPIFFS_CHECK_RES(res);
                          *reload_lu = 1;
                        }
                      else
                        {
                          /* cannot safely do anything */

                          spiffs_checkinfo
                            ("LU: FIXUP: nothing to do, just delete\n");
                        }
                    }
                }
            }
        }
      else
        if (((lu_obj_id & SPIFFS_OBJ_ID_IX_FLAG) &&
             (p_hdr->flags & SPIFFS_PH_FLAG_INDEX)) ||
            ((lu_obj_id & SPIFFS_OBJ_ID_IX_FLAG) == 0 &&
             (p_hdr->flags & SPIFFS_PH_FLAG_INDEX) == 0))
        {
          spiffs_checkinfo("LU: " _SPIPRIpg " lu/page index marking differ\n",
                           cur_pix);
          int16_t data_pix, objix_pix_d;

          /* see if other data page exists for given obj id and span index */

          res = spiffs_obj_lu_find_id_and_span(fs,
                                               lu_obj_id & ~SPIFFS_OBJ_ID_IX_FLAG,
                                               p_hdr->span_ix, cur_pix, &data_pix);
          if (res == SPIFFS_ERR_NOT_FOUND)
            {
              res = OK;
              data_pix = 0;
            }

          SPIFFS_CHECK_RES(res);

          /* see if other object index exists for given obj id and span index */

          res = spiffs_obj_lu_find_id_and_span(fs,
                                               lu_obj_id | SPIFFS_OBJ_ID_IX_FLAG,
                                               p_hdr->span_ix, cur_pix,
                                               &objix_pix_d);
          if (res == SPIFFS_ERR_NOT_FOUND)
            {
              res = OK;
              objix_pix_d = 0;
            }

          SPIFFS_CHECK_RES(res);

          delete_page = 1;

          /* if other data page exists and object index exists, just delete
           * page
           */

          if (data_pix && objix_pix_d)
            {
              spiffs_checkinfo
                ("LU: FIXUP: other index and data page exists, simply remove\n");
            }

          /* if only data page exists, make this page index */

          else if (data_pix && objix_pix_d == 0)
            {
              spiffs_checkinfo
                ("LU: FIXUP: other data page exists, make this index\n");
              CHECK_CB(fs, SPIFFS_CHECK_LOOKUP, SPIFFS_CHECK_FIX_INDEX,
                       lu_obj_id, p_hdr->span_ix);
              struct spiffs_page_header_s new_ph;
              int16_t new_pix;
              new_ph.flags = 0xff & ~(SPIFFS_PH_FLAG_USED |
                                      SPIFFS_PH_FLAG_FINAL |
                                      SPIFFS_PH_FLAG_INDEX);
              new_ph.id = lu_obj_id | SPIFFS_OBJ_ID_IX_FLAG;
              new_ph.span_ix = p_hdr->span_ix;
              res = spiffs_page_allocate_data(fs, new_ph.id, &new_ph,
                                              0, 0, 0, 1, &new_pix);
              SPIFFS_CHECK_RES(res);
              res = spiffs_phys_cpy(fs, 0, SPIFFS_PAGE_TO_PADDR(fs, new_pix) +
                                    sizeof(struct spiffs_page_header_s),
                                    SPIFFS_PAGE_TO_PADDR(fs, cur_pix) +
                                    sizeof(struct spiffs_page_header_s),
                                    SPIFFS_CFG_LOG_PAGE_SZ(fs) -
                                    sizeof(struct spiffs_page_header_s));
              SPIFFS_CHECK_RES(res);
            }

          /* if only index exists, make data page */

          else if (data_pix == 0 && objix_pix_d)
            {
              spiffs_checkinfo
                ("LU: FIXUP: other index page exists, make this data\n");
              CHECK_CB(fs, SPIFFS_CHECK_LOOKUP, SPIFFS_CHECK_FIX_LOOKUP,
                       lu_obj_id, p_hdr->span_ix);
              struct spiffs_page_header_s new_ph;
              int16_t new_pix;
              new_ph.flags = 0xff & ~(SPIFFS_PH_FLAG_USED | SPIFFS_PH_FLAG_FINAL);
              new_ph.id = lu_obj_id & ~SPIFFS_OBJ_ID_IX_FLAG;
              new_ph.span_ix = p_hdr->span_ix;
              res = spiffs_page_allocate_data(fs, new_ph.id, &new_ph,
                                              0, 0, 0, 1, &new_pix);
              SPIFFS_CHECK_RES(res);
              res = spiffs_phys_cpy(fs, 0, SPIFFS_PAGE_TO_PADDR(fs, new_pix) +
                                sizeof(struct spiffs_page_header_s),
                                SPIFFS_PAGE_TO_PADDR(fs, cur_pix) +
                                sizeof(struct spiffs_page_header_s),
                                SPIFFS_CFG_LOG_PAGE_SZ(fs) -
                                sizeof(struct spiffs_page_header_s));
              SPIFFS_CHECK_RES(res);
            }
          else
            {
              /* if nothing exists, we cannot safely make a decision - delete */
            }
        }
      else if ((p_hdr->flags & SPIFFS_PH_FLAG_DELET) == 0)
        {
          spiffs_checkinfo("LU: pix " _SPIPRIpg
                           " busy in lu but deleted on page\n", cur_pix);
          delete_page = 1;
        }
      else if ((p_hdr->flags & SPIFFS_PH_FLAG_FINAL))
        {
          spiffs_checkinfo("LU: pix " _SPIPRIpg " busy but not final\n",
                           cur_pix);

          /* page can be removed if not referenced by object index */

          *reload_lu = 1;
          res = spiffs_object_get_data_page_index_reference(fs, lu_obj_id,
                                                            p_hdr->span_ix,
                                                            &ref_pix, &objix_pix);
          if (res == SPIFFS_ERR_NOT_FOUND)
            {
              /* no object with this id, so remove page safely */

              res = OK;
              delete_page = 1;
            }
          else
            {
              SPIFFS_CHECK_RES(res);
              if (ref_pix != cur_pix)
                {
                  spiffs_checkinfo
                    ("LU: FIXUP: other finalized page is referred, just delete\n");
                  delete_page = 1;
                }
              else
                {
                  uint8_t flags = 0xff;

                  /* page referenced by object index but not final
                   * just finalize
                   */

                  spiffs_checkinfo
                    ("LU: FIXUP: unfinalized page is referred, finalizing\n");
                  CHECK_CB(fs, SPIFFS_CHECK_LOOKUP, SPIFFS_CHECK_FIX_LOOKUP,
                           p_hdr->id, p_hdr->span_ix);
#if SPIFFS_NO_BLIND_WRITES
                  res = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_READ,
                                   0, SPIFFS_PAGE_TO_PADDR(fs, cur_pix) +
                                   offsetof(struct spiffs_page_header_s, flags),
                                   sizeof(flags), &flags);
                  SPIFFS_CHECK_RES(res);
#endif
                  flags &= ~SPIFFS_PH_FLAG_FINAL;
                  res = _spiffs_wr(fs, SPIFFS_OP_T_OBJ_DA | SPIFFS_OP_C_UPDT,
                                   0, SPIFFS_PAGE_TO_PADDR(fs, cur_pix) +
                                   offsetof(struct spiffs_page_header_s, flags),
                                   sizeof(flags), &flags);
                }
            }
        }
    }

  if (delete_page)
    {
      spiffs_checkinfo("LU: FIXUP: deleting page " _SPIPRIpg "\n", cur_pix);
      CHECK_CB(fs, SPIFFS_CHECK_LOOKUP, SPIFFS_CHECK_DELETE_PAGE, cur_pix, 0);
      res = spiffs_page_delete(fs, cur_pix);
      SPIFFS_CHECK_RES(res);
    }

  return res;
}

static int32_t spiffs_lookup_check_v(FAR struct spiffs_s *fs, int16_t id,
                                     int16_t cur_block, int cur_entry,
                                     const void *user_const_p, void *user_var_p)
{
  (void)user_const_p;
  struct spiffs_page_header_s p_hdr;
  int16_t cur_pix =
    SPIFFS_OBJ_LOOKUP_ENTRY_TO_PIX(fs, cur_block, cur_entry);

  (void)user_var_p;
  int32_t res = OK;

  CHECK_CB(fs, SPIFFS_CHECK_LOOKUP, SPIFFS_CHECK_PROGRESS,
           (cur_block * 256) / fs->block_count, 0);

  /* load header */

  res = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ,
                   0, SPIFFS_PAGE_TO_PADDR(fs, cur_pix),
                   sizeof(struct spiffs_page_header_s), (uint8_t *) & p_hdr);
  SPIFFS_CHECK_RES(res);

  int reload_lu = 0;

  res = spiffs_lookup_check_validate(fs, id, &p_hdr, cur_pix, cur_block,
                                     cur_entry, &reload_lu);
  SPIFFS_CHECK_RES(res);

  if (res == OK)
    {
      return reload_lu ? SPIFFS_VIS_COUNTINUE_RELOAD : SPIFFS_VIS_COUNTINUE;
    }

  return res;
}

/* Scans all object look up. For each entry, corresponding page header is checked for validity.
 * If an object index header page is found, this is also checked
 */

int32_t spiffs_lookup_consistency_check(FAR struct spiffs_s *fs, uint8_t check_all_objects)
{
  int32_t res = OK;

  (void)check_all_objects;
  CHECK_CB(fs, SPIFFS_CHECK_LOOKUP, SPIFFS_CHECK_PROGRESS, 0, 0);

  res = spiffs_foreach_objlu(fs, 0, 0, 0, 0, spiffs_lookup_check_v,
                                         0, 0, 0, 0);

  if (res == SPIFFS_VIS_END)
    {
      res = OK;
    }

  if (res != OK)
    {
      CHECK_CB(fs, SPIFFS_CHECK_LOOKUP, SPIFFS_CHECK_ERROR, res, 0);
    }

  CHECK_CB(fs, SPIFFS_CHECK_LOOKUP, SPIFFS_CHECK_PROGRESS, 256, 0);
  return res;
}

/* Page consistency
 *
 * Scans all pages (except lu pages), reserves 4 bits in working memory for each page
 * bit 0: 0 == FREE|DELETED, 1 == USED
 * bit 1: 0 == UNREFERENCED, 1 == REFERENCED
 * bit 2: 0 == NOT_INDEX,    1 == INDEX
 * bit 3: unused
 * A consistent file system will have only pages being
 *  - x000 free, unreferenced, not index
 *  - x011 used, referenced only once, not index
 *  - x101 used, unreferenced, index
 * The working memory might not fit all pages so several scans might be needed
 */

static int32_t spiffs_page_consistency_check_i(FAR struct spiffs_s *fs)
{
  const uint32_t bits = 4;
  const int16_t pages_per_scan = SPIFFS_CFG_LOG_PAGE_SZ(fs) * 8 / bits;

  int32_t res = OK;
  int16_t pix_offset = 0;

  /* for each range of pages fitting into work memory */

  while (pix_offset < SPIFFS_PAGES_PER_BLOCK(fs) * fs->block_count)
    {
      /* set this flag to abort all checks and rescan the page range */

      uint8_t restart = 0;
      memset(fs->work, 0, SPIFFS_CFG_LOG_PAGE_SZ(fs));

      int16_t cur_block = 0;

      /* build consistency bitmap for id range traversing all blocks */

      while (!restart && cur_block < fs->block_count)
        {
          CHECK_CB(fs, SPIFFS_CHECK_PAGE, SPIFFS_CHECK_PROGRESS,
                   (pix_offset * 256) / (SPIFFS_PAGES_PER_BLOCK(fs) * fs->block_count) +
                   ((((cur_block * pages_per_scan * 256) /
                      (SPIFFS_PAGES_PER_BLOCK(fs) * fs->block_count))) /
                    fs->block_count), 0);

          /* traverse each page except for lookup pages */

          int16_t cur_pix =
            SPIFFS_OBJ_LOOKUP_PAGES(fs) +
            SPIFFS_PAGES_PER_BLOCK(fs) * cur_block;
          while (!restart &&
                 cur_pix < SPIFFS_PAGES_PER_BLOCK(fs) * (cur_block + 1))
            {
              uint8_t within_range;

              /* read header */

              struct spiffs_page_header_s p_hdr;
              res = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ,
                               0, SPIFFS_PAGE_TO_PADDR(fs, cur_pix),
                               sizeof(struct spiffs_page_header_s), (uint8_t *) & p_hdr);
              SPIFFS_CHECK_RES(res);

              within_range = (cur_pix >= pix_offset &&
                              cur_pix < pix_offset + pages_per_scan);
              const uint32_t pix_byte_ix = (cur_pix - pix_offset) / (8 / bits);
              const uint8_t pix_bit_ix = (cur_pix & ((8 / bits) - 1)) * bits;

              if (within_range &&
                  (p_hdr.flags & SPIFFS_PH_FLAG_DELET) &&
                  (p_hdr.flags & SPIFFS_PH_FLAG_USED) == 0)
                {
                  /* used */

                  fs->work[pix_byte_ix] |= (1 << (pix_bit_ix + 0));
                }

              if ((p_hdr.flags & SPIFFS_PH_FLAG_DELET) &&
                  (p_hdr.flags & SPIFFS_PH_FLAG_IXDELE) &&
                  (p_hdr.flags & (SPIFFS_PH_FLAG_INDEX | SPIFFS_PH_FLAG_USED)) == 0)
                {
                  int entries;
                  int i;

                  /* found non-deleted index */

                  if (within_range)
                    {
                      fs->work[pix_byte_ix] |= (1 << (pix_bit_ix + 2));
                    }

                  /* load non-deleted index */

                  res = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ,
                                   0, SPIFFS_PAGE_TO_PADDR(fs, cur_pix),
                                   SPIFFS_CFG_LOG_PAGE_SZ(fs), fs->lu_work);
                  SPIFFS_CHECK_RES(res);

                  /* traverse index for referenced pages */

                  int16_t *object_page_index;
                  struct spiffs_page_header_s *objix_p_hdr =
                    (struct spiffs_page_header_s *) fs->lu_work;

                  int16_t data_spix_offset;
                  if (p_hdr.span_ix == 0)
                    {
                      /* object header page index */

                      entries = SPIFFS_OBJ_HDR_IX_LEN(fs);
                      data_spix_offset = 0;
                      object_page_index =
                        (int16_t *) ((uint8_t *) fs->lu_work +
                                            sizeof(struct spiffs_pgobj_ixheader_s));
                    }
                  else
                    {
                      /* object page index */

                      entries = SPIFFS_OBJ_IX_LEN(fs);
                      data_spix_offset =
                        SPIFFS_OBJ_HDR_IX_LEN(fs) +
                        SPIFFS_OBJ_IX_LEN(fs) * (p_hdr.span_ix - 1);
                      object_page_index =
                        (int16_t *) ((uint8_t *) fs->lu_work +
                                            sizeof(spiffs_page_object_ix));
                    }

                  /* for all entries in index */

                  for (i = 0; !restart && i < entries; i++)
                    {
                      int16_t rpix = object_page_index[i];
                      uint8_t rpix_within_range = rpix >= pix_offset &&
                        rpix < pix_offset + pages_per_scan;

                      if ((rpix != (int16_t) - 1 &&
                           rpix > SPIFFS_MAX_PAGES(fs)) || (rpix_within_range &&
                                                            SPIFFS_IS_LOOKUP_PAGE
                                                            (fs, rpix)))
                        {

                          /* bad reference */

                          spiffs_checkinfo("PA: pix " _SPIPRIpg
                                           "x bad pix / LU referenced from page "
                                           _SPIPRIpg "\n", rpix, cur_pix);

                          /* check for data page elsewhere */

                          int16_t data_pix;
                          res = spiffs_obj_lu_find_id_and_span(fs,
                                                               objix_p_hdr->id & ~SPIFFS_OBJ_ID_IX_FLAG,
                                                               data_spix_offset + i,
                                                               0, &data_pix);
                          if (res == SPIFFS_ERR_NOT_FOUND)
                            {
                              res = OK;
                              data_pix = 0;
                            }

                          SPIFFS_CHECK_RES(res);

                          if (data_pix == 0)
                            {
                              /* if not, allocate free page */

                              struct spiffs_page_header_s new_ph;
                              new_ph.flags = 0xff & ~(SPIFFS_PH_FLAG_USED |
                                                      SPIFFS_PH_FLAG_FINAL);
                              new_ph.id = objix_p_hdr->id & ~SPIFFS_OBJ_ID_IX_FLAG;
                              new_ph.span_ix = data_spix_offset + i;
                              res = spiffs_page_allocate_data(fs, new_ph.id,
                                                              &new_ph, 0, 0, 0, 1,
                                                              &data_pix);
                              SPIFFS_CHECK_RES(res);
                              spiffs_checkinfo
                                ("PA: FIXUP: found no existing data page, created new @ "
                                 _SPIPRIpg "\n", data_pix);
                            }

                          /* remap index */

                          spiffs_checkinfo("PA: FIXUP: rewriting index pix "
                                           _SPIPRIpg "\n", cur_pix);
                          res = spiffs_rewrite_index(fs,
                                                     objix_p_hdr->id | SPIFFS_OBJ_ID_IX_FLAG,
                                                     data_spix_offset + i, data_pix,
                                                     cur_pix);
                          if (res <= _SPIFFS_ERR_CHECK_FIRST &&
                              res > _SPIFFS_ERR_CHECK_LAST)
                            {
                              /* index bad also, cannot mend this file */

                              spiffs_checkinfo("PA: FIXUP: index bad " _SPIPRIi
                                               ", cannot mend - delete object\n",
                                               res);
                              CHECK_CB(fs, SPIFFS_CHECK_PAGE,
                                       SPIFFS_CHECK_DELETE_BAD_FILE,
                                       objix_p_hdr->id, 0);

                              /* delete file */

                              res = spiffs_page_delete(fs, cur_pix);
                            }
                          else
                            {
                              CHECK_CB(fs, SPIFFS_CHECK_PAGE,
                                       SPIFFS_CHECK_FIX_INDEX,
                                       objix_p_hdr->id,
                                       objix_p_hdr->span_ix);
                            }

                          SPIFFS_CHECK_RES(res);
                          restart = 1;
                        }
                      else if (rpix_within_range)
                        {

                          /* valid reference. read referenced page header */

                          struct spiffs_page_header_s rp_hdr;
                          res = _spiffs_rd(fs,
                                           SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ,
                                           0, SPIFFS_PAGE_TO_PADDR(fs, rpix),
                                           sizeof(struct spiffs_page_header_s),
                                           (uint8_t *) & rp_hdr);
                          SPIFFS_CHECK_RES(res);

                          /* cross reference page header check */

                          if (rp_hdr.id != (p_hdr.id & ~SPIFFS_OBJ_ID_IX_FLAG) ||
                              rp_hdr.span_ix != data_spix_offset + i ||
                              (rp_hdr.flags & (SPIFFS_PH_FLAG_DELET |
                                               SPIFFS_PH_FLAG_INDEX |
                                               SPIFFS_PH_FLAG_USED)) !=
                              (SPIFFS_PH_FLAG_DELET | SPIFFS_PH_FLAG_INDEX))
                            {
                              spiffs_checkinfo("PA: pix " _SPIPRIpg
                                               " has inconsistent page header ix id/span:"
                                               _SPIPRIid "/" _SPIPRIsp
                                               ", ref id/span:" _SPIPRIid "/"
                                               _SPIPRIsp " flags:" _SPIPRIfl
                                               "\n", rpix,
                                               p_hdr.
                                               id & ~SPIFFS_OBJ_ID_IX_FLAG,
                                               data_spix_offset + i,
                                               rp_hdr.id, rp_hdr.span_ix,
                                               rp_hdr.flags);

                              /* try finding correct page */

                              int16_t data_pix;
                              res = spiffs_obj_lu_find_id_and_span(fs,
                                                                   p_hdr.id & ~SPIFFS_OBJ_ID_IX_FLAG,
                                                                   data_spix_offset + i, rpix,
                                                                   &data_pix);
                              if (res == SPIFFS_ERR_NOT_FOUND)
                                {
                                  res = OK;
                                  data_pix = 0;
                                }

                              SPIFFS_CHECK_RES(res);

                              if (data_pix == 0)
                                {
                                  /* not found, this index is badly borked */

                                  spiffs_checkinfo
                                    ("PA: FIXUP: index bad, delete object id "
                                     _SPIPRIid "\n", p_hdr.id);
                                  CHECK_CB(fs, SPIFFS_CHECK_PAGE,
                                           SPIFFS_CHECK_DELETE_BAD_FILE,
                                           p_hdr.id, 0);
                                  res = spiffs_delete_obj_lazy(fs, p_hdr.id);
                                  SPIFFS_CHECK_RES(res);
                                  break;
                                }
                              else
                                {
                                  /* found it, so rewrite index */

                                  spiffs_checkinfo
                                    ("PA: FIXUP: found correct data pix "
                                     _SPIPRIpg ", rewrite ix pix " _SPIPRIpg
                                     " id " _SPIPRIid "\n", data_pix, cur_pix,
                                     p_hdr.id);
                                  res = spiffs_rewrite_index(fs, p_hdr.id,
                                                             data_spix_offset + i,
                                                             data_pix, cur_pix);
                                  if (res <= _SPIFFS_ERR_CHECK_FIRST &&
                                      res > _SPIFFS_ERR_CHECK_LAST)
                                    {
                                      /* index bad also, cannot mend this file */

                                      spiffs_checkinfo("PA: FIXUP: index bad "
                                                       _SPIPRIi
                                                       ", cannot mend!\n", res);
                                      CHECK_CB(fs, SPIFFS_CHECK_PAGE,
                                               SPIFFS_CHECK_DELETE_BAD_FILE,
                                               p_hdr.id, 0);
                                      res = spiffs_delete_obj_lazy(fs, p_hdr.id);
                                    }
                                  else
                                    {
                                      CHECK_CB(fs, SPIFFS_CHECK_PAGE,
                                               SPIFFS_CHECK_FIX_INDEX,
                                               p_hdr.id, p_hdr.span_ix);
                                    }

                                  SPIFFS_CHECK_RES(res);
                                  restart = 1;
                                }
                            }
                          else
                            {
                              /* mark rpix as referenced */

                              const uint32_t rpix_byte_ix =
                                (rpix - pix_offset) / (8 / bits);
                              const uint8_t rpix_bit_ix =
                                (rpix & ((8 / bits) - 1)) * bits;

                              if (fs->
                                  work[rpix_byte_ix] & (1 << (rpix_bit_ix + 1)))
                                {
                                  spiffs_checkinfo("PA: pix " _SPIPRIpg
                                                   " multiple referenced from page "
                                                   _SPIPRIpg "\n", rpix,
                                                   cur_pix);
                                  /* Here, we should have fixed all broken
                                   * references - getting this means there
                                   * must be multiple files with same object
                                   * id. Only solution is to delete
                                   * the object which is referring to this page
                                   */

                                  spiffs_checkinfo("PA: FIXUP: removing object "
                                                   _SPIPRIid " and page "
                                                   _SPIPRIpg "\n", p_hdr.id,
                                                   cur_pix);
                                  CHECK_CB(fs, SPIFFS_CHECK_PAGE,
                                           SPIFFS_CHECK_DELETE_BAD_FILE,
                                           p_hdr.id, 0);
                                  res =
                                    spiffs_delete_obj_lazy(fs, p_hdr.id);
                                  SPIFFS_CHECK_RES(res);

                                  /* extra precaution, delete this page also */

                                  res = spiffs_page_delete(fs, cur_pix);
                                  SPIFFS_CHECK_RES(res);
                                  restart = 1;
                                }
                              fs->work[rpix_byte_ix] |=
                                (1 << (rpix_bit_ix + 1));
                            }
                        }
                    }
                }

              /* next page */

              cur_pix++;
            }

          /* next block */

          cur_block++;
        }

      /* check consistency bitmap */

      if (!restart)
        {
          int16_t objix_pix;
          int16_t rpix;

          uint32_t byte_ix;
          uint8_t bit_ix;
          for (byte_ix = 0; !restart && byte_ix < SPIFFS_CFG_LOG_PAGE_SZ(fs);
               byte_ix++)
            {
              for (bit_ix = 0; !restart && bit_ix < 8 / bits; bit_ix++)
                {
                  uint8_t bitmask = (fs->work[byte_ix] >> (bit_ix * bits)) & 0x7;
                  int16_t cur_pix =
                    pix_offset + byte_ix * (8 / bits) + bit_ix;

                  /* 000 ok - free, unreferenced, not index */

                  if (bitmask == 0x1)
                    {

                      /* 001 */

                      spiffs_checkinfo("PA: pix " _SPIPRIpg
                                       " USED, UNREFERENCED, not index\n",
                                       cur_pix);

                      uint8_t rewrite_ix_to_this = 0;
                      uint8_t delete_page = 0;

                      /* check corresponding object index entry */

                      struct spiffs_page_header_s p_hdr;
                      res =
                        _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ,
                                   0, SPIFFS_PAGE_TO_PADDR(fs, cur_pix),
                                   sizeof(struct spiffs_page_header_s),
                                   (uint8_t *) & p_hdr);
                      SPIFFS_CHECK_RES(res);

                      res = spiffs_object_get_data_page_index_reference(fs,
                                                                        p_hdr.id,
                                                                        p_hdr.span_ix,
                                                                        &rpix,
                                                                        &objix_pix);
                      if (res == OK)
                        {
                          if (((rpix == (int16_t) - 1 ||
                                rpix > SPIFFS_MAX_PAGES(fs)) ||
                               (SPIFFS_IS_LOOKUP_PAGE(fs, rpix))))
                            {
                              /* pointing to a bad page altogether, rewrite
                               * index to this
                               */

                              rewrite_ix_to_this = 1;
                              spiffs_checkinfo("PA: corresponding ref is bad: "
                                               _SPIPRIpg ", rewrite to this "
                                               _SPIPRIpg "\n", rpix, cur_pix);
                            }
                          else
                            {
                              /* pointing to something else, check what */

                              struct spiffs_page_header_s rp_hdr;
                              res = _spiffs_rd(fs,
                                               SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ, 0,
                                               SPIFFS_PAGE_TO_PADDR(fs, rpix),
                                               sizeof(struct spiffs_page_header_s),
                                               (uint8_t *) & rp_hdr);
                              SPIFFS_CHECK_RES(res);
                              if (((p_hdr.id & ~SPIFFS_OBJ_ID_IX_FLAG) ==
                                   rp_hdr.id) &&
                                  ((rp_hdr.flags & (SPIFFS_PH_FLAG_INDEX |
                                                    SPIFFS_PH_FLAG_DELET |
                                                    SPIFFS_PH_FLAG_USED |
                                                    SPIFFS_PH_FLAG_FINAL)) ==
                                   (SPIFFS_PH_FLAG_INDEX | SPIFFS_PH_FLAG_DELET)))
                                {
                                  /* pointing to something else valid, just
                                   * delete this page then
                                   */

                                  spiffs_checkinfo
                                    ("PA: corresponding ref is good but different: "
                                     _SPIPRIpg ", delete this " _SPIPRIpg "\n",
                                     rpix, cur_pix);
                                  delete_page = 1;
                                }
                              else
                                {
                                  /* pointing to something weird, update index
                                   * to point to this page instead
                                   */

                                  if (rpix != cur_pix)
                                    {
                                      spiffs_checkinfo
                                        ("PA: corresponding ref is weird: "
                                         _SPIPRIpg " %s%s%s%s, rewrite this "
                                         _SPIPRIpg "\n", rpix,
                                         (rp_hdr.flags & SPIFFS_PH_FLAG_INDEX) ? "" :
                                         "INDEX ",
                                         (rp_hdr.flags & SPIFFS_PH_FLAG_DELET) ? "" :
                                         "DELETED ",
                                         (rp_hdr.flags & SPIFFS_PH_FLAG_USED) ?
                                         "NOTUSED " : "",
                                         (rp_hdr.flags & SPIFFS_PH_FLAG_FINAL) ?
                                         "NOTFINAL " : "", cur_pix);
                                      rewrite_ix_to_this = 1;
                                    }
                                  else
                                    {
                                      /* should not happen, destined for fubar */
                                    }
                                }
                            }
                        }
                      else if (res == SPIFFS_ERR_NOT_FOUND)
                        {
                          spiffs_checkinfo
                            ("PA: corresponding ref not found, delete "
                             _SPIPRIpg "\n", cur_pix);
                          delete_page = 1;
                          res = OK;
                        }

                      if (rewrite_ix_to_this)
                        {
                          /* if pointing to invalid page, redirect index to
                           * this page
                           */

                          spiffs_checkinfo("PA: FIXUP: rewrite index id "
                                           _SPIPRIid " data spix " _SPIPRIsp
                                           " to point to this pix: " _SPIPRIpg
                                           "\n", p_hdr.id, p_hdr.span_ix,
                                           cur_pix);
                          res =  spiffs_rewrite_index(fs, p_hdr.id,
                                                      p_hdr.span_ix, cur_pix,
                                                     objix_pix);
                          if (res <= _SPIFFS_ERR_CHECK_FIRST &&
                              res > _SPIFFS_ERR_CHECK_LAST)
                            {
                              /* index bad also, cannot mend this file */

                              spiffs_checkinfo("PA: FIXUP: index bad " _SPIPRIi
                                               ", cannot mend!\n", res);
                              CHECK_CB(fs, SPIFFS_CHECK_PAGE,
                                       SPIFFS_CHECK_DELETE_BAD_FILE,
                                       p_hdr.id, 0);
                              res = spiffs_page_delete(fs, cur_pix);
                              SPIFFS_CHECK_RES(res);
                              res = spiffs_delete_obj_lazy(fs, p_hdr.id);
                            }
                          else
                            {
                              CHECK_CB(fs, SPIFFS_CHECK_PAGE,
                                       SPIFFS_CHECK_FIX_INDEX, p_hdr.id,
                                       p_hdr.span_ix);
                            }

                          SPIFFS_CHECK_RES(res);
                          restart = 1;
                          continue;
                        }
                      else if (delete_page)
                        {
                          spiffs_checkinfo("PA: FIXUP: deleting page " _SPIPRIpg
                                           "\n", cur_pix);
                          CHECK_CB(fs, SPIFFS_CHECK_PAGE,
                                   SPIFFS_CHECK_DELETE_PAGE, cur_pix, 0);
                          res = spiffs_page_delete(fs, cur_pix);
                        }

                      SPIFFS_CHECK_RES(res);
                    }

                  if (bitmask == 0x2)
                    {
                      /* 010 */

                      spiffs_checkinfo("PA: pix " _SPIPRIpg
                                       " FREE, REFERENCED, not index\n",
                                       cur_pix);

                      /* no op, this should be taken care of when checking
                       * valid references
                       */
                    }

                  /* 011 ok - busy, referenced, not index */

                  if (bitmask == 0x4)
                    {
                      /* 100 */

                      spiffs_checkinfo("PA: pix " _SPIPRIpg
                                       " FREE, unreferenced, INDEX\n", cur_pix);

                      /* this should never happen, major fubar */
                    }

                  /* 101 ok - busy, unreferenced, index */

                  if (bitmask == 0x6)
                    {
                      /* 110 */

                      spiffs_checkinfo("PA: pix " _SPIPRIpg
                                       " FREE, REFERENCED, INDEX\n", cur_pix);

                      /* no op, this should be taken care of when checking
                       * valid references
                       */
                    }

                  if (bitmask == 0x7)
                    {
                      /* 111 */

                      spiffs_checkinfo("PA: pix " _SPIPRIpg
                                       " USED, REFERENCED, INDEX\n", cur_pix);

                      /* no op, this should be taken care of when checking
                       * valid references
                       */
                    }
                }
            }
        }

      spiffs_checkinfo("PA: processed " _SPIPRIpg ", restart " _SPIPRIi "\n",
                       pix_offset, restart);

      /* next page range */

      if (!restart)
        {
          pix_offset += pages_per_scan;
        }
    }

  return res;
}

/* Checks consistency amongst all pages and fixes irregularities */

int32_t spiffs_page_consistency_check(FAR struct spiffs_s *fs)
{
  CHECK_CB(fs, SPIFFS_CHECK_PAGE, SPIFFS_CHECK_PROGRESS, 0, 0);
  int32_t res = spiffs_page_consistency_check_i(fs);
  if (res != OK)
    {
      CHECK_CB(fs, SPIFFS_CHECK_PAGE, SPIFFS_CHECK_ERROR, res, 0);
    }

  CHECK_CB(fs, SPIFFS_CHECK_PAGE, SPIFFS_CHECK_PROGRESS, 256, 0);
  return res;
}

/* Object index consistency */

/* searches for given object id in temporary object id index,
 * returns the index or -1
 */

static int spiffs_object_index_search(FAR struct spiffs_s *fs, int16_t id)
{
  uint32_t i;
  int16_t *obj_table = (int16_t *) fs->work;
  id &= ~SPIFFS_OBJ_ID_IX_FLAG;
  for (i = 0; i < SPIFFS_CFG_LOG_PAGE_SZ(fs) / sizeof(int16_t); i++)
    {
      if ((obj_table[i] & ~SPIFFS_OBJ_ID_IX_FLAG) == id)
        {
          return i;
        }
    }
  return -1;
}

static int32_t spiffs_object_index_consistency_check_v(FAR struct spiffs_s *fs,
                                                     int16_t id,
                                                     int16_t cur_block,
                                                     int cur_entry,
                                                     const void *user_const_p,
                                                     void *user_var_p)
{
  (void)user_const_p;
  int32_t res_c = SPIFFS_VIS_COUNTINUE;
  int32_t res = OK;
  uint32_t *log_ix = (uint32_t *) user_var_p;
  int16_t *obj_table = (int16_t *) fs->work;

  CHECK_CB(fs, SPIFFS_CHECK_INDEX, SPIFFS_CHECK_PROGRESS,
           (cur_block * 256) / fs->block_count, 0);

  if (id != SPIFFS_OBJ_ID_FREE && id != SPIFFS_OBJ_ID_DELETED &&
      (id & SPIFFS_OBJ_ID_IX_FLAG))
    {
      struct spiffs_page_header_s p_hdr;
      int16_t cur_pix =
        SPIFFS_OBJ_LOOKUP_ENTRY_TO_PIX(fs, cur_block, cur_entry);

      /* load header */

      res = _spiffs_rd(fs, SPIFFS_OP_T_OBJ_LU2 | SPIFFS_OP_C_READ,
                       0, SPIFFS_PAGE_TO_PADDR(fs, cur_pix),
                       sizeof(struct spiffs_page_header_s), (uint8_t *) & p_hdr);
      SPIFFS_CHECK_RES(res);

      if (p_hdr.span_ix == 0 &&
          (p_hdr.
           flags & (SPIFFS_PH_FLAG_INDEX | SPIFFS_PH_FLAG_FINAL |
                    SPIFFS_PH_FLAG_DELET | SPIFFS_PH_FLAG_IXDELE)) ==
          (SPIFFS_PH_FLAG_DELET))
        {
          spiffs_checkinfo("IX: pix " _SPIPRIpg ", obj id:" _SPIPRIid " spix:"
                           _SPIPRIsp " header not fully deleted - deleting\n",
                           cur_pix, id, p_hdr.span_ix);
          CHECK_CB(fs, SPIFFS_CHECK_INDEX, SPIFFS_CHECK_DELETE_PAGE, cur_pix,
                   id);
          res = spiffs_page_delete(fs, cur_pix);
          SPIFFS_CHECK_RES(res);
          return res_c;
        }

      if ((p_hdr.
           flags & (SPIFFS_PH_FLAG_INDEX | SPIFFS_PH_FLAG_FINAL |
                    SPIFFS_PH_FLAG_DELET | SPIFFS_PH_FLAG_IXDELE)) ==
          (SPIFFS_PH_FLAG_DELET | SPIFFS_PH_FLAG_IXDELE))
        {
          return res_c;
        }

      if (p_hdr.span_ix == 0)
        {
          /* objix header page, register objid as reachable */

          int r = spiffs_object_index_search(fs, id);
          if (r == -1)
            {
              /* not registered, do it */

              obj_table[*log_ix] = id & ~SPIFFS_OBJ_ID_IX_FLAG;
              (*log_ix)++;
              if (*log_ix >= SPIFFS_CFG_LOG_PAGE_SZ(fs) / sizeof(int16_t))
                {
                  *log_ix = 0;
                }
            }
        }
      else
        {
          /* span index
           * objix page, see if header can be found
           */

          int r = spiffs_object_index_search(fs, id);
          uint8_t delete = 0;
          if (r == -1)
            {
              /* not in temporary index, try finding it */

              int16_t objix_hdr_pix;
              res =
                spiffs_obj_lu_find_id_and_span(fs, id | SPIFFS_OBJ_ID_IX_FLAG,
                                               0, 0, &objix_hdr_pix);
              res_c = SPIFFS_VIS_COUNTINUE_RELOAD;
              if (res == OK)
                {
                  /* found, register as reachable */

                  obj_table[*log_ix] = id & ~SPIFFS_OBJ_ID_IX_FLAG;
                }
              else if (res == SPIFFS_ERR_NOT_FOUND)
                {
                  /* not found, register as unreachable */

                  delete = 1;
                  obj_table[*log_ix] = id | SPIFFS_OBJ_ID_IX_FLAG;
                }
              else
                {
                  SPIFFS_CHECK_RES(res);
                }
              (*log_ix)++;
              if (*log_ix >= SPIFFS_CFG_LOG_PAGE_SZ(fs) / sizeof(int16_t))
                {
                  *log_ix = 0;
                }
            }
          else
            {
              /* in temporary index, check reachable flag */

              if ((obj_table[r] & SPIFFS_OBJ_ID_IX_FLAG))
                {
                  /* registered as unreachable */

                  delete = 1;
                }
            }

          if (delete)
            {
              spiffs_checkinfo("IX: FIXUP: pix " _SPIPRIpg ", obj id:" _SPIPRIid
                               " spix:" _SPIPRIsp
                               " is orphan index - deleting\n", cur_pix, id,
                               p_hdr.span_ix);
              CHECK_CB(fs, SPIFFS_CHECK_INDEX,
                       SPIFFS_CHECK_DELETE_ORPHANED_INDEX, cur_pix, id);
              res = spiffs_page_delete(fs, cur_pix);
              SPIFFS_CHECK_RES(res);
            }
        }
    }

  return res_c;
}

/* Removes orphaned and partially deleted index pages.
 * Scans for index pages. When an index page is found, corresponding index header is searched for.
 * If no such page exists, the index page cannot be reached as no index header exists and must be
 * deleted.
 */

int32_t spiffs_object_index_consistency_check(FAR struct spiffs_s *fs)
{
  int32_t res = OK;

  /* impl note:
   * fs->work is used for a temporary object index memory, listing found object 
   * ids and indicating whether they can be reached or not. Acting as a fifo if
   * object ids cannot fit.  In the temporary object index memory,
   * SPIFFS_OBJ_ID_IX_FLAG bit is used to indicate a reachable/unreachable
   * object id.
   */

  memset(fs->work, 0, SPIFFS_CFG_LOG_PAGE_SZ(fs));
  uint32_t obj_id_log_ix = 0;
  CHECK_CB(fs, SPIFFS_CHECK_INDEX, SPIFFS_CHECK_PROGRESS, 0, 0);
  res =
    spiffs_foreach_objlu(fs, 0, 0, 0, 0,
                                     spiffs_object_index_consistency_check_v, 0,
                                     &obj_id_log_ix, 0, 0);
  if (res == SPIFFS_VIS_END)
    {
      res = OK;
    }
  if (res != OK)
    {
      CHECK_CB(fs, SPIFFS_CHECK_INDEX, SPIFFS_CHECK_ERROR, res, 0);
    }
  CHECK_CB(fs, SPIFFS_CHECK_INDEX, SPIFFS_CHECK_PROGRESS, 256, 0);
  return res;
}
