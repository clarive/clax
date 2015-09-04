/*
 *  Copyright (C) 2015, Clarive Software, All Rights Reserved
 *
 *  This file is part of clax.
 *
 *  Clax is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Clax is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Clax.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "clax_http.h"
#include "u.h"

int _parse(http_parser *parser, clax_http_request_t *req, const char *data)
{
    return clax_http_parse(parser, req, data, strlen(data));
}

TEST_START(clax_http_parse_returns_error_when_error)
{
    http_parser parser;
    clax_http_request_t request;

    http_parser_init(&parser, HTTP_REQUEST);
    clax_http_request_init(&request);

    int rv = _parse(&parser, &request, "foobarbaz");

    ASSERT_EQ(rv, -1)

    clax_http_request_free(&request);
}
TEST_END

TEST_START(clax_http_parse_returns_0_when_need_more)
{
    http_parser parser;
    clax_http_request_t request;

    http_parser_init(&parser, HTTP_REQUEST);
    clax_http_request_init(&request);

    int rv = _parse(&parser, &request, "GET / HTTP/1.1\r\n");

    ASSERT_EQ(rv, 0)

    clax_http_request_free(&request);
}
TEST_END

TEST_START(clax_http_parse_returns_ok_chunks)
{
    http_parser parser;
    clax_http_request_t request;

    http_parser_init(&parser, HTTP_REQUEST);
    clax_http_request_init(&request);

    int rv = _parse(&parser, &request, "GET / ");
    ASSERT_EQ(request.headers_done, 0);
    ASSERT_EQ(request.message_done, 0);
    ASSERT_EQ(rv, 0)

    rv = _parse(&parser, &request, "HTTP/1.1\r\n");
    ASSERT_EQ(request.headers_done, 0);
    ASSERT_EQ(request.message_done, 0);
    ASSERT_EQ(rv, 0)

    rv = _parse(&parser, &request, "Host: localhost\r\nConnection: close\r\nContent-Length: 5\r\n");
    ASSERT_EQ(request.headers_done, 0);
    ASSERT_EQ(request.message_done, 0);
    ASSERT_EQ(rv, 0)

    rv = _parse(&parser, &request, "\r\n");
    ASSERT_EQ(request.headers_done, 1);
    ASSERT_EQ(request.message_done, 0);
    ASSERT_EQ(rv, 0)

    rv = _parse(&parser, &request, "hello");
    ASSERT_EQ(request.headers_done, 1);
    ASSERT_EQ(request.message_done, 1);
    ASSERT_EQ((int)request.body_len, 5);
    ASSERT_EQ(rv, 1)

    clax_http_request_free(&request);
}
TEST_END

TEST_START(clax_http_parse_returns_done_when_100_continue)
{
    http_parser parser;
    clax_http_request_t request;

    http_parser_init(&parser, HTTP_REQUEST);
    clax_http_request_init(&request);

    int rv = _parse(&parser, &request, "GET / HTTP/1.1\r\n");
    rv = _parse(&parser, &request, "Host: localhost\r\nConnection: close\r\nContent-Length: 5\r\nExpect: 100-continue\r\n\r\n");

    ASSERT_EQ(rv, 1)

    clax_http_request_free(&request);
}
TEST_END

TEST_START(clax_http_parse_returns_ok)
{
    http_parser parser;
    clax_http_request_t request;

    http_parser_init(&parser, HTTP_REQUEST);
    clax_http_request_init(&request);

    int rv = _parse(&parser, &request, "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n");
    ASSERT_EQ(rv, 1)

    clax_http_request_free(&request);
}
TEST_END

TEST_START(clax_http_parse_saves_request)
{
    http_parser parser;
    clax_http_request_t request;

    http_parser_init(&parser, HTTP_REQUEST);
    clax_http_request_init(&request);

    _parse(&parser, &request, "GET /there?foo=bar HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n");

    ASSERT(request.headers_done)
    ASSERT(request.message_done)
    ASSERT(request.method == HTTP_GET)
    ASSERT_STR_EQ(request.url, "/there?foo=bar")
    ASSERT_STR_EQ(request.path_info, "/there")

    ASSERT_STR_EQ(clax_kv_list_find(&request.headers, "Host"), "localhost");
    ASSERT_STR_EQ(clax_kv_list_find(&request.headers, "Connection"), "close");

    clax_http_request_free(&request);
}
TEST_END

TEST_START(clax_http_parse_saves_body)
{
    http_parser parser;
    clax_http_request_t request;

    http_parser_init(&parser, HTTP_REQUEST);
    clax_http_request_init(&request);

    _parse(&parser, &request, "POST / HTTP/1.1\r\n");
    _parse(&parser, &request, "Host: localhost\r\n");
    _parse(&parser, &request, "Content-Length: 32\r\n");
    _parse(&parser, &request, "\r\n");
    _parse(&parser, &request, "&&&&&foo&foo=&&&foo=bar&=bar&&&&");

    ASSERT_EQ(request.body_len, 32)
    ASSERT_BUF_EQ(request.body, "&&&&&foo&foo=&&&foo=bar&=bar&&&&", 32);

    clax_http_request_free(&request);
}
TEST_END

TEST_START(clax_http_parse_parses_query_string)
{
    http_parser parser;
    clax_http_request_t request;

    http_parser_init(&parser, HTTP_REQUEST);
    clax_http_request_init(&request);

    _parse(&parser, &request, "GET /?foo=&foo=&foo=bar&=bar HTTP/1.1\r\n");
    _parse(&parser, &request, "Host: localhost\r\n");
    _parse(&parser, &request, "Content-Length: 0\r\n");
    _parse(&parser, &request, "\r\n");

    ASSERT_EQ(request.query_params.size, 4);
    ASSERT_STR_EQ(clax_kv_list_at(&request.query_params, 0)->key, "foo");
    ASSERT_STR_EQ(clax_kv_list_at(&request.query_params, 0)->val, "");
    ASSERT_STR_EQ(clax_kv_list_at(&request.query_params, 1)->key, "foo");
    ASSERT_STR_EQ(clax_kv_list_at(&request.query_params, 1)->val, "");
    ASSERT_STR_EQ(clax_kv_list_at(&request.query_params, 2)->key, "foo");
    ASSERT_STR_EQ(clax_kv_list_at(&request.query_params, 2)->val, "bar");
    ASSERT_STR_EQ(clax_kv_list_at(&request.query_params, 3)->key, "");
    ASSERT_STR_EQ(clax_kv_list_at(&request.query_params, 3)->val, "bar");

    clax_http_request_free(&request);
}
TEST_END

TEST_START(clax_http_parse_parses_form_body)
{
    http_parser parser;
    clax_http_request_t request;

    http_parser_init(&parser, HTTP_REQUEST);
    clax_http_request_init(&request);

    _parse(&parser, &request, "POST / HTTP/1.1\r\n");
    _parse(&parser, &request, "Host: localhost\r\n");
    _parse(&parser, &request, "Content-Type: application/x-www-form-urlencoded\r\n");
    _parse(&parser, &request, "Content-Length: 32\r\n");
    _parse(&parser, &request, "\r\n");
    _parse(&parser, &request, "&&&&&foo&foo=&&&foo=bar&=bar&&&&");

    ASSERT_EQ(request.body_params.size, 4);
    ASSERT_STR_EQ(clax_kv_list_at(&request.body_params, 0)->key, "foo");
    ASSERT_STR_EQ(clax_kv_list_at(&request.body_params, 0)->val, "");
    ASSERT_STR_EQ(clax_kv_list_at(&request.body_params, 1)->key, "foo");
    ASSERT_STR_EQ(clax_kv_list_at(&request.body_params, 1)->val, "");
    ASSERT_STR_EQ(clax_kv_list_at(&request.body_params, 2)->key, "foo");
    ASSERT_STR_EQ(clax_kv_list_at(&request.body_params, 2)->val, "bar");
    ASSERT_STR_EQ(clax_kv_list_at(&request.body_params, 3)->key, "");
    ASSERT_STR_EQ(clax_kv_list_at(&request.body_params, 3)->val, "bar");

    clax_http_request_free(&request);
}
TEST_END

TEST_START(clax_http_parse_parses_form_body_with_decoding)
{
    http_parser parser;
    clax_http_request_t request;

    http_parser_init(&parser, HTTP_REQUEST);
    clax_http_request_init(&request);

    _parse(&parser, &request, "POST / HTTP/1.1\r\n");
    _parse(&parser, &request, "Host: localhost\r\n");
    _parse(&parser, &request, "Content-Type: application/x-www-form-urlencoded\r\n");
    _parse(&parser, &request, "Content-Length: 16\r\n");
    _parse(&parser, &request, "\r\n");
    _parse(&parser, &request, "f%20o=b%2Fr+baz%");

    clax_kv_list_dump(&request.body_params);

    ASSERT_EQ(request.body_params.size, 1);
    ASSERT_STR_EQ(clax_kv_list_find(&request.body_params, "f o"), "b/r baz%");

    clax_http_request_free(&request);
}
TEST_END

TEST_START(clax_http_parse_parses_multipart_body)
{
    http_parser parser;
    clax_http_request_t request;

    http_parser_init(&parser, HTTP_REQUEST);
    clax_http_request_init(&request);

    _parse(&parser, &request, "POST / HTTP/1.1\r\n");
    _parse(&parser, &request, "Host: localhost\r\n");
    _parse(&parser, &request, "Content-Type: multipart/form-data; boundary=------------------------7ca8ddb13928aa86\r\n");
    _parse(&parser, &request, "Content-Length: 482\r\n");
    _parse(&parser, &request, "\r\n");
    _parse(&parser, &request, "--------------------------7ca8ddb13928aa86\r\n");
    _parse(&parser, &request, "Content-Disposition: form-data; name=\"datafile1\"; filename=\"r.gif\"\r\n");
    _parse(&parser, &request, "Content-Type: image/gif\r\n");
    _parse(&parser, &request, "\r\n");
    _parse(&parser, &request, "foo");
    _parse(&parser, &request, "bar\r\n");
    _parse(&parser, &request, "--------------------------7ca8ddb13928aa86\r\n");
    _parse(&parser, &request, "Content-Disposition: form-data; name=\"datafile2\"; filename=\"g.gif\"\r\n");
    _parse(&parser, &request, "Content-Type: image/gif\r\n");
    _parse(&parser, &request, "\r\n");
    _parse(&parser, &request, "bar");
    _parse(&parser, &request, "baz\r\n");
    _parse(&parser, &request, "--------------------------7ca8ddb13928aa86\r\n");
    _parse(&parser, &request, "Content-Disposition: form-data; name=\"datafile3\"; filename=\"b.gif\"\r\n");
    _parse(&parser, &request, "Content-Type: image/gif\r\n");
    _parse(&parser, &request, "\r\n");
    _parse(&parser, &request, "end\r\n");
    _parse(&parser, &request, "--------------------------7ca8ddb13928aa86--\r\n");

    ASSERT_EQ(request.message_done, 1);

    ASSERT_STR_EQ(request.multipart_boundary, "--------------------------7ca8ddb13928aa86");
    ASSERT_EQ(request.multiparts_num, 3);
    ASSERT_EQ(request.multiparts[0].headers_num, 2);

    ASSERT_STR_EQ(request.multiparts[0].headers[0].key, "Content-Disposition");
    ASSERT_STR_EQ(request.multiparts[0].headers[0].val, "form-data; name=\"datafile1\"; filename=\"r.gif\"");
    ASSERT_STR_EQ(request.multiparts[0].headers[1].key, "Content-Type");
    ASSERT_STR_EQ(request.multiparts[0].headers[1].val, "image/gif");
    ASSERT_EQ(request.multiparts[0].part_len, 6);
    ASSERT_BUF_EQ(request.multiparts[0].part, "foobar", 6);

    ASSERT_STR_EQ(request.multiparts[1].headers[0].key, "Content-Disposition");
    ASSERT_STR_EQ(request.multiparts[1].headers[0].val, "form-data; name=\"datafile2\"; filename=\"g.gif\"");
    ASSERT_STR_EQ(request.multiparts[1].headers[1].key, "Content-Type");
    ASSERT_STR_EQ(request.multiparts[1].headers[1].val, "image/gif");
    ASSERT_EQ(request.multiparts[1].part_len, 6);
    ASSERT_BUF_EQ(request.multiparts[1].part, "barbaz", 6);

    ASSERT_STR_EQ(request.multiparts[2].headers[0].key, "Content-Disposition");
    ASSERT_STR_EQ(request.multiparts[2].headers[0].val, "form-data; name=\"datafile3\"; filename=\"b.gif\"");
    ASSERT_STR_EQ(request.multiparts[2].headers[1].key, "Content-Type");
    ASSERT_STR_EQ(request.multiparts[2].headers[1].val, "image/gif");
    ASSERT_EQ(request.multiparts[2].part_len, 3);
    ASSERT_BUF_EQ(request.multiparts[2].part, "end", 3);

    clax_http_request_free(&request);
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

TEST_START(clax_http_extract_kv_extracts_val)
{
    const char *val;
    size_t len;

    val = clax_http_extract_kv("name=\"upload\"; filename=\"file.zip\"", "name", &len);
    ASSERT_STRN_EQ(val, "upload", (int)len);
    ASSERT_EQ((int)len, 6);

    val = clax_http_extract_kv("name=\"upload\"; filename=\"file.zip\"", "filename", &len);
    ASSERT_STRN_EQ(val, "file.zip", (int)len);
    ASSERT_EQ((int)len, 8);

    val = clax_http_extract_kv("name=\"upload\"; filename=\"file.zip\"", "something", &len);
    ASSERT(val == NULL);
    ASSERT_EQ((int)len, 0);
}
TEST_END

char test_send_cb_buf[1024] = {0};
size_t test_send_cb_buf_len = 0;
size_t test_send_cb(void *ctx, const unsigned char *buf, size_t len)
{
    memcpy(test_send_cb_buf + test_send_cb_buf_len, buf, len);

    test_send_cb_buf_len += len;

    return len;
}

size_t clax_http_chunked_va(char *buf, size_t len, ...)
{
    va_list a_list;
    size_t ret;

    va_start(a_list, len);

    ret = clax_http_chunked(buf, len, a_list);

    va_end(a_list);

    return ret;
}

TEST_START(clax_http_chunked_writes_chunks)
{
    test_send_cb_buf_len = 0;

    clax_http_chunked_va("hello", 5, test_send_cb, NULL);
    ASSERT_EQ(test_send_cb_buf_len, 10);
    ASSERT_BUF_EQ(test_send_cb_buf, "5\r\nhello\r\n", 10);

    test_send_cb_buf_len = 0;

    clax_http_chunked_va(NULL, 0, test_send_cb, NULL);
    ASSERT_EQ(test_send_cb_buf_len, 5);
    ASSERT_BUF_EQ(test_send_cb_buf, "0\r\n\r\n", 5);

    test_send_cb_buf_len = 0;

    clax_http_chunked_va("Foo: bar\r\nBar: baz", 0, test_send_cb, NULL);
    ASSERT_EQ(test_send_cb_buf_len, 25);
    ASSERT_BUF_EQ(test_send_cb_buf, "0\r\nFoo: bar\r\nBar: baz\r\n\r\n", 25);
}
TEST_END
