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
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#include "contrib/u/u.h"

#include "clax_http.h"

double get_time()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec*1e-6;
}

char output[1024];
size_t output_len = 0;
int command_cb(char *buf, size_t len, va_list a_list)
{
    if (len) {
        memcpy(output + output_len, buf, len);
        output_len += len;
    }
    else {
        output[output_len] = 0;
    }

    return 0;
}

char *context = "";
int command_vaargs_cb(char *buf, size_t len, va_list a_list)
{
    if (len) {
        context = va_arg(a_list, char *);
    }

    return 0;
}

SUITE_START(clax_command)

/*TEST_START(start_runs_command)*/
/*{*/
/*#ifdef _WIN32*/
    /*command_ctx_t ctx = {.command="set foo=bar"};*/
/*#else*/
    /*command_ctx_t ctx = {.command="echo 'bar'"};*/
/*#endif*/

    /*pid_t pid = clax_command_start(&ctx);*/
    /*ASSERT(pid > 0)*/

    /*int exit_code = clax_command_close(&ctx);*/
    /*ASSERT_EQ(exit_code, 0)*/

    /*ASSERT_EQ(clax_command_is_running(&ctx), 0)*/
/*}*/
/*TEST_END*/

/*TEST_START(start_returns_error)*/
/*{*/
    /*command_ctx_t ctx = {.command="unknown-command"};*/

    /*int ret = clax_command_start(&ctx);*/
    /*ASSERT(ret != 0)*/

    /*int exit_code = clax_command_close(&ctx);*/
    /*ASSERT(exit_code != 0)*/

    /*ASSERT_EQ(clax_command_is_running(&ctx), 0)*/
/*}*/
/*TEST_END*/

/*TEST_START(read_reads_output)*/
/*{*/
/*#ifdef _WIN32*/
    /*command_ctx_t ctx = {.command="cmd.exe /c echo bar"};*/
/*#else*/
    /*command_ctx_t ctx = {.command="echo bar"};*/
/*#endif*/

    /*clax_command_start(&ctx);*/

    /*clax_command_read_va(&ctx, command_cb);*/

    /*clax_command_close(&ctx);*/

/*#ifdef _WIN32*/
    /*ASSERT_EQ((int)output_len, 5)*/
    /*ASSERT_STR_EQ(output, "bar\r\n");*/
/*#else*/
    /*ASSERT_EQ((int)output_len, 4)*/
    /*ASSERT_STR_EQ(output, "bar\n");*/
/*#endif*/

    /*ASSERT_EQ(clax_command_is_running(&ctx), 0)*/
/*}*/
/*TEST_END*/

/*TEST_START(runs_command_vaargs)*/
/*{*/
/*#ifdef _WIN32*/
    /*command_ctx_t ctx = {.command="cmd.exe /c echo bar"};*/
/*#else*/
    /*command_ctx_t ctx = {.command="echo 'bar'"};*/
/*#endif*/

    /*clax_command_start(&ctx);*/

    /*clax_command_read_va(&ctx, command_vaargs_cb, "context");*/

    /*clax_command_close(&ctx);*/

    /*ASSERT_STR_EQ(context, "context");*/
/*}*/
/*TEST_END*/

/*TEST_START(kills command after timeout)*/
/*{*/
    /*command_ctx_t ctx = {.command = "echo foo && sleep 1 && echo bar && sleep 1 && echo baz && sleep 1", .timeout = 1};*/

    /*double start = get_time();*/

    /*clax_command_start(&ctx);*/

    /*clax_command_read_va(&ctx, command_vaargs_cb);*/

    /*int exit_code = clax_command_close(&ctx);*/

    /*double end = get_time();*/

    /*ASSERT(end - start < 3)*/
    /*ASSERT(exit_code != 0)*/
/*}*/
/*TEST_END*/

/*TEST_START(clax_command_init_env - inits environment)*/
/*{*/
    /*command_ctx_t ctx;*/

    /*clax_command_init(&ctx);*/

    /*char *env[] = {"foo=bar", "bar=baz", NULL};*/

    /*clax_command_init_env(&ctx, env);*/

    /*ASSERT_STR_EQ(ctx.env[0], "foo=bar");*/
    /*ASSERT_STR_EQ(ctx.env[1], "bar=baz");*/
    /*ASSERT_NULL(ctx.env[2]);*/

    /*clax_command_free(&ctx);*/
/*}*/
/*TEST_END*/

/*TEST_START(clax_command_set_env - sets new environment)*/
/*{*/
    /*command_ctx_t ctx;*/

    /*clax_command_init(&ctx);*/

    /*char *env[] = {NULL};*/

    /*clax_command_init_env(&ctx, env);*/

    /*clax_command_set_env(&ctx, "foo", "baz");*/

    /*ASSERT_STR_EQ(ctx.env[0], "foo=baz");*/

    /*clax_command_free(&ctx);*/
