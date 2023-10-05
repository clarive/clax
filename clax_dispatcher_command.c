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
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "contrib/libuv/include/uv.h"
#include "contrib/frozen/frozen.c"

#include "clax_http.h"
#include "clax_log.h"
#include "clax_dispatcher.h"
#include "clax_util.h"

#define MAX_ARGS 100

extern char **environ;

typedef struct process_t {
    uv_process_t req;
    uv_process_options_t options;
    uv_pipe_t *out;
    long int exit_status;
    void *data;
} process_t;

void free_process(process_t *req) {
    /*if (req->out)*/
        /*free(req->out);*/
    if (req->options.args)
        free(req->options.args);
}

void process_close_cb(uv_handle_t *handle)
{
    process_t *process = (process_t *)handle;

    clax_log("Closing process");

    if (process && process->data) {
        clax_log("Writing trailing headers");

        clax_ctx_t *clax_ctx = process->data;

        char tail[255];
        snprintf(tail, sizeof(tail), "X-Clax-Exit: %ld\r\nX-Clax-Status: %s",
                process->exit_status, process->exit_status == 0 ? "success" : "error");

        clax_http_push_response(clax_ctx, &clax_ctx->response, (const unsigned char *)tail, 0);
    }

    if (process)
        free_process(process);

    if (handle)
        free(handle);
}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
}

void exit_cb(uv_process_t *req, int64_t exit_status, int term_signal) {
    clax_log("Process pid=%d exited with status %d, signal %d", req->pid, exit_status, term_signal);

    process_t *process = (process_t *)req;

    process->exit_status = exit_status;

    uv_close((uv_handle_t *)req, process_close_cb);
}

static void on_read(uv_stream_t *pipe, ssize_t nread, const uv_buf_t *buf) {
    process_t *process = pipe->data;

    if (nread > 0) {
        clax_ctx_t *clax_ctx = process->data;

        clax_http_push_response(clax_ctx, &clax_ctx->response, (const unsigned char *)buf->base, nread);
    } else if (nread < 0) {
        if (nread != UV_EOF) {
            clax_log("Error: read failed: %s", uv_strerror(nread));
        }

        uv_close((uv_handle_t *)pipe, NULL);
    }
}

static void scan_array(const char *str, int len, char **args) {
    struct json_token t;
    int i;
    for (i = 0; json_scanf_array_elem(str, len, "", i, &t) > 0; i++) {
        char *buf = malloc( t.len + 1 );
        sprintf(buf, "%.*s", t.len, t.ptr);
        args[i] = buf;
    }
}

