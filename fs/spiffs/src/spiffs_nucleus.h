/****************************************************************************
 * fs/spiffs.h/spiffs_nucleus.h
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

#ifndef __FS_SPIFFS_SRC_SPIFFS_NUCLEIUS_H
#define __FS_SPIFFS_SRC_SPIFFS_NUCLEIUS_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include "spiffs.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* SPIFFS layout
 *
 * spiffs is designed for following spi flash characteristics:
 *   - only big areas of data (blocks) can be erased
 *   - erasing resets all bits in a block to ones
 *   - writing pulls ones to zeroes
 *   - zeroes cannot be pulled to ones, without erase
 *   - wear leveling
 *
 * spiffs is also meant to be run on embedded, memory constraint devices.
 *
 * Entire area is divided in blocks. Entire area is also divided in pages.
 * Each block contains same number of pages. A page cannot be erased, but a
 * block can be erased.
 *
 * Entire area must be block_size * x
 * page_size must be block_size / (2^y) where y > 2
 *
 * ex: area = 1024*1024 bytes, block size = 65536 bytes, page size = 256 bytes
 *
 * BLOCK 0  PAGE 0       object lookup 1
 *          PAGE 1       object lookup 2
 *          ...
 *          PAGE n-1     object lookup n
 *          PAGE n       object data 1
 *          PAGE n+1     object data 2
 *          ...
 *          PAGE n+m-1   object data m
 *
 * BLOCK 1  PAGE n+m     object lookup 1
 *          PAGE n+m+1   object lookup 2
 *          ...
 *          PAGE 2n+m-1  object lookup n
 *          PAGE 2n+m    object data 1
 *          PAGE 2n+m    object data 2
 *          ...
 *          PAGE 2n+2m-1 object data m
 * ...
 *
 * n is number of object lookup pages, which is number of pages needed to index all pages
 * in a block by object id
 *   : block_size / page_size * sizeof(id) / page_size
 * m is number data pages, which is number of pages in block minus number of lookup pages
 *   : block_size / page_size - block_size / page_size * sizeof(id) / page_size
 * thus, n+m is total number of pages in a block
 *   : block_size / page_size
 *
 * ex: n = 65536/256*2/256 = 2, m = 65536/256 - 2 = 254 => n+m = 65536/256 = 256
 *
 * Object lookup pages contain object id entries. Each entry represent the corresponding
 * data page.
 * Assuming a 16 bit object id, an object id being 0xffff represents a free page.
 * An object id being 0x0000 represents a deleted page.
 *
 * ex: page 0 : lookup : 0008 0001 0aaa ffff ffff ffff ffff ffff ..
 *     page 1 : lookup : ffff ffff ffff ffff ffff ffff ffff ffff ..
 *     page 2 : data   : data for object id 0008
 *     page 3 : data   : data for object id 0001
 *     page 4 : data   : data for object id 0aaa
 *     ...
 *
 *
 * Object data pages can be either object index pages or object content.
 * All object data pages contains a data page header, containing object id and span index.
 * The span index denotes the object page ordering amongst data pages with same object id.
 * This applies to both object index pages (when index spans more than one page of entries),
 * and object data pages.
 * An object index page contains page entries pointing to object content page. The entry index
 * in a object index page correlates to the span index in the actual object data page.
 * The first object index page (span index 0) is called object index header page, and also
 * contains object flags (directory/file), size, object name etc.
 *
 * ex:
 *  BLOCK 1
 *    PAGE 256: objectl lookup page 1
 *      [*123] [ 123] [ 123] [ 123]
 *      [ 123] [*123] [ 123] [ 123]
 *      [free] [free] [free] [free] ...
 *    PAGE 257: objectl lookup page 2
 *      [free] [free] [free] [free] ...
 *    PAGE 258: object index page (header)
 *      obj.id:0123 span.ix:0000 flags:INDEX
 *      size:1600 name:ex.txt type:file
 *      [259] [260] [261] [262]
 *    PAGE 259: object data page
 *      obj.id:0123 span.ix:0000 flags:DATA
 *    PAGE 260: object data page
 *      obj.id:0123 span.ix:0001 flags:DATA
 *    PAGE 261: object data page
 *      obj.id:0123 span.ix:0002 flags:DATA
 *    PAGE 262: object data page
 *      obj.id:0123 span.ix:0003 flags:DATA
 *    PAGE 263: object index page
 *      obj.id:0123 span.ix:0001 flags:INDEX
 *      [264] [265] [fre] [fre]
 *      [fre] [fre] [fre] [fre]
 *    PAGE 264: object data page
 *      obj.id:0123 span.ix:0004 flags:DATA
 *    PAGE 265: object data page
 *      obj.id:0123 span.ix:0005 flags:DATA
 */

