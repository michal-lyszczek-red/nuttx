/****************************************************************************
 * fs/spiffs.h/spiffs.h
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

#ifndef __FS_SPIFFS_SRC_SPIFFS_H
#define __FS_SPIFFS_SRC_SPIFFS_H

#if defined(__cplusplus)
extern "C"
{
#endif

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <sys/mount.h>
#include <queue.h>

#include <nuttx/mtd/mtd.h>

#include "spiffs_config.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define SPIFFS_ERR_DELETED              -10004
#define SPIFFS_ERR_NOT_FINALIZED        -10005
#define SPIFFS_ERR_NOT_INDEX            -10006
#define SPIFFS_ERR_FILE_DELETED         -10009
#define SPIFFS_ERR_IS_INDEX             -10011
#define SPIFFS_ERR_IS_FREE              -10012
#define SPIFFS_ERR_INDEX_SPAN_MISMATCH  -10013
#define SPIFFS_ERR_DATA_SPAN_MISMATCH   -10014
#define SPIFFS_ERR_INDEX_REF_FREE       -10015
#define SPIFFS_ERR_INDEX_REF_LU         -10016
#define SPIFFS_ERR_INDEX_REF_INVALID    -10017
#define SPIFFS_ERR_INDEX_FREE           -10018
#define SPIFFS_ERR_INDEX_LU             -10019
#define SPIFFS_ERR_INDEX_INVALID        -10020
#define SPIFFS_ERR_NOT_A_FS             -10025
#define SPIFFS_ERR_MAGIC_NOT_POSSIBLE   -10028
#define SPIFFS_ERR_NO_DELETED_BLOCKS    -10029
#define SPIFFS_ERR_PROBE_TOO_FEW_BLOCKS -10034
#define SPIFFS_ERR_PROBE_NOT_A_FS       -10035
#define SPIFFS_ERR_IX_MAP_UNMAPPED      -10037
#define SPIFFS_ERR_IX_MAP_MAPPED        -10038
#define SPIFFS_ERR_IX_MAP_BAD_RANGE     -10039
#define SPIFFS_ERR_INTERNAL             -10050

/* Flags on open file/directory options */

#define SFO_FLAG_UNLINKED               (1 << 0)

/* Re-entrant semaphore definitions */

#define SPIFFS_NO_HOLDER                ((pid_t)-1)

/****************************************************************************
 * Public Types
 ****************************************************************************/

enum spiffs_check_e
{
  SPIFFS_CHECK_LOOKUP = 0,
  SPIFFS_CHECK_INDEX,
  SPIFFS_CHECK_PAGE
};

/* file system check callback report type */

enum spiffs_check_report_e
{
  SPIFFS_CHECK_PROGRESS = 0,
  SPIFFS_CHECK_ERROR,
  SPIFFS_CHECK_FIX_INDEX,
  SPIFFS_CHECK_FIX_LOOKUP,
  SPIFFS_CHECK_DELETE_ORPHANED_INDEX,
  SPIFFS_CHECK_DELETE_PAGE,
  SPIFFS_CHECK_DELETE_BAD_FILE
};

/* spi read call function type */

typedef int32_t(*spiffs_read_t)(uint32_t addr, uint32_t size, uint8_t * dst);

/* spi write call function type */

typedef int32_t(*spiffs_write_t)(uint32_t addr, uint32_t size, uint8_t * src);

/* spi erase call function type */

typedef int32_t(*spiffs_erase_t)(uint32_t addr, uint32_t size);

/* Re-entrant semaphore */

struct spiffs_sem_s
{
  sem_t    sem;                     /* The actual semaphore */
  pid_t    holder;                  /* Current older (-1 if not held) */
  uint16_t count;                   /* Number of counts held */
};

/* spiffs SPI configuration struct */

/* This structure represents the current state of an SPIFFS volume */

struct spiffs_file_s;               /* Forward reference */

struct spiffs_s
{
  struct mtd_geometry_s geo;        /* FLASH geometry */
  struct spiffs_sem_s exclsem;      /* Supports mutually exclusive access */
  dq_queue_t objq;                  /* A doubly linked list of open file objects */
  FAR struct mtd_dev_s *mtd;        /* The contained MTD interface */
  FAR uint8_t *lu_work;             /* Primary work buffer, size of a logical page */
  FAR uint8_t *work;                /* Secondary work buffer, size of a logical page */
  FAR void *cache;                  /* Cache memory */
  off_t phys_size;                  /* physical size of the SPI flash */
  int free_entry;                   /* Cursor for free blocks, entry index */
  int lu_entry;                     /* Cursor when searching, entry index */
  uint32_t free_blocks;             /* Current number of free blocks */
  uint32_t stats_p_allocated;       /* Current number of busy pages */
  uint32_t stats_p_deleted;         /* Current number of deleted pages */
#if CONFIG_SPIFFS_GCDBG
  uint32_t stats_gc_runs;
#endif
  uint32_t cache_size;              /* Cache size */
#if CONFIG_SPIFFS_CACHEDBG
  uint32_t cache_hits;              /* Number of cache hits */
  uint32_t cache_misses;            /* Number of cache misses */
#endif
  uint32_t config_magic;            /* Config magic */
  int16_t free_blkndx;              /* cursor for free blocks, block index */
  int16_t lu_blkndx;                /* Cursor when searching, block index */
  int16_t max_erase_count;          /* Max erase count amongst all blocks */
};

