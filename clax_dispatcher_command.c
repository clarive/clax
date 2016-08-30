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
#include "clax_util.h"
#include "clax_dispatcher.h"
#include "clax_dispatcher_command.h"

static command_ctx_t command_ctx;

extern char **environ;

void clax_command_read_cb(void *ctx, clax_http_chunk_cb_t chunk_cb, ...)
{
    command_ctx_t *command_ctx = ctx;
    va_list a_list;
    char buf[255];
    char *status;

    va_start(a_list, chunk_cb);

    int exit_code = clax_command_read(command_ctx, chunk_cb, a_list);

    if (command_ctx->timeout_fired) {
        status = "timeout";
    }
    else if (exit_code != 0) {
        status = "error";
    }
    else {
        status = "success";
    }

    /* Chunked trailing header */
    snprintf(buf, sizeof(buf), "X-Clax-Exit: %d\r\nX-Clax-Status: %s", exit_code, status);

    chunk_cb(buf, 0, a_list);

    va_end(a_list);

    return;
}

void clax_dispatch_command(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    clax_command_init(&command_ctx);

    char *command = clax_kv_list_find(&req->body_params, "command");
    if (!command || !strlen(command)) {
        clax_dispatch_bad_request(clax_ctx, req, res, "Command is requried");
        return;
    }

    strncpy(command_ctx.command, command, sizeof_struct_member(command_ctx_t, command));

    char *timeout = clax_kv_list_find(&req->body_params, "timeout");
    if (timeout && strlen(timeout)) {
        command_ctx.timeout = atoi(timeout);
    }

    char *env = clax_kv_list_find(&req->body_params, "env");
    if (env && strlen(env)) {
        char *start = env;
        char *end = NULL;

        clax_command_init_env(&command_ctx, environ);

        while (start) {
            end = strstr(start, "\n");

            if (end == NULL) {
                clax_command_set_env_pair(&command_ctx, start);

                start = NULL;
            }
            else {
                char *p = clax_strndup(start, end - start);

                clax_command_set_env_pair(&command_ctx, p);

                free(p);

                start = end + 1;
            }
        }
    }

    /* TODO: free env */
    pid_t pid = clax_command_start(&command_ctx);
    if (pid <= 0) {
        clax_dispatch_system_error(clax_ctx, req, res, "Can't start command");
        return;
    }

    res->status_code = 200;

    clax_kv_list_push(&res->headers, "Transfer-Encoding", "chunked");

    char buf_pid[15];
    snprintf(buf_pid, sizeof(buf_pid), "%d", pid);

    clax_kv_list_push(&res->headers, "Trailer", "X-Clax-Exit, X-Clax-Status");
    clax_kv_list_push(&res->headers, "X-Clax-PID", buf_pid);

    res->body_cb_ctx = &command_ctx;
    res->body_cb = clax_command_read_cb;
}