#define _SPIFFS_ERR_CHECK_FIRST         (SPIFFS_ERR_INTERNAL - 1)
#define SPIFFS_ERR_CHECK_OBJ_ID_MISM    (SPIFFS_ERR_INTERNAL - 1)
#define SPIFFS_ERR_CHECK_SPIX_MISM      (SPIFFS_ERR_INTERNAL - 2)
#define SPIFFS_ERR_CHECK_FLAGS_BAD      (SPIFFS_ERR_INTERNAL - 3)
#define _SPIFFS_ERR_CHECK_LAST          (SPIFFS_ERR_INTERNAL - 4)

/* visitor result, continue searching */

#define SPIFFS_VIS_COUNTINUE            (SPIFFS_ERR_INTERNAL - 20)

/* visitor result, continue searching after reloading lu buffer */

#define SPIFFS_VIS_COUNTINUE_RELOAD     (SPIFFS_ERR_INTERNAL - 21)

/* visitor result, stop searching */

#define SPIFFS_VIS_END                  (SPIFFS_ERR_INTERNAL - 22)

/* updating an object index contents */

#define SPIFFS_EV_IX_UPD                (0)

/* creating a new object index */

#define SPIFFS_EV_IX_NEW                (1)

/* deleting an object index */

#define SPIFFS_EV_IX_DEL                (2)

/* moving an object index without updating contents */

#define SPIFFS_EV_IX_MOV                (3)

/* updating an object index header data only, not the table itself */

#define SPIFFS_EV_IX_UPD_HDR            (4)

#define SPIFFS_OBJ_ID_IX_FLAG           ((int16_t)(1<<(8*sizeof(int16_t)-1)))

#define SPIFFS_UNDEFINED_LEN            (uint32_t)(-1)

#define SPIFFS_OBJ_ID_DELETED           ((int16_t)0)
#define SPIFFS_OBJ_ID_FREE              ((int16_t)-1)

#if SPIFFS_USE_MAGIC
#  if !SPIFFS_USE_MAGIC_LENGTH
#    define SPIFFS_MAGIC(fs, bix)           \
  ((int16_t)(0x20140529 ^ SPIFFS_CFG_LOG_PAGE_SZ(fs)))
#  else
#    define SPIFFS_MAGIC(fs, bix)           \
  ((int16_t)(0x20140529 ^ SPIFFS_CFG_LOG_PAGE_SZ(fs) ^ ((fs)->block_count - (bix))))
#  endif
#endif

#define SPIFFS_CFG_LOG_PAGE_SZ(fs)      ((fs)->cfg.log_page_size)
#define SPIFFS_CFG_LOG_BLOCK_SZ(fs)     ((fs)->cfg.log_block_size)
#define SPIFFS_CFG_PHYS_SZ(fs)          ((fs)->cfg.phys_size)
#define SPIFFS_CFG_PHYS_ERASE_SZ(fs)    ((fs)->cfg.phys_erase_block)
#define SPIFFS_CFG_PHYS_ADDR(fs)        ((fs)->cfg.phys_addr)

/* total number of pages */

#define SPIFFS_MAX_PAGES(fs) \
  (SPIFFS_CFG_PHYS_SZ(fs)/SPIFFS_CFG_LOG_PAGE_SZ(fs) )

/* total number of pages per block, including object lookup pages */

#define SPIFFS_PAGES_PER_BLOCK(fs) \
  (SPIFFS_CFG_LOG_BLOCK_SZ(fs)/SPIFFS_CFG_LOG_PAGE_SZ(fs) )

/* number of object lookup pages per block */

#define SPIFFS_OBJ_LOOKUP_PAGES(fs)     \
  (MAX(1, (SPIFFS_PAGES_PER_BLOCK(fs) * sizeof(int16_t)) / SPIFFS_CFG_LOG_PAGE_SZ(fs)) )

/* checks if page index belongs to object lookup */

