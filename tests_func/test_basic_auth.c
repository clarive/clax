#include <string.h>

#include "u.h"
#include "util.h"

TEST_START(test_basic_auth)
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

TEST_START(test_basic_auth_header_missing)
{
    int rcount;
    char output[1024];

    rcount = execute("../clax -n -r . -l /dev/null -a 'clax:password'", "GET /\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_NOT_NULL(strstr(output, "401 Unauthorized"));
}
TEST_END

TEST_START(test_basic_auth_header_invalid)
{
    int rcount;
    char output[1024];

    rcount = execute("../clax -n -r . -l /dev/null -a 'clax:password'", "GET /\r\nAuthorization: invalid\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_NOT_NULL(strstr(output, "401 Unauthorized"));
}
TEST_END

TEST_START(test_basic_auth_header_invalid_base64)
{
    int rcount;
    char output[1024];

    rcount = execute("../clax -n -r . -l /dev/null -a 'clax:password'", "GET /\r\nAuthorization: Basic invalid64\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_NOT_NULL(strstr(output, "401 Unauthorized"));
}
TEST_END

TEST_START(test_basic_auth_no_colon)
{
    int rcount;
    char output[1024];

    rcount = execute("../clax -n -r . -l /dev/null -a 'clax:password'", "GET /\r\nAuthorization: Basic Zm9vYmFy\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_NOT_NULL(strstr(output, "401 Unauthorized"));
}
TEST_END

TEST_START(test_basic_auth_wrong_username)
{
    int rcount;
    char output[1024];

    rcount = execute("../clax -n -r . -l /dev/null -a 'clax:password'", "GET /\r\nAuthorization: Basic Y2xhOnBhc3N3b3Jk\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_NOT_NULL(strstr(output, "401 Unauthorized"));
}
TEST_END

TEST_START(test_basic_auth_wrong_password)
{
    int rcount;
    char output[1024];

    rcount = execute("../clax -n -r . -l /dev/null -a 'clax:password'", "GET /\r\nAuthorization: Basic Y2xheDpwYXNzd29y\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT(strstr(output, "401 Unauthorized"));
}
TEST_END
