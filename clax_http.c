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

#include "http-parser/http_parser.h"
#include "multipart-parser-c/multipart_parser.h"
#include "base64/base64.h"
#include "clax.h"
#include "clax_ctx.h"
#include "clax_dispatcher.h"
#include "clax_http.h"
#include "clax_http_multipart.h"
#include "clax_log.h"
#include "clax_http.h"
#include "clax_big_buf.h"
#include "clax_util.h"
#include "clax_platform.h"

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
    {401, "Unauthorized"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {500, "Internal System Error"},
    {502, "Bad Gateway"}
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

int on_http_parser_message_complete(http_parser *p)
{
    clax_http_request_t *req = p->data;

    req->message_done = 1;

    return 0;
}

int on_http_parser_headers_complete(http_parser *p)
{
    clax_http_request_t *req = p->data;

    req->method = p->method;
    req->headers_done = 1;

    const char *content_type = clax_kv_list_find(&req->headers, "Content-Type");
    if (content_type && strncmp(content_type, "multipart/form-data; boundary=", 30) == 0) {
        size_t boundary_len = strlen(content_type) - 30;
        size_t tocopy = MIN(sizeof_struct_member(clax_http_request_t, multipart_boundary) - 2, boundary_len);

        strcpy(req->multipart_boundary, "--");
        strncpy(req->multipart_boundary + 2, content_type + 30, tocopy);
    }

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

int on_multipart_header_name(multipart_parser* p, const char *buf, size_t len)
{
    clax_http_request_t *request = multipart_parser_get_data(p);

    clax_http_multipart_t *multipart = clax_http_multipart_list_last(&request->multiparts);

    if (!multipart || multipart->done) {
        multipart = clax_http_multipart_list_push(&request->multiparts);
    }

    char *key = clax_buf2str(buf, len);

#ifdef MVS
    clax_atoe(key, strlen(key));
#endif

    clax_kv_list_push(&multipart->headers, key, "");

    free(key);

    return 0;
}

int on_multipart_header_value(multipart_parser* p, const char *buf, size_t len)
{
    clax_http_request_t *request = multipart_parser_get_data(p);

    clax_http_multipart_t *multipart = clax_http_multipart_list_last(&request->multiparts);

    clax_kv_list_item_t *item = clax_kv_list_at(&multipart->headers, multipart->headers.size - 1);

    char *val = clax_buf2str(buf, len);

#ifdef MVS
    clax_atoe(val, strlen(val));
#endif

    clax_kv_list_set(&multipart->headers, item->key, val);

    free(val);

    return 0;
}

int on_part_data(multipart_parser* p, const char *buf, size_t len)
{
    clax_http_request_t *request = multipart_parser_get_data(p);

    clax_http_multipart_t *multipart = clax_http_multipart_list_last(&request->multiparts);

    int ret = clax_big_buf_append(&multipart->bbuf, (const unsigned char *)buf, len);

    request->multipart_status = ret;

    return ret;
}

int on_part_data_end(multipart_parser* p)
{
    clax_http_request_t *request = multipart_parser_get_data(p);

    clax_http_multipart_t *multipart = clax_http_multipart_list_last(&request->multiparts);
    multipart->done++;

    return clax_big_buf_close(&multipart->bbuf);
}

int on_body_end(multipart_parser* p)
{
    clax_log("Done multipart parsing");

    return 0;
}

int on_http_parser_body(http_parser *p, const char *buf, size_t len)
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

            char *boundary = req->multipart_boundary;
#ifdef MVS
            boundary = clax_etoa_alloc(boundary, strlen(boundary) + 1);
#endif

            req->multipart_parser = multipart_parser_init(boundary, &req->multipart_callbacks);
            multipart_parser_set_data(req->multipart_parser, req);

#ifdef MVS
            free(boundary);
#endif
        }

        size_t nparsed = multipart_parser_execute(req->multipart_parser, buf, len);

        if (nparsed != len) {
            clax_log("Multipart parsing failed");
            return -1;
        }

        if (req->multipart_status != 0) {
            clax_log("Multipart processing failed");
            return -1;
        }
    } else {
        char *b = (char *)buf;
#ifdef MVS
        b = clax_atoe_alloc(buf, len);
#endif
        clax_buf_append(&req->body, &req->body_len, b, len);
#ifdef MVS
        free((void *)b);
#endif
    }

    return 0;
}