#define SPIFFS_IS_LOOKUP_PAGE(fs,pix)     \
  (((pix) % SPIFFS_PAGES_PER_BLOCK(fs)) < SPIFFS_OBJ_LOOKUP_PAGES(fs))

/* number of object lookup entries in all object lookup pages */

#define SPIFFS_OBJ_LOOKUP_MAX_ENTRIES(fs) \
  (SPIFFS_PAGES_PER_BLOCK(fs)-SPIFFS_OBJ_LOOKUP_PAGES(fs))

/* converts a block to physical address */

#define SPIFFS_BLOCK_TO_PADDR(fs, block) \
  ( SPIFFS_CFG_PHYS_ADDR(fs) + (block)* SPIFFS_CFG_LOG_BLOCK_SZ(fs) )

/* converts a object lookup entry to page index */

#define SPIFFS_OBJ_LOOKUP_ENTRY_TO_PIX(fs, block, entry) \
  ((block)*SPIFFS_PAGES_PER_BLOCK(fs) + (SPIFFS_OBJ_LOOKUP_PAGES(fs) + entry))

/* converts a object lookup entry to physical address of corresponding page */

#define SPIFFS_OBJ_LOOKUP_ENTRY_TO_PADDR(fs, block, entry) \
  (SPIFFS_BLOCK_TO_PADDR(fs, block) + (SPIFFS_OBJ_LOOKUP_PAGES(fs) + entry) * SPIFFS_CFG_LOG_PAGE_SZ(fs) )

/* converts a page to physical address */

#define SPIFFS_PAGE_TO_PADDR(fs, page) \
  ( SPIFFS_CFG_PHYS_ADDR(fs) + (page) * SPIFFS_CFG_LOG_PAGE_SZ(fs) )

/* converts a physical address to page */

#define SPIFFS_PADDR_TO_PAGE(fs, addr) \
  ( ((addr) -  SPIFFS_CFG_PHYS_ADDR(fs)) / SPIFFS_CFG_LOG_PAGE_SZ(fs) )

/* gives index in page for a physical address */

#define SPIFFS_PADDR_TO_PAGE_OFFSET(fs, addr) \
  ( ((addr) - SPIFFS_CFG_PHYS_ADDR(fs)) % SPIFFS_CFG_LOG_PAGE_SZ(fs) )

/* returns containing block for given page */

#define SPIFFS_BLOCK_FOR_PAGE(fs, page) \
  ( (page) / SPIFFS_PAGES_PER_BLOCK(fs) )

/* returns starting page for block */

#define SPIFFS_PAGE_FOR_BLOCK(fs, block) \
  ( (block) * SPIFFS_PAGES_PER_BLOCK(fs) )

/* converts page to entry in object lookup page */

#define SPIFFS_OBJ_LOOKUP_ENTRY_FOR_PAGE(fs, page) \
  ( (page) % SPIFFS_PAGES_PER_BLOCK(fs) - SPIFFS_OBJ_LOOKUP_PAGES(fs) )

/* returns data size in a data page */

#define SPIFFS_DATA_PAGE_SIZE(fs) \
    ( SPIFFS_CFG_LOG_PAGE_SZ(fs) - sizeof(struct spiffs_page_header_s) )

/* returns physical address for block's erase count,
 * always in the physical last entry of the last object lookup page
 */

#define SPIFFS_ERASE_COUNT_PADDR(fs, bix) \
  ( SPIFFS_BLOCK_TO_PADDR(fs, bix) + SPIFFS_OBJ_LOOKUP_PAGES(fs) * SPIFFS_CFG_LOG_PAGE_SZ(fs) - sizeof(int16_t) )

/* returns physical address for block's magic,
 * always in the physical second last entry of the last object lookup page
 */

#define SPIFFS_MAGIC_PADDR(fs, bix) \
  ( SPIFFS_BLOCK_TO_PADDR(fs, bix) + SPIFFS_OBJ_LOOKUP_PAGES(fs) * SPIFFS_CFG_LOG_PAGE_SZ(fs) - sizeof(int16_t)*2 )

/* checks if there is any room for magic in the object luts */

