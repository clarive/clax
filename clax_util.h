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

#ifndef CLAX_UTIL_H
#define CLAX_UTIL_H

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

#define sizeof_array(array) sizeof(array) / sizeof(array[0])
#define sizeof_struct_member(type, member) sizeof(((type *)0)->member)

#define TRY if ((
#define GOTO ) < 0) {goto error;}

#define MIN(a,b) ((a) < (b) ? (a) : (b))

typedef struct {
    char *key;
    char *val;
} clax_kv_list_item_t;

typedef struct {
    clax_kv_list_item_t **items;
    size_t size;
} clax_kv_list_t;

void clax_kv_list_init(clax_kv_list_t *list);
void clax_kv_list_free(clax_kv_list_t *list);
int clax_kv_list_push(clax_kv_list_t *list, char *key, char *val);
int clax_kv_list_pushn(clax_kv_list_t *list, char *key, size_t key_len, char *val, size_t val_len);
char *clax_kv_list_find_all(clax_kv_list_t *list, char *key, size_t *start);
char *clax_kv_list_find(clax_kv_list_t *list, char *key);
clax_kv_list_item_t *clax_kv_list_find_item(clax_kv_list_t *list, char *key);
clax_kv_list_item_t *clax_kv_list_at(clax_kv_list_t *list, size_t index);
clax_kv_list_item_t *clax_kv_list_next(clax_kv_list_t *list, size_t *start);
int clax_kv_list_set(clax_kv_list_t *list, char *key, char *val);
void clax_kv_list_dump(clax_kv_list_t *list);

char *clax_buf2str(const char *buf, size_t len);
void clax_str_append(char **dst, const char *src);
void clax_buf_append(unsigned char **dst, size_t *dst_len, const char *src, size_t src_len);
void clax_hexdump(unsigned char *buf, size_t len);

void clax_san_path(char *buf);
char *clax_strjoin(char *sep, ...);

int clax_mkdir(char *dirname, int mode);
char *clax_mktmpdir_alloc();
char *clax_mktmpfile_alloc(const char *tmpdir, const char *template);

unsigned long int clax_htol(char *buf);

char *clax_randstr_alloc(size_t len);
char *clax_randstr_template_alloc(const char *template);
char *clax_strdup(const char *str);
char *clax_strndup(const char *str, size_t max);

#ifdef MVS
size_t clax_etoa(char *from, size_t from_len);
char *clax_etoa_alloc(char *from, size_t from_len);
size_t clax_atoe(char *from, size_t from_len);
char *clax_atoe_alloc(char *from, size_t from_len);
#endif

unsigned char *clax_slurp_alloc(char *filename, size_t *olen);

#endif
