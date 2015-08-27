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

int _clax_command(char *command, clax_http_chunk_cb_t chunk_cb, ...)
{
    int ret;
    va_list a_list;

    va_start(a_list, chunk_cb);

    ret = clax_command(command, chunk_cb, a_list);

    va_end(a_list);

    return ret;
}

TEST_START(clax_command_runs_command)
{
    int ret = _clax_command("echo 'bar'", command_cb);

    ASSERT_EQ(ret, 0)
    ASSERT_EQ(output_len, 4)
    ASSERT_STR_EQ(output, "bar\n");
}
TEST_END

TEST_START(clax_command_runs_command_vaargs)
{
    _clax_command("echo 'bar'", command_vaargs_cb, "context");

    ASSERT_STR_EQ(context, "context");
}
TEST_END