#define SPIFFS_CHECK_MAGIC_POSSIBLE(fs) \
  ( (SPIFFS_OBJ_LOOKUP_MAX_ENTRIES(fs) % (SPIFFS_CFG_LOG_PAGE_SZ(fs)/sizeof(int16_t))) * sizeof(int16_t) \
    <= (SPIFFS_CFG_LOG_PAGE_SZ(fs)-sizeof(int16_t)*2) )

/* define helpers object */

/* entries in an object header page index */

#define SPIFFS_OBJ_HDR_IX_LEN(fs) \
  ((SPIFFS_CFG_LOG_PAGE_SZ(fs) - sizeof(struct spiffs_pgobj_ixheader_s))/sizeof(int16_t))

/* entries in an object page index */

#define SPIFFS_OBJ_IX_LEN(fs) \
  ((SPIFFS_CFG_LOG_PAGE_SZ(fs) - sizeof(spiffs_page_object_ix))/sizeof(int16_t))

/* object index entry for given data span index */

#define SPIFFS_OBJ_IX_ENTRY(fs, spix) \
  ((spix) < SPIFFS_OBJ_HDR_IX_LEN(fs) ? (spix) : (((spix)-SPIFFS_OBJ_HDR_IX_LEN(fs))%SPIFFS_OBJ_IX_LEN(fs)))

/* object index span index number for given data span index or entry */

#define SPIFFS_OBJ_IX_ENTRY_SPAN_IX(fs, spix) \
  ((spix) < SPIFFS_OBJ_HDR_IX_LEN(fs) ? 0 : (1+((spix)-SPIFFS_OBJ_HDR_IX_LEN(fs))/SPIFFS_OBJ_IX_LEN(fs)))

/* get data span index for object index span index */

#define SPIFFS_DATA_SPAN_IX_FOR_OBJ_IX_SPAN_IX(fs, spix) \
  ( (spix) == 0 ? 0 : (SPIFFS_OBJ_HDR_IX_LEN(fs) + (((spix)-1) * SPIFFS_OBJ_IX_LEN(fs))) )

#define SPIFFS_OP_T_OBJ_LU    (0<<0)
#define SPIFFS_OP_T_OBJ_LU2   (1<<0)
#define SPIFFS_OP_T_OBJ_IX    (2<<0)
#define SPIFFS_OP_T_OBJ_DA    (3<<0)
#define SPIFFS_OP_C_DELE      (0<<2)
#define SPIFFS_OP_C_UPDT      (1<<2)
#define SPIFFS_OP_C_MOVS      (2<<2)
#define SPIFFS_OP_C_MOVD      (3<<2)
#define SPIFFS_OP_C_FLSH      (4<<2)
#define SPIFFS_OP_C_READ      (5<<2)
#define SPIFFS_OP_C_WRTHRU    (6<<2)

#define SPIFFS_OP_TYPE_MASK (3<<0)
#define SPIFFS_OP_COM_MASK  (7<<2)

/* if 0, this page is written to, else clean */

#define SPIFFS_PH_FLAG_USED   (1<<0)

/* if 0, writing is finalized, else under modification */

#define SPIFFS_PH_FLAG_FINAL  (1<<1)

/* if 0, this is an index page, else a data page */

#define SPIFFS_PH_FLAG_INDEX  (1<<2)

/* if 0, page is deleted, else valid */

#define SPIFFS_PH_FLAG_DELET  (1<<7)

/* if 0, this index header is being deleted */

#define SPIFFS_PH_FLAG_IXDELE (1<<6)

#define SPIFFS_CHECK_RES(res) \
  do { \
    if ((res) < OK) return (res); \
  } while (0);

#define SPIFFS_VALIDATE_OBJIX(ph, id, spix) \
    if (((ph).flags & SPIFFS_PH_FLAG_USED) != 0) return SPIFFS_ERR_IS_FREE; \
    if (((ph).flags & SPIFFS_PH_FLAG_DELET) == 0) return SPIFFS_ERR_DELETED; \
    if (((ph).flags & SPIFFS_PH_FLAG_FINAL) != 0) return SPIFFS_ERR_NOT_FINALIZED; \
    if (((ph).flags & SPIFFS_PH_FLAG_INDEX) != 0) return SPIFFS_ERR_NOT_INDEX; \
    if (((id) & SPIFFS_OBJ_ID_IX_FLAG) == 0) return SPIFFS_ERR_NOT_INDEX; \
    if ((ph).span_ix != (spix)) return SPIFFS_ERR_INDEX_SPAN_MISMATCH;
    /* if ((spix) == 0 && ((ph).flags & SPIFFS_PH_FLAG_IXDELE) == 0) return
     * SPIFFS_ERR_DELETED;
     */

