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

#include "contrib/u/u.h"

#include "contrib/base64/base64.h"
#include "clax_http.h"

int _parse(http_parser *parser, clax_http_request_t *req, const char *data)
{
    int ret;
    const char *p = data;

#ifdef MVS
    p = clax_etoa_alloc(data, strlen(data) + 1);
#endif

    ret = clax_http_parse(parser, req, p, strlen(p));

#ifdef MVS
    free(p);
#endif

    return ret;
}

char test_send_cb_buf[1024] = {0};
size_t test_send_cb_buf_len = 0;
int test_send_cb(void *ctx, const unsigned char *buf, size_t len)
{
    const unsigned char *b = buf;

/*#ifdef MVS*/
    /*b = clax_atoe_alloc(buf, len);*/
/*#endif*/

    memcpy(test_send_cb_buf + test_send_cb_buf_len, b, len);

    test_send_cb_buf_len += len;

/*#ifdef MVS*/
    /*free(b);*/
/*#endif*/

    return len;
}

/*size_t clax_http_chunked_va(char *buf, size_t len, ...)*/
/*{*/
    /*va_list a_list;*/
    /*size_t ret;*/

    /*va_start(a_list, len);*/

    /*ret = clax_http_chunked(buf, len, a_list);*/

    /*va_end(a_list);*/

    /*return ret;*/
/*}*/

SUITE_START(clax_http)

/*TEST_START(parse_returns_error_when_error)*/
/*{*/
    /*http_parser parser;*/
    /*clax_http_request_t request;*/

    /*http_parser_init(&parser, HTTP_REQUEST);*/
    /*clax_http_request_init(&request, NULL);*/

    /*int rv = _parse(&parser, &request, "foobarbaz");*/

    /*ASSERT_EQ(rv, -1)*/

    /*clax_http_request_free(&request);*/
/*}*/
/*TEST_END*/

/*TEST_START(parse_returns_0_when_need_more)*/
/*{*/
    /*http_parser parser;*/
    /*clax_http_request_t request;*/

    /*http_parser_init(&parser, HTTP_REQUEST);*/
    /*clax_http_request_init(&request, NULL);*/

    /*int rv = _parse(&parser, &request, "GET / HTTP/1.1\r\n");*/

    /*ASSERT_EQ(rv, 0)*/

    /*clax_http_request_free(&request);*/
/*}*/
/*TEST_END*/

/*TEST_START(parse_returns_ok_chunks)*/
/*{*/
    /*http_parser parser;*/
    /*clax_http_request_t request;*/

    /*http_parser_init(&parser, HTTP_REQUEST);*/
    /*clax_http_request_init(&request, NULL);*/

    /*int rv = _parse(&parser, &request, "GET / ");*/
    /*ASSERT_EQ(request.headers_done, 0);*/
    /*ASSERT_EQ(request.message_done, 0);*/
    /*ASSERT_EQ(rv, 0)*/

    /*rv = _parse(&parser, &request, "HTTP/1.1\r\n");*/
    /*ASSERT_EQ(request.headers_done, 0);*/
    /*ASSERT_EQ(request.message_done, 0);*/
    /*ASSERT_EQ(rv, 0)*/

    /*rv = _parse(&parser, &request, "Host: localhost\r\nConnection: close\r\nContent-Length: 5\r\n");*/
    /*ASSERT_EQ(request.headers_done, 0);*/
    /*ASSERT_EQ(request.message_done, 0);*/
    /*ASSERT_EQ(rv, 0)*/

    /*rv = _parse(&parser, &request, "\r\n");*/
    /*ASSERT_EQ(request.headers_done, 1);*/
    /*ASSERT_EQ(request.message_done, 0);*/
    /*ASSERT_EQ(rv, 0)*/

    /*rv = _parse(&parser, &request, "hello");*/
    /*ASSERT_EQ(request.headers_done, 1);*/
    /*ASSERT_EQ(request.message_done, 1);*/
    /*ASSERT_EQ((int)request.body_len, 5);*/
    /*ASSERT_EQ(rv, 1)*/

    /*clax_http_request_free(&request);*/
