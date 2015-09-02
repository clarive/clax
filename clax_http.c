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
#include <strings.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

#include "http_parser/http_parser.h"
#include "multipart_parser.h"
#include "clax.h"
#include "clax_dispatcher.h"
#include "clax_http.h"
#include "clax_log.h"
#include "clax_http.h"
#include "clax_util.h"

enum {
    PARAM_MODE_KEY,
    PARAM_MODE_VAL
};

struct clax_http_status_message_s {
    int code;
    const char *message;
};
struct clax_http_status_message_s clax_http_messages[] = {
    {100, "Continue"},
    {200, "OK"},
    {400, "Bad Request"},
    {404, "Not Found"}
};

const char *clax_http_status_message(int code)
{
    int i;
    size_t len = sizeof clax_http_messages / sizeof clax_http_messages[0];

    for (i = 0; i < len; i++) {
        if (clax_http_messages[i].code == code) {
            return clax_http_messages[i].message;
        }
    }

    return "Unknown";
}

int message_complete_cb(http_parser *p)
{
    clax_http_request_t *req = p->data;

    req->message_done = 1;

    return 0;
}

int headers_complete_cb(http_parser *p)
{
    clax_http_request_t *req = p->data;

    req->method = p->method;
    req->headers_done = 1;

    const char *content_type = clax_http_header_get(req->headers, req->headers_num, "Content-Type");
    if (content_type && strncmp(content_type, "multipart/form-data; boundary=", 30) == 0) {
        size_t boundary_len = strlen(content_type) - 30;
        size_t tocopy = MIN(sizeof_struct_member(clax_http_request_t, multipart_boundary) - 2, boundary_len);

        strcpy(req->multipart_boundary, "--");
        strncpy(req->multipart_boundary + 2, content_type + 30, tocopy);
    }

    return 0;
}

int header_field_cb(http_parser *p, const char *buf, size_t len)
{
    clax_http_request_t *req = p->data;

    if (req->headers_num < MAX_HEADERS) {
        clax_http_request_t *req = p->data;

        strncpy(req->headers[req->headers_num].key, buf, len);
    }

    return 0;
}

int header_value_cb(http_parser *p, const char *buf, size_t len)
{
    clax_http_request_t *req = p->data;

    if (req->headers_num < MAX_HEADERS) {
        clax_http_request_t *req = p->data;

        char *key = req->headers[req->headers_num].key;
        char *val = req->headers[req->headers_num].val;

        strncpy(val, buf, len);

        if (strcmp(key, "Content-Length") == 0) {
            req->content_length = atoi(val);
        }
        else if (strcmp(key, "Expect") == 0 && strcmp(val, "100-continue") == 0) {
            req->continue_expected = 1;
        }

        req->headers_num++;
    }

    return 0;
}

int request_url_cb(http_parser *p, const char *buf, size_t len)
{
    clax_http_request_t *request = p->data;
    struct http_parser_url u;
    int rv;

    rv = http_parser_parse_url(buf, len, 0, &u);
    if (rv != 0)
        return -1;

    int path_from = u.field_data[3].off;
    int path_len = u.field_data[3].len;

    strncpy(request->url, buf, len);
    request->url[len] = 0;

    strncpy(request->path_info, buf + path_from, path_len);
    request->path_info[path_len] = 0;

    return 0;
}

void save_param(clax_http_request_t *req, const char *key, size_t key_len, const char *val, size_t val_len)
{
    if (key_len || val_len) {
        if (req->params_num < MAX_PARAMS) {
            if (key_len == 0)
                key = "";
            if (val_len == 0)
                val = "";

            strncpy(req->params[req->params_num].key, key, MIN(key_len, MAX_ELEMENT_SIZE));
            strncpy(req->params[req->params_num].val, val, MIN(val_len, MAX_ELEMENT_SIZE));

            clax_http_url_decode(req->params[req->params_num].key);
            clax_http_url_decode(req->params[req->params_num].val);

            req->params_num++;
        }
    }
}

void clax_http_parse_form(clax_http_request_t *req, const char *buf, size_t len)
{
    const char *key = NULL;
    size_t key_len = 0;
    const char *val = NULL;
    size_t val_len = 0;
    char mode = PARAM_MODE_KEY;

    int j;
    for (j = 0; j < len; j++) {
        if (buf[j] == '&') {
            mode = PARAM_MODE_KEY;

            save_param(req, key, key_len, val, val_len);

            key = NULL;
            key_len = 0;
            val = NULL;
            val_len = 0;
        }
        else if (buf[j] == '=') {
            mode = PARAM_MODE_VAL;
        }
        else if (mode == PARAM_MODE_KEY) {
            if (key == NULL)
                key = buf + j;

            key_len++;
        }
        else if (mode == PARAM_MODE_VAL) {
            if (val == NULL)
                val = buf + j;

            val_len++;
        }
    }

    save_param(req, key, key_len, val, val_len);
}

