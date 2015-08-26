#include <stdio.h>
#include <string.h>
#include "clax_command.h"
#include "u.h"

char output[1024];
size_t output_len = 0;
int command_cb(char *buf, size_t len)
{
    if (len) {
        memcpy(output + output_len, buf, len);
        output_len += len;
    }
    else {
        output[output_len] = 0;
    }
}

TEST_START(clax_command_runs_command)
{
    int ret = clax_command("echo 'bar'", command_cb);

    ASSERT_EQ(ret, 0)
    ASSERT_EQ(output_len, 4)
    ASSERT_STR_EQ(output, "bar\n");
}
TEST_END
