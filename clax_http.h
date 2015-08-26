#ifndef CLAX_HTTP_H
#define CLAX_HTTP_H

#define MAX_HEADERS 13
#define MAX_ELEMENT_SIZE 2048
#define MAX_CHUNKS 16

#include "http_parser/http_parser.h"

typedef struct {
  enum http_method method;
  char url[MAX_ELEMENT_SIZE];
  char path_info[MAX_ELEMENT_SIZE];
} clax_http_request_t;

typedef struct {
  unsigned int status_code;
  char body[MAX_ELEMENT_SIZE];
  size_t body_len;
} clax_http_response_t;

void clax_http_init();
int clax_http_parse(clax_http_request_t *request, const char *buf, size_t len);

#endif
