#ifndef CLAX_HTTP_H
#define CLAX_HTTP_H

#define MAX_PARAMS 255
#define MAX_HEADERS 13
#define MAX_ELEMENT_SIZE 2048
#define MAX_CHUNKS 16

#include "http_parser/http_parser.h"

typedef struct {
    char key[MAX_ELEMENT_SIZE];
    char val[MAX_ELEMENT_SIZE];
} clax_http_kv_t;

typedef struct {
  enum http_method method;
  char url[MAX_ELEMENT_SIZE];
  char path_info[MAX_ELEMENT_SIZE];
  char is_complete;
  clax_http_kv_t headers[MAX_HEADERS];
  size_t headers_num;
  clax_http_kv_t params[MAX_PARAMS];
  size_t params_num;
} clax_http_request_t;

typedef struct {
  unsigned int status_code;
  char *content_type;
  char *transfer_encoding;
  char body[MAX_ELEMENT_SIZE];
  size_t body_len;
  void (*body_cb)();
} clax_http_response_t;

void clax_http_init();
int clax_http_parse(clax_http_request_t *request, const char *buf, size_t len);

#endif