/*}*/
/*TEST_END*/

/*TEST_START(clax_command_set_env - overwrites environment)*/
/*{*/
    /*command_ctx_t ctx;*/

    /*clax_command_init(&ctx);*/

    /*char *env[] = {"foo=bar", "bar=baz", NULL};*/

    /*clax_command_init_env(&ctx, env);*/

    /*clax_command_set_env(&ctx, "foo", "baz");*/

    /*ASSERT_STR_EQ(ctx.env[0], "foo=baz");*/

    /*clax_command_free(&ctx);*/
/*}*/
/*TEST_END*/

/*TEST_START(clax_command_set_env_pair - sets environment from pair)*/
/*{*/
    /*command_ctx_t ctx;*/

    /*clax_command_init(&ctx);*/

    /*char *env[] = {"foo=bar", "bar=baz", NULL};*/

    /*clax_command_init_env(&ctx, env);*/

    /*clax_command_set_env_pair(&ctx, "foo=baz");*/

    /*ASSERT_STR_EQ(ctx.env[0], "foo=baz");*/

    /*clax_command_free(&ctx);*/
/*}*/
/*TEST_END*/

/*TEST_START(clax_command_get_env - correctly handles NULL environment)*/
/*{*/
    /*command_ctx_t ctx;*/

    /*clax_command_init(&ctx);*/

    /*clax_command_init_env(&ctx, NULL);*/

    /*const char *val = clax_command_get_env(&ctx, "foo");*/

    /*ASSERT_NULL(val);*/

    /*clax_command_free(&ctx);*/
/*}*/
/*TEST_END*/

/*TEST_START(clax_command_get_env - gets environment)*/
/*{*/
    /*command_ctx_t ctx;*/

    /*clax_command_init(&ctx);*/

    /*char *env[] = {"foo=bar", "bar=baz", NULL};*/

    /*clax_command_init_env(&ctx, env);*/

    /*const char *val = clax_command_get_env(&ctx, "foo");*/

    /*ASSERT_STR_EQ(val, "bar");*/

    /*clax_command_free(&ctx);*/
/*}*/
/*TEST_END*/

/*TEST_START(clax_command_env_expand_a - expands win environment)*/
/*{*/
    /*command_ctx_t ctx;*/

    /*clax_command_init(&ctx);*/

    /*char *env[] = {"foo=bar", NULL};*/

    /*clax_command_init_env(&ctx, env);*/

    /*char *new_val = clax_command_env_expand_a(&ctx, "%foo%;baz");*/

    /*ASSERT_STR_EQ(new_val, "bar;baz");*/

    /*free(new_val);*/

    /*clax_command_free(&ctx);*/
/*}*/
/*TEST_END*/

/*TEST_START(clax_command_env_expand_a - expands unix environment)*/
/*{*/
    /*command_ctx_t ctx;*/

    /*clax_command_init(&ctx);*/

    /*char *env[] = {"foo=bar", NULL};*/

    /*clax_command_init_env(&ctx, env);*/

    /*char *new_val = clax_command_env_expand_a(&ctx, "$foo:baz");*/

    /*ASSERT_STR_EQ(new_val, "bar:baz");*/

    /*free(new_val);*/

    /*new_val = clax_command_env_expand_a(&ctx, "$foo");*/

    /*ASSERT_STR_EQ(new_val, "bar");*/

    /*free(new_val);*/

    /*clax_command_free(&ctx);*/
/*}*/
/*TEST_END*/

/*TEST_START(clax_command_set_env - sets env with expansion)*/
/*{*/
    /*command_ctx_t ctx;*/

    /*clax_command_init(&ctx);*/

    /*char *env[] = {"foo=bar", "bar=baz", NULL};*/

    /*clax_command_init_env(&ctx, env);*/

    /*clax_command_set_env(&ctx, "foo", "%foo%;baz");*/

    /*ASSERT_STR_EQ(ctx.env[0], "foo=bar;baz");*/

    /*clax_command_free(&ctx);*/
/*}*/
/*TEST_END*/

/*#ifndef _WIN32*/
/*TEST_START(kills command after timeout without any output)*/
/*{*/
    /*command_ctx_t ctx = {.command = "sleep 5", .timeout = 1};*/

    /*double start = get_time();*/

    /*clax_command_start(&ctx);*/

    /*clax_command_read_va(&ctx, command_vaargs_cb);*/

    /*clax_command_close(&ctx);*/

    /*double end = get_time();*/

    /*ASSERT(end - start < 2)*/
/*}*/
/*TEST_END*/
/*#endif*/

SUITE_END
