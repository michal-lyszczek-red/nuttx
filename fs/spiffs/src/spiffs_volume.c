/****************************************************************************
 * fs/spiffs.h/spiffs_volume.c
 * SPIFFS Utility Functions for Volume and File Object Support
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

#include <sys/stat.h>
#include <fcntl.h>

#include <nuttx/kmalloc.h>

#include "spiffs.h"
#include "spiffs_nucleus.h"

/****************************************************************************
 * Public Functions
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
                      FAR struct stat *buf)
{
  struct spiffs_pgobj_ixheader_s objhdr;
  uint32_t obj_id_addr;
  int16_t ndx;
  mode_t mode;
  int ret;

  ret = spiffs_phys_rd(fs, SPIFFS_OP_T_OBJ_IX | SPIFFS_OP_C_READ, objid,
                       SPIFFS_PAGE_TO_PADDR(fs, pgndx),
                       sizeof(struct spiffs_pgobj_ixheader_s),
                       (FAR uint8_t *) & objhdr);
  if (ret < 0)
    {
      return ret;
    }

  obj_id_addr =
    SPIFFS_BLOCK_TO_PADDR(fs, SPIFFS_BLOCK_FOR_PAGE(fs, pgndx)) +
    SPIFFS_OBJ_LOOKUP_ENTRY_FOR_PAGE(fs, pgndx) * sizeof(int16_t);

  ret = spiffs_phys_rd(fs, SPIFFS_OP_T_OBJ_LU | SPIFFS_OP_C_READ, objid,
                       obj_id_addr, sizeof(int16_t), (FAR uint8_t *)&ndx);
  if (ret < 0)
    {
      return ret;
    }

  /* Build the struct stat */

  mode  = S_IRWXO | S_IRWXG  | S_IRWXU;  /* Assume all permissions */
  mode |= S_IFREG;                       /* Assume regular file */

  /* REVISIT:  Should the file object type derive from objhdr.type? */

  memset(buf, 0, sizeof(struct stat));
  buf->st_mode = mode;
  buf->st_size = objhdr.size == SPIFFS_UNDEFINED_LEN ? 0 : objhdr.size;

#warning REVISIT: Need st_blksize and st_blocks

  return ret;
}

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
 *   Zero (OK) is returned on success; A negated errno value is returned on
 *   any failure.
 *
 ****************************************************************************/

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
 *   Zero (OK) is returned on success; A negated errno value is returned on
 *   any failure.
 *
 ****************************************************************************/

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

/****************************************************************************
 * Name: spiffs_fflush_cache
 *
 * Description:
 *   Checks if there are any cached writes for the object ID associated with
 *   given file object. If so, these writes are flushed.
 *
 * Input Parameters:
 *   fs     - A reference to the SPIFFS volume object instance
 *   fobj   - A reference to the file object to flush
 *
 * Returned Value:
 *   On success, then number of bytes flushed is returned.  A negated errno
 *   value is returned on any failure.
 *
 ****************************************************************************/

ssize_t spiffs_fflush_cache(FAR struct spiffs_s *fs,
                            FAR struct spiffs_file_s *fobj)
{
  ssize_t nwritten = 0;

  /* If we were asked to use direct hardware accessed, then don't bother
   * flushing the cache.
   */

  if ((fobj->flags & O_DIRECT) == 0)
    {
      if (fobj->cache_page == 0)
        {
          /* See if object ID is associated with cache already */

          fobj->cache_page = spiffs_cache_page_get_byfd(fs, fobj);
        }

      if (fobj->cache_page)
        {
          spiffs_cacheinfo("Flushing cache page %d for objid=%d offset=%d size=%d\n",
                           fobj->cache_page->ix, fobj->objid,
                           fobj->cache_page->offset, fobj->cache_page->size);

          nwritten =
            spiffs_fobj_write(fs, fobj,
                              spiffs_get_cache_page(fs, spiffs_get_cache(fs),
                                                    fobj->cache_page->ix),
                              fobj->cache_page->offset, fobj->cache_page->size);
          if (nwritten < 0)
            {
              ferr("ERROR: spiffs_fobj_write failed %d\n", (int)nwritten);
            }

          spiffs_cache_fd_release(fs, fobj->cache_page);
        }
    }

  return nwritten;
}

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
                          FAR const void *buffer, off_t offset, size_t len)
{
  ssize_t remaining = len;
  ssize_t nwritten = 0;

  if (fobj->size != SPIFFS_UNDEFINED_LEN && offset < fobj->size)
    {
      FAR const uint8_t *tmp;
      ssize_t wrsize;

      wrsize   = MIN((ssize_t)(fobj->size - offset), len);
      nwritten = spiffs_object_modify(fs, fobj, offset,
                                      (FAR uint8_t *)buffer, wrsize);
      if (nwritten <= 0)
        {
          return nwritten;
        }

      remaining -= nwritten;

      tmp        = (FAR const uint8_t *)buffer;
      tmp       += nwritten;
      buffer     = tmp;
      offset    += nwritten;
    }

  if (remaining > 0)
    {
      ssize_t nappend;
      nappend = spiffs_object_append(fs, fobj, offset,
                                     (FAR uint8_t *)buffer, remaining);
      if (nappend < 0)
        {
          return (ssize_t)nappend;
        }

      nwritten += nappend;
    }

  return (ssize_t)nwritten;
}

