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
#include "clax_command.h"
#include "clax_http.h"
#include "u.h"

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
}

char *context = "";
int command_vaargs_cb(char *buf, size_t len, va_list a_list)
{
    if (len) {
        context = va_arg(a_list, char *);
    }
}

int _clax_command(command_ctx_t *ctx, clax_http_chunk_cb_t chunk_cb, ...)
{
    int ret;
    va_list a_list;

    va_start(a_list, chunk_cb);

    ret = clax_command(ctx, chunk_cb, a_list);

    va_end(a_list);

    return ret;
}

TEST_START(clax_command_runs_command)
{
    command_ctx_t ctx = {.command="echo 'bar'"};

    int ret = _clax_command(&ctx, command_cb);

    ASSERT_EQ(ret, 0)
    ASSERT_EQ(output_len, 4)
    ASSERT_STR_EQ(output, "bar\n");
}
TEST_END

TEST_START(clax_command_runs_command_with_error)
{
    command_ctx_t ctx = {.command="unknown-command 2>&1 > /dev/null"};

    int ret = _clax_command(&ctx, command_cb);

    ASSERT(ret != 0)
}
TEST_END

TEST_START(clax_command_runs_command_vaargs)
{
    command_ctx_t ctx = {.command="echo 'bar'"};

    _clax_command(&ctx, command_vaargs_cb, "context");

    ASSERT_STR_EQ(context, "context");
}
TEST_END