/*}*/
/*TEST_END*/

/*TEST_START(parse_returns_done_when_100_continue)*/
/*{*/
    /*http_parser parser;*/
    /*clax_http_request_t request;*/

    /*http_parser_init(&parser, HTTP_REQUEST);*/
    /*clax_http_request_init(&request, NULL);*/

    /*int rv = _parse(&parser, &request, "GET / HTTP/1.1\r\n");*/
    /*rv = _parse(&parser, &request, "Host: localhost\r\nConnection: close\r\nContent-Length: 5\r\nExpect: 100-continue\r\n\r\n");*/

    /*ASSERT_EQ(rv, 1)*/

    /*clax_http_request_free(&request);*/
/*}*/
/*TEST_END*/

/*TEST_START(parse_returns_ok)*/
/*{*/
    /*http_parser parser;*/
    /*clax_http_request_t request;*/

    /*http_parser_init(&parser, HTTP_REQUEST);*/
    /*clax_http_request_init(&request, NULL);*/

    /*int rv = _parse(&parser, &request, "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n");*/
    /*ASSERT_EQ(rv, 1)*/

    /*clax_http_request_free(&request);*/
/*}*/
/*TEST_END*/

/*TEST_START(parse_returns_error_when_path_has_zeros)*/
/*{*/
    /*http_parser parser;*/
    /*clax_http_request_t request;*/

    /*http_parser_init(&parser, HTTP_REQUEST);*/
    /*clax_http_request_init(&request, NULL);*/

    /*int rv = _parse(&parser, &request, "GET /%00 HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n");*/
    /*ASSERT_EQ(rv, -1)*/

    /*clax_http_request_free(&request);*/
/*}*/
/*TEST_END*/

/*TEST_START(parse_saves_request)*/
/*{*/
    /*http_parser parser;*/
    /*clax_http_request_t request;*/

    /*http_parser_init(&parser, HTTP_REQUEST);*/
    /*clax_http_request_init(&request, NULL);*/

    /*_parse(&parser, &request, "GET /the%20re?foo=bar HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n");*/

    /*ASSERT(request.headers_done)*/
    /*ASSERT(request.message_done)*/
    /*ASSERT(request.method == HTTP_GET)*/
    /*ASSERT_STR_EQ(request.url, "/the%20re?foo=bar")*/
    /*ASSERT_STR_EQ(request.path_info, "/the re")*/

    /*ASSERT_STR_EQ(clax_kv_list_find(&request.headers, "Host"), "localhost");*/
    /*ASSERT_STR_EQ(clax_kv_list_find(&request.headers, "Connection"), "close");*/

    /*clax_http_request_free(&request);*/
/*}*/
/*TEST_END*/

/*TEST_START(parse_saves_body)*/
/*{*/
    /*http_parser parser;*/
    /*clax_http_request_t request;*/

    /*http_parser_init(&parser, HTTP_REQUEST);*/
    /*clax_http_request_init(&request, NULL);*/

    /*_parse(&parser, &request, "POST / HTTP/1.1\r\n");*/
    /*_parse(&parser, &request, "Host: localhost\r\n");*/
    /*_parse(&parser, &request, "Content-Length: 32\r\n");*/
    /*_parse(&parser, &request, "\r\n");*/
    /*_parse(&parser, &request, "&&&&&foo&foo=&&&foo=bar&=bar&&&&");*/

    /*ASSERT_EQ(request.body_len, 32)*/
    /*ASSERT_BUF_EQ(request.body, "&&&&&foo&foo=&&&foo=bar&=bar&&&&", 32);*/

    /*clax_http_request_free(&request);*/
/*}*/
/*TEST_END*/

