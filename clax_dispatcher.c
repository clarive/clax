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
#include <stdlib.h>
#include <stdarg.h>
#include <libgen.h> /* basename */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h> /* stat */
#include <unistd.h>
#include <time.h>
#include <utime.h>
#include <errno.h>

#ifdef _WIN32
# include <windows.h>
#endif

#include "clax.h"
#include "clax_version.h"
#include "clax_http.h"
#include "clax_log.h"
#include "clax_command.h"
#include "clax_big_buf.h"
#include "clax_crc32.h"
#include "clax_dispatcher.h"
#include "clax_util.h"
#include "clax_platform.h"

#include "clax_dispatcher_index.h"
#include "clax_dispatcher_ping.h"
#include "clax_dispatcher_tree.h"
#include "clax_dispatcher_command.h"

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

void clax_dispatch_not_found(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
    res->status_code = 404;
    clax_big_buf_append_str(&res->body, "Not found");
}

void clax_dispatch_bad_gateway(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
    res->status_code = 502;
    clax_big_buf_append_str(&res->body, "Bad Gateway");
}

void clax_dispatch_method_not_allowed(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
    res->status_code = 405;
    clax_big_buf_append_str(&res->body, "Method not allowed");
}

void clax_dispatch_system_error(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res, char *msg)
{
    char *p = msg;

    if (p == NULL) {
        p = "System error";
    }

    clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
    res->status_code = 500;
    clax_big_buf_append_str(&res->body, p);
}

void clax_dispatch_bad_request(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res, char *msg)
{
    char *p = msg;

    if (p == NULL) {
        p = "Bad Request";
    }

    res->status_code = 400;
    clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
    clax_big_buf_append_str(&res->body, p);
}

void clax_dispatch_not_authorized(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    res->status_code = 401;
    clax_kv_list_push(&res->headers, "Content-Type", "text/plain");
    clax_kv_list_push(&res->headers, "WWW-Authenticate", "Basic realm=\"clax\"");
    clax_big_buf_append_str(&res->body, "Authorization required");
}

clax_dispatcher_action_t clax_dispatcher_actions[] = {
    {"/", (1 << HTTP_GET), CLAX_DISPATCHER_NO_FLAGS, clax_dispatch_index},
    {"/ping", (1 << HTTP_GET), CLAX_DISPATCHER_NO_FLAGS, clax_dispatch_ping},
    {"/command", (1 << HTTP_POST), CLAX_DISPATCHER_NO_FLAGS, clax_dispatch_command},
    {"^/tree/", (
            1 << HTTP_HEAD |
            1 << HTTP_GET  |
            1 << HTTP_POST |
            1 << HTTP_DELETE
        ), CLAX_DISPATCHER_FLAG_100_CONTINUE, clax_dispatch_tree},
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
