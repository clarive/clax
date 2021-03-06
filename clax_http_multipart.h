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

#ifndef CLAX_HTTP_MULTIPART_H
#define CLAX_HTTP_MULTIPART_H

#include <stdio.h>

#include "clax_big_buf.h"
#include "clax_util.h"

typedef struct {
  clax_kv_list_t headers;
  clax_big_buf_t bbuf;
  int done;
} clax_http_multipart_t;

typedef struct {
    clax_http_multipart_t **items;
    size_t size;
    char *tempdir;
} clax_http_multipart_list_t;

void clax_http_multipart_list_init(clax_http_multipart_list_t *list, char *tempdir);
void clax_http_multipart_list_free(clax_http_multipart_list_t *list);
clax_http_multipart_t *clax_http_multipart_list_push(clax_http_multipart_list_t *list);
clax_http_multipart_t *clax_http_multipart_list_at(clax_http_multipart_list_t *list, size_t index);
clax_http_multipart_t *clax_http_multipart_list_last(clax_http_multipart_list_t *list);

#endif
