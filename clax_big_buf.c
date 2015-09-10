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

#include "clax_log.h"
#include "clax_big_buf.h"
#include "clax_util.h"

clax_big_buf_t *clax_big_buf_init(clax_big_buf_t *bbuf, char *temp_dir, size_t max_size)
{
    memset(bbuf, 0, sizeof(clax_big_buf_t));

    bbuf->temp_dir = strdup(temp_dir ? temp_dir : "/tmp");
    bbuf->max_size = max_size;

    return bbuf;
}

clax_big_buf_t *clax_big_buf_free(clax_big_buf_t *bbuf)
{
    free(bbuf->memory);

    if (bbuf->fpath && strlen(bbuf->fpath)) {
        if (access(bbuf->fpath, F_OK) != -1) {
            unlink(bbuf->fpath);
        }
    }

    free(bbuf->temp_dir);
    free(bbuf->fpath);
    if (bbuf->fh) {
        fclose(bbuf->fh);
    }

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
            int fd;
            const char *template = "/.fileXXXXXX";
            char fpath[1024] = {0};
            strncat(fpath, bbuf->temp_dir, sizeof(fpath) - strlen(template));
            strcat(fpath, template);
            fd = mkstemp(fpath);
            if (fd < 0) return -1;
            close(fd);

            clax_log("memory is too big, saving to file '%s'", fpath);

            bbuf->fh = fopen(fpath, "wb");

            if (bbuf->fh == NULL) {
                clax_log("Creating file '%s' failed", fpath);
                return -1;
            }

            bbuf->fpath = strdup(fpath);

            if (bbuf->len) {
                size_t wcount = fwrite(bbuf->memory, 1, bbuf->len, bbuf->fh);
                if (wcount != bbuf->len) {
                    clax_log("Error writing to file");
                    return -1;
                }

                free(bbuf->memory);
                bbuf->memory = NULL;
            }
        }

        size_t wcount = fwrite(buf, 1, len, bbuf->fh);
        if (wcount != len) {
            clax_log("Error writing to file");
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
        clax_log("Renaming to file '%s' -> '%s'", bbuf->fpath, fpath);
        return rename(bbuf->fpath, fpath);
    }
    else {
        clax_log("Saving to file '%s'", fpath);

        FILE *fh;

        fh = fopen(fpath, "wb");

        if (fh == NULL) {
            return -1;
        }

        fwrite(bbuf->memory, 1, bbuf->len, fh);

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
