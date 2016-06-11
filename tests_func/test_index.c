#include <errno.h>
#include "u/u.h"

#include "util.h"

SUITE_START(index)

TEST_START(index_page)
{
    int rcount;
    char output[1024];

    rcount = execute(CMD " -r . -l " DEVNULL, "GET /\r\nContent-Length: 0\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))
}
TEST_END

TEST_START(ping)
{
    int rcount;
    char output[1024];

    rcount = execute(CMD " -r . -l " DEVNULL, "GET /ping\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))
    ASSERT_MATCHES(output, "200 OK")
    ASSERT_MATCHES(output, "{\"message\":\"pong\"}")
}
TEST_END

TEST_START(not found)
{
    int rcount;
    char output[1024];

    rcount = execute(CMD " -r . -l " DEVNULL, "GET /unknown\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))
    ASSERT_MATCHES(output, "404 Not Found")
}
TEST_END

TEST_START(method not allowed)
{
    int rcount;
    char output[1024];

    rcount = execute(CMD " -r . -l " DEVNULL, "POST /\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))
    ASSERT_MATCHES(output, "405 Method Not Allowed")
}
TEST_END

SUITE_END