int on_multipart_header_name(multipart_parser* p, const char *buf, size_t len)
{
    clax_http_request_t *request = multipart_parser_get_data(p);

    clax_log("header %.*s", len, buf);

    if (request->multiparts_num < MAX_MULTIPARTS) {
        clax_http_multipart_t *multipart = &request->multiparts[request->multiparts_num];

        if (multipart->headers_num < MAX_HEADERS) {
            strncpy(multipart->headers[multipart->headers_num].key, buf, len);
        }
    }

    return 0;
}

int on_multipart_header_value(multipart_parser* p, const char *buf, size_t len)
{
    clax_http_request_t *request = multipart_parser_get_data(p);

    clax_log("value %.*s", len, buf);

    if (request->multiparts_num < MAX_MULTIPARTS) {
        clax_http_multipart_t *multipart = &request->multiparts[request->multiparts_num];

        if (multipart->headers_num < MAX_HEADERS) {
            strncpy(multipart->headers[multipart->headers_num].val, buf, len);
            multipart->headers_num++;
        }
    }

    return 0;
}

void append_str(unsigned char **str, size_t *olen, const char *buf, size_t len)
{
    if (!*str) {
        *str = (unsigned char *)malloc(sizeof(char) * len);
        memcpy((void *)*str, (const void*)buf, len);
        *olen = len;
    }
    else {
        *str = (unsigned char *)realloc((void *)*str, sizeof(char) * *olen + len);
        memcpy((void *)(*str + *olen), (const void*)buf, len);
        *olen += len;
    }
}

int on_part_data(multipart_parser* p, const char *buf, size_t len)
{
    clax_http_request_t *request = multipart_parser_get_data(p);

    if (request->multiparts_num < MAX_MULTIPARTS) {
        clax_http_multipart_t *multipart = &request->multiparts[request->multiparts_num];

        if (multipart->part_len + len > 1024 * 1024) {
            if (!multipart->part_fh) {
                int fd;
                const char *template = ".fileXXXXXX";
                char fpath[1024] = {0};
                strncat(fpath, request->clax_ctx->options->root, sizeof(fpath) - strlen(template));
                strcat(fpath, template);
                fd = mkstemp(fpath);
                if (fd < 0) return -1;
                close(fd);

                clax_log("Part is too big, saving to file '%s'", fpath);

                multipart->part_fh = fopen(fpath, "wb");

                if (multipart->part_fh == NULL) {
                    clax_log("Creating file '%s' failed", fpath);
                    return -1;
                }

                strcpy(multipart->part_fpath, fpath);

                if (multipart->part_len) {
                    size_t wcount = fwrite(multipart->part, 1, multipart->part_len, multipart->part_fh);
                    if (wcount != multipart->part_len) {
                        clax_log("Error writing to file");
                        return -1;
                    }

                    free(multipart->part);
                    multipart->part = NULL;
                }
            }

            size_t wcount = fwrite(buf, 1, len, multipart->part_fh);
            if (wcount != len) {
                clax_log("Error writing to file");
                return -1;
            }

            multipart->part_len += len;
        }
        else {
            append_str(&multipart->part, &multipart->part_len, buf, len);
        }
    }

    return 0;
}

int on_part_data_end(multipart_parser* p)
{
    clax_http_request_t *request = multipart_parser_get_data(p);

    if (request->multiparts_num < MAX_MULTIPARTS) {
        clax_http_multipart_t *multipart = &request->multiparts[request->multiparts_num];

        if (multipart->part_fh) {
            clax_log("Closing part file");
            fclose(multipart->part_fh);
        }

        request->multiparts_num++;
    }

    return 0;
}

int on_body_end(multipart_parser* p)
{
    clax_http_request_t *req = multipart_parser_get_data(p);

    clax_log("Done multipart parsing");
    multipart_parser_free(req->multipart_parser);

    return 0;
}

