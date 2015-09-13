#include <string.h>

#include "u/u.h"

#include "util.h"

SUITE_START(basic_auth)

TEST_START(success)
{
    int rcount;
    char output[1024];

    rcount = execute("../clax -n -r . -l /dev/null -a 'clax:password'",
            "GET /\r\nAuthorization: Basic Y2xheDpwYXNzd29yZA==\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_NOT_NULL(strstr(output, "200 OK"));
}
TEST_END

TEST_START(missing header)
{
    int rcount;
    char output[1024];

    rcount = execute("../clax -n -r . -l /dev/null -a 'clax:password'", "GET /\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_NOT_NULL(strstr(output, "401 Unauthorized"));
}
TEST_END

TEST_START(invalid header)
{
    int rcount;
    char output[1024];

    rcount = execute("../clax -n -r . -l /dev/null -a 'clax:password'", "GET /\r\nAuthorization: invalid\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_NOT_NULL(strstr(output, "401 Unauthorized"));
}
TEST_END

TEST_START(invalid base64 in header)
{
    int rcount;
    char output[1024];

    rcount = execute("../clax -n -r . -l /dev/null -a 'clax:password'", "GET /\r\nAuthorization: Basic invalid64\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_NOT_NULL(strstr(output, "401 Unauthorized"));
}
TEST_END

TEST_START(no colon)
{
    int rcount;
    char output[1024];

    rcount = execute("../clax -n -r . -l /dev/null -a 'clax:password'", "GET /\r\nAuthorization: Basic Zm9vYmFy\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_NOT_NULL(strstr(output, "401 Unauthorized"));
}
TEST_END

TEST_START(wrong username)
{
    int rcount;
    char output[1024];

    rcount = execute("../clax -n -r . -l /dev/null -a 'clax:password'", "GET /\r\nAuthorization: Basic Y2xhOnBhc3N3b3Jk\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_MATCHES(output, "401 Unauthorized");
}
TEST_END

TEST_START(wrong password)
{
    int rcount;
    char output[1024];

    rcount = execute("../clax -n -r . -l /dev/null -a 'clax:password'", "GET /\r\nAuthorization: Basic Y2xheDpwYXNzd29y\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_MATCHES(output, "401 Unauthorized");
}
TEST_END

SUITE_END