void clax_http_parse_urlencoded(clax_kv_list_t *params, const char *buf, size_t len)
{
    const char *key = NULL;
    size_t key_len = 0;
    const char *val = NULL;
    size_t val_len = 0;
    char mode = PARAM_MODE_KEY;

    const char *b = buf;

    int j;
    for (j = 0; j < len; j++) {
        if (b[j] == '&') {
            mode = PARAM_MODE_KEY;

            if (key_len || val_len) {
                char *k = clax_buf2str(key, key_len);
                clax_http_url_decode(k);
                char *v = clax_buf2str(val, val_len);
                clax_http_url_decode(v);

                clax_kv_list_push(params, k, v);

                free(k);
                free(v);
            }

            key = NULL;
            key_len = 0;
            val = NULL;
            val_len = 0;
        }
        else if (b[j] == '=') {
            mode = PARAM_MODE_VAL;
        }
        else if (mode == PARAM_MODE_KEY) {
            if (key == NULL)
                key = b + j;

            key_len++;
        }
        else if (mode == PARAM_MODE_VAL) {
            if (val == NULL)
                val = b + j;

            val_len++;
        }
    }

    if (key_len || val_len) {
        char *k = clax_buf2str(key, key_len);
        clax_http_url_decode(k);
        char *v = clax_buf2str(val, val_len);
        clax_http_url_decode(v);

        clax_kv_list_push(params, k, v);

        free(k);
        free(v);
    }
}

size_t clax_http_url_decode(char *str)
{
    int code;
    char hex[3];
    size_t len;
    size_t new_len;

    if (str == NULL)
        return 0;

    new_len = len = strlen(str);

    char *p = str;

    /* we cannot use *p++ pattern here, since we can get \0 from %00 */
    while (p - str <= len) {
        if (*p == '+') {
            *p = ' ';
        }
        else if (*p == '%' && *(p + 1) && *(p + 2)) {
            hex[0] = *(p + 1);
            hex[1] = *(p + 2);
            hex[2] = 0;

            sscanf(hex, "%x", (unsigned int *)&code);
            new_len -= 2;

            *p = code;

#ifdef MVS
            clax_atoe(p, 1);
#endif

            if (*(p + 3)) {
                memmove(p + 1, p + 3, strlen(p + 3) + 1);
            }
            else {
                *(p + 1) = 0;
                break;
            }
        }

        p++;
    }

    return new_len;
}

static http_parser_settings http_parser_callbacks =
  {//.on_message_begin = message_begin_cb
  .on_header_field = on_http_parser_header_field
  ,.on_header_value = on_http_parser_header_value
  ,.on_url = on_http_parser_url
  /*,.on_status = response_status_cb*/
  ,.on_body = on_http_parser_body
  ,.on_headers_complete = on_http_parser_headers_complete
  ,.on_message_complete = on_http_parser_message_complete
  /*,.on_chunk_header = chunk_header_cb*/
  /*,.on_chunk_complete = chunk_complete_cb*/
  };

int clax_http_parse(http_parser *parser, clax_http_request_t *request, const char *buf, size_t len)
{
    size_t nparsed;

    parser->data = request;
    nparsed = http_parser_execute(parser, &http_parser_callbacks, buf, len);

    if (nparsed != len) {
        return -1;
    }

    if (request->headers_done && request->continue_expected) {
        clax_log("100-continue expected");
        return 1;
    }

    if (request->message_done) {
        const char *content_type = clax_kv_list_find(&request->headers, "Content-Type");

        if (content_type && strcmp(content_type, "application/x-www-form-urlencoded") == 0) {
            clax_http_parse_urlencoded(&request->body_params, (char *)request->body, request->body_len);
        }

        return 1;
    }

    return 0;
}

