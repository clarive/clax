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
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#ifdef _WIN32
# include <windows.h>
#endif

#include "popen2.h"
#include "clax_log.h"
#include "clax_http.h"
#include "clax_command.h"
#include "clax_platform.h"

volatile int alarm_fired = 0;

#if defined(_WIN32)
VOID CALLBACK TimerProc(
    HWND hwnd,
    UINT uMsg,
    UINT idEvent,
    DWORD dwTime
   ) {
    alarm_fired = 1;
   }
#else
void clax_command_timeout(int dummy)
{
    signal(SIGALRM, SIG_IGN);

    alarm_fired = 1;
}
#endif

void clax_kill_kid(popen2_t *kid)
{
#if defined(_WIN32)
    clax_log("Killing pid=%d", kid->pid);

    HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, kid->pid);
    TerminateProcess(hProcess, 1);
    CloseHandle(hProcess);
#else
    int ret = 0;

    clax_log("Killing pgroup=%d", -kid->pid);
    ret = kill(-kid->pid, SIGTERM);
    if (ret != 0)
        clax_log("Killing pgroup failed");

    clax_log("Killing pid=%d", kid->pid);
    ret = kill(kid->pid, SIGTERM);
    if (ret != 0)
        kill(kid->pid, SIGKILL);
#endif
}

void clax_command_init(command_ctx_t *command_ctx) {
    memset(command_ctx, 0, sizeof(command_ctx_t));
}

void clax_command_free(command_ctx_t *command_ctx) {
    if (command_ctx->env) {
        int i = 0;
        while (command_ctx->env[i] != NULL) {
            free(command_ctx->env[i]);

            i++;
        }

        free(command_ctx->env);
    }
}

int clax_command_init_env(command_ctx_t *command_ctx, char **env)
{
    size_t len = 0;
    char *p;

    if (env == NULL)
        return 0;

    while ((p = env[len++]) != NULL);

    if ((command_ctx->env = malloc(sizeof(char *) * len)) != NULL) {
        memset(command_ctx->env, 0, sizeof(char *) * len);

        int i = 0;
        while (env[i] != NULL) {
            command_ctx->env[i] = clax_strdup(env[i]);

            i++;
        }
    }
    else {
        return -1;
    }

    return 0;
}

const char *clax_command_get_env(command_ctx_t *ctx, const char *key)
{
    char **env = ctx->env;

    if (env == NULL)
        return NULL;

    int i = 0;
    while (env[i] != NULL) {
        if (strncmp(env[i], key, strlen(key)) == 0 && *(env[i] + strlen(key)) == '=') {
            return env[i] + strlen(key) + 1;
        }

        i++;
    }

    return NULL;
}

char *clax_command_set_env_pair(command_ctx_t *ctx, const char *pair)
{
    char *end;

    if ((end = strstr(pair, "=")) != NULL) {
        char *key = clax_strndup(pair, end - pair);

        char *p = clax_command_set_env(ctx, key, end + 1);

        free(key);

        return p;
    }

    return NULL;
}

char *clax_command_set_env(command_ctx_t *ctx, const char *key, const char *val)
{
    char **env = ctx->env;

    int i = 0;
    while (env[i] != NULL) {
        if (strncmp(env[i], key, strlen(key)) == 0 && *(env[i] + strlen(key)) == '=') {
            char *p = NULL;
            char *val_e = clax_command_env_expand_a(ctx, val);

            clax_strapp_a(&p, key);
            clax_strapp_a(&p, "=");
            clax_strapp_a(&p, val_e);

            free(val_e);
            free(env[i]);
            env[i] = p;

            return p;
        }

        i++;
    }

    size_t len = 0;
    char *p;
    while ((p = ctx->env[len++]) != NULL);

    if ((ctx->env = realloc(ctx->env, sizeof(char *) * (len + 1))) != NULL) {
        ctx->env[len] = NULL;

        char *p = NULL;

        char *val_e = clax_command_env_expand_a(ctx, val);

        clax_strapp_a(&p, key);
        clax_strapp_a(&p, "=");
        clax_strapp_a(&p, val_e);

        ctx->env[len - 1] = p;

        free(val_e);
    }

    return NULL;
}

