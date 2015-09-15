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

#include "clax_http.h"
#include "clax_http_multipart.h"
#include "clax_util.h"

void clax_http_multipart_list_init(clax_http_multipart_list_t *list, char *tempdir)
{
    memset(list, 0, sizeof(clax_http_multipart_list_t));

    list->tempdir = strdup(tempdir ? tempdir : ".");
}

void clax_http_multipart_list_free(clax_http_multipart_list_t *list)
{
    for (int i = 0; i < list->size; i++) {
        clax_http_multipart_t *item = list->items[i];

        clax_kv_list_free(&item->headers);
        clax_big_buf_free(&item->bbuf);

        free(item);
    }

    free(list->items);

    free(list->tempdir);

    memset(list, 0, sizeof(clax_http_multipart_list_t));
}

clax_http_multipart_t *clax_http_multipart_list_push(clax_http_multipart_list_t *list)
{
    if (list->items == NULL) {
        list->items = malloc(sizeof(clax_http_multipart_t *));
    } else {
        list->items = realloc(list->items, sizeof(clax_http_multipart_t *) * (list->size + 1));
    }

    list->items[list->size] = malloc(sizeof(clax_http_multipart_t));
    clax_http_multipart_t *multipart = list->items[list->size];

    memset(multipart, 0, sizeof(clax_http_multipart_t));

    clax_kv_list_init(&multipart->headers);
    clax_big_buf_init(&multipart->bbuf, list->tempdir, 1024 * 1024);

    list->size++;

    return list->items[list->size - 1];
}

clax_http_multipart_t *clax_http_multipart_list_at(clax_http_multipart_list_t *list, size_t index)
{
    if (list->size == 0)
        return NULL;

    if (index >= list->size)
        return NULL;

    return list->items[index];
}

clax_http_multipart_t *clax_http_multipart_list_last(clax_http_multipart_list_t *list)
{
    if (list->size == 0)
        return NULL;

    return list->items[list->size - 1];
}
