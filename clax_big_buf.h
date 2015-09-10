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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Clax.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CLAX_BIG_BUF_H
#define CLAX_BIG_BUF_H

#include <stdio.h>

typedef struct {
    unsigned char *memory;
    FILE *fh;
    size_t len;

    size_t max_size;
    char *temp_dir;
    char *fpath;
} clax_big_buf_t;

clax_big_buf_t *clax_big_buf_init(clax_big_buf_t *bbuf, char *temp_dir, size_t max_size);
clax_big_buf_t *clax_big_buf_free(clax_big_buf_t *bbuf);
int clax_big_buf_append(clax_big_buf_t *bbuf, const unsigned char *buf, size_t len);
int clax_big_buf_append_str(clax_big_buf_t *bbuf, const char *str);
int clax_big_buf_write_file(clax_big_buf_t *bbuf, char *fpath);
int clax_big_buf_close(clax_big_buf_t *bbuf);
size_t clax_big_buf_read(clax_big_buf_t *bbuf, unsigned char *buf, size_t len, size_t offset);

#endif
