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

#include "clax.h"
#include "clax_http.h"
#include "clax_log.h"
#include "clax_command.h"
#include "clax_big_buf.h"
#include "clax_util.h"
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
                res->status_code = 500;
                clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
                memcpy(res->body, "System error", 12);
                res->body_len = 12;
            }
        } else {
            res->status_code = 400;
            clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
            memcpy(res->body, "Invalid params", 14);
            res->body_len = 14;
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
                        char path_to_file[1024] = {0};
                        strncat(path_to_file, filename, MIN(sizeof(path_to_file) - strlen(path_to_file), filename_len));

                        int ret = clax_big_buf_write_file(&multipart->bbuf, path_to_file);

                        if (ret < 0) {
                            res->status_code = 500;
                            clax_kv_list_push(&res->headers, "Content-Type", "application/json");
                            memcpy(res->body, "{\"message\":\"System error\"}", 26);
                            res->body_len = 26;
                        }
                        else {
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
            res->status_code = 400;
            clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
            memcpy(res->body, "Bad request", 14);
            res->body_len = 14;
        }
    }
    else if (req->method == HTTP_GET && strcmp(path_info, "/download") == 0) {
        char *file = clax_kv_list_find(&req->query_params, "file");

        if (file && access(file, F_OK) != -1) {
            FILE *fh = fopen(file, "rb");

            if (fh == NULL) {
                res->status_code = 500;
                clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
                memcpy(res->body, "System error", 12);
                res->body_len = 12;
            }
            else {
                char buf[255];
                char base_buf[255];
                struct stat st;
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

                res->body_fh = fh;
            }
        }
        else {
            clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
            res->status_code = 404;
            memcpy(res->body, "Not found", 9);
            res->body_len = 9;
        }
    }
    else {
        clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
        res->status_code = 404;
        memcpy(res->body, "Not found", 9);
        res->body_len = 9;
    }
}