#define SPIFFS_VALIDATE_DATA(ph, id, spix) \
    if (((ph).flags & SPIFFS_PH_FLAG_USED) != 0) return SPIFFS_ERR_IS_FREE; \
    if (((ph).flags & SPIFFS_PH_FLAG_DELET) == 0) return SPIFFS_ERR_DELETED; \
    if (((ph).flags & SPIFFS_PH_FLAG_FINAL) != 0) return SPIFFS_ERR_NOT_FINALIZED; \
    if (((ph).flags & SPIFFS_PH_FLAG_INDEX) == 0) return SPIFFS_ERR_IS_INDEX; \
    if ((id) & SPIFFS_OBJ_ID_IX_FLAG) return SPIFFS_ERR_IS_INDEX; \
    if ((ph).span_ix != (spix)) return SPIFFS_ERR_DATA_SPAN_MISMATCH;

/* check id, only visit matching objec ids */

#define SPIFFS_VIS_CHECK_ID     (1<<0)

/* report argument object id to visitor - else object lookup id is reported */

#define SPIFFS_VIS_CHECK_PH     (1<<1)

/* stop searching at end of all look up pages */

#define SPIFFS_VIS_NO_WRAP      (1<<2)

#define SPIFFS_HAL_WRITE(_fs, _paddr, _len, _src) \
  (_fs)->cfg.hal_write_f((_paddr), (_len), (_src))
#define SPIFFS_HAL_READ(_fs, _paddr, _len, _dst) \
  (_fs)->cfg.hal_read_f((_paddr), (_len), (_dst))
#define SPIFFS_HAL_ERASE(_fs, _paddr, _len) \
  (_fs)->cfg.hal_erase_f((_paddr), (_len))

#define SPIFFS_CACHE_FLAG_DIRTY       (1<<0)
#define SPIFFS_CACHE_FLAG_WRTHRU      (1<<1)
#define SPIFFS_CACHE_FLAG_OBJLU       (1<<2)
#define SPIFFS_CACHE_FLAG_OBJIX       (1<<3)
#define SPIFFS_CACHE_FLAG_DATA        (1<<4)
#define SPIFFS_CACHE_FLAG_TYPE_WR     (1<<7)

#define SPIFFS_CACHE_PAGE_SIZE(fs) \
  (sizeof(struct spiffs_cache_page_s) + SPIFFS_CFG_LOG_PAGE_SZ(fs))

#define spiffs_get_cache(fs) \
  ((FAR struct spiffs_cache_s *)((fs)->cache))

#define spiffs_get_cache_page_hdr(fs, c, ix) \
  ((struct spiffs_cache_page_s *)(&((c)->cpages[(ix) * SPIFFS_CACHE_PAGE_SIZE(fs)])))

#define spiffs_get_cache_page(fs, c, ix) \
  ((uint8_t *)(&((c)->cpages[(ix) * SPIFFS_CACHE_PAGE_SIZE(fs)])) + sizeof(struct spiffs_cache_page_s))

#define _spiffs_rd(fs, op, id, addr, len, dst) \
  spiffs_phys_rd((fs), (op), (id), (addr), (len), (dst))
#define _spiffs_wr(fs, op, id, addr, len, src) \
  spiffs_phys_wr((fs), (op), (id), (addr), (len), (src))

#ifndef MIN
#  define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#  define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* cache page struct */

struct spiffs_cache_page_s
{
  /* cache flags */

  uint8_t flags;

  /* cache page index */

  uint8_t ix;

  /* last access of this cache page */

  uint32_t last_access;
  union
    {
      /* type read cache */

      struct
        {
          /* read cache page index */

          int16_t pix;
        };

      /* type write cache */

      struct
        {
          /* write cache */

          int16_t id;

          /* offset in cache page */

          uint32_t offset;

          /* size of cache page */

          uint16_t size;
        };
    };
};

/* cache structure */

struct spiffs_cache_s
{
  uint8_t cpage_count;
  uint32_t last_access;
  uint32_t cpage_use_map;
  uint32_t cpage_use_mask;
  uint8_t *cpages;
};

/* object structs */

