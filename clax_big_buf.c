/*
 *  Copyright (C) 2015, Clarive Software, All Rights Reserved
 *
 *  This file is part of clax.
 *
 *  Clax is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Clax is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A memoryICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Clax.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "clax_log.h"
#include "clax_big_buf.h"
#include "clax_util.h"
#include "clax_platform.h"

clax_big_buf_t *clax_big_buf_init(clax_big_buf_t *bbuf, char *temp_dir, size_t max_size)
{
    memset(bbuf, 0, sizeof(clax_big_buf_t));

    bbuf->temp_dir = clax_strdup(temp_dir ? temp_dir : ".");
    bbuf->max_size = max_size;

    return bbuf;
}

clax_big_buf_t *clax_big_buf_free(clax_big_buf_t *bbuf)
{
    free(bbuf->memory);

    if (bbuf->fh) {
        fclose(bbuf->fh);
    }

    if (bbuf->fpath && strlen(bbuf->fpath)) {
        unlink(bbuf->fpath);
    }

    free(bbuf->temp_dir);
    free(bbuf->fpath);

    return bbuf;
}

int clax_big_buf_append_str(clax_big_buf_t *bbuf, const char *buf)
{
    return clax_big_buf_append(bbuf, (const unsigned char *)buf, strlen(buf));
}

int clax_big_buf_append(clax_big_buf_t *bbuf, const unsigned char *buf, size_t len)
{
    if (bbuf->temp_dir && bbuf->max_size && bbuf->len + len > bbuf->max_size) {
        if (!bbuf->fh) {
            char *tmpfile = clax_mktmpfile_alloc(bbuf->temp_dir, ".fileXXXXXXXX");

            clax_log("Memory is too big, saving to file '%s'", tmpfile);

            bbuf->fh = fopen(tmpfile, "wb");

            if (bbuf->fh == NULL) {
                clax_log("Creating file '%s' failed", tmpfile);
                return -1;
            }

            bbuf->fpath = clax_strdup(tmpfile);

            if (bbuf->len) {
                size_t wcount = fwrite(bbuf->memory, 1, bbuf->len, bbuf->fh);
                if (wcount != bbuf->len) {
                    clax_log("Error writing to file '%s': %s", tmpfile, strerror(errno));
                    return -1;
                }

                free(bbuf->memory);
                bbuf->memory = NULL;
            }

            free(tmpfile);
        }

        size_t wcount = fwrite(buf, 1, len, bbuf->fh);
        if (wcount != len) {
            clax_log("Error writing to file '%s': %s", bbuf->fpath, strerror(errno));
            return -1;
        }

        bbuf->len += len;
    }
    else {
        clax_buf_append(&bbuf->memory, &bbuf->len, (const char *)buf, len);
    }

    return 0;
}

int clax_big_buf_write_file(clax_big_buf_t *bbuf, char *fpath)
{
    if (bbuf->fpath && strlen(bbuf->fpath)) {
        clax_log("Renaming '%s' -> '%s'", bbuf->fpath, fpath);

        /* TODO: after files is closed big_buf is unusable, this has to be handled somehow */
        fclose(bbuf->fh);
        bbuf->fh = NULL;

        int ret = rename(bbuf->fpath, fpath);

        if (ret < 0) {
            clax_log("Renaming '%s' -> '%s' failed: %s'", bbuf->fpath, fpath, strerror(errno));
        }

        return ret;
    }
    else {
        clax_log("Saving to file '%s'", fpath);

        FILE *fh;

        fh = fopen(fpath, "wb");

        if (fh == NULL) {
            clax_log("Error opening file '%s': %s", fpath, strerror(errno));
            return -1;
        }

        if (fwrite(bbuf->memory, 1, bbuf->len, fh) != bbuf->len) {
            clax_log("Saving to file '%s' failed: %s", fpath, strerror(errno));
        }

        fclose(fh);

        return 0;
    }
}

size_t clax_big_buf_read(clax_big_buf_t *bbuf, unsigned char *buf, size_t len, size_t offset)
{
    size_t ret = 0;

    if (bbuf->fpath && strlen(bbuf->fpath)) {
        FILE *fh;

        fh = fopen(bbuf->fpath, "rb");

        if (fh == NULL) {
            return -1;
        }

        fseek(fh, offset, 0);
        ret = fread(buf, 1, len, fh);

        fclose(fh);
    }
    else {
        if (offset > bbuf->len)
            return 0;

        size_t rcount = MIN(bbuf->len - offset, len);
        memcpy(buf, bbuf->memory + offset, rcount);

        ret = rcount;
    }

    return ret;
}

int clax_big_buf_close(clax_big_buf_t *bbuf)
{
    int ret;

    if (bbuf->fh) {
        clax_log("Closing part file");
        ret = fclose(bbuf->fh);

        bbuf->fh = NULL;

        return ret;
    }

    return 0;
}
