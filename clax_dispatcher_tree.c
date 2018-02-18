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

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <libgen.h> /* basename */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h> /* stat */
#include <unistd.h>
#include <errno.h>
#include <dirent.h> /* DIR */

#ifdef _WIN32
# include <windows.h>
#endif

#include "clax_http.h"
#include "clax_log.h"
#include "clax_crc32.h"
#include "clax_dispatcher.h"
#include "clax_dispatcher_tree.h"
#include "clax_util.h"

typedef struct {
    uv_fs_t req;
    void *data;
} delete_req_t;

char *clax_last_modified(uv_stat_t *stat, char *buf, size_t max_len)
{
    struct tm last_modified_time;
    struct tm *last_modified_time_p;

    long epoch = stat->st_mtim.tv_sec;
    time_t time = epoch;

#ifdef _WIN32
    gmtime_s(&last_modified_time, &time);

    last_modified_time_p = &last_modified_time;
#else
    last_modified_time_p = gmtime_r(&time, &last_modified_time);
#endif

    strftime(buf, max_len, "%a, %d %b %Y %H:%M:%S GMT", last_modified_time_p);

    return buf;
}

char *clax_file_size(const char *path, char *buf, size_t max_len)
{
    FILE *fh = fopen(path, "rb");
    size_t size;

    if (fh == NULL) {
        return NULL;
    }

    fseek(fh, 0L, SEEK_END);
    size = ftell(fh);
    fseek(fh, 0L, SEEK_SET);

    fclose(fh);

    snprintf(buf, max_len, "%d", (int)size);

    return buf;
}

void clax_dispatch_tree(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    if (req->method == HTTP_HEAD || req->method == HTTP_GET) {
        clax_dispatch_download(clax_ctx, req, res);
    } else if (req->method == HTTP_POST) {
        clax_dispatch_upload(clax_ctx, req, res);
    } else if (req->method == HTTP_DELETE) {
        clax_dispatch_delete(clax_ctx, req, res);
    } else {
        clax_dispatch_method_not_allowed(clax_ctx, req, res);
    }
}

typedef struct {
    uv_fs_t req;
    uv_buf_t buf;
    void *data;
    char last_modified[30];
    uv_file handle;
    char *path;
    unsigned long crc32;
    uv_fs_cb cb;
} file_req_t;

void clax_uv_crc32_(uv_fs_t *req)
{
    file_req_t *file_req = (file_req_t *)req;

    if (req->fs_type == UV_FS_OPEN) {
        if (req->result > 0) {
            clax_log("File opened successfully");

            file_req->handle = req->result;

            int r = uv_fs_read(uv_default_loop(), (uv_fs_t *)file_req,
                    file_req->handle, &file_req->buf, 1, -1, clax_uv_crc32_);

            if (r < 0) {
                clax_log("File read error: %s", uv_strerror(r));

                req->result = -1;
                file_req->cb(req);

                return;
            }
        }
        else {
            clax_log("Cannot open file");

            req->result = -1;
            file_req->cb(req);
        }
    }
    else if (req->fs_type == UV_FS_READ) {
        if (req->result == 0) {
            clax_log("Finished reading file");

            file_req->crc32 = clax_crc32_finalize(file_req->crc32);

            uv_fs_t close_req;
            uv_fs_close(uv_default_loop(), &close_req, file_req->handle, NULL);

            file_req->cb(req);
        }
        else if (req->result > 0) {
            file_req->crc32 = clax_crc32_calc(file_req->crc32,
                    (unsigned char *)file_req->buf.base, req->result);

            int r = uv_fs_read(uv_default_loop(), (uv_fs_t *)file_req,
                    file_req->handle, &file_req->buf, 1, -1, clax_uv_crc32_);

            if (r < 0) {
                clax_log("File read error: %s", uv_strerror(r));

                req->result = -1;
                file_req->cb(req);

                return;
            }
        }
        else {
            clax_log("File read error: %s", uv_strerror(req->result));

            req->result = -1;
            file_req->cb(req);
        }
    }
}

int clax_uv_crc32(file_req_t *file_req, const char *path, uv_fs_cb cb)
{
    uv_fs_t *req = (uv_fs_t *)file_req;
    file_req->cb = cb;

    file_req->buf.base = malloc(1024);
    file_req->buf.len = 1024;

    file_req->path = (char *)path;
    file_req->crc32 = clax_crc32_init();

    int r = uv_fs_open(uv_default_loop(), (uv_fs_t *)file_req, path, O_RDONLY, 0, clax_uv_crc32_);

    if (r < 0) {
        clax_log("File read error: %s", uv_strerror(r));

        req->result = -1;
        cb(req);

        return - 1;
    }

    return 0;
}

