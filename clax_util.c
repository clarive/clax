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
#include <sys/time.h>
#include <time.h>

#ifdef _WIN32
# include <windows.h>
#else
# include <sys/stat.h>
# include <sys/types.h>
#endif

#include "clax_util.h"
#include "clax_platform.h"

static int srand_called = 0;

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

char *clax_strcat_alloc(char *dst, char *src)
{
    char *res;
    size_t src_len = strlen(src);

    if (dst == NULL) {
        res = malloc(src_len + 1);
        strcpy(res, src);
    }
    else {
        size_t dst_len = strlen(dst);

        res = realloc(dst, dst_len + src_len + 1);
        strcat(res, src);
    }

    return res;
}

size_t clax_strcat(char *dst, size_t dst_max_len, char *src)
{
    size_t orig_len = strlen(dst);
    size_t src_len = strlen(src);
    size_t to_copy = src_len;

    if (orig_len + to_copy > dst_max_len - 1) {
        to_copy = dst_max_len - orig_len - 1;
    }

    if (to_copy > 0) {
        strncat(dst, src, to_copy);
        dst[orig_len + to_copy] = 0;

        return to_copy;
    }
    else {
        return 0;
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

    if (!srand_called) {
        struct timeval t1;
        gettimeofday(&t1, NULL);
        srand(t1.tv_usec * t1.tv_sec);

        srand_called++;
    }

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
    if (str == NULL) {
        return NULL;
    }

    size_t len = strlen(str);

    char *p = malloc(len + 1);

    if (p == NULL) {
        return NULL;
    }

    strncpy(p, str, len);
    p[len] = 0;

    return p;
}

char *clax_strndup(const char *str, size_t max_len)
{
    if (str == NULL) {
        return NULL;
    }

    size_t len = MIN(strlen(str), max_len);

    char *p = malloc(len + 1);

    if (p == NULL) {
        return NULL;
    }

    strncpy(p, str, len);
    p[len] = 0;

    return p;
}

#ifdef MVS
size_t clax_etoa(char *from, size_t from_len)
{
    return __etoa_l(from, from_len);
}

char *clax_etoa_alloc(char *from, size_t from_len)
{
    char *b = malloc(from_len);
    memcpy(b, from, from_len);
    __etoa_l(b, from_len);

    return b;
}

size_t clax_atoe(char *from, size_t from_len)
{
    return __atoe_l(from, from_len);
}

char *clax_atoe_alloc(char *from, size_t from_len)
{
    char *b = malloc(from_len);
    memcpy(b, from, from_len);
    __atoe_l(b, from_len);

    return b;
}
#endif

unsigned char *clax_slurp_alloc(char *filename, size_t *olen)
{
    char buf[1024];
    unsigned char *slurp = NULL;
    FILE *fh = fopen(filename, "rb");
    *olen = 0;

    if (fh == NULL)
        return NULL;

    size_t rcount = 0;
    while ((rcount = fread(buf, 1, sizeof(buf), fh)) > 0) {
        slurp = realloc(slurp, *olen + rcount + 1);

        memcpy(slurp + *olen, buf, rcount);

        *olen += rcount;

        slurp[*olen] = 0;
    }

    (*olen)++;

    fclose(fh);

    return slurp;
}