int send_cb_wrapper(send_cb_t send_cb, void *ctx, const unsigned char *buf, size_t len)
{
    const unsigned char *b = buf;
    int ret;

#ifdef MVS
    if (!isatty(fileno(stdin))) {
        b = (const unsigned char *)clax_etoa_alloc((const char *)buf, len);
    }
#endif

    ret = send_cb(ctx, b, len);

#ifdef MVS
    free((void *)b);
#endif

    return ret;
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
        olen = snprintf(obuf, sizeof(obuf), "%x\r\n", (int)len);
        TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)obuf, olen) GOTO
        TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)buf, len) GOTO
        TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)"\r\n", 2) GOTO
    }
    else {
        TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)"0\r\n", 3) GOTO

        /* Trailing headers */
        if (buf && strlen(buf)) {
            TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)buf, strlen(buf)) GOTO
            TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)"\r\n", 2) GOTO
        }

        TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)"\r\n", 2) GOTO
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
            clax_log("Parsing failed: %s", http_errno_description(parser->http_errno));
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
    char buf[5];

    const char *status_message = clax_http_status_message(response->status_code);

    TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)"HTTP/1.1 ", 9) GOTO
    sprintf(buf, "%d ", response->status_code);
    TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)buf, MIN(strlen(buf), sizeof(buf))) GOTO
    TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)status_message, strlen(status_message)) GOTO
    TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)"\r\n", 2) GOTO

    size_t header_iter = 0;
    clax_kv_list_item_t *header;
    while ((header = clax_kv_list_next(&response->headers, &header_iter)) != NULL) {
        TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)header->key, strlen(header->key)) GOTO;
        TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)": ", 2) GOTO;
        TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)header->val, strlen(header->val)) GOTO;
        TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)"\r\n", 2) GOTO;
    }

    if (response->body.len) {
        size_t rcount;
        size_t offset;
        unsigned char buf[1024];

        TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)"Content-Length: ", 16) GOTO;
        sprintf((char *)buf, "%d\r\n\r\n", (int)response->body.len);
        TRY send_cb_wrapper(send_cb, ctx, buf, strlen((char *)buf)) GOTO;

        offset = 0;
        while ((rcount = clax_big_buf_read(&response->body, buf, sizeof(buf), offset)) > 0) {
            TRY send_cb_wrapper(send_cb, ctx, buf, rcount) GOTO;
            offset += rcount;
        }
    } else if (response->body_cb) {
        TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)"\r\n", 2) GOTO;

        /* TODO: error handling */
        response->body_cb(response->body_cb_ctx, clax_http_chunked, send_cb, ctx);
    } else if (response->body_fh) {
        const unsigned char buf[1024];
        size_t rcount;

        TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)"\r\n", 2) GOTO;
        while ((rcount = fread((void *)buf, 1, sizeof(buf), response->body_fh)) > 0) {
            TRY send_cb_wrapper(send_cb, ctx, buf, rcount) GOTO;
        }

        fclose(response->body_fh);
        response->body_fh = NULL;
    } else {
        TRY send_cb_wrapper(send_cb, ctx, (const unsigned char *)"\r\n", 2) GOTO;
    }

    return 0;

error:
    return -1;
}

void clax_http_request_init(clax_http_request_t *request, char *tempdir)
{
    memset(request, 0, sizeof(clax_http_request_t));

    clax_kv_list_init(&request->headers);
    clax_kv_list_init(&request->query_params);
    clax_kv_list_init(&request->body_params);

    clax_http_multipart_list_init(&request->multiparts, tempdir);
}

void clax_http_request_free(clax_http_request_t *request)
{
    free((void *)request->body);

    if (request->multipart_parser)
        multipart_parser_free(request->multipart_parser);

    clax_http_multipart_list_free(&request->multiparts);

    clax_kv_list_free(&request->headers);
    clax_kv_list_free(&request->query_params);
    clax_kv_list_free(&request->body_params);
}

void clax_http_response_init(clax_http_response_t *response, char *tempdir, size_t max_size)
{
    memset(response, 0, sizeof(clax_http_response_t));

    clax_kv_list_init(&response->headers);
    clax_big_buf_init(&response->body, tempdir, max_size);
}

void clax_http_response_free(clax_http_response_t *response)
{
    clax_kv_list_free(&response->headers);

    clax_big_buf_free(&response->body);

    if (response->body_fh)
        fclose(response->body_fh);
}

int clax_http_check_basic_auth(char *header, char *username, char *password)
{
    int ok = 0;
    char *prefix = "Basic ";
    size_t prefix_len = strlen(prefix);
    char *auth = NULL;
    size_t auth_len = 0;
    char *sep;

    ok = header && strlen(header) > prefix_len && strncmp(header, prefix, prefix_len) == 0;

    if (ok)
        ok = base64_decode_alloc(header + prefix_len, strlen(header) - prefix_len, &auth, &auth_len) && auth != NULL;

    if (ok) {
        auth[auth_len] = 0;
#ifdef MVS
        clax_atoe(auth, strlen(auth));
#endif
        sep = strstr(auth, ":");
        ok = sep != NULL;
    }

    if (ok) {
        size_t username_len = sep - auth;
        ok = strlen(username) == username_len
            && strncmp(username, auth, username_len) == 0;
    }

    if (ok) {
        size_t password_len = auth_len - (sep - auth) - 1;
        ok = strlen(password) == password_len
            && strncmp(sep + 1, password, password_len) == 0;
    }

    free(auth);

    return ok;
}

