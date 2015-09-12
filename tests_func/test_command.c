#include <string.h>

#include "u.h"
#include "util.h"

TEST_START(test_command)
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

    int rcount = execute("../clax -n -r . -l /dev/null", request, output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_MATCHES(output, "X-Clax-PID: \\d+")
    ASSERT_MATCHES(output, "X-Clax-Exit: 0")
    ASSERT_MATCHES(output, "foo")
}
TEST_END

TEST_START(test_command_failure)
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

    int rcount = execute("../clax -n -r . -l /dev/null", request, output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_MATCHES(output, "X-Clax-PID: \\d+")
    ASSERT_MATCHES(output, "X-Clax-Exit: 0")
}
TEST_END
