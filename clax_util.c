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
#include <stdarg.h>
#include <unistd.h>
#include <iconv.h>

#ifdef _WIN32
# include <windows.h>
#else
# include <sys/stat.h>
# include <sys/types.h>
#endif

#include "clax_util.h"
#include "clax_platform.h"

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

char *clax_buf2str(const char *buf, size_t len)
{
    char *str = malloc(len + 1);
    strncpy(str, (const char *)buf, len);
    str[len] = 0;

    return str;
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

char *clax_strjoin(char *sep, ...)
{
    char *p;
    char *join = clax_strdup("");
    size_t sep_len = strlen(sep);
    va_list a_list;

    va_start(a_list, sep);

    int first = 1;
    while ((p = (char *)va_arg(a_list, void *)) != NULL) {
        if (!strlen(p))
            continue;

        if (first) {
            join = realloc(join, strlen(p) + 1);
            strcat(join, p);

            first = 0;
        } else {
            join = realloc(join, strlen(join) + sep_len + strlen(p) + 1);
            strcat(join, sep);
            strcat(join, p);
        }
    }

    va_end(a_list);

    return join;
}

int clax_mkdir(char *dirname, int mode)
{
    int ret;

#ifdef _WIN32
    ret = mkdir(dirname);
#else
    ret = mkdir(dirname, mode);
#endif

    return ret;
}

char *clax_mktmpdir_alloc()
{
#ifdef _WIN32
    char *template = "clax.tmpdir.XXXXXX";
#else
    char *template = "/tmp/clax.tmpdir.XXXXXX";

#endif

    char *tmpdir = clax_randstr_template_alloc(template);

    if (clax_mkdir(tmpdir, 0755) == 0)
        return tmpdir;

    return NULL;
}

char *clax_mktmpfile_alloc(const char *tmpdir, const char *template)
{
    const char *tmpdir_ = tmpdir && strlen(tmpdir) ? tmpdir : ".";
    const char *template_ = template && strlen(template) ? template : "XXXXXXXX";
    char *filename = clax_randstr_template_alloc(template_);

    char *fpath = malloc(strlen(tmpdir_) + 1 + strlen(filename) + 1);
    strcpy(fpath, tmpdir_);
    strcat(fpath, "/");
    strcat(fpath, filename);

    free(filename);

    return fpath;
}

char *clax_randstr_template_alloc(const char *template)
{
    size_t rand_len = 0;

    const char *p = template;
    while (*p++) {
        if (*p == 'X')
            rand_len++;
    }

    char *rand_str = clax_randstr_alloc(rand_len);
    char *result = clax_strdup(template);

    int i = 0;
    char *r = result;
    while (*r++) {
        if (*r == 'X')
            *r = rand_str[i++];
    }

    free(rand_str);

    return result;
}

char *clax_randstr_alloc(size_t len)
{
    char *p = malloc(len + 1);

    char alphabet[] = "0123456789"
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; i++) {
        p[i] = alphabet[rand() % (sizeof(alphabet) - 1)];
    }

    p[len] = 0;

    return p;
}

unsigned long clax_htol(char *buf)
{
    unsigned long int res = 0;

    char *p = buf;
    while (*p) {
        if (*p >= '0' && *p <= '9') {
            res += *p - '0';
        }
        else if (*p >= 'a' && *p <= 'f') {
            res += *p - 'a' + 10;
        }
        else {
            return -1;
        }

        p++;

        if (*p)
            res <<= 4;
    }

    return res;
}

char *clax_strdup(const char *str)
{
    size_t len = strlen(str);

    char *p = malloc(len + 1);
    strncpy(p, str, len);
    p[len] = 0;
    return p;
}

char *clax_strndup(const char *str, size_t max_len)
{
    size_t len = MIN(strlen(str), max_len);

    char *p = malloc(len + 1);
    strncpy(p, str, len);
    p[len] = 0;
    return p;
}

static iconv_t iconv_e_to_a_cd;
static iconv_t iconv_a_to_e_cd;

size_t clax_iconv(iconv_t cd, char *from, size_t from_len, char *to, size_t to_len)
{
    size_t from_left = from_len;
    size_t to_left   = to_len;

    iconv(cd, &from, &from_left, &to, &to_left);

    return to_len - to_left;
}

size_t clax_ebcdic_to_ascii(char *from, size_t from_len, char *to, size_t to_len)
{
    return clax_iconv(iconv_e_to_a_cd, from, from_len, to, to_len);
}

size_t clax_ascii_to_ebcdic(char *from, size_t from_len, char *to, size_t to_len)
{
    return clax_iconv(iconv_a_to_e_cd, from, from_len, to, to_len);
}

void clax_iconv_open()
{
    /* In iconv it's (_to_, _from_) */
    iconv_e_to_a_cd = iconv_open("ISO8859-1", "IBM-1047");
    iconv_a_to_e_cd = iconv_open("IBM-1047", "ISO8859-1");
}

void clax_iconv_close()
{
    iconv_close(iconv_e_to_a_cd);
    iconv_close(iconv_a_to_e_cd);
}