int clax_http_body(http_parser *p, const char *buf, size_t len)
{
    clax_http_request_t *req = p->data;

    if (strlen(req->multipart_boundary)) {
        if (!req->multipart_parser) {
            clax_log("Init multipart parser");

            req->multipart_callbacks.on_header_field = on_multipart_header_name;
            req->multipart_callbacks.on_header_value = on_multipart_header_value;
            req->multipart_callbacks.on_part_data = on_part_data;
            req->multipart_callbacks.on_part_data_end = on_part_data_end;
            req->multipart_callbacks.on_body_end = on_body_end;

            req->multipart_parser = multipart_parser_init(req->multipart_boundary, &req->multipart_callbacks);
            multipart_parser_set_data(req->multipart_parser, req);
        }

        size_t nparsed = multipart_parser_execute(req->multipart_parser, buf, len);

        if (nparsed != len) {
            clax_log("Multipart failed!");
            return -1;
        }
    } else {
        append_str(&req->body, &req->body_len, buf, len);
    }

    return 0;
}

void clax_http_url_decode(char *str)
{
    int code;
    char buf[3];

    char *p = str;
    while (*p++) {
        if (*p == '+') {
            *p = ' ';
        }
        else if (*p == '%' && *(p + 1) && *(p + 2)) {
            buf[0] = *(p + 1);
            buf[1] = *(p + 2);
            buf[2] = 0;

            sscanf(buf, "%x", (unsigned int *)&code);

            *p = code;

            if (*(p + 3)) {
                memmove(p + 1, p + 3, strlen(p + 3) + 1);
            }
            else {
                *(p + 1) = 0;
            }
        }
    }
}

static http_parser_settings settings =
  {//.on_message_begin = message_begin_cb
  .on_header_field = header_field_cb
  ,.on_header_value = header_value_cb
  ,.on_url = request_url_cb
  /*,.on_status = response_status_cb*/
  ,.on_body = clax_http_body
  ,.on_headers_complete = headers_complete_cb
  ,.on_message_complete = message_complete_cb
  /*,.on_chunk_header = chunk_header_cb*/
  /*,.on_chunk_complete = chunk_complete_cb*/
  };

int clax_http_parse(http_parser *parser, clax_http_request_t *request, const char *buf, size_t len)
{
    size_t nparsed;

    parser->data = request;
    nparsed = http_parser_execute(parser, &settings, buf, len);

    if (nparsed != len) {
        return -1;
    }

    if (request->headers_done && request->continue_expected) {
        clax_log("100-continue expected");
        return 1;
    }

    if (request->message_done) {
        const char *content_type = clax_http_header_get(request->headers, request->headers_num, "Content-Type");

        if (content_type && strcmp(content_type, "application/x-www-form-urlencoded") == 0) {
            clax_http_parse_form(request, (char *)request->body, request->body_len);
        }

        return 1;
    }

    return 0;
}

int clax_http_chunked(char *buf, size_t len, va_list a_list_)
{
    char obuf[255];
    int olen;
    send_cb_t send_cb;
    void *ctx;
    va_list a_list;

    va_copy(a_list, a_list_);

    /* This is a nasty hack to fix this warning (sorry):
     * ISO C forbids assignment between function pointer and ‘void *’
     */
    *(void **)(&send_cb) = va_arg(a_list, void *);

    ctx = va_arg(a_list, void *);

    if (len) {
        olen = sprintf(obuf, "%x\r\n", (int)len);
        TRY send_cb(ctx, (const unsigned char *)obuf, olen) GOTO
        TRY send_cb(ctx, (const unsigned char *)buf, len) GOTO
        TRY send_cb(ctx, (const unsigned char *)"\r\n", 2) GOTO
    }
    else {
        TRY send_cb(ctx, (const unsigned char *)"0\r\n\r\n", 5) GOTO
    }

    return 0;

error:
    return -1;
}

int clax_http_read_parse(void *ctx, recv_cb_t recv_cb, http_parser *parser, clax_http_request_t *request)
{
    int ret = 0;
    unsigned char buf[1024];

    do {
        memset(buf, 0, sizeof(buf));
        ret = recv_cb(ctx, buf, sizeof(buf));

        if (ret == EAGAIN) {
            continue;
        }

        if (ret < 0) {
            clax_log("Reading failed!");
            return -1;
        }
        else if (ret == 0) {
            clax_log("Connection closed");
            return -1;
        }

        ret = clax_http_parse(parser, request, (char *)buf, ret);

        if (ret < 0) {
            clax_log("Parsing failed!");
            return -1;
        }
        else if (ret == 1) {
            clax_log("Request parsing done");
            break;
        } else if (ret == 0) {
        }
    } while (1);

    return 0;
}

