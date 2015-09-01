#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include "http_parser/http_parser.h"
#include "multipart_parser.h"
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

    return 0;
}

int header_field_cb(http_parser *p, const char *buf, size_t len)
{
    size_t toread = MIN(MAX_ELEMENT_SIZE - 1, len);
    clax_http_request_t *req = p->data;

    if (req->headers_num < MAX_HEADERS) {
        clax_http_request_t *req = p->data;

        strncpy(req->headers[req->headers_num].key, buf, len);
    }

    return 0;
}

int header_value_cb(http_parser *p, const char *buf, size_t len)
{
    size_t toread = MIN(MAX_ELEMENT_SIZE - 1, len);
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

int on_part_data(multipart_parser* p, const char *buf, size_t len)
{
    clax_http_request_t *request = multipart_parser_get_data(p);

    if (request->multiparts_num < MAX_MULTIPARTS) {
        clax_http_multipart_t *multipart = &request->multiparts[request->multiparts_num];

        if (!multipart->part) {
            multipart->part = (char *)malloc(sizeof(char) * len);
            memcpy((void *)multipart->part, (const void*)buf, len);
            multipart->part_len = len;
        }
        else {
            multipart->part = (char *)realloc((void *)multipart->part, sizeof(char) * multipart->part_len + len);
            memcpy((void *)multipart->part + multipart->part_len, (const void*)buf, len);
            multipart->part_len += len;
        }
    }

    return 0;
}

int on_part_data_end(multipart_parser* p)
{
    clax_http_request_t *request = multipart_parser_get_data(p);

    clax_log("data end");

    if (request->multiparts_num < MAX_MULTIPARTS) {
        request->multiparts_num++;
    }

    return 0;
}

int clax_http_parse_multipart(clax_http_request_t *req)
{
    size_t ret;
    size_t nparsed;

    multipart_parser_settings callbacks;
    memset(&callbacks, 0, sizeof(multipart_parser_settings));

    callbacks.on_header_field = on_multipart_header_name;
    callbacks.on_header_value = on_multipart_header_value;
    callbacks.on_part_data = on_part_data;
    callbacks.on_part_data_end = on_part_data_end;

    multipart_parser* parser = multipart_parser_init(req->multipart_boundary, &callbacks);
    multipart_parser_set_data(parser, req);

    nparsed = multipart_parser_execute(parser, req->body, req->body_len);

    if (nparsed == req->body_len) {
        ret = 0;
    }
    else {
        ret = -1;
    }

    multipart_parser_free(parser);

    return ret;
}

int clax_http_body(http_parser *p, const char *buf, size_t len)
{
    clax_http_request_t *req = p->data;

    clax_log("Reading body (%d)...", len);

    if (!req->body) {
        req->body = (char *)malloc(sizeof(char) * len);
        memcpy((void *)req->body, (const void*)buf, len);
        req->body_len = len;
    }
    else {
        req->body = (char *)realloc((void *)req->body, sizeof(char) * req->body_len + len);
        memcpy((void *)req->body + req->body_len, (const void*)buf, len);
        req->body_len += len;
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

            sscanf(buf, "%x", &code);

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

        if (content_type) {
            if (strcmp(content_type, "application/x-www-form-urlencoded") == 0) {
                clax_http_parse_form(request, buf, len);
            }
            else if (strncmp(content_type, "multipart/form-data; boundary=", 30) == 0) {
                size_t boundary_len = strlen(content_type) - 30;
                size_t tocopy = MIN(sizeof_struct_member(clax_http_request_t, multipart_boundary) - 2, boundary_len);

                strcpy(request->multipart_boundary, "--");
                strncpy(request->multipart_boundary + 2, content_type + 30, tocopy);

                clax_http_parse_multipart(request);
            }
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

    send_cb = va_arg(a_list, void *);

    ctx = va_arg(a_list, void *);

    if (len) {
        olen = sprintf(obuf, "%x\r\n", len);
        TRY send_cb(ctx, obuf, olen) GOTO
        TRY send_cb(ctx, buf, len) GOTO
        TRY send_cb(ctx, "\r\n", 2) GOTO
    }
    else {
        TRY send_cb(ctx, "0\r\n\r\n", 5) GOTO
    }

    return 0;

error:
    return -1;
}

int clax_http_read_parse(void *ctx, recv_cb_t recv_cb, http_parser *parser, clax_http_request_t *request)
{
    int ret = 0;
    int len = 0;
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

        ret = clax_http_parse(parser, request, buf, ret);

        if (ret < 0) {
            clax_log("Parsing failed!");
            return -1;
        }
        else if (ret == 1) {
            clax_log("Request parsing done");
            break;
        } else if (ret == 0) {
            clax_log("Waiting for more data...");
        }
    } while (1);

    return 0;
}

int clax_http_write_response(void *ctx, send_cb_t send_cb, clax_http_response_t *response)
{
    int ret = 0;
    int len = 0;
    unsigned char buf[1024];

    const char *status_message = clax_http_status_message(response->status_code);

    TRY send_cb(ctx, "HTTP/1.1 ", 9) GOTO
    sprintf(buf, "%d ", response->status_code);
    TRY send_cb(ctx, buf, strlen(buf)) GOTO
    TRY send_cb(ctx, status_message, strlen(status_message)) GOTO
    TRY send_cb(ctx, "\r\n", 2) GOTO

    if (response->content_type) {
        TRY send_cb(ctx, "Content-Type: ", 14) GOTO;
        TRY send_cb(ctx, response->content_type, strlen(response->content_type)) GOTO;
        TRY send_cb(ctx, "\r\n", 2) GOTO;
    }

    if (response->transfer_encoding) {
        TRY send_cb(ctx, "Transfer-Encoding: ", 19) GOTO;
        TRY send_cb(ctx, response->transfer_encoding, strlen(response->transfer_encoding)) GOTO;
        TRY send_cb(ctx, "\r\n", 2) GOTO;
    }

    if (response->body_len) {
        char buf[255];

        TRY send_cb(ctx, "Content-Length: ", 16) GOTO;
        sprintf(buf, "%d\r\n\r\n", response->body_len);
        TRY send_cb(ctx, buf, strlen(buf)) GOTO;

        TRY send_cb(ctx, response->body, response->body_len) GOTO;
    } else if (response->body_cb) {
        TRY send_cb(ctx, "\r\n", 2) GOTO;

        /* TODO: error handling */
        response->body_cb(response->body_cb_ctx, clax_http_chunked, send_cb, ctx);
    } else {
        TRY send_cb(ctx, "\r\n", 2) GOTO;
    }

    return 0;

error:
    return -1;
}

int clax_http_dispatch(void *ctx, send_cb_t send_cb, recv_cb_t recv_cb) {
    http_parser parser;
    clax_http_request_t request;
    clax_http_response_t response;

    http_parser_init(&parser, HTTP_REQUEST);

    memset(&request, 0, sizeof(clax_http_request_t));
    memset(&response, 0, sizeof(clax_http_response_t));

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
    clax_dispatch(&request, &response);
    clax_log("ok");

    clax_log("Writing response...");
    TRY clax_http_write_response(ctx, send_cb, &response) GOTO;
    clax_log("ok");

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