void clax_dispatch_download_crc32_(uv_fs_t *req)
{
    file_req_t *file_req = (file_req_t *)req;
    clax_ctx_t *clax_ctx = file_req->data;

    if (req->result >= 0) {
        char crc32_hex[9] = {0};
        snprintf(crc32_hex, sizeof(crc32_hex), "%08lx", file_req->crc32);

        clax_http_response_status(clax_ctx, &clax_ctx->response, 200);
        clax_http_response_header(clax_ctx, &clax_ctx->response, "Last-Modified", file_req->last_modified);
        clax_http_response_header(clax_ctx, &clax_ctx->response, "Content-Type", "application/octet-stream");
        clax_http_response_header(clax_ctx, &clax_ctx->response, "Pragma", "no-cache");
        clax_http_response_header(clax_ctx, &clax_ctx->response, "X-Clax-CRC32", crc32_hex);

        uv_fs_req_cleanup(req);
        free(file_req->buf.base);
        free(file_req);

        clax_http_dispatch_done_cb(clax_ctx, &clax_ctx->request, &clax_ctx->response);
    }
    else {
        clax_log("Error: crc32 failed");

        uv_fs_req_cleanup(req);
        free(file_req->buf.base);
        free(file_req);

        clax_dispatch_system_error(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);
    }
}

void clax_dispatch_download_(uv_fs_t *req)
{
    file_req_t *file_req = (file_req_t *)req;
    clax_ctx_t *clax_ctx = file_req->data;

    if (req->fs_type == UV_FS_STAT) {
        if (req->result == 0) {
            if (S_ISREG(req->statbuf.st_mode)) {
                clax_last_modified(&req->statbuf, file_req->last_modified, 30);
                clax_ctx->response.body_len = req->statbuf.st_size;

                int r = uv_fs_open(uv_default_loop(), (uv_fs_t *)file_req, req->path, O_RDONLY, 0, clax_dispatch_download_);

                if (r < 0) {
                    clax_log("Error: file open failed: %s", uv_strerror(r));

                    uv_fs_req_cleanup(req);
                    free(file_req);

                    clax_dispatch_system_error(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);
                }
            }
            else if (S_ISDIR(req->statbuf.st_mode)) {
                clax_http_response_status(clax_ctx, &clax_ctx->response, 200);
                clax_http_response_header(clax_ctx, &clax_ctx->response, "Content-Type", "application/vnd.clax.folder");
                clax_http_dispatch_done_cb(clax_ctx, &clax_ctx->request, &clax_ctx->response);
            }
            else {
                clax_log("File '%s' is not a regular file", req->path);

                uv_fs_req_cleanup(req);
                free(file_req);

                clax_dispatch_bad_request(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);
            }
        }
        else {
            clax_log("File '%s' not found", req->path);

            uv_fs_req_cleanup(req);
            free(file_req);

            clax_dispatch_not_found(clax_ctx, &clax_ctx->request, &clax_ctx->response);
        }
    }
    else if (req->fs_type == UV_FS_OPEN) {
        if (req->result > 0) {
            clax_log("File opened successfully");

            clax_ctx->response.body_handle = req->result;

            int r = clax_uv_crc32(file_req, req->path, clax_dispatch_download_crc32_);

            if (r < 0) {
                clax_log("Error: crc32 failed");

                uv_fs_req_cleanup(req);
                free(file_req->buf.base);
                free(file_req);

                clax_dispatch_system_error(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);

                return;
            }
        }
        else {
            clax_log("Cannot open file");

            clax_dispatch_system_error(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);

            uv_fs_req_cleanup(req);
            free(file_req);
        }
    }
    else {
        uv_fs_req_cleanup(req);
        free(file_req);

        clax_dispatch_system_error(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);
    }
}

void clax_dispatch_download(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    char *path = req->path_info + strlen("/tree/");

    clax_log("Serving file '%s'", path);

    file_req_t *file_req = malloc(sizeof(file_req_t));

    if (file_req == NULL) {
        clax_dispatch_system_error(clax_ctx, req, res, NULL);
        return;
    }

    file_req->data = clax_ctx;

    int r = uv_fs_stat(uv_default_loop(), (uv_fs_t *)file_req, path, clax_dispatch_download_);

    if (r < 0) {
        clax_log("Error: stat failed: %s", uv_strerror(r));

        free(file_req);
    }
}

