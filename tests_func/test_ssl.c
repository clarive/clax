#include <errno.h>
#include "u/u.h"

#include "util.h"

SUITE_START(ssl)

TEST_START(rejects not ssl request)
{
    int rcount;
    char output[1024];

    rcount = execute(CMD " -t ssl/server.crt -k ssl/server.key -r . ", "GET /\r\nContent-Length: 0\r\n\r\n", output, sizeof(output));

    ASSERT(rcount < 0);
    ASSERT_NULL(strstr(output, "200 OK"));
}
TEST_END

SUITE_END
