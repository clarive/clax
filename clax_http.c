#include <stdio.h>
#include <string.h>
#include "http_parser/http_parser.h"
#include "clax_http.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))

enum {
    PARAM_MODE_KEY,
    PARAM_MODE_VAL
};

static http_parser parser;
static int clax_http_complete = 0;

struct clax_http_status_message {
    int code;
    const char *message;
};
struct clax_http_status_message clax_http_messages[] = {
    {200, "OK"},
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
    clax_http_complete = 1;

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

        strncpy(req->headers[req->headers_num].val, buf, len);

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

int body_cb(http_parser *p, const char *buf, size_t len)
{
    clax_http_request_t *req = p->data;

    if (req->headers_num > 0) {
        int i;
        for (i = 0; i < req->headers_num; i++) {
            if (strcmp(req->headers[i].key, "Content-Type") == 0) {
                if (strcmp(req->headers[i].val, "application/x-www-form-urlencoded") == 0) {
                    const char *key = NULL;
                    size_t key_len = 0;
                    const char *val = NULL;
                    size_t val_len = 0;
                    char mode = PARAM_MODE_KEY;

                    int j;
                    for (j = 0; j < len; j++) {
                        if (buf[j] == '&' || buf[j] == ';') {
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

                    break;
                }
            }
        }
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
                strcpy(p + 1, p + 3);
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
  ,.on_body = body_cb
  /*,.on_headers_complete = headers_complete_cb*/
  ,.on_message_complete = message_complete_cb
  /*,.on_chunk_header = chunk_header_cb*/
  /*,.on_chunk_complete = chunk_complete_cb*/
  };

void clax_http_init()
{
    clax_http_complete = 0;

    http_parser_init(&parser, HTTP_REQUEST);
}

int clax_http_parse(clax_http_request_t *request, const char *buf, size_t len)
{
    size_t nparsed;

    parser.data = request;
    nparsed = http_parser_execute(&parser, &settings, buf, len);

    if (nparsed != len) {
        return -1;
    }

    if (clax_http_complete) {
        request->is_complete = 1;
        request->method = parser.method;
        return 1;
    }

    return 0;
}
