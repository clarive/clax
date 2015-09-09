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

command_ctx_t command_ctx;

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
    memcpy(res->body, "Not found", 9);
    res->body_len = 9;
}

void clax_dispatch_system_error(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
    res->status_code = 500;
    memcpy(res->body, "System error", 12);
    res->body_len = 12;
}

void clax_dispatch_bad_request(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    res->status_code = 400;
    clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
    memcpy(res->body, "Bad request", 14);
    res->body_len = 14;
}

void clax_dispatch(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    char *path_info = req->path_info;

    if (strcmp(path_info, "/ping") == 0) {
        res->status_code = 200;
        clax_kv_list_push(&res->headers, "Content-Type", "application/json");
        memcpy(res->body, "{\"message\":\"pong\"}", 20);
        res->body_len = 20;
    }
    else if (req->method == HTTP_POST && strcmp(path_info, "/command") == 0) {
        memset(&command_ctx, 0, sizeof(command_ctx_t));

        int command_found = 0;

        char *command = clax_kv_list_find(&req->body_params, "command");
        if (command && strlen(command)) {
            strncpy(command_ctx.command, command, sizeof_struct_member(command_ctx_t, command));

            command_found = 1;
        }

        char *timeout = clax_kv_list_find(&req->body_params, "timeout");
        if (timeout && strlen(timeout)) {
            command_ctx.timeout = atoi(timeout);
        }

        if (command_found) {
            pid_t pid = clax_command_start(&command_ctx);

            clax_kv_list_push(&res->headers, "Transfer-Encoding", "chunked");

            if (pid > 0) {
                res->status_code = 200;

                char buf_pid[15];
                snprintf(buf_pid, sizeof(buf_pid), "%d", pid);

                clax_kv_list_push(&res->headers, "X-Clax-PID", buf_pid);

                res->body_cb_ctx = &command_ctx;
                res->body_cb = clax_command_read_cb;
            }
            else {
                clax_dispatch_system_error(clax_ctx, req, res);
            }
        } else {
            clax_dispatch_bad_request(clax_ctx, req, res);
        }
    }
    else if (req->method == HTTP_POST && strcmp(path_info, "/upload") == 0) {
        if (strlen(req->multipart_boundary)) {
            int i;
            for (i = 0; i < req->multiparts.size; i++) {
                clax_http_multipart_t *multipart = clax_http_multipart_list_at(&req->multiparts, i);

                const char *content_disposition = clax_kv_list_find(&multipart->headers, "Content-Disposition");
                if (!content_disposition)
                    continue;

                char prefix[] = "form-data; ";
                if (strncmp(content_disposition, prefix, strlen(prefix)) == 0) {
                    const char *kv = content_disposition + strlen(prefix);
                    size_t name_len;
                    size_t filename_len;

                    const char *name = clax_http_extract_kv(kv, "name", &name_len);
                    const char *filename = clax_http_extract_kv(kv, "filename", &filename_len);

                    if (name && (strncmp(name, "file", name_len) == 0) && filename) {
                        char fpath[1024] = {0};
                        char *new_name = clax_kv_list_find(&req->query_params, "name");
                        char *new_dir = clax_kv_list_find(&req->query_params, "dir");
                        char *crc32 = clax_kv_list_find(&req->query_params, "crc");
                        char *time = clax_kv_list_find(&req->query_params, "time");

                        if (crc32 && strlen(crc32) != 8) {
                            clax_dispatch_bad_request(clax_ctx, req, res);
                            return;
                        }

                        if (new_dir && strlen(new_dir)) {
                            if (strlen(new_dir) >= sizeof(fpath) - 1) {
                                clax_dispatch_bad_request(clax_ctx, req, res);
                                return;
                            }

                            struct stat info;

                            if (stat(new_dir, &info) == 0 && info.st_mode & S_IFDIR) {
                                strncpy(fpath, new_dir, MIN(strlen(new_dir), sizeof(fpath)));

                                if (new_dir[strlen(new_dir) - 1] != '/') {
                                    strcat(fpath, "/");
                                }
                            }
                            else {
                                clax_log("Output directory does not exist");

                                clax_dispatch_bad_request(clax_ctx, req, res);
                                break;
                            }
                        }

                        if (new_name && strlen(new_name)) {
                            strncat(fpath, new_name, MIN(strlen(new_name), sizeof(fpath) - strlen(fpath)));
                        }
                        else {
                            strncat(fpath, filename, MIN(filename_len, sizeof(fpath) - strlen(fpath)));
                        }

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
                            memcpy(res->body, "{\"status\":\"ok\"}", 15);
                            res->body_len = 15;
                        }

                        break;
                    }
                }
            }
        }

        if (!res->status_code) {
            clax_dispatch_bad_request(clax_ctx, req, res);
        }
    }
    else if (req->method == HTTP_GET && strcmp(path_info, "/download") == 0) {
        char *file = clax_kv_list_find(&req->query_params, "file");

        if (file && access(file, F_OK) != -1) {
            FILE *fh = fopen(file, "rb");

            if (fh == NULL) {
                clax_dispatch_system_error(clax_ctx, req, res);
            }
            else {
                char buf[255];
                char base_buf[255];
                char last_modified_buf[30];
                struct stat st;
                struct tm last_modified_time;

                stat(file, &st);

                snprintf(buf, sizeof(buf), "%d", (int)st.st_size);

                char *base = basename(file);
                strcpy(base_buf, "attachment; filename=");
                strcat(base_buf, base);

                res->status_code = 200;
                clax_kv_list_push(&res->headers, "Content-Type", "application/octet-stream");
                clax_kv_list_push(&res->headers, "Content-Disposition", base_buf);
                clax_kv_list_push(&res->headers, "Pragma", "no-cache");
                clax_kv_list_push(&res->headers, "Content-Length", buf);

                gmtime_r(&st.st_mtime, &last_modified_time);
                strftime(last_modified_buf, sizeof(last_modified_buf), "%a, %d %b %Y %T GMT", &last_modified_time);
                clax_kv_list_push(&res->headers, "Last-Modified", last_modified_buf);

                res->body_fh = fh;
            }
        }
        else {
            clax_dispatch_not_found(clax_ctx, req, res);
        }
    }
    else {
        clax_dispatch_not_found(clax_ctx, req, res);
    }
}
