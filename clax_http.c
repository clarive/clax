#include <stdio.h>
#include <string.h>
#include "http_parser/http_parser.h"
#include "clax_http.h"

static http_parser parser;
static int clax_http_complete = 0;

int message_complete_cb(http_parser *p)
{
    clax_http_complete = 1;

    return 0;
}

int request_url_cb(http_parser *p, const char *buf, size_t len)
{
    clax_http_message_t *message = p->data;
    struct http_parser_url u;
    int rv;

    rv = http_parser_parse_url(buf, len, 0, &u);
    if (rv != 0)
        return -1;

    int path_from = u.field_data[3].off;
    int path_len = u.field_data[3].len;

    strncpy(message->url, buf, len);
    message->url[len] = 0;

    strncpy(message->path_info, buf + path_from, path_len);
    message->path_info[path_len] = 0;

    return 0;
}

static http_parser_settings settings =
  {//.on_message_begin = message_begin_cb
  /*,.on_header_field = header_field_cb*/
  /*,.on_header_value = header_value_cb*/
  .on_url = request_url_cb
  /*,.on_status = response_status_cb*/
  /*,.on_body = body_cb*/
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

int clax_http_parse(clax_http_message_t *message, const char *buf, size_t len)
{
    size_t nparsed;

    parser.data = message;
    nparsed = http_parser_execute(&parser, &settings, buf, len);

    if (nparsed != len) {
        return -1;
    }

    if (clax_http_complete) {
        message->method = parser.method;
        return 1;
    }

    return 0;
}