void clax_dispatch_command(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    char *command = clax_kv_list_find(&req->body_params, "command");
    if (!command || !strlen(command)) {
        clax_dispatch_bad_request(clax_ctx, req, res, "Missing required field: command");
        return;
    }

    char *json = clax_kv_list_find(&req->body_params, "json");

    //char *timeout = clax_kv_list_find(&req->body_params, "timeout");
    //if (timeout && strlen(timeout)) {
    //    command_ctx.timeout = atoi(timeout);
    //}

    //char *env = clax_kv_list_find(&req->body_params, "env");
    //if (env && strlen(env)) {
    //    char *start = env;
    //    char *end = NULL;

    //    clax_command_init_env(&command_ctx, environ);

    //    while (start) {
    //        end = strstr(start, "\n");

    //        if (end == NULL) {
    //            clax_command_set_env_pair(&command_ctx, start);

    //            start = NULL;
    //        }
    //        else {
    //            char *p = clax_strndup(start, end - start);

    //            clax_command_set_env_pair(&command_ctx, p);

    //            free(p);

    //            start = end + 1;
    //        }
    //    }
    //}

    process_t *process = malloc(sizeof(process_t));

    int argc = 3;
    char **args = malloc(sizeof(char *) * MAX_ARGS);

#ifdef _WIN32
    args[0] = "cmd.exe";
    args[1] = "/c";
#else
    args[0] = "sh";
    args[1] = "-c";
#endif
    if( json == NULL || strlen(json) == 0 ) {
        args[2] = command;
        args[3] = NULL;
    }
    else {
        clax_log("JSON payload: %s", json);
        int json_argc;
        bool bare = false;
        char *cmd = (char *) malloc(sizeof(char) * strlen(json));
        char **args_array = (char **) malloc(sizeof(char *) * MAX_ARGS);

        int json_status = json_scanf(json, strlen(json), "{ cmd:%Q, args:%M, argc:%d, bare:%B }",
                &cmd, scan_array, args_array, &json_argc, &bare);

        clax_log("JSON cmd=%s, argc=%d, bare=%d, status=%d", cmd, json_argc, bare, json_status);

        if( bare ) {
            args[0] = cmd;
            argc = json_argc + 1;
            for( int a=0; a<argc; a++ )
                args[a+1] = args_array[a];
            args[argc] = NULL;
        }
        else {
            args[2] = cmd;
            for( int a=0; a<argc; a++ )
                args[a+3] = args_array[a];
            argc = json_argc + 3;
            args[argc] = NULL;
        }
    }

    uv_process_options_t *options = &process->options;
    memset(options, 0, sizeof(uv_process_options_t));

    options->file = args[0];
    options->args = args;
    options->exit_cb = exit_cb;
    options->flags = 0;

    uv_pipe_t *out = malloc(sizeof(uv_pipe_t));
    out->data = process;

    process->out = out;

    int r = uv_pipe_init(uv_default_loop(), out, 0);
    if (r < 0) {
        clax_log("Error: pipe init failed: %s", uv_strerror(r));

        clax_dispatch_system_error(clax_ctx, req, res, "Can't start command");

        return;
    }

    uv_stdio_container_t child_stdio[3];
    child_stdio[0].flags = UV_IGNORE;
    child_stdio[1].flags = UV_CREATE_PIPE | UV_WRITABLE_PIPE;
    child_stdio[1].data.stream = (uv_stream_t *)out;
    child_stdio[2].flags = UV_IGNORE;

    options->stdio_count = 3;
    options->stdio = child_stdio;

    process->data = clax_ctx;

    char cwd[1024] = {0};
    size_t size = sizeof(cwd);

    uv_cwd(cwd, &size);
    clax_log("Process cwd='%s'", cwd);

    r = uv_spawn(uv_default_loop(), (uv_process_t *)process, options);
    if (r < 0) {
        /*free_process(process);*/

        clax_log("Error: can't start command: %s", uv_strerror(r));

        for (int i = 0; i < argc; i++) {
            clax_log("arg[%d]=%s", i, args[i]);
        }

        clax_dispatch_system_error(clax_ctx, req, res, "Can't start command");

        return;
    }

    uv_process_t *uv_process = (uv_process_t *)process;
    int pid = uv_process->pid;

    clax_log("Started command '%s' with pid=%d", command, pid);
    clax_log("Command arguments...");
    for (int i = 0; i < argc; i++) {
        clax_log("arg[%d]=%s", i, args[i]);
    }

    r = uv_read_start((uv_stream_t *)out, alloc_buffer, on_read);
    if (r < 0) {
        clax_log("Error: failed reading from command: %s", uv_strerror(r));

        clax_dispatch_system_error(clax_ctx, req, res, "Can't start command");

        return;
    }

    clax_http_response_status(clax_ctx, res, 200);

    clax_http_response_header(clax_ctx, res, "Transfer-Encoding", "chunked");
    clax_http_response_header(clax_ctx, res, "Trailer", "X-Clax-Exit, X-Clax-Status");

    char buf_pid[15];
    snprintf(buf_pid, sizeof(buf_pid), "%d", pid);
    clax_http_response_header(clax_ctx, res, "X-Clax-PID", buf_pid);

    clax_http_write_response_headers(clax_ctx, res);
}