typedef struct {
    uv_fs_t req;
    void *data;
    char *path;
} mkdir_req_t;

typedef struct {
    uv_fs_t req;
    char *path;
    void *data;
} copy_req_t;

void clax_dispatch_upload_crc32_invalid_(uv_fs_t *req)
{
    delete_req_t *delete_req = (delete_req_t*)req;
    clax_ctx_t *clax_ctx = delete_req->data;

    if (req->result == 0 || req->result == UV_ENOENT) {
        clax_log("File deleted");

        clax_http_response_status(clax_ctx, &clax_ctx->response, 400);

        clax_http_dispatch_done_cb(clax_ctx, &clax_ctx->request, &clax_ctx->response);
    }
    else {
        clax_log("Error: file delete failed: %s", uv_strerror(req->result));

        clax_dispatch_system_error(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);
    }

    uv_fs_req_cleanup(req);
    free(delete_req);
}

void clax_dispatch_upload_crc32_(uv_fs_t *req)
{
    file_req_t *file_req = (file_req_t*)req;
    clax_ctx_t *clax_ctx = file_req->data;

    if (req->result >= 0) {
        char *exp_hex = clax_kv_list_find(&clax_ctx->request.query_params, "crc");
        long long int exp = strtoll(exp_hex, NULL, 16);

        clax_log("Checking crc32 of '%s': got=%08lx (%d) exp=%s (%d)",
                file_req->path, file_req->crc32, file_req->crc32, exp_hex, exp);

        if (exp != file_req->crc32) {
            clax_log("Error: crc32 is invalid, removing file");

            delete_req_t *delete_req = malloc(sizeof(delete_req_t));
            delete_req->data = clax_ctx;

            int r = uv_fs_unlink(uv_default_loop(), (uv_fs_t *)delete_req, file_req->path, clax_dispatch_upload_crc32_invalid_);

            if (r < 0) {
                free(delete_req);

                clax_log("Error: removing file failed: %s", uv_strerror(r));

                clax_dispatch_system_error(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);
            }
        }
        else {
            clax_log("CRC32 check successful");

            clax_http_response_status(clax_ctx, &clax_ctx->response, 204);

            clax_http_dispatch_done_cb(clax_ctx, &clax_ctx->request, &clax_ctx->response);
        }
    }
    else {
        clax_log("Error: crc32 failed");

        clax_dispatch_system_error(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);
    }
}

void clax_dispatch_upload_(uv_fs_t *req)
{
    copy_req_t *copy_req = (copy_req_t*)req;
    clax_ctx_t *clax_ctx = copy_req->data;

    if (req->result == 0) {
        clax_log("File copy succeeded");

        char *time = clax_kv_list_find(&clax_ctx->request.query_params, "time");
        if (time) {
            uv_fs_t utime_req;
            double time_d = (double)atoi(time);
            int r = uv_fs_utime(uv_default_loop(), &utime_req, copy_req->path, time_d, time_d, NULL);

            if (r < 0) {
                clax_log("Error (ignored): setting atime/utime failed: %s", uv_strerror(r));
            }
        }

        char *crc32 = clax_kv_list_find(&clax_ctx->request.query_params, "crc");

        if (crc32) {
            clax_log("Checking crc32");

            file_req_t *file_req = malloc(sizeof(file_req_t));
            file_req->data = clax_ctx;

            int r = clax_uv_crc32(file_req, copy_req->path, clax_dispatch_upload_crc32_);

            if (r < 0) {
                clax_log("Error: crc32 failed");
            }
        }
        else {
            clax_http_response_status(clax_ctx, &clax_ctx->response, 204);

            clax_http_dispatch_done_cb(clax_ctx, &clax_ctx->request, &clax_ctx->response);
        }
    }
    else {
        clax_log("Error: file copy failed: %s", uv_strerror(req->result));

        clax_dispatch_system_error(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);
    }

    uv_fs_req_cleanup(req);
    free(copy_req);
}

