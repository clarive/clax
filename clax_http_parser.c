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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

#ifdef _WIN32
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "contrib/http-parser/http_parser.h"
#include "contrib/base64/base64.h"
#include "clax_http_parser.h"
#include "clax_http_util.h"
#include "clax_log.h"
#include "clax_util.h"
#include "clax_platform.h"

int on_http_parser_message_complete(http_parser *p)
{
    clax_http_request_t *request = p->data;

    const char *content_type = clax_kv_list_find(&request->headers, "Content-Type");

    if (content_type && strcmp(content_type, "application/x-www-form-urlencoded") == 0) {
        clax_log("Parsing x-www-form");
        clax_http_parse_urlencoded(&request->body_params, (char *)request->body, request->body_len);
    }

    if (request->body_tmpfile == NULL) {
        clax_log("Request parsing done");

        if (request->done_cb)
            request->done_cb(request);
    }
    else {
        clax_log("Waiting for body to be written (%d)", request->to_read);
    }

    return 0;
}

int on_http_parser_headers_complete(http_parser *p)
{
    clax_http_request_t *request = p->data;

    request->method = p->method;
    request->headers_done = 1;

    if (request->headers_done_cb)
        request->headers_done_cb(request);

    return 0;
}

int on_http_parser_header_field(http_parser *p, const char *buf, size_t len)
{
    clax_http_request_t *req = p->data;

    char *key = clax_buf2str(buf, len);

#ifdef MVS
    clax_atoe(key, strlen(key));
#endif

    clax_kv_list_push(&req->headers, key, "");

    free(key);

    return 0;
}

int on_http_parser_header_value(http_parser *p, const char *buf, size_t len)
{
    clax_http_request_t *req = p->data;

    clax_kv_list_item_t *item = clax_kv_list_at(&req->headers, req->headers.size - 1);

    char *key = item->key;
    char *val = clax_buf2str(buf, len);

#ifdef MVS
    clax_atoe(val, strlen(val));
#endif

    clax_kv_list_set(&req->headers, key, val);

    if (strcmp(key, "Content-Length") == 0) {
        req->content_length = atoi(val);
    }
    else if (strcmp(key, "Expect") == 0 && strcmp(val, "100-continue") == 0) {
        req->continue_expected = 1;
    }

    free(val);

    return 0;
}

int on_http_parser_url(http_parser *p, const char *buf, size_t len)
{
    clax_http_request_t *request = p->data;
    struct http_parser_url u;
    int rv;

    rv = http_parser_parse_url(buf, len, 0, &u);
    if (rv != 0)
        return -1;

    int path_from = u.field_data[UF_PATH].off;
    int path_len = u.field_data[UF_PATH].len;

    strncpy(request->url, buf, len);
    request->url[len] = 0;

#ifdef MVS
    clax_atoe(request->url, len);
#endif

    strncpy(request->path_info, buf + path_from, path_len);
    request->path_info[path_len] = 0;

#ifdef MVS
    clax_atoe(request->path_info, path_len);
#endif

    size_t path_info_len = clax_http_url_decode(request->path_info);

    /* path_info can have %00 -> \0, and we can have security problems */
    for (int i = 0; i < path_info_len; i++) {
        if (request->path_info[i] == 0) {
            return -1;
        }
    }

    if (u.field_set & (1 << UF_QUERY)) {
        int query_from = u.field_data[UF_QUERY].off;
        size_t query_len = u.field_data[UF_QUERY].len;

        if (query_len) {
            const char *b = buf + query_from;

#ifdef MVS
            b = clax_atoe_alloc(b, query_len);
#endif

            clax_http_parse_urlencoded(&request->query_params, b, query_len);

#ifdef MVS
            free((void *)b);
#endif
        }
    }

    return 0;
}

typedef struct {
    uv_fs_t req;
    uv_buf_t buf;
    void *data;
} clax_http_body_write_req_t;

void clax_http_request_body_closed_cb(uv_fs_t *req)
{
    clax_http_body_write_req_t *close_req = (clax_http_body_write_req_t *)req;

    clax_http_request_t *request = close_req->data;

    if (request && request->done_cb) {
        clax_log("Request body parsing done");
        request->done_cb(request);
    }
}

void clax_http_request_body_written_cb(uv_fs_t *req)
{
    clax_http_body_write_req_t *write_req = (clax_http_body_write_req_t *)req;

    if (req->result < 0) {
        clax_log("Error: writing to file: %s", uv_strerror(req->result));

        uv_fs_req_cleanup(req);

        if (write_req->buf.base) {
            free(write_req->buf.base);
        }
        free(write_req);

        return;
    }

    clax_http_request_t *request = write_req->data;

    request->to_read -= req->result;

    if (request->to_read <= 0) {
        clax_http_body_write_req_t *close_req = malloc(sizeof(clax_http_body_write_req_t));

        close_req->data = request;

        int r = uv_fs_close(uv_default_loop(), (uv_fs_t *)close_req, request->body_file, clax_http_request_body_closed_cb);

        if (r < 0) {
            clax_log("Error: closing file: %s", uv_strerror(r));
        }
    }

    uv_fs_req_cleanup(req);

    if (write_req->buf.base) {
        free(write_req->buf.base);
    }
    free(write_req);
}