/*TEST_START(parse_parses_query_string)*/
/*{*/
    /*http_parser parser;*/
    /*clax_http_request_t request;*/

    /*http_parser_init(&parser, HTTP_REQUEST);*/
    /*clax_http_request_init(&request, NULL);*/

    /*_parse(&parser, &request, "GET /?foo=&foo=&foo=bar&=bar HTTP/1.1\r\n");*/
    /*_parse(&parser, &request, "Host: localhost\r\n");*/
    /*_parse(&parser, &request, "Content-Length: 0\r\n");*/
    /*_parse(&parser, &request, "\r\n");*/

    /*ASSERT_EQ(request.query_params.size, 4);*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&request.query_params, 0)->key, "foo");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&request.query_params, 0)->val, "");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&request.query_params, 1)->key, "foo");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&request.query_params, 1)->val, "");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&request.query_params, 2)->key, "foo");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&request.query_params, 2)->val, "bar");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&request.query_params, 3)->key, "");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&request.query_params, 3)->val, "bar");*/

    /*clax_http_request_free(&request);*/
/*}*/
/*TEST_END*/

/*TEST_START(parse_parses_form_body)*/
/*{*/
    /*http_parser parser;*/
    /*clax_http_request_t request;*/

    /*http_parser_init(&parser, HTTP_REQUEST);*/
    /*clax_http_request_init(&request, NULL);*/

    /*_parse(&parser, &request, "POST / HTTP/1.1\r\n");*/
    /*_parse(&parser, &request, "Host: localhost\r\n");*/
    /*_parse(&parser, &request, "Content-Type: application/x-www-form-urlencoded\r\n");*/
    /*_parse(&parser, &request, "Content-Length: 32\r\n");*/
    /*_parse(&parser, &request, "\r\n");*/
    /*_parse(&parser, &request, "&&&&&foo&foo=&&&foo=bar&=bar&&&&");*/

    /*ASSERT_EQ(request.body_params.size, 4);*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&request.body_params, 0)->key, "foo");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&request.body_params, 0)->val, "");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&request.body_params, 1)->key, "foo");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&request.body_params, 1)->val, "");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&request.body_params, 2)->key, "foo");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&request.body_params, 2)->val, "bar");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&request.body_params, 3)->key, "");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&request.body_params, 3)->val, "bar");*/

    /*clax_http_request_free(&request);*/
/*}*/
/*TEST_END*/

/*TEST_START(parse_parses_form_body_with_decoding)*/
/*{*/
    /*http_parser parser;*/
    /*clax_http_request_t request;*/

    /*http_parser_init(&parser, HTTP_REQUEST);*/
    /*clax_http_request_init(&request, NULL);*/

    /*_parse(&parser, &request, "POST / HTTP/1.1\r\n");*/
    /*_parse(&parser, &request, "Host: localhost\r\n");*/
    /*_parse(&parser, &request, "Content-Type: application/x-www-form-urlencoded\r\n");*/
    /*_parse(&parser, &request, "Content-Length: 16\r\n");*/
    /*_parse(&parser, &request, "\r\n");*/
    /*_parse(&parser, &request, "f%20o=b%2Fr+baz%");*/

    /*ASSERT_EQ(request.body_params.size, 1);*/
    /*ASSERT_STR_EQ(clax_kv_list_find(&request.body_params, "f o"), "b/r baz%");*/

    /*clax_http_request_free(&request);*/
/*}*/
/*TEST_END*/