void clax_dispatch_upload_mkdir(uv_fs_t *req)
{
    mkdir_req_t *mkdir_req = (mkdir_req_t *)req;
    clax_ctx_t *clax_ctx = mkdir_req->data;
    char *tmpfile = clax_ctx->request.body_tmpfile;

    if (!tmpfile || !strlen(tmpfile)) {
        uv_fs_req_cleanup(req);
        free(mkdir_req);

        clax_dispatch_bad_request(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);

        return;
    }

    clax_log("Saving upload '%s' to file '%s'", tmpfile, mkdir_req->path);

    copy_req_t *copy_req = malloc(sizeof(copy_req_t));
    copy_req->path = mkdir_req->path;
    copy_req->data = clax_ctx;

    int r = uv_fs_copyfile(uv_default_loop(), (uv_fs_t *)copy_req,
            (char *)tmpfile, mkdir_req->path, 0, clax_dispatch_upload_);

    if (r < 0) {
        clax_log("Error: copy failed: %s", uv_strerror(r));

        clax_dispatch_system_error(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);
        return;
    }

    uv_fs_req_cleanup(req);
    free(mkdir_req);
}

void clax_dispatch_upload(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    char *path = req->path_info + strlen("/tree/");

    if (req->continue_expected) {
        clax_dispatch_continue(clax_ctx, req, res);
        return;
    }

    const char *content_type = clax_http_request_header(clax_ctx, req, "Content-Type");
    if (content_type && strcmp(content_type, "application/vnd.clax.folder") == 0) {
        clax_log("Creating directory");

        int r = clax_mkdir_p(path);

        if (r < 0) {
            clax_log("Error: mkdirp failed");

            clax_dispatch_system_error(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);

            return;
        }

        clax_http_response_status(clax_ctx, &clax_ctx->response, 204);

        clax_http_dispatch_done_cb(clax_ctx, &clax_ctx->request, &clax_ctx->response);

        return;
    }

    char *pathdup = clax_strdup((const char *)path);
    char *dir = dirname(pathdup);

    clax_log("Creating intermediate path '%s'", dir);

    int r = clax_mkdir_p(dir);

    free(pathdup);

    if (r < 0) {
        clax_log("Error: mkdirp failed");

        clax_dispatch_system_error(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);

        return;
    }

    char *tmpfile = clax_ctx->request.body_tmpfile;

    if (tmpfile && strlen(tmpfile)) {
        clax_log("Saving upload '%s' to file '%s'", tmpfile, path);

        copy_req_t *copy_req = malloc(sizeof(copy_req_t));
        copy_req->path = path;
        copy_req->data = clax_ctx;

        int rcopy = uv_fs_copyfile(uv_default_loop(), (uv_fs_t *)copy_req,
                (char *)tmpfile, path, 0, clax_dispatch_upload_);

        if (rcopy < 0) {
            clax_log("Error: copy failed: %s", uv_strerror(r));

            clax_dispatch_system_error(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);
            return;
        }
    }

    /* Empty file */
    else {
        uv_fs_t open_req;

        int r = uv_fs_open(uv_default_loop(), &open_req,
                path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, NULL);

        if (r < 0) {
            clax_log("Error creating body file: %s", uv_strerror(r));

            return;
        }

        uv_fs_t close_req;

        uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);

        copy_req_t *copy_req = calloc(1, sizeof(copy_req_t));
        copy_req->path = path;
        copy_req->data = clax_ctx;
        ((uv_fs_t *)copy_req)->result = 0;

        clax_dispatch_upload_((uv_fs_t *)copy_req);
    }
}

void clax_dispatch_delete_(uv_fs_t *req)
{
    delete_req_t *delete_req = (delete_req_t*)req;
    clax_ctx_t *clax_ctx = delete_req->data;

    if (req->result == 0) {
        clax_log("File deleted succeeded");

        clax_http_response_status(clax_ctx, &clax_ctx->response, 204);

        clax_http_dispatch_done_cb(clax_ctx, &clax_ctx->request, &clax_ctx->response);
    }
    else {
        clax_log("Error: file delete failed: %s", uv_strerror(req->result));

        if (req->result == UV_ENOENT) {
            clax_dispatch_not_found(clax_ctx, &clax_ctx->request, &clax_ctx->response);
        }
        else {
            clax_dispatch_system_error(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);
        }
    }

    uv_fs_req_cleanup(req);
    free(delete_req);
}

void clax_dispatch_delete(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    char *path = req->path_info + strlen("/tree/");

    delete_req_t *delete_req = malloc(sizeof(delete_req_t));
    delete_req->data = clax_ctx;

    int r = uv_fs_unlink(uv_default_loop(), (uv_fs_t *)delete_req, path, clax_dispatch_delete_);

    if (r < 0) {
        free(delete_req);

        clax_dispatch_system_error(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);
    }
}
