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
#include <sys/stat.h> /* stat */
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <libgen.h> /* dirname */

#ifdef _WIN32
# include <windows.h>
#else
# include <sys/types.h>
#endif

#include "contrib/libuv/include/uv.h"

#include "clax_log.h"
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

void clax_buf_append(unsigned char **dst, size_t *dst_len, const unsigned char *src, size_t src_len)
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

char *clax_strapp_a(char **dst, const char *src)
{
    return clax_strnapp_a(dst, src, strlen(src));
}

char *clax_strnapp_a(char **dst, const char *src, size_t src_len)
{
    if (dst == NULL || src == NULL) {
        return NULL;
    }

    if (*dst == NULL) {
        *dst = malloc(src_len + 1);

        if (*dst) {
            memcpy((void *)*dst, (const void *)src, src_len);
            (*dst)[src_len] = 0;
        }
    }
    else {
        size_t dst_len = strlen(*dst);

        *dst = realloc((void *)*dst, dst_len + src_len + 1);

        if (*dst) {
            memcpy((void *)(*dst + dst_len), (const void *)src, src_len);
            (*dst)[dst_len + src_len] = 0;
        }
    }

    return *dst;
}

int clax_strcat(char *dst, size_t dst_max_len, const char *src)
{
    size_t orig_len = strlen(dst);
    size_t src_len = strlen(src);

    if (src_len == 0) {
        return 0;
    }

    if (orig_len + src_len > dst_max_len - 1) {
        return -1;
    }

    strcat(dst, src);

    return 0;
}

int clax_strcatfile(char *dst, size_t dst_max_len, const char *src)
{
    if (strlen(dst) == 0) {
        TRY clax_strcat(dst, dst_max_len, src) GOTO;
    }
    else {
        if (dst[strlen(dst) - 1] != '/') {
            if (src[0] != '/') {
                TRY clax_strcat(dst, dst_max_len, "/") GOTO;
            }

            TRY clax_strcat(dst, dst_max_len, src) GOTO;
        }
        else {
            if (*src == '/') {
                src++;
            }

            TRY clax_strcat(dst, dst_max_len, src) GOTO;
        }
    }

    return 0;

error:
    errno = ENAMETOOLONG;
    return -1;
}

int clax_strcatdir(char *dst, size_t dst_max_len, const char *src)
{
    if (clax_strcatfile(dst, dst_max_len, src) != 0) {
        return -1;
    }

    if (src[strlen(src) - 1] != '/') {
        return clax_strcatfile(dst, dst_max_len, "/");
    }

    return 0;
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

        if ((p = strstr(buf, "\\")) != NULL) {
            *p = '/';
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

        char *join_new;

        if (first) {
            join_new = realloc(join, strlen(p) + 1);

            if (join) {
                join = join_new;

                strcat(join_new, p);

                first = 0;
            }
            else {
                free(p);
            }
        } else {
            join_new = realloc(join, strlen(join) + sep_len + strlen(p) + 1);

            if (join_new) {
                join = join_new;

                strcat(join, sep);
                strcat(join, p);
            }
            else {
                free(join);
            }
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

char *clax_etoa_alloc(const char *from, size_t from_len)
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

char *clax_atoe_alloc(const char *from, size_t from_len)
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
        char *slurp_new = realloc(slurp, *olen + rcount + 1);

        if (slurp_new) {
            slurp = slurp_new;

            memcpy(slurp + *olen, buf, rcount);

            *olen += rcount;

            slurp[*olen] = 0;
        }
        else {
            free(slurp);
        }
    }

    (*olen)++;

    fclose(fh);

    return slurp;
}

typedef struct {
    uv_fs_t req;
    uv_fs_t *orig_req;
    uv_fs_cb orig_cb;
    const char *orig_path;
    int orig_mode;
    char *prefix;
} clax_uv_mkdir_p_req_t;

int clax_uv_mkdir_p_(uv_loop_t *loop, clax_uv_mkdir_p_req_t *req, const char *root, const char *path, int mode, uv_fs_cb cb);

void clax_uv_mkdir_p_cb(uv_fs_t *req)
{
    clax_uv_mkdir_p_req_t *clax_req = (clax_uv_mkdir_p_req_t *)req;

    if (req->result == 0 || req->result == UV_EEXIST) {
        if (clax_req->prefix) {
            clax_uv_mkdir_p_(req->loop, clax_req, clax_req->orig_path, req->path, clax_req->orig_mode, clax_uv_mkdir_p_cb);
        }
        else {
            if (clax_req->prefix) {
                free(clax_req->prefix);
            }

            uv_fs_req_cleanup(req);
        }
    }
    else {
        if (clax_req->prefix) {
            free(clax_req->prefix);
        }

        uv_fs_req_cleanup(req);
    }
}

int clax_uv_mkdir_p_(uv_loop_t *loop, clax_uv_mkdir_p_req_t *mkdir_req, const char *root, const char *path, int mode, uv_fs_cb cb) {
    char *p = (char *)root;

    if (path) {
        p += strlen(path);
    }

    while (*p == '/')
        p++;
    p = strchr(p, '/');

    if (mkdir_req->prefix) {
        free(mkdir_req->prefix);
        mkdir_req->prefix = NULL;
    }

    if (p) {
        mkdir_req->prefix = malloc(sizeof(char) * (p - root) + 1);
        strncpy(mkdir_req->prefix, root, p - root);
        mkdir_req->prefix[p - root] = 0;

        uv_fs_mkdir(loop, (uv_fs_t *)mkdir_req, mkdir_req->prefix, mode, cb);
    }
    else if (path) {
        uv_fs_mkdir(loop, (uv_fs_t *)mkdir_req, path, mode, mkdir_req->orig_cb);
    }
    else {
        uv_fs_mkdir(loop, mkdir_req->orig_req, root, mode, mkdir_req->orig_cb);
    }

    return 0;
}

int clax_uv_mkdir_p(uv_loop_t *loop, uv_fs_t *req, const char *path, int mode, uv_fs_cb cb)
{
    clax_uv_mkdir_p_req_t *mkdir_req = malloc(sizeof(clax_uv_mkdir_p_req_t));
    if (mkdir_req == NULL) {
        return -1;
    }

    mkdir_req->orig_req = req;
    mkdir_req->orig_path = path;
    mkdir_req->orig_mode = mode;
    mkdir_req->orig_cb = cb;
    mkdir_req->prefix = NULL;

    return clax_uv_mkdir_p_(loop, mkdir_req, path, NULL, mode, clax_uv_mkdir_p_cb);
}

/* Based on https://gist.github.com/JonathonReinhart/8c0d90191c38af2dcadb102c4e202950 */
int clax_mkdir_p(const char *path)
{
    const size_t len = strlen(path);
    char _path[255];
    char *p;

    errno = 0;

    if (len > sizeof(_path) - 1) {
        errno = ENAMETOOLONG;
        return -1;
    }

    strcpy(_path, path);

    for (p = _path + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';

            if (clax_mkdir(_path, 0755) != 0) {
                if (errno != EEXIST)
                    return -1;
            }

            *p = '/';
        }
    }

    if (clax_mkdir(_path, 0755) != 0) {
        if (errno != EEXIST)
            return -1;
    }

    return 0;
}