/*TEST_START(parse_parses_multipart_body)*/
/*{*/
    /*http_parser parser;*/
    /*clax_http_request_t request;*/
    /*unsigned char content[1024];*/

    /*http_parser_init(&parser, HTTP_REQUEST);*/
    /*clax_http_request_init(&request, NULL);*/

    /*_parse(&parser, &request, "POST / HTTP/1.1\r\n");*/
    /*_parse(&parser, &request, "Host: localhost\r\n");*/
    /*_parse(&parser, &request, "Content-Type: multipart/form-data; boundary=------------------------7ca8ddb13928aa86\r\n");*/
    /*_parse(&parser, &request, "Content-Length: 482\r\n");*/
    /*_parse(&parser, &request, "\r\n");*/
    /*_parse(&parser, &request, "--------------------------7ca8ddb13928aa86\r\n");*/
    /*_parse(&parser, &request, "Content-Disposition: form-data; name=\"datafile1\"; filename=\"r.gif\"\r\n");*/
    /*_parse(&parser, &request, "Content-Type: image/gif\r\n");*/
    /*_parse(&parser, &request, "\r\n");*/
    /*clax_http_parse(&parser, &request, "\xaa\xbb\xcc", 3);*/
    /*clax_http_parse(&parser, &request, "\xdd\xee\xff", 3);*/
    /*_parse(&parser, &request, "\r\n");*/
    /*_parse(&parser, &request, "--------------------------7ca8ddb13928aa86\r\n");*/
    /*_parse(&parser, &request, "Content-Disposition: form-data; name=\"datafile2\"; filename=\"g.gif\"\r\n");*/
    /*_parse(&parser, &request, "Content-Type: image/gif\r\n");*/
    /*_parse(&parser, &request, "\r\n");*/
    /*clax_http_parse(&parser, &request, "\xdd\xee\xff", 3);*/
    /*clax_http_parse(&parser, &request, "\xda\xea\xfa", 3);*/
    /*_parse(&parser, &request, "\r\n");*/
    /*_parse(&parser, &request, "--------------------------7ca8ddb13928aa86\r\n");*/
    /*_parse(&parser, &request, "Content-Disposition: form-data; name=\"datafile3\"; filename=\"b.gif\"\r\n");*/
    /*_parse(&parser, &request, "Content-Type: image/gif\r\n");*/
    /*_parse(&parser, &request, "\r\n");*/
    /*clax_http_parse(&parser, &request, "\xff\xff\xff", 3);*/
    /*_parse(&parser, &request, "\r\n");*/
    /*_parse(&parser, &request, "--------------------------7ca8ddb13928aa86--\r\n");*/

    /*ASSERT_EQ(request.message_done, 1);*/

    /*ASSERT_STR_EQ(request.multipart_boundary, "--------------------------7ca8ddb13928aa86");*/
    /*ASSERT_EQ(request.multiparts.size, 3);*/

    /*clax_http_multipart_t *multipart;*/

    /*multipart = clax_http_multipart_list_at(&request.multiparts, 0);*/

    /*ASSERT_EQ(multipart->headers.size, 2);*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&multipart->headers, 0)->key, "Content-Disposition");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&multipart->headers, 0)->val, "form-data; name=\"datafile1\"; filename=\"r.gif\"");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&multipart->headers, 1)->key, "Content-Type");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&multipart->headers, 1)->val, "image/gif");*/

    /*clax_big_buf_read(&multipart->bbuf, content, sizeof(content), 0);*/
    /*ASSERT_EQ(multipart->bbuf.len, 6);*/
    /*ASSERT_BUF_EQ(content, "\xaa\xbb\xcc\xdd\xee\xff", 6);*/

    /*multipart = clax_http_multipart_list_at(&request.multiparts, 1);*/

    /*ASSERT_EQ(multipart->headers.size, 2);*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&multipart->headers, 0)->key, "Content-Disposition");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&multipart->headers, 0)->val, "form-data; name=\"datafile2\"; filename=\"g.gif\"");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&multipart->headers, 1)->key, "Content-Type");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&multipart->headers, 1)->val, "image/gif");*/

    /*clax_big_buf_read(&multipart->bbuf, content, sizeof(content), 0);*/
    /*ASSERT_EQ(multipart->bbuf.len, 6);*/
    /*ASSERT_BUF_EQ(content, "\xdd\xee\xff\xda\xea\xfa", 6);*/

    /*multipart = clax_http_multipart_list_at(&request.multiparts, 2);*/

    /*ASSERT_EQ(multipart->headers.size, 2);*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&multipart->headers, 0)->key, "Content-Disposition");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&multipart->headers, 0)->val, "form-data; name=\"datafile3\"; filename=\"b.gif\"");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&multipart->headers, 1)->key, "Content-Type");*/
    /*ASSERT_STR_EQ(clax_kv_list_at(&multipart->headers, 1)->val, "image/gif");*/

    /*clax_big_buf_read(&multipart->bbuf, content, sizeof(content), 0);*/
    /*ASSERT_EQ(multipart->bbuf.len, 3);*/
    /*ASSERT_BUF_EQ(content, "\xff\xff\xff", 3);*/

    /*clax_http_request_free(&request);*/