/* page header, part of each page except object lookup pages
 * NB: this is always aligned when the data page is an object index,
 * as in this case struct spiffs_page_object_ix is used
 */

begin_packed_struct struct spiffs_page_header_s
{
  int16_t id;                          /* object id */
  int16_t span_ix;                     /* object span index */
  uint8_t flags;                       /* flags */
} end_packed_struct;

/* object index header page header */

begin_packed_struct struct spiffs_pgobj_ixheader_s
{
  struct spiffs_page_header_s p_hdr;   /* common page header */
  uint32_t size;                       /* size of object */
  uint8_t type;                        /* type of object */
  uint8_t name[SPIFFS_NAME_MAX];       /* name of object */
#if SPIFFS_OBJ_META_LEN
  uint8_t meta[SPIFFS_OBJ_META_LEN];   /* metadata. not interpreted by SPIFFS in any way. */
#endif
} end_packed_struct;

/* object index page header */

typedef begin_packed_struct struct
{
  struct spiffs_page_header_s p_hdr;
  uint8_t _align[4 - ((sizeof(struct spiffs_page_header_s) & 3) ==
                 0 ? 4 : (sizeof(struct spiffs_page_header_s) & 3))];
} begin_packed_struct spiffs_page_object_ix;

/* callback func for object lookup visitor */

typedef int32_t(*spiffs_visitor_f)(FAR struct spiffs_s *fs, int16_t id,
                                   int16_t bix, int ix_entry,
                                   const void *user_const_p, void *user_var_p);

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

int32_t spiffs_phys_rd(FAR struct spiffs_s *fs,
                     uint8_t op, int16_t id,
                     uint32_t addr, uint32_t len, uint8_t *dst);

int32_t spiffs_phys_wr(FAR struct spiffs_s *fs,
                     uint8_t op, int16_t id,
                     uint32_t addr, uint32_t len, uint8_t *src);

int32_t spiffs_phys_cpy(FAR struct spiffs_s *fs,
                      int16_t id, uint32_t dst, uint32_t src, uint32_t len);

int32_t spiffs_phys_count_free_blocks(FAR struct spiffs_s *fs);

int32_t spiffs_foreach_objlu(FAR struct spiffs_s *fs, int16_t starting_block,
                             int starting_lu_entry, uint8_t flags,
                             int16_t id, spiffs_visitor_f v,
                             FAR const void *user_const_p, FAR void *user_var_p,
                             FAR int16_t *block_ix, int *lu_entry);

int32_t spiffs_erase_block(FAR struct spiffs_s *fs, int16_t bix);

#if SPIFFS_USE_MAGIC && SPIFFS_USE_MAGIC_LENGTH
int32_t spiffs_probe(spiffs_config * cfg);
#endif

int32_t spiffs_obj_lu_scan(FAR struct spiffs_s *fs);

int32_t spiffs_obj_lu_find_free_obj_id(FAR struct spiffs_s *fs,
                                     int16_t *id,
                                     const uint8_t *conflicting_name);

int32_t spiffs_obj_lu_find_free(FAR struct spiffs_s *fs,
                              int16_t starting_block,
                              int starting_lu_entry,
                              int16_t *block_ix, int *lu_entry);

int32_t spiffs_obj_lu_find_id(FAR struct spiffs_s *fs,
                            int16_t starting_block,
                            int starting_lu_entry,
                            int16_t id,
                            int16_t *block_ix, int *lu_entry);

int32_t spiffs_obj_lu_find_id_and_span(FAR struct spiffs_s *fs,
                                     int16_t id,
                                     int16_t spix,
                                     int16_t exclusion_pix,
                                     int16_t *pix);

int32_t spiffs_obj_lu_find_id_and_span_by_phdr(FAR struct spiffs_s *fs,
                                             int16_t id,
                                             int16_t spix,
                                             int16_t exclusion_pix,
                                             int16_t *pix);

int32_t spiffs_page_allocate_data(FAR struct spiffs_s *fs,
                                  int16_t id,
                                  FAR struct spiffs_page_header_s *ph,
                                  FAR uint8_t *data,
                                  uint32_t len,
                                  uint32_t page_offs,
                                  uint8_t finalize, int16_t *pix);