char *clax_command_env_expand_a(command_ctx_t *ctx, const char *val)
{
    char *rp_start = NULL;
    char *rp_end = NULL;
    char *key = NULL;

    if ((rp_start = strstr(val, "%")) != NULL) {
        if ((rp_end = strstr(rp_start + 1, "%")) != NULL) {
            key = clax_strndup(rp_start + 1, rp_end - rp_start - 1);
        }
    }

    if ((rp_start = strstr(val, "$")) != NULL) {
        if ((rp_end = strstr(rp_start + 1, ":")) != NULL) {
            rp_end--;
            key = clax_strndup(rp_start + 1, rp_end - rp_start);
        }
        else {
            rp_end = val + strlen(val) - 1;
            key = clax_strdup(rp_start + 1);
        }
    }

    if (key) {
        const char *old_value = clax_command_get_env(ctx, key);

        free(key);

        if (old_value) {
            char *val_expanded = NULL;

            clax_strnapp_a(&val_expanded, val, rp_start - val);
            clax_strnapp_a(&val_expanded, old_value, strlen(old_value));
            clax_strnapp_a(&val_expanded, rp_end + 1, strlen(val) - (rp_end - val) - 1);

            return val_expanded;
        }
    }

    return clax_strdup(val);
}

int clax_command_start(command_ctx_t *ctx)
{
    int ret;

    char *command = ctx->command;
    char **env = ctx->env;

    memset(&ctx->kid, 0, sizeof(popen2_t));

    clax_log("Running command '%s'", command);

    ret = popen2((const char *)command, env, &ctx->kid);
    if (ret < 0) {
        clax_log("Command failed=%d", ret);
        return -1;
    }

    clax_log("Command started, pid=%d", ctx->kid.pid);

    return ctx->kid.pid;
}

int clax_command_read_va(command_ctx_t *ctx, clax_http_chunk_cb_t chunk_cb, ...)
{
    va_list a_list;
    int ret;

    va_start(a_list, chunk_cb);

    ret = clax_command_read(ctx, chunk_cb, a_list);

    va_end(a_list);

    return ret;
}

int clax_command_read(command_ctx_t *ctx, clax_http_chunk_cb_t chunk_cb, va_list a_list_)
{
    char buf[1024];
    int ret;
    va_list a_list;

#ifdef _WIN32
    MSG msg;
#endif

    int timeout = ctx->timeout;
    popen2_t *kid = &ctx->kid;

    va_copy(a_list, a_list_);

    alarm_fired = 0;

    if (timeout) {
        clax_log("Setting command timeout=%d", timeout);

#if defined(_WIN32)
        SetTimer(NULL, 0, timeout * 1000, (TIMERPROC)TimerProc);
#else
        signal(SIGALRM, clax_command_timeout);
        alarm(timeout);
#endif
    }

    while(1) {
        ret = read(kid->out, buf, sizeof(buf));

        if (ret == 0)
            break;

        if (alarm_fired) {
            clax_log("Command timeout reached=%d", timeout);

            clax_kill_kid(kid);

            break;
        }

        if (ret < 0) {
            if (errno == EAGAIN) {

                usleep(0.2 * 1000000);

                continue;
            }

            break;
        }

        /* We ignore errors here, even if the client disconnects we have to
         * continue running the command */
        chunk_cb(buf, ret, a_list);

#ifdef _WIN32
        if (timeout) {
            GetMessage(&msg, NULL, 0, 0);
            DispatchMessage(&msg);
        }
#endif
    };

    int exit_code = clax_command_close(ctx);

    if (alarm_fired) {
        ctx->timeout_fired = 1;
        exit_code = 255;
    }

    return exit_code;
}

int clax_command_close(command_ctx_t *ctx)
{
    int ret;

    ret = pclose2(&ctx->kid);
    clax_log("Command finished, exit_code=%d", ret);

    return ret;
}

int clax_command_is_running(command_ctx_t *ctx)
{
#if defined(_WIN32)
    HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, ctx->kid.pid);
    unsigned long int exit_code;

    if (GetExitCodeProcess(hProcess, &exit_code)) {
        if (exit_code == STILL_ACTIVE) {
            return 1;
        }
    }

#else
    if (kill(ctx->kid.pid, 0) == 0) {
        return 1;
    }
#endif

    return 0;
}
