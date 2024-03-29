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

#include "contrib/libuv/include/uv.h"

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
char *clax_strapp_a(char **dst, const char *src);
char *clax_strnapp_a(char **dst, const char *src, size_t src_len);
void clax_buf_append(unsigned char **dst, size_t *dst_len, const unsigned char *src, size_t src_len);
void clax_hexdump(unsigned char *buf, size_t len);

void clax_san_path(char *buf);
char *clax_strjoin(char *sep, ...);

int clax_mkdir(char *dirname, int mode);
char *clax_mktmpdir_alloc();
char *clax_mktmpfile_alloc(const char *tmpdir, const char *tmpl);

unsigned long int clax_htol(char *buf);

char *clax_randstr_alloc(size_t len);
char *clax_randstr_template_alloc(const char *tmpl);
char *clax_strdup(const char *str);
char *clax_strndup(const char *str, size_t max);

#ifdef MVS
size_t clax_etoa(char *from, size_t from_len);
char *clax_etoa_alloc(const char *from, size_t from_len);
size_t clax_atoe(char *from, size_t from_len);
char *clax_atoe_alloc(const char *from, size_t from_len);
#endif

unsigned char *clax_slurp_alloc(char *filename, size_t *olen);
int clax_strcat(char *dst, size_t dst_max_len, const char *src);

int clax_is_path_d(const char *path);
int clax_is_path_f(const char *path);
int clax_mkdir_p(const char *path);
int clax_rmpath_r(const char *path);
int clax_rmdir_r(const char *path);
int clax_touch(const char *path);

const char *clax_strerror(int error);
char *clax_strncatt(char *dst, size_t max_len, char *src);
int clax_chdir(char *path);

char *clax_detect_root(char *root, size_t root_size, char **argv);
char *clax_detect_exe_from_argv(char *root, size_t root_size, char **argv);
char *clax_detect_exe_from_proc(char *root, size_t root_size);

int clax_strcatfile(char *dst, size_t dst_max_len, const char *src);
int clax_strcatdir(char *dst, size_t dst_max_len, const char *src);

char *clax_sprintf_alloc(const char *fmt, ...);

char *clax_str_alloc(size_t len);

int clax_uv_mkdir_p(uv_loop_t *loop, uv_fs_t *req, const char *path, int mode, uv_fs_cb cb);

#endif
