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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <libgen.h> /* basename */
#include <sys/stat.h> /* stat */
#include <time.h>
#include <utime.h>
#include <fcntl.h>

#include "clax.h"
#include "clax_http.h"
#include "clax_log.h"
#include "clax_command.h"
#include "clax_big_buf.h"
#include "clax_util.h"
#include "clax_crc32.h"
#include "clax_dispatcher.h"

static command_ctx_t command_ctx;

enum {
    CLAX_DISPATCHER_NO_FLAGS = 0,
    CLAX_DISPATCHER_FLAG_100_CONTINUE
};

typedef struct {
    char *path;
    int method_mask;
    int flags_mask;
    void (*fn)(clax_ctx_t *, clax_http_request_t *, clax_http_response_t *);
} clax_dispatcher_action_t;

void clax_command_read_cb(void *ctx, clax_http_chunk_cb_t chunk_cb, ...)
{
    command_ctx_t *command_ctx = ctx;
    va_list a_list;
    char buf[16];

    va_start(a_list, chunk_cb);

    int exit_code = clax_command_read(command_ctx, chunk_cb, a_list);

    /* Chunked trailing header */
    snprintf(buf, sizeof(buf), "X-Clax-Exit: %d", exit_code);
    chunk_cb(buf, 0, a_list);

    va_end(a_list);

    return;
}

void clax_dispatch_not_found(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
    res->status_code = 404;
    clax_big_buf_append_str(&res->body, "Not found");
}

void clax_dispatch_method_not_allowed(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
    res->status_code = 405;
    clax_big_buf_append_str(&res->body, "Method not allowed");
}

void clax_dispatch_system_error(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
    res->status_code = 500;
    clax_big_buf_append_str(&res->body, "System error");
}

void clax_dispatch_bad_request(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    res->status_code = 400;
    clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
    clax_big_buf_append_str(&res->body, "Bad request");
}

void clax_dispatch_ping(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    res->status_code = 200;
    clax_kv_list_push(&res->headers, "Content-Type", "application/json");
    clax_big_buf_append_str(&res->body, "{\"message\":\"pong\"}");
}

void clax_dispatch_index(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    res->status_code = 200;
    clax_kv_list_push(&res->headers, "Content-Type", "application/json");
    clax_big_buf_append_str(&res->body, "{\"message\":\"Hello, world!\"}");
}

void clax_dispatch_command(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    memset(&command_ctx, 0, sizeof(command_ctx_t));

    char *command = clax_kv_list_find(&req->body_params, "command");
    if (!command || !strlen(command)) {
        clax_dispatch_bad_request(clax_ctx, req, res);
        return;
    }

    strncpy(command_ctx.command, command, sizeof_struct_member(command_ctx_t, command));

    char *timeout = clax_kv_list_find(&req->body_params, "timeout");
    if (timeout && strlen(timeout)) {
        command_ctx.timeout = atoi(timeout);
    }

    pid_t pid = clax_command_start(&command_ctx);
    if (pid <= 0) {
        clax_dispatch_system_error(clax_ctx, req, res);
        return;
    }

    res->status_code = 200;

    clax_kv_list_push(&res->headers, "Transfer-Encoding", "chunked");

    char buf_pid[15];
    snprintf(buf_pid, sizeof(buf_pid), "%d", pid);

    clax_kv_list_push(&res->headers, "Trailer", "X-Clax-Exit");
    clax_kv_list_push(&res->headers, "X-Clax-PID", buf_pid);

    res->body_cb_ctx = &command_ctx;
    res->body_cb = clax_command_read_cb;
}

void clax_dispatch_download(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    char *file = req->path_info + strlen("/tree/");

    char buf[255];
    char base_buf[255];
    char last_modified_buf[30];
    struct stat st;
    struct tm last_modified_time;

    if (stat(file, &st) != 0 || !(st.st_mode & S_IFREG)) {
        clax_dispatch_not_found(clax_ctx, req, res);
        return;
    }

    FILE *fh = fopen(file, "rb");

    if (fh == NULL) {
        clax_dispatch_not_found(clax_ctx, req, res);
        return;
    }

    snprintf(buf, sizeof(buf), "%d", (int)st.st_size);

    char *base = basename(file);
    strcpy(base_buf, "attachment; filename=\"");
    strcat(base_buf, base);
    strcat(base_buf, "\"");

    res->status_code = 200;
    clax_kv_list_push(&res->headers, "Content-Type", "application/octet-stream");
    clax_kv_list_push(&res->headers, "Content-Disposition", base_buf);
    clax_kv_list_push(&res->headers, "Pragma", "no-cache");
    clax_kv_list_push(&res->headers, "Content-Length", buf);

    gmtime_r(&st.st_mtime, &last_modified_time);
    strftime(last_modified_buf, sizeof(last_modified_buf), "%a, %d %b %Y %T GMT", &last_modified_time);
    clax_kv_list_push(&res->headers, "Last-Modified", last_modified_buf);

    if (req->method == HTTP_GET)
        res->body_fh = fh;
    else
        fclose(fh);
}