/* This structure represents the state of an open file */

struct spiffs_cache_page_s;         /* Forward reference */

struct spiffs_file_s
{
  dq_entry_t entry;                 /* Supports a doubly linked list */
  FAR struct spiffs_cache_page_s *cache_page;
  int16_t crefs;                    /* Reference count */
  int16_t objid;                    /* Unique ID of the file object */
  uint8_t flags;                    /* See SFO_FLAG_* definitions */
  int16_t objhdr_pgndx;             /* Cached object index header page index */
  int16_t objndx_pgndx;             /* Cached offset object index page index */
  int16_t objndx_spndx;             /* Cached offset object index span index */
  uint16_t oflags;                  /* File object open flags */
  off_t size;                       /* Size of the file */
  off_t offset;                     /* Current absolute offset */
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: spiffs_stat_pgndx
 *
 * Description:
 *   Checks if there are any cached writes for the object ID associated with
 *   given file object. If so, these writes are flushed.
 *
 * Input Parameters:
 *   fobj   - A reference to the file object to flush
 *
 * Returned Value:
 *   On success, then number of bytes flushed is returned.  A negated errno
 *   value is returned on any failure.
 *
 ****************************************************************************/

int spiffs_stat_pgndx(FAR struct spiffs_s *fs, int16_t pgndx, int16_t objid,
                      FAR struct stat *buf);

/****************************************************************************
 * Name: spiffs_find_fobj_bypgndx
 *
 * Description:
 *   Given the page index of the object header, find the corresponding file
 *   object instance.
 *
 * Input Parameters:
 *   fs     - A reference to the SPIFFS volume object instance
 *   pgndx  - The page index to match
 *   ppfobj - A user provided location in which to return the matching file
 *            file object instance
 *
 * Returned Value:
 *   Zero (OK) is returned on success; A negated errno value is returnd on
 *   any failure.
 *
 ****************************************************************************/

int spiffs_find_fobj_bypgndx(FAR struct spiffs_s *fs, int16_t pgndx,
                             FAR struct spiffs_file_s **ppfobj);

/****************************************************************************
 * Name: spiffs_find_fobj_byobjid
 *
 * Description:
 *   Given a object ID, find the corresponding file object instance
 *
 * Input Parameters:
 *   fs     - A reference to the SPIFFS volume object instance
 *   objid  - The object ID to match
 *   ppfobj - A user provided location in which to return the matching file
 *            file object instance
 *
 * Returned Value:
 *   Zero (OK) is returned on success; A negated errno value is returnd on
 *   any failure.
 *
 ****************************************************************************/

int spiffs_find_fobj_byobjid(FAR struct spiffs_s *fs, int16_t objid,
                             FAR struct spiffs_file_s **ppfobj);

/****************************************************************************
 * Name: spiffs_fobj_write
 *
 * Description:
 *   Checks if there are any cached writes for the object ID associated with
 *   given file object. If so, these writes are flushed.
 *
 * Input Parameters:
 *   fobj   - A reference to the file object to flush
 *
 * Returned Value:
 *   On success, then number of bytes flushed is returned.  A negated errno
 *   value is returned on any failure.
 *
 ****************************************************************************/

ssize_t spiffs_fflush_cache(FAR struct spiffs_file_s *fobj);

/****************************************************************************
 * Name: spiffs_fobj_write
 *
 * Description:
 *   Write to a file object
 *
 * Input Parameters:
 *   fs     - A reference to the volume structure
 *   fobj   - A reference to the file object to write to
 *   buffer - The data to be written
 *   offset - The FLASH offset to be written
 *   len    - The number of bytes to be written
 *
 * Returned Value:
 *   On success, then number of bytes written is returned.  A negated errno
 *   value is returned on any failure.
 *
 ****************************************************************************/

ssize_t spiffs_fobj_write(FAR struct spiffs_s *fs,
                          FAR struct spiffs_file_s *fobj,
                          FAR const void *buffer, off_t offset, size_t len);

/****************************************************************************
 * Name: spiffs_fobj_free
 *
 * Description:
 *   Free all resources used by a file object
 *
 * Input Parameters:
 *   fs   - A reference to the volume structure
 *   fobj - A reference to the file object to be removed
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void spiffs_fobj_free(FAR struct spiffs_s *fs, FAR struct spiffs_file_s *fobj);

#if defined(__cplusplus)
}
#endif

#endif  /* __FS_SPIFFS_SRC_SPIFFS_H */