int32_t spiffs_page_move(FAR struct spiffs_s *fs,
                         int16_t id, FAR uint8_t *page_data, int16_t ix,
                         FAR struct spiffs_page_header_s *page_hdr,
                         int16_t src_pix, FAR int16_t *dst_pix);

int32_t spiffs_page_delete(FAR struct spiffs_s *fs, int16_t pix);

int32_t spiffs_object_create(FAR struct spiffs_s *fs,
                           int16_t id,
                           const uint8_t name[],
                           const uint8_t meta[],
                           uint8_t type,
                           int16_t *objix_hdr_pix);

int32_t spiffs_object_update_index_hdr(FAR struct spiffs_s *fs,
                                     FAR struct spiffs_file_s *fobj,
                                     int16_t id,
                                     int16_t objix_hdr_pix,
                                     uint8_t *new_objix_hdr_data,
                                     const uint8_t name[],
                                     const uint8_t meta[],
                                     uint32_t size, int16_t *new_pix);

#if SPIFFS_IX_MAP
int32_t spiffs_populate_ix_map(FAR struct spiffs_s *fs,
                             FAR struct spiffs_file_s *fobj,
                             uint32_t vec_entry_start, uint32_t vec_entry_end);
#endif

void spiffs_cb_object_event(FAR struct spiffs_s *fs,
                            spiffs_page_object_ix * objix,
                            int ev,
                            int16_t id,
                            int16_t spix,
                            int16_t new_pix, uint32_t new_size);

int32_t spiffs_object_open_by_id(FAR struct spiffs_s *fs,
                                 int16_t id,
                                 FAR struct spiffs_file_s *f,
                                 uint16_t flags, uint16_t mode);

int32_t spiffs_object_open_by_page(FAR struct spiffs_s *fs,
                                   int16_t pix,
                                   FAR struct spiffs_file_s *f,
                                   uint16_t flags, uint16_t mode);

int32_t spiffs_object_append(FAR struct spiffs_file_s *fobj,
                             uint32_t offset, uint8_t *data, uint32_t len);

int32_t spiffs_object_modify(FAR struct spiffs_file_s *fobj,
                             uint32_t offset, uint8_t *data, uint32_t len);

int32_t spiffs_object_read(FAR struct spiffs_file_s *fobj, uint32_t offset,
                           uint32_t len, uint8_t *dst);

int32_t spiffs_object_truncate(FAR struct spiffs_file_s *fobj,
                               uint32_t new_len, uint8_t remove_object);

int32_t spiffs_object_find_object_index_header_by_name(FAR struct spiffs_s *fs,
                                                     const uint8_t
                                                     name[SPIFFS_NAME_MAX],
                                                     int16_t *pix);

int32_t spiffs_gc_check(FAR struct spiffs_s *fs, uint32_t len);

int32_t spiffs_gc_erase_page_stats(FAR struct spiffs_s *fs, int16_t bix);

int32_t spiffs_gc_find_candidate(FAR struct spiffs_s *fs,
                               int16_t ** block_candidate,
                               int *candidate_count, char fs_crammed);

int32_t spiffs_gc_clean(FAR struct spiffs_s *fs, int16_t bix);

int32_t spiffs_gc_quick(FAR struct spiffs_s *fs, uint16_t max_free_pages);

int32_t spiffs_find_fileobject(FAR struct spiffs_s *fs, int16_t f, FAR struct spiffs_file_s ** fobj);

void spiffs_cache_init(FAR struct spiffs_s *fs);

void spiffs_cache_drop_page(FAR struct spiffs_s *fs, int16_t pix);

struct spiffs_cache_page_s *spiffs_cache_page_allocate_by_fd(FAR struct spiffs_s *fs,
                                                    FAR struct spiffs_file_s *fobj);

void spiffs_cache_fd_release(FAR struct spiffs_s *fs, struct spiffs_cache_page_s *cp);

struct spiffs_cache_page_s *spiffs_cache_page_get_by_fd(FAR struct spiffs_s *fs, FAR struct spiffs_file_s *fobj);

int32_t spiffs_lookup_consistency_check(FAR struct spiffs_s *fs, uint8_t check_all_objects);

int32_t spiffs_page_consistency_check(FAR struct spiffs_s *fs);

int32_t spiffs_object_index_consistency_check(FAR struct spiffs_s *fs);

#endif /* __FS_SPIFFS_SRC_SPIFFS_NUCLEIUS_H */
