#include <stdio.h>
#include "clax_http.h"
#include "u.h"

TEST_START(clax_http_parse_returns_error_when_error)
{
    clax_http_request_t request;

    clax_http_init();
    int rv = clax_http_parse(&request, "foobarbaz", 9);

    ASSERT(rv == -1)
}
TEST_END

TEST_START(clax_http_parse_returns_0_when_need_more)
{
    clax_http_request_t request;

    clax_http_init();
    int rv = clax_http_parse(&request, "GET / HTTP/1.1\r\n", 16);

    ASSERT(rv == 0)
}
TEST_END

TEST_START(clax_http_parse_returns_ok_chunks)
{
    clax_http_request_t request;

    clax_http_init();

    int rv = clax_http_parse(&request, "GET / ", 6);
    ASSERT(rv == 0)

    rv = clax_http_parse(&request, "HTTP/1.1\r\n", 10);
    ASSERT(rv == 0)

    rv = clax_http_parse(&request, "Host: localhost\r\nConnection: close\r\n\r\n", 15 + 2 + 17 + 4);
    ASSERT(rv == 1)
}
TEST_END

TEST_START(clax_http_parse_returns_ok)
{
    clax_http_request_t request;

    clax_http_init();

    int rv = clax_http_parse(&request, "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n", 14 + 2 + 15 + 2 + 17 + 4);
    ASSERT(rv == 1)
}
TEST_END

TEST_START(clax_http_parse_saves_request)
{
    clax_http_request_t request;

    clax_http_init();

    clax_http_parse(&request, "GET /there?foo=bar HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n", 14 + 2 + 15 + 2 + 17 + 4);
    ASSERT(request.method == HTTP_GET)
    ASSERT(strcmp(request.url, "/there?foo=bar") == 0)
    ASSERT(strcmp(request.path_info, "/there") == 0)
}
TEST_END