int clax_http_write_response(void *ctx, send_cb_t send_cb, clax_http_response_t *response)
{
    char buf[1024];

    const char *status_message = clax_http_status_message(response->status_code);

    TRY send_cb(ctx, (const unsigned char *)"HTTP/1.1 ", 9) GOTO
    sprintf(buf, "%d ", response->status_code);
    TRY send_cb(ctx, (const unsigned char *)buf, strlen(buf)) GOTO
    TRY send_cb(ctx, (const unsigned char *)status_message, strlen(status_message)) GOTO
    TRY send_cb(ctx, (const unsigned char *)"\r\n", 2) GOTO

    if (response->content_type) {
        TRY send_cb(ctx, (const unsigned char *)"Content-Type: ", 14) GOTO;
        TRY send_cb(ctx, (const unsigned char *)response->content_type, strlen(response->content_type)) GOTO;
        TRY send_cb(ctx, (const unsigned char *)"\r\n", 2) GOTO;
    }

    if (response->transfer_encoding) {
        TRY send_cb(ctx, (const unsigned char *)"Transfer-Encoding: ", 19) GOTO;
        TRY send_cb(ctx, (const unsigned char *)response->transfer_encoding, strlen(response->transfer_encoding)) GOTO;
        TRY send_cb(ctx, (const unsigned char *)"\r\n", 2) GOTO;
    }

    if (response->body_len) {
        char buf[255];

        TRY send_cb(ctx, (const unsigned char *)"Content-Length: ", 16) GOTO;
        sprintf(buf, "%d\r\n\r\n", (int)response->body_len);
        TRY send_cb(ctx, (const unsigned char *)buf, strlen(buf)) GOTO;

        TRY send_cb(ctx, response->body, response->body_len) GOTO;
    } else if (response->body_cb) {
        TRY send_cb(ctx, (const unsigned char *)"\r\n", 2) GOTO;

        /* TODO: error handling */
        response->body_cb(response->body_cb_ctx, clax_http_chunked, send_cb, ctx);
    } else {
        TRY send_cb(ctx, (const unsigned char *)"\r\n", 2) GOTO;
    }

    return 0;

error:
    return -1;
}

int clax_http_dispatch(clax_ctx_t *clax_ctx, send_cb_t send_cb, recv_cb_t recv_cb, void *ctx) {
    http_parser parser;
    clax_http_request_t request;
    clax_http_response_t response;

    http_parser_init(&parser, HTTP_REQUEST);

    memset(&request, 0, sizeof(clax_http_request_t));
    memset(&response, 0, sizeof(clax_http_response_t));

    request.clax_ctx = clax_ctx;

    clax_log("Reading & parsing request...");
    TRY clax_http_read_parse(ctx, recv_cb, &parser, &request) GOTO;
    clax_log("ok");

    if (request.continue_expected) {
        response.status_code = 100;

        clax_log("Writing 100 Continue response...");
        TRY clax_http_write_response(ctx, send_cb, &response) GOTO;
        clax_log("ok");

        request.continue_expected = 0;
        memset(&response, 0, sizeof(clax_http_response_t));

        clax_log("Continuing reading & parsing request...");
        TRY clax_http_read_parse(ctx, recv_cb, &parser, &request) GOTO;
        clax_log("ok");
    }

    clax_log("Dispatching request...");
    clax_dispatch(clax_ctx, &request, &response);
    clax_log("ok");

    clax_log("Writing response...");
    TRY clax_http_write_response(ctx, send_cb, &response) GOTO;
    clax_log("ok");

    if (request.multiparts_num) {
        int i;
        for (i = 0; i < request.multiparts_num; i++) {
            free((void *)request.multiparts[i].part);
        }
    }

    free((void *)request.body);

    return 1;

error:
    clax_log("failed!");

    return -1;
}

const char *clax_http_extract_kv(const char *str, const char *key, size_t *len)
{
    char *p;
    char *q;

    if (len)
        *len = 0;

    p = strstr(str, key);
    if (!p) return NULL;

    p += strlen(key);
    if (*p++ != '=') return NULL;
    if (*p++ != '"') return NULL;

    q = index(p, '"');
    if (!q) return NULL;

    *len = q - p;

    return p;
}

const char *clax_http_header_get(clax_http_kv_t *headers, size_t size, char *name)
{
    int i;
    for (i = 0; i < size; i++) {
        if (strcmp(headers[i].key, name) == 0) {
            return headers[i].val;
        }
    }

    return NULL;
}
