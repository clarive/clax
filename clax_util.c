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

#include <stdlib.h>
#include <string.h>

#include "clax_util.h"

void clax_kv_list_init(clax_kv_list_t *list)
{
    memset(list, 0, sizeof(clax_kv_list_t));
}

void clax_kv_list_free(clax_kv_list_t *list)
{
    for (int i = 0; i < list->size; i++) {
        clax_kv_list_item_t *item = list->items[i];

        free(item->key);
        free(item->val);

        free(item);
    }

    free(list->items);

    clax_kv_list_init(list);
}

int clax_kv_list_pushn(clax_kv_list_t *list, char *key, size_t key_len, char *val, size_t val_len)
{
    if (list->items == NULL) {
        list->items = malloc(sizeof(clax_kv_list_item_t *));
    } else {
        list->items = realloc(list->items, sizeof(clax_kv_list_item_t *) * (list->size + 1));
    }

    list->items[list->size] = malloc(sizeof(clax_kv_list_item_t));

    list->items[list->size]->key = malloc(key_len + 1);
    if (key)
        strcpy(list->items[list->size]->key, key);
    list->items[list->size]->key[key_len] = 0;

    list->items[list->size]->val = malloc(val_len + 1);
    if (val)
        strcpy(list->items[list->size]->val, val);
    list->items[list->size]->val[val_len] = 0;

    list->size++;

    return 0;
}

int clax_kv_list_push(clax_kv_list_t *list, char *key, char *val)
{
    return clax_kv_list_pushn(list, key, key ? strlen(key) : 0, val, val ? strlen(val) : 0);
}

int clax_kv_list_set(clax_kv_list_t *list, char *key, char *val)
{
    clax_kv_list_item_t *item = clax_kv_list_find_item(list, key);

    if (item == NULL) {
        return clax_kv_list_push(list, key, val);
    }

    free(item->val);

    item->val = malloc(strlen(val) + 1);
    item->val[0] = 0;
    strcat(item->val, val);

    return 0;
}

char *clax_kv_list_find(clax_kv_list_t *list, char *key)
{
    clax_kv_list_item_t *item = clax_kv_list_find_item(list, key);

    if (!item)
        return NULL;

    return item->val;
}

clax_kv_list_item_t *clax_kv_list_find_item(clax_kv_list_t *list, char *key)
{
    for (int i = 0; i < list->size; i++) {
        if (strcmp(list->items[i]->key, key) == 0) {
            return list->items[i];
        }
    }

    return NULL;
}

char *clax_kv_list_find_last(clax_kv_list_t *list, char *key)
{
    for (int i = list->size - 1; i > 0; i--) {
        if (strcmp(list->items[i]->key, key) == 0) {
            return list->items[i]->val;
        }
    }

    return NULL;
}

char *clax_kv_list_find_all(clax_kv_list_t *list, char *key, size_t *start)
{
    for (int i = *start; i < list->size; i++) {
        if (strcmp(list->items[i]->key, key) == 0) {
            *start = i + 1;
            return list->items[i]->val;
        }
    }

    return NULL;
}

clax_kv_list_item_t *clax_kv_list_at(clax_kv_list_t *list, size_t index)
{
    if (list->size == 0 || index > list->size - 1)
        return NULL;

    return list->items[index];
}

clax_kv_list_item_t *clax_kv_list_next(clax_kv_list_t *list, size_t *start)
{
    clax_kv_list_item_t *item = clax_kv_list_at(list, *start);

    if (item)
        (*start)++;

    return item;
}

void clax_kv_list_dump(clax_kv_list_t *list)
{
    clax_kv_list_item_t *item;
    size_t iter = 0;
    char *str = NULL;

    while ((item = clax_kv_list_next(list, &iter)) != NULL) {
        clax_str_append(&str, item->key);
        clax_str_append(&str, "=");
        clax_str_append(&str, item->val);
        clax_str_append(&str, "\n");
    }

    printf("%s", str);

    free(str);
}

char *clax_buf2str(const char *buf, size_t len)
{
    char *str = malloc(len + 1);
    strncpy(str, (const char *)buf, len);
    str[len] = 0;

    return str;
}

void clax_str_append(char **dst, const char *src)
{
    size_t dst_len = *dst ? strlen(*dst) : 0;
    size_t src_len = strlen(src);

    if (!*dst) {
        *dst = malloc(src_len + 1);
        (*dst)[0] = 0;
        strcat(*dst, src);
    }
    else {
        *dst = realloc(*dst, dst_len + src_len + 1);
        memmove(*dst + dst_len, src, src_len);
        (*dst)[dst_len + src_len] = 0;
    }
}

void clax_buf_append(unsigned char **dst, size_t *dst_len, const char *src, size_t src_len)
{
    if (!*dst) {
        *dst = malloc(src_len);
        memcpy((void *)*dst, (const void *)src, src_len);
        *dst_len = src_len;
    }
    else {
        *dst = realloc((void *)*dst, *dst_len + src_len);
        memcpy((void *)(*dst + *dst_len), (const void *)src, src_len);
        *dst_len += src_len;
    }
}

void clax_hexdump(unsigned char *buf, size_t len)
{
    int i;
    int w = 32;
    unsigned char *line = malloc(w + 1);

    printf("\n");

    for (i = 0; i < len; i++) {
        if ((i % w) == 0) {
            if (i != 0)
                printf("  %s\n", line);

            printf("  %04x ", i);
        }

        printf(" %02x", buf[i]);

        if ((buf[i] < 0x20) || (buf[i] > 0x7e))
            line[i % w] = '.';
        else
            line[i % w] = buf[i];
        line[(i % w) + 1] = '\0';
    }

    while ((i % w) != 0) {
        printf("   ");
        i++;
    }

    printf("  %s\n\n", line);

    free(line);
}

void clax_san_path(char *buf)
{
    char *p = buf;

    while (1) {
        if ((p = strstr(buf, "/../")) != NULL) {
            memmove(p, p + 3, strlen(buf) - (p - buf));
            continue;
        }

        if ((p = strstr(buf, "/./")) != NULL) {
            memmove(p, p + 2, strlen(buf) - (p - buf));
            continue;
        }

        if ((p = strstr(buf, "//")) != NULL) {
            memmove(p, p + 1, strlen(buf) - (p - buf));
            continue;
        }

        break;
    }
}
