#include <stdio.h>
#include <string.h>
#include "clax_http.h"
#include "u.h"

int _parse(clax_http_request_t *req, const char *data)
{
    return clax_http_parse(req, data, strlen(data));
}

TEST_START(clax_http_parse_returns_error_when_error)
{
    clax_http_request_t request;

    clax_http_init();
    int rv = _parse(&request, "foobarbaz");

    ASSERT(rv == -1)
}
TEST_END

TEST_START(clax_http_parse_returns_0_when_need_more)
{
    clax_http_request_t request;

    clax_http_init();
    int rv = _parse(&request, "GET / HTTP/1.1\r\n");

    ASSERT(rv == 0)
}
TEST_END

TEST_START(clax_http_parse_returns_ok_chunks)
{
    clax_http_request_t request;

    clax_http_init();

    int rv = _parse(&request, "GET / ");
    ASSERT(rv == 0)

    rv = _parse(&request, "HTTP/1.1\r\n");
    ASSERT(rv == 0)

    rv = _parse(&request, "Host: localhost\r\nConnection: close\r\n\r\n");
    ASSERT(rv == 1)
}
TEST_END

TEST_START(clax_http_parse_returns_ok)
{
    clax_http_request_t request;

    clax_http_init();

    int rv = _parse(&request, "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n");
    ASSERT(rv == 1)
}
TEST_END

TEST_START(clax_http_parse_saves_request)
{
    clax_http_request_t request;
    memset(&request, 0, sizeof(clax_http_request_t));

    clax_http_init();

    _parse(&request, "GET /there?foo=bar HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n");

    ASSERT(request.is_complete)
    ASSERT(request.method == HTTP_GET)
    ASSERT_STR_EQ(request.url, "/there?foo=bar")
    ASSERT_STR_EQ(request.path_info, "/there")
    ASSERT_EQ(request.headers_num, 2)
    ASSERT_STR_EQ(request.headers[0].key, "Host")
    ASSERT_STR_EQ(request.headers[0].val, "localhost")
    ASSERT_STR_EQ(request.headers[1].key, "Connection")
    ASSERT_STR_EQ(request.headers[1].val, "close")
}
TEST_END

TEST_START(clax_http_parse_parses_form_body)
{
    char *p;
    clax_http_request_t request;
    memset(&request, 0, sizeof(clax_http_request_t));

    clax_http_init();

    _parse(&request, "POST / HTTP/1.1\r\n");
    _parse(&request, "Host: localhost\r\n");
    _parse(&request, "Content-Type: application/x-www-form-urlencoded\r\n");
    _parse(&request, "Content-Length: 32\r\n");
    _parse(&request, "\r\n");
    _parse(&request, "&&&&&foo&foo=&&&foo=bar&=bar&&&&");

    ASSERT_EQ(request.params_num, 4)
    ASSERT_STR_EQ(request.params[0].key, "foo")
    ASSERT_STR_EQ(request.params[0].val, "")
    ASSERT_STR_EQ(request.params[1].key, "foo")
    ASSERT_STR_EQ(request.params[1].val, "")
    ASSERT_STR_EQ(request.params[2].key, "foo")
    ASSERT_STR_EQ(request.params[2].val, "bar")
    ASSERT_STR_EQ(request.params[3].key, "")
    ASSERT_STR_EQ(request.params[3].val, "bar")
}
TEST_END

TEST_START(clax_http_parse_parses_form_body_with_decoding)
{
    char *p;
    clax_http_request_t request;
    memset(&request, 0, sizeof(clax_http_request_t));

    clax_http_init();

    _parse(&request, "POST / HTTP/1.1\r\n");
    _parse(&request, "Host: localhost\r\n");
    _parse(&request, "Content-Type: application/x-www-form-urlencoded\r\n");
    _parse(&request, "Content-Length: 32\r\n");
    _parse(&request, "\r\n");
    _parse(&request, "f%20o=b%2Fr+baz%");

    ASSERT_EQ(request.params_num, 1)
    ASSERT_STR_EQ(request.params[0].key, "f o")
    ASSERT_STR_EQ(request.params[0].val, "b/r baz%")
}
TEST_END

TEST_START(clax_http_url_decode_decodes_string_inplace)
{
    char buf[1024];

    strcpy(buf, "hello");
    clax_http_url_decode(buf);
    ASSERT_STR_EQ(buf, "hello")

    strcpy(buf, "f%20o");
    clax_http_url_decode(buf);
    ASSERT_STR_EQ(buf, "f o")

    strcpy(buf, "f+o");
    clax_http_url_decode(buf);
    ASSERT_STR_EQ(buf, "f o")

    strcpy(buf, "fo%");
    clax_http_url_decode(buf);
    ASSERT_STR_EQ(buf, "fo%")

    strcpy(buf, "fo%2");
    clax_http_url_decode(buf);
    ASSERT_STR_EQ(buf, "fo%2")

    strcpy(buf, "fo%20");
    clax_http_url_decode(buf);
    ASSERT_STR_EQ(buf, "fo ")

    strcpy(buf, "fo%20%20");
    clax_http_url_decode(buf);
    ASSERT_STR_EQ(buf, "fo  ")

    strcpy(buf, "fo%20%20bar");
    clax_http_url_decode(buf);
    ASSERT_STR_EQ(buf, "fo  bar")

    strcpy(buf, "while%20true%3B%20do%20echo%20foo%3B%20done");
    clax_http_url_decode(buf);
    ASSERT_STR_EQ(buf, "while true; do echo foo; done")
}
TEST_END

TEST_START(clax_http_status_message_returns_message)
{
    ASSERT_STR_EQ(clax_http_status_message(200), "OK")
}
TEST_END

TEST_START(clax_http_status_message_returns_unknown_message)
{
    ASSERT_STR_EQ(clax_http_status_message(999), "Unknown")
}
TEST_END
