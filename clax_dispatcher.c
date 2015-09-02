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

#include "clax.h"
#include "clax_http.h"
#include "clax_log.h"
#include "clax_command.h"
#include "clax_util.h"

command_ctx_t command_ctx;

void clax_command_cb(void *ctx, clax_http_chunk_cb_t chunk_cb, ...)
{
    command_ctx_t *command_ctx = ctx;
    va_list a_list;
    int ret;

    char *command = command_ctx->command;

    va_start(a_list, chunk_cb);

    if (command && *command && strlen(command)) {
        char buf[255];

        int ret = clax_command(command_ctx, chunk_cb, a_list);

        snprintf(buf, sizeof(buf), "--\nexit=%d", ret);
        ret = chunk_cb(buf, strlen(buf), a_list);

        if (ret < 0) goto exit;
    }

    ret = chunk_cb(NULL, 0, a_list);

exit:
    va_end(a_list);

    return;
}

void clax_dispatch(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    char *path_info = req->path_info;

    if (strcmp(path_info, "/ping") == 0) {
        res->status_code = 200;
        res->content_type = "application/json";
        memcpy(res->body, "{\"message\":\"pong\"}", 20);
        res->body_len = 20;
    }
    else if (req->method == HTTP_POST && strcmp(path_info, "/command") == 0) {
        memset(&command_ctx, 0, sizeof(command_ctx_t));

        if (req->params_num) {
            int i;
            for (i = 0; i < req->params_num; i++) {
                char *key = req->params[i].key;

                if (strcmp(key, "command") == 0) {
                    res->status_code = 200;
                    res->transfer_encoding = "chunked";

                    strncpy(command_ctx.command, req->params[i].val, sizeof_struct_member(command_ctx_t, command));
                }
                else if (strcmp(key, "timeout") == 0) {
                    command_ctx.timeout = atoi(req->params[i].val);
                }
            }
        }

        if (res->status_code) {
            res->body_cb_ctx = &command_ctx;
            res->body_cb = clax_command_cb;
        } else {
            res->status_code = 400;
            res->content_type = "text/plain";
            memcpy(res->body, "Invalid params", 14);
            res->body_len = 14;
        }
    }
    else if (req->method == HTTP_POST && strcmp(path_info, "/upload") == 0) {
        if (strlen(req->multipart_boundary)) {
            int i;
            for (i = 0; i < req->multiparts_num; i++) {
                clax_http_multipart_t *multipart = &req->multiparts[i];

                const char *content_disposition = clax_http_header_get(multipart->headers, multipart->headers_num, "Content-Disposition");
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
                        strncpy(path_to_file, clax_ctx->options->root, sizeof(path_to_file));
                        strncat(path_to_file, filename, MIN(sizeof(path_to_file) - strlen(path_to_file), filename_len));

                        int ret;
                        if (multipart->part_fpath && strlen(multipart->part_fpath)) {
                            clax_log("Renaming to file '%s' -> '%s'", multipart->part_fpath, path_to_file);
                            ret = rename(multipart->part_fpath, path_to_file);
                        }
                        else {
                            clax_log("Saving to file '%s'", path_to_file);
                            ret = clax_dispatcher_write_file(path_to_file, multipart->part, multipart->part_len);
                        }

                        if (ret < 0) {
                            res->status_code = 500;
                            res->content_type = "application/json";
                            memcpy(res->body, "{\"message\":\"System error\"}", 26);
                            res->body_len = 26;
                        }
                        else {
                            res->status_code = 200;
                            res->content_type = "application/json";
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
            res->content_type = "text/plain";
            memcpy(res->body, "Bad request", 14);
            res->body_len = 14;
        }
    }
    else {
        res->content_type = "text/plain";
        res->status_code = 404;
        memcpy(res->body, "Not found", 9);
        res->body_len = 9;
    }
}

int clax_dispatcher_write_file(char *fname, char *buf, size_t len)
{
    FILE *fh;

    fh = fopen(fname, "w");

    if (fh == NULL) {
        return -1;
    }

    fwrite(buf, 1, len, fh);

    fclose(fh);

    return 0;
}
