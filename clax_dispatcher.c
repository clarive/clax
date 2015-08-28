#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "clax_http.h"
#include "clax_log.h"
#include "clax_command.h"

void clax_command_cb(void *ctx, clax_http_chunk_cb_t chunk_cb, ...)
{
    char *command = ctx;
    va_list a_list;

    va_start(a_list, chunk_cb);

    if (command && *command && strlen(command)) {
        char buf[255];

        int ret = clax_command(command, chunk_cb, a_list);

        snprintf(buf, sizeof(buf), "--\nexit=%d", ret);
        chunk_cb(buf, strlen(buf), a_list);
    }

    chunk_cb(NULL, 0, a_list);

    va_end(a_list);
}

void clax_dispatch(clax_http_request_t *req, clax_http_response_t *res)
{
    char *path_info = req->path_info;

    memset(res, 0, sizeof(clax_http_response_t));

    if (strcmp(path_info, "/ping") == 0) {
        res->status_code = 200;
        res->content_type = "application/json";
        memcpy(res->body, "{\"message\":\"pong\"}", 20);
        res->body_len = 20;
    }
    else if (req->method == HTTP_POST && strcmp(path_info, "/command") == 0) {
        if (req->params_num) {
            int i;
            for (i = 0; i < req->params_num; i++) {
                if (strcmp(req->params[i].key, "command") == 0) {
                    res->status_code = 200;
                    res->transfer_encoding = "chunked";

                    res->body_cb_ctx = req->params[i].val;
                    res->body_cb = clax_command_cb;

                    break;
                }
            }
        }

        if (!res->status_code) {
            res->status_code = 400;
            res->content_type = "text/plain";
            memcpy(res->body, "Invalid params", 14);
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