/****************************************************************************
 * Name: spiffs_fobj_read
 *
 * Description:
 *   Read from a file object
 *
 * Input Parameters:
 *   fs     - A reference to the volume structure
 *   fobj   - A reference to the file object to read from
 *   buffer - The location that the data is read to
 *   offset - The FLASH offset to be read
 *   len    - The number of bytes to be read
 *   fpos   - The file position to read from
 *
 * Returned Value:
 *   On success, then number of bytes written is returned.  A negated errno
 *   value is returned on any failure.
 *
 ****************************************************************************/

ssize_t spiffs_fobj_read(FAR struct spiffs_s *fs,
                         FAR struct spiffs_file_s *fobj, FAR void *buffer,
                         size_t buflen, off_t fpos)
{
  ssize_t nread;

  /* Make sure that read access is supported */

  if ((fobj->flags & O_RDONLY) == 0)
    {
      return -EACCES;
    }

  /* Handle the special case of zero-length files */

  if (fobj->size == SPIFFS_UNDEFINED_LEN && buflen > 0)
    {
      /* Return zero (meaning EOF) */

      return 0;
    }

  spiffs_fflush_cache(fs, fobj);

  /* Check for attempts to read beyond the end of the file */

  if (fpos + buflen >= fobj->size)
    {
      /* Truncate */

      buflen = fobj->size - fpos;

      /* Check if we are already at or beyond the end of the file */

      if (buflen <= 0)
        {
          /* Return zero (meaning EOF) */

          nread = 0;
        }
      else
        {
          /* Then read from the file object */

          nread = spiffs_object_read(fs, fobj, fpos, buflen,
                                     (FAR uint8_t *)buffer);
        }
    }
  else
    {
      /* Reading within file size */

      nread = spiffs_object_read(fs, fobj, fpos, buflen,
                                 (FAR uint8_t *)buffer);
    }

  return nread;
}

/****************************************************************************
 * Name: spiffs_fobj_free
 *
 * Description:
 *   Free all resources used a file object
 *
 * Input Parameters:
 *   fs   - A reference to the volume structure
 *   fobj - A reference to the file object to be removed
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void spiffs_fobj_free(FAR struct spiffs_s *fs, FAR struct spiffs_file_s *fobj)
{
  FAR struct spiffs_file_s *curr;
  int ret;

  /* Flush any buffered write data */

  ret = spiffs_fflush_cache(fs, fobj);
  if (ret < 0)
    {
      ferr("ERROR: spiffs_fflush_cache failed: %d\n", ret);
    }

  /* Remove the file object from the list of file objects in the volume
   * structure.
   */

  for (curr  = (FAR struct spiffs_file_s *)dq_peek(&fs->objq);
       curr != NULL;
       curr  = (FAR struct spiffs_file_s *)dq_next((FAR dq_entry_t *)curr))
    {
      /* Is this the entry we are searching for? */

      if (curr == fobj)
        {
          /* Yes, remove it from the list of of file objects */

          dq_rem((FAR dq_entry_t *)curr, &fs->objq);
        }
    }

  DEBUGASSERT(curr != NULL);

  /* Now we can remove the file by truncating it to zero length */

  ret = spiffs_object_truncate(fs, fobj, 0, true);
  if (ret < 0)
    {
      ferr("ERROR: spiffs_object_truncate failed: %d\n", ret);
    }

  /* Then free the file object itself (which contains the lock we hold) */

  kmm_free(fobj);
}