int clax_is_path_f(const char *path)
{
    struct stat path_stat;

    if (stat(path, &path_stat) == 0) {
        return S_ISREG(path_stat.st_mode);
    }

    return 0;
}

int clax_is_path_d(const char *path)
{
    struct stat path_stat;

    if (stat(path, &path_stat) == 0) {
        return S_ISDIR(path_stat.st_mode);
    }

    return 0;
}

int clax_rmdir(const char *path)
{
    return rmdir(path);
}

int clax_rmdir_r(const char *path)
{
    DIR *dir = opendir(path);

    if (dir == NULL) {
        return -1;
    }

    return clax_rmpath_r(path);
}

int clax_rmpath_r(const char *path)
{
    struct dirent *d;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        if (errno == ENOTDIR) {
            unlink(path);
        }

        return 0;
    }

    while ((d = readdir(dir)) != NULL) {
        if (strcmp(d->d_name, ".") == 0)
            continue;
        if (strcmp(d->d_name, "..") == 0)
            continue;

        char *p = clax_strjoin("/", path, d->d_name, (char *)NULL);
        clax_rmpath_r(p);
        free(p);

        break;
    }

    closedir(dir);

    return clax_rmdir(path);
}

int clax_touch(const char *path)
{
    int fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);

    if (fd) {
        return 0;
    }
    else {
        return -1;
    }
}

char *clax_detect_root(char *root, size_t root_size, char **argv)
{
#ifdef _WIN32

    GetModuleFileName(NULL, root, root_size);

#else

    char *p = clax_detect_exe_from_proc(root, root_size);

    if (p == NULL) {
        clax_detect_exe_from_argv(root, root_size, argv);
    }

#endif

    clax_san_path(root);

    char *dir = dirname(root);
    /*strcpy(root, dir);*/
    memmove(root, dir, strlen(dir));

    if (clax_strcatdir(root, root_size, "/") < 0) {
        return NULL;
    }

    return root;
}

char *clax_detect_exe_from_proc(char *root, size_t root_size)
{
#ifdef __APPLE__

    uint32_t size = root_size;

    if (_NSGetExecutablePath(root, &size) == 0) {
        return root;
    }

#elif defined(__sun) && defined(__SVR4)

    root = getexecname();

#elif !defined(_WIN32)

    if (readlink("/proc/self/exe", root, root_size) != -1) {
        return root;
    }
    else if (readlink("/proc/curproc/exe", root, root_size) != -1) {
        return root;
    }
    else if (readlink("/proc/curproc/file", root, root_size) != -1) {
        return root;
    }

#endif

    return NULL;
}

char *clax_detect_exe_from_argv(char *root, size_t root_size, char **argv)
{
    if (argv && argv[0] && argv[0][0] != '/') {
        getcwd(root, root_size);

        fprintf(stderr, root);
    }

    if (clax_strcatdir(root, root_size, argv[0]) != 0) {
        return NULL;
    }

    return root;
}

int clax_chdir(char *path)
{
    DIR *dir = opendir(path);
    if (dir) {
        closedir(dir);

        if (chdir(path) < 0) {
            return -1;
        }
    } else if (ENOENT == errno) {
        return -1;
    } else {
        return -1;
    }

    return 0;
}

char *clax_sprintf_alloc(const char *fmt, ...)
{
    char *p = NULL;
    va_list ap;

    va_start(ap, fmt);

    int ret = vasprintf(&p, fmt, ap);
    if (ret == -1) {
        p = NULL;
    }

    va_end(ap);

    return p;
}

char *clax_str_alloc(size_t len)
{
    char *p = malloc(len + 1);

    if (p == NULL) {
        return NULL;
    }

    p[0] = 0;

    return p;
}