/*}*/
/*TEST_END*/

/*TEST_START(url_decode_decodes_string_inplace)*/
/*{*/
    /*char buf[1024];*/

    /*ASSERT_EQ(clax_http_url_decode(NULL), 0);*/

    /*strcpy(buf, "hello");*/
    /*ASSERT_EQ(clax_http_url_decode(buf), 5);*/
    /*ASSERT_STR_EQ(buf, "hello")*/

    /*strcpy(buf, "f%20o");*/
    /*ASSERT_EQ(clax_http_url_decode(buf), 3);*/
    /*ASSERT_STR_EQ(buf, "f o")*/

    /*strcpy(buf, "f+o");*/
    /*ASSERT_EQ(clax_http_url_decode(buf), 3);*/
    /*ASSERT_STR_EQ(buf, "f o")*/

    /*strcpy(buf, "fo%");*/
    /*ASSERT_EQ(clax_http_url_decode(buf), 3);*/
    /*ASSERT_STR_EQ(buf, "fo%")*/

    /*strcpy(buf, "fo%2");*/
    /*ASSERT_EQ(clax_http_url_decode(buf), 4);*/
    /*ASSERT_STR_EQ(buf, "fo%2")*/

    /*strcpy(buf, "fo%20");*/
    /*ASSERT_EQ(clax_http_url_decode(buf), 3);*/
    /*ASSERT_STR_EQ(buf, "fo ")*/

    /*strcpy(buf, "fo%20%20");*/
    /*ASSERT_EQ(clax_http_url_decode(buf), 4);*/
    /*ASSERT_STR_EQ(buf, "fo  ")*/

    /*strcpy(buf, "fo%20%20bar");*/
    /*ASSERT_EQ(clax_http_url_decode(buf), 7);*/
    /*ASSERT_STR_EQ(buf, "fo  bar")*/

    /*strcpy(buf, "while%20true%3B%20do%20echo%20foo%3B%20done");*/
    /*ASSERT_EQ(clax_http_url_decode(buf), 29);*/
    /*ASSERT_STR_EQ(buf, "while true; do echo foo; done")*/

    /*strcpy(buf, "null%00byte");*/
    /*ASSERT_EQ(clax_http_url_decode(buf), 9);*/
    /*ASSERT_STR_EQ(buf, "null\0byte")*/
/*}*/
/*TEST_END*/

/*TEST_START(status_message_returns_message)*/
/*{*/
    /*ASSERT_STR_EQ(clax_http_status_message(200), "OK")*/
/*}*/
/*TEST_END*/

/*TEST_START(status_message_returns_unknown_message)*/
/*{*/
    /*ASSERT_STR_EQ(clax_http_status_message(999), "Unknown")*/
/*}*/
/*TEST_END*/

/*TEST_START(extract_kv_extracts_val)*/
/*{*/
    /*const char *val;*/
    /*size_t len;*/

    /*val = clax_http_extract_kv("name=\"upload\"; filename=\"file.zip\"", "name", &len);*/
    /*ASSERT_STRN_EQ(val, "upload", (int)len);*/
    /*ASSERT_EQ((int)len, 6);*/

    /*val = clax_http_extract_kv("name=\"upload\"; filename=\"file.zip\"", "filename", &len);*/
    /*ASSERT_STRN_EQ(val, "file.zip", (int)len);*/
    /*ASSERT_EQ((int)len, 8);*/

    /*val = clax_http_extract_kv("name=\"upload\"; filename=\"file.zip\"", "something", &len);*/
    /*ASSERT(val == NULL);*/
    /*ASSERT_EQ((int)len, 0);*/
