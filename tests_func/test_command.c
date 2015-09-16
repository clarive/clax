#include <string.h>

#include "u/u.h"

#include "util.h"

SUITE_START(command)

TEST_START(simple command)
{
    char output[1024];
    char request[1024];
    char body[] = "command=echo 'foo'";

    sprintf(request,
            "POST /command\r\n"
            "Content-Type: application/x-www-form-urlencoded\r\n"
            "Content-Length: %d\r\n"
            "\r\n"
            "%s", (int)strlen(body), body);

    int rcount = execute(CMD " -n -r . -l " DEVNULL, request, output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_MATCHES(output, "X-Clax-PID: \\d+")
    ASSERT_MATCHES(output, "X-Clax-Exit: 0")
    ASSERT_MATCHES(output, "foo")
}
TEST_END

TEST_START(command failure)
{
    char output[1024];
    char request[1024];
    char body[] = "command=unlikely-to-exist";

    sprintf(request,
            "POST /command\r\n"
            "Content-Type: application/x-www-form-urlencoded\r\n"
            "Content-Length: %d\r\n"
            "\r\n"
            "%s", (int)strlen(body), body);

    int rcount = execute(CMD " -n -r . -l " DEVNULL, request, output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_MATCHES(output, "X-Clax-PID: \\d+")
    ASSERT_MATCHES(output, "X-Clax-Exit: 0")
}
TEST_END

SUITE_END