int clax_http_dispatch(clax_ctx_t *clax_ctx, send_cb_t send_cb, recv_cb_t recv_cb, void *ctx) {
    http_parser parser;
    clax_http_request_t request;
    clax_http_response_t response;

    http_parser_init(&parser, HTTP_REQUEST);

    clax_http_request_init(&request, clax_ctx->options->root);
    clax_http_response_init(&response, clax_ctx->options->root, 1024 * 1024);

    struct sockaddr addr;
    struct sockaddr_in *addr_in;
    socklen_t len = sizeof(addr);

    getpeername(fileno(stdout), &addr, &len);
    addr_in = (struct sockaddr_in *)&addr;

    char *ip_str = inet_ntoa(addr_in->sin_addr);
    int port = ntohs(addr_in->sin_port);

    clax_log("Connection from '%s:%d'", ip_str, port);

    clax_log("Reading & parsing request...");
    if (clax_http_read_parse(ctx, recv_cb, &parser, &request) < 0) {
        clax_dispatch_bad_request(ctx, &request, &response, NULL);

        TRY clax_http_write_response(ctx, send_cb, &response) GOTO;

        goto error;
    }
    clax_log("ok");

    if (clax_ctx->options->basic_auth_username && clax_ctx->options->basic_auth_password) {
        int ok = clax_http_check_basic_auth(
                    clax_kv_list_find(&request.headers, "Authorization"),
                    clax_ctx->options->basic_auth_username,
                    clax_ctx->options->basic_auth_password
                    );

        if (ok) {
            clax_log("User '%s'", clax_ctx->options->basic_auth_username);
        } else {
            clax_log("Basic authorization failed");

            clax_dispatch_not_authorized(ctx, &request, &response);
            TRY clax_http_write_response(ctx, send_cb, &response) GOTO;
            goto error;
        }
    }

    if (clax_http_is_proxy(&request)) {
        clax_http_dispatch_proxy(clax_ctx, &parser, &request, &response, recv_cb, send_cb, ctx);
    }
    else {
        if (request.continue_expected) {
            clax_dispatch(clax_ctx, &request, &response);

            if (!response.status_code)
                response.status_code = 100;

            clax_log("Writing 100 Continue response...");
            TRY clax_http_write_response(ctx, send_cb, &response) GOTO;
            clax_log("ok");

            if (response.status_code != 100) {
                clax_log("100 Continue cancelled");
                goto cleanup;
            }

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
    }

cleanup:
    clax_http_request_free(&request);
    clax_http_response_free(&response);

    return 1;

error:
    clax_log("failed!");

    return -1;
}

int clax_http_is_proxy(clax_http_request_t *req)
{
    char *hops = clax_kv_list_find(&req->headers, "X-Hops");
    if (hops == NULL || strlen(hops) == 0) {
        return 0;
    }

    return 1;
}

int clax_http_connect_to_proxy(char *hostname, char *port)
{
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    portno = atoi(port);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return -1;
    }

    server = gethostbyname(hostname);
    if (server == NULL) {
        return -1;
    }

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        return -1;
    }

    return sockfd;
}

int clax_http_proxy_write_request_body(clax_http_request_t *req, int sockfd)
{
    if (req->body && !req->continue_expected) {
        clax_log("Writing proxy body");
        size_t offset = 0;
        size_t wcount = 0;

        while ((wcount = write(sockfd, req->body + offset, req->body_len - offset)) > 0) {
            offset += wcount;
        }

        if (wcount < 0) {
            return -1;
        }
    }

    return 0;
}