void clax_dispatch_upload(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    char *subdir = req->path_info + strlen("/tree/");

    if (strlen(subdir)) {
        struct stat info;

        if (stat(subdir, &info) == 0 && info.st_mode & S_IFDIR) {
        } else {
            clax_log("Output directory does not exist");

            clax_dispatch_bad_request(clax_ctx, req, res);
            return;
        }
    }

    if (req->continue_expected) {
        return;
    }

    if (strlen(req->multipart_boundary)) {
        int i;
        for (i = 0; i < req->multiparts.size; i++) {
            clax_http_multipart_t *multipart = clax_http_multipart_list_at(&req->multiparts, i);

            const char *content_disposition = clax_kv_list_find(&multipart->headers, "Content-Disposition");
            if (!content_disposition)
                continue;

            char prefix[] = "form-data; ";
            if (strncmp(content_disposition, prefix, strlen(prefix)) != 0)
                continue;

            const char *kv = content_disposition + strlen(prefix);
            size_t name_len;
            size_t filename_len;
            const char *name = clax_http_extract_kv(kv, "name", &name_len);
            const char *filename = clax_http_extract_kv(kv, "filename", &filename_len);

            if (!name || !filename || (strncmp(name, "file", name_len) != 0))
                continue;

            char *new_name = clax_kv_list_find(&req->query_params, "name");
            char *crc32 = clax_kv_list_find(&req->query_params, "crc");
            char *time = clax_kv_list_find(&req->query_params, "time");

            if (crc32 && strlen(crc32) != 8) {
                clax_dispatch_bad_request(clax_ctx, req, res);
                return;
            }

            char *fpath;

            if (new_name && strlen(new_name)) {
                fpath = clax_strjoin("/", clax_ctx->options->root, subdir, new_name, NULL);
            }
            else {
                char *p = strndup(filename, filename_len);
                fpath = clax_strjoin("/", clax_ctx->options->root, subdir, p, NULL);
                free(p);
            }

            clax_san_path(fpath);

            int ret = clax_big_buf_write_file(&multipart->bbuf, fpath);

            if (ret < 0) {
                clax_dispatch_system_error(clax_ctx, req, res);
            }
            else {
                if (crc32 && strlen(crc32)) {
                    unsigned long got_crc32 = strtol(crc32, NULL, 16);

                    int fd = open(fpath, O_RDONLY);
                    unsigned long real_crc32 = clax_crc32_calc_fd(fd);
                    close(fd);

                    if (got_crc32 != real_crc32) {
                        clax_log("CRC mismatch %d != %d", got_crc32, real_crc32);
                        clax_dispatch_bad_request(clax_ctx, req, res);

                        unlink(fpath);
                        free(fpath);

                        return;
                    } else {
                        clax_log("CRC ok");
                    }
                }

                if (time && strlen(time)) {
                    int mtime = atol(time);

#ifdef _WIN32
#else
                    struct utimbuf t;
                    t.actime = mtime;
                    t.modtime = mtime;
                    utime(fpath, &t);
#endif
                }

                res->status_code = 200;
                clax_kv_list_push(&res->headers, "Content-Type", "application/json");

                clax_big_buf_append_str(&res->body, "{\"message\":\"ok\"}");
            }

            free(fpath);

            break;
        }
    }

    if (!res->status_code) {
        clax_dispatch_bad_request(clax_ctx, req, res);
    }
}

void clax_dispatch_tree(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    if (req->method == HTTP_HEAD || req->method == HTTP_GET) {
        clax_dispatch_download(clax_ctx, req, res);
    } else if (req->method == HTTP_POST) {
        clax_dispatch_upload(clax_ctx, req, res);
    } else {
        clax_dispatch_not_found(clax_ctx, req, res);
    }
}

clax_dispatcher_action_t clax_dispatcher_actions[] = {
    {"/", (1 << HTTP_GET), CLAX_DISPATCHER_NO_FLAGS, clax_dispatch_index},
    {"/ping", (1 << HTTP_GET), CLAX_DISPATCHER_NO_FLAGS, clax_dispatch_ping},
    {"/command", (1 << HTTP_POST), CLAX_DISPATCHER_NO_FLAGS, clax_dispatch_command},
    {"^/tree/", (1 << HTTP_HEAD | 1 << HTTP_GET | 1 << HTTP_POST), CLAX_DISPATCHER_FLAG_100_CONTINUE, clax_dispatch_tree},
};

size_t clax_dispatcher_match(const char *path_info, size_t path_info_len, const char *path, size_t path_len)
{
    int match;

    if (path[0] == '^') {
        path++;
        path_len--;

        match = path_info_len >= path_len && strncmp(path_info, path, path_len) == 0;
    }
    else {
        match = strcmp(path_info, path) == 0;
    }

    return match ? path_len : 0;
}

void clax_dispatch(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    char *path_info = req->path_info;
    size_t path_info_len = strlen(path_info);

    size_t len = sizeof clax_dispatcher_actions / sizeof clax_dispatcher_actions[0];

    for (int i = 0; i < len; i++) {
        clax_dispatcher_action_t *action = &clax_dispatcher_actions[i];

        size_t match = clax_dispatcher_match(path_info, path_info_len, action->path, strlen(action->path));

        if (match) {
            if (action->method_mask & (1 << req->method)) {
                if (req->continue_expected) {
                    if (action->flags_mask & (1 << CLAX_DISPATCHER_FLAG_100_CONTINUE)) {
                        action->fn(clax_ctx, req, res);
                    }
                    else {
                        return;
                    }
                } else {
                    action->fn(clax_ctx, req, res);
                }

                return;
            } else {
                clax_dispatch_method_not_allowed(clax_ctx, req, res);

                return;
            }
        }
    }

    clax_log("No suitable action matched");

    clax_dispatch_not_found(clax_ctx, req, res);
    return;
}