/*}*/
/*TEST_END*/

/*TEST_START(chunked_writes_chunks)*/
/*{*/
    /*test_send_cb_buf_len = 0;*/

    /*clax_http_chunked_va("hello", 5, test_send_cb, NULL);*/
    /*ASSERT_EQ(test_send_cb_buf_len, 10);*/
    /*ASSERT_BUF_EQ(test_send_cb_buf, "5\r\nhello\r\n", 10);*/

    /*test_send_cb_buf_len = 0;*/

    /*clax_http_chunked_va(NULL, 0, test_send_cb, NULL);*/
    /*ASSERT_EQ(test_send_cb_buf_len, 5);*/
    /*ASSERT_BUF_EQ(test_send_cb_buf, "0\r\n\r\n", 5);*/

    /*test_send_cb_buf_len = 0;*/

    /*clax_http_chunked_va("Foo: bar\r\nBar: baz", 0, test_send_cb, NULL);*/
    /*ASSERT_EQ(test_send_cb_buf_len, 25);*/
    /*ASSERT_BUF_EQ(test_send_cb_buf, "0\r\nFoo: bar\r\nBar: baz\r\n\r\n", 25);*/
/*}*/
/*TEST_END*/

/*TEST_START(write_response_writes)*/
/*{*/
    /*clax_http_response_t response;*/
    /*test_send_cb_buf_len = 0;*/

    /*clax_http_response_init(&response, NULL, 0);*/

    /*response.status_code = 200;*/
    /*clax_http_write_response(NULL, test_send_cb, &response);*/

    /*ASSERT_EQ(test_send_cb_buf_len, 19);*/
    /*ASSERT_BUF_EQ(test_send_cb_buf, "HTTP/1.1 200 OK\r\n\r\n", 19);*/

    /*clax_http_response_free(&response);*/
/*}*/
/*TEST_END*/

/*TEST_START(check_basic_auth_checks_auth)*/
/*{*/
    /*char *header = NULL;*/
    /*char *base64 = NULL;*/

    /*base64_encode_alloc("clax:password", 13, &base64);*/
    /*header = clax_strjoin("", "Basic ", base64, NULL);*/
    /*ASSERT_EQ(clax_http_check_basic_auth(header, "clax", "password"), 1);*/
    /*free(header);*/
    /*free(base64);*/

    /*ASSERT_EQ(clax_http_check_basic_auth(NULL, "clax", "password"), 0);*/
    /*ASSERT_EQ(clax_http_check_basic_auth("", "clax", "password"), 0);*/
    /*ASSERT_EQ(clax_http_check_basic_auth("invalid", "clax", "password"), 0);*/

    /*ASSERT_EQ(clax_http_check_basic_auth("Basic invalid", "clax", "password"), 0);*/

    /*ASSERT_EQ(clax_http_check_basic_auth("Basic Zm9vYmFy", "clax", "password"), 0);*/

    /*base64_encode_alloc("cla:password", 13, &base64);*/
    /*header = clax_strjoin("", "Basic ", base64, NULL);*/
    /*ASSERT_EQ(clax_http_check_basic_auth(header, "clax", "password"), 0);*/
    /*free(header);*/
    /*free(base64);*/

    /*base64_encode_alloc("clax:passwor", 13, &base64);*/
    /*header = clax_strjoin("", "Basic ", base64, NULL);*/
    /*ASSERT_EQ(clax_http_check_basic_auth(header, "clax", "password"), 0);*/
    /*free(header);*/
    /*free(base64);*/
/*}*/
/*TEST_END*/

SUITE_END