int clax_http_proxy_write_request(char *hostname, char *port, int sockfd, clax_http_request_t *req, char *hops)
{
    const char *method = http_method_str(req->method);
    TRY write(sockfd, method, strlen(method)) GOTO;
    TRY write(sockfd, " ", 1) GOTO;
    TRY write(sockfd, req->url, strlen(req->url)) GOTO;
    TRY write(sockfd, " HTTP/1.1\r\n", 11) GOTO;

    clax_kv_list_item_t *header;
    size_t header_iter = 0;

    while ((header = clax_kv_list_next(&req->headers, &header_iter)) != NULL) {
        if (strcmp(header->key, "X-Hops") == 0 && (hops == NULL || strlen(hops) == 0)) {
            continue;
        }

        TRY write(sockfd, header->key, strlen(header->key)) GOTO;
        TRY write(sockfd, ": ", 2) GOTO;

        if (strcmp(header->key, "Host") == 0) {
            TRY write(sockfd, hostname, strlen(hostname)) GOTO;
            TRY write(sockfd, ":", 1) GOTO;
            TRY write(sockfd, port, strlen(port)) GOTO;
        }
        else if (strcmp(header->key, "X-Hops") == 0 && hops) {
            TRY write(sockfd, hops, strlen(hops)) GOTO;
        }
        else {
            TRY write(sockfd, header->val, strlen(header->val)) GOTO;
        }

        TRY write(sockfd, "\r\n", 2) GOTO;
    }

    TRY write(sockfd, "\r\n", 2) GOTO;

    TRY clax_http_proxy_write_request_body(req, sockfd) GOTO;

    return 0;

error:
    return -1;
}

void clax_http_dispatch_proxy(clax_ctx_t *clax_ctx, http_parser *parser, clax_http_request_t *req, clax_http_response_t *res,
        recv_cb_t recv_cb, send_cb_t send_cb, void *ctx)
{
    char *hostname = NULL;
    char *port = NULL;
    char *hops = clax_strdup(clax_kv_list_find(&req->headers, "X-Hops"));
    char *sep;

    if (hops == NULL || strlen(hops) == 0) {
        clax_dispatch_bad_request(clax_ctx, req, res, "Invalid X-Hops header");
        TRY clax_http_write_response(ctx, send_cb, res) GOTO;
        return;
    }

    if ((sep = strstr(hops, ",")) != NULL) {
        hostname = clax_strndup(hops, sep - hops);
        strcpy(hops, sep + 1);
    }
    else {
        hostname = clax_strdup(hops);
        hops[0] = 0;
    }

    if (hostname == NULL || strlen(hostname) == 0) {
        clax_dispatch_bad_request(clax_ctx, req, res, "Invalid X-Hops header");
        TRY clax_http_write_response(ctx, send_cb, res) GOTO;
        return;
    }

    if ((sep = strstr(hostname, ":")) != NULL) {
        port = clax_strdup(sep + 1);
        *sep = 0;
    }
    else {
        port = clax_strdup("80");
    }

    clax_log("Connecting to proxy %s:%s", hostname, port);

    int sockfd = clax_http_connect_to_proxy(hostname, port);

    if (sockfd < 0) {
        clax_dispatch_bad_gateway(clax_ctx, req, res);
        TRY clax_http_write_response(ctx, send_cb, res) GOTO;
    }

    clax_log("Connected to proxy");

    if (clax_http_proxy_write_request(hostname, port, sockfd, req, hops) < 0) {
        clax_dispatch_bad_gateway(clax_ctx, req, res);
        TRY clax_http_write_response(ctx, send_cb, res) GOTO;
    }

    clax_log("Proxy request written. Waiting for response...");

    unsigned char buffer[1024];
    memset(buffer, 0, sizeof(buffer) * sizeof(char));

    while (1) {
        int rcount = read(sockfd, buffer, sizeof(buffer));

        if (rcount <= 0) {
            goto error;
        }

        TRY send_cb_wrapper(send_cb, ctx, buffer, rcount) GOTO;

        if (req->continue_expected) {
            req->continue_expected = 0;

            int ret = 0;
            unsigned char buf[1024];

            do {
                memset(buf, 0, sizeof(buf));
                ret = recv_cb(ctx, buf, sizeof(buf));

                clax_log("RECV_CB=%d", ret);

                if (ret == EAGAIN) {
                    clax_log("EAGAIN");
                    continue;
                }

                if (ret < 0) {
                    clax_log("Reading failed!");
                    goto error;
                }
                else if (ret == 0) {
                    clax_log("Connection closed");
                    goto error;
                }

                TRY write(sockfd, buf, ret) GOTO;

                ret = clax_http_parse(parser, req, (char *)buf, ret);

                if (ret < 0) {
                    goto error;
                }
                else if (ret == 1) {
                    clax_log("Request parsing done");
                    break;
                } else if (ret == 0) {
                }
            } while (1);

            clax_log("DONE");
        }
    }

    clax_log("Disconnecting from proxy");

error:
    if (sockfd > 0)
        close(sockfd);

    free(hops);
    free(hostname);
    free(port);

    return;
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

    q = strchr(p, '"');
    if (!q) return NULL;

    *len = q - p;

    return p;
}