int on_http_parser_body(http_parser *p, const char *buf, size_t len)
{
    clax_http_request_t *request = p->data;

    const char *content_type = clax_kv_list_find(&request->headers, "Content-Type");

    /* Do not worry about writing form into the file */
    if (content_type && strcmp(content_type, "application/x-www-form-urlencoded") == 0) {
        if (!request->body) {
            request->body = malloc(len + 1);
            memcpy(request->body, buf, len);
            request->body[len] = 0;
            request->body_len = len;
        }
        else {
            unsigned char *p = malloc(request->body_len + len + 1);
            memcpy(p, request->body, request->body_len);
            memcpy(p + request->body_len, buf, len);

            request->body_len += len;
            p[request->body_len] = 0;

            free(request->body);
            request->body = p;
        }
    }
    else {
        if (!request->body_file) {
            char tmpbuf[1024];
            size_t tmpsize = 1024;

            if (uv_os_tmpdir(tmpbuf, &tmpsize) == 0) {
                char *tmpdir = malloc(tmpsize + 1);
                memcpy(tmpdir, tmpbuf, tmpsize + 1);

                request->body_tmpfile = clax_mktmpfile_alloc(tmpdir, NULL);
                request->to_read = request->content_length;

                clax_log("Opening body tempfile %s", request->body_tmpfile);

                uv_fs_t open_req;

                /* We have to open file here synchronously because it's pretty hard to avoid racing between opening this
                 * file and getting new data, making this sync mitigates the problem and keeps code simpler */
                int r = uv_fs_open(uv_default_loop(), &open_req,
                        request->body_tmpfile, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, NULL);

                if (r < 0) {
                    clax_log("Error creating body tmpfile");

                    return -1;
                }

                request->body_file = open_req.result;
            }
            else {
                clax_log("Error: not tmpdir found");

                return -1;
            }
        }

        clax_http_body_write_req_t *write_req = malloc(sizeof(clax_http_body_write_req_t));

        if (write_req) {
            write_req->buf.base = malloc(len);
            write_req->buf.len = len;
            memcpy(write_req->buf.base, buf, len);

            write_req->data = request;
        }
        else {
            return -1;
        }

        int r = uv_fs_write(uv_default_loop(), (uv_fs_t *)write_req,
                request->body_file, &write_req->buf, 1, -1, NULL);

        clax_http_request_body_written_cb((uv_fs_t *)write_req);

        if (r < 0) {
            clax_log("Error: appending file: %s", uv_strerror(r));

            return -1;
        }
    }

//        char *b = (char *)buf;
//#ifdef MVS
//        b = clax_atoe_alloc(buf, len);
//#endif
//        clax_buf_append(&req->body, &req->body_len, b, len);
//#ifdef MVS
//        free((void *)b);
//#endif

    return 0;
}

static http_parser_settings http_parser_callbacks =
  {
  .on_header_field = on_http_parser_header_field
  ,.on_header_value = on_http_parser_header_value
  ,.on_url = on_http_parser_url
  ,.on_body = on_http_parser_body
  ,.on_headers_complete = on_http_parser_headers_complete
  ,.on_message_complete = on_http_parser_message_complete
  };

int clax_http_parse(http_parser *parser, clax_http_request_t *request, const char *buf, ssize_t len)
{
    size_t nparsed;

    parser->data = request;

    nparsed = http_parser_execute(parser, &http_parser_callbacks, buf, len);

    if (nparsed != len) {
        clax_log("Http parsing error");
        return -1;
    }

    return 0;
}

void clax_http_request_init(clax_http_request_t *request, char *tempdir)
{
    memset(request, 0, sizeof(clax_http_request_t));

    clax_kv_list_init(&request->headers);
    clax_kv_list_init(&request->query_params);
    clax_kv_list_init(&request->body_params);
}

void clax_http_request_free(clax_http_request_t *request)
{
    if (request->body) {
        free((void *)request->body);
        request->body = NULL;
    }

    clax_kv_list_free(&request->headers);
    clax_kv_list_free(&request->query_params);
    clax_kv_list_free(&request->body_params);

    if (request->body_tmpfile) {
        uv_fs_unlink(uv_default_loop(), NULL, request->body_tmpfile, NULL);

        free(request->body_tmpfile);
        request->body_tmpfile = NULL;
    }
}

void clax_http_response_init(clax_http_response_t *response, char *tempdir, size_t max_size)
{
    memset(response, 0, sizeof(clax_http_response_t));

    clax_kv_list_init(&response->headers);
}

void clax_http_response_free(clax_http_response_t *response)
{
    clax_kv_list_free(&response->headers);

    if (response->body_buf) {
        free((void *)response->body_buf);
        response->body_buf = NULL;
    }
}

int clax_http_is_proxy(clax_http_request_t *req)
{
    char *hops = clax_kv_list_find(&req->headers, "X-Hops");
    if (hops == NULL || strlen(hops) == 0) {
        return 0;
    }

    return 1;
}
