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

#ifndef CLAX_HTTP_PARSER_H
#define CLAX_HTTP_PARSER_H

#define MAX_ELEMENT_SIZE 2048

#include <stdio.h>
#include <stdarg.h>

#include "contrib/libuv/include/uv.h"
#include "contrib/http-parser/http_parser.h"
#include "clax_util.h"

typedef struct clax_http_request_t clax_http_request_t;
typedef struct clax_http_response_t clax_http_response_t;

typedef int (*clax_http_chunk_cb_t)(char *buf, size_t len, va_list a_list);
typedef void (*clax_http_parser_request_done_cb_t)(clax_http_request_t *request);
typedef void (*clax_http_parser_response_done_cb_t)(clax_http_response_t *response);

struct clax_http_request_t {
  enum http_method method;
  char url[MAX_ELEMENT_SIZE];
  char path_info[MAX_ELEMENT_SIZE];

  clax_kv_list_t headers;
  size_t content_length;

  uv_file body_file;
  char *body_tmpfile;

  unsigned char *body;
  size_t body_len;

  clax_kv_list_t query_params;
  clax_kv_list_t body_params;

  /* Flags */
  char headers_done;
  char message_done;
  char continue_expected;
  int finalized;

  int to_read;

  void *data;
  clax_http_parser_request_done_cb_t headers_done_cb;
  clax_http_parser_request_done_cb_t done_cb;
};

struct clax_http_response_t {
  unsigned int status_code;
  clax_kv_list_t headers;

  unsigned char *body_buf;
  uv_file body_handle;
  size_t body_len;

  int to_write;
  int finalized;

  void *data;
  clax_http_parser_response_done_cb_t done_cb;
};

/* Public */

void clax_http_request_init(clax_http_request_t *request, char *tempdir);
void clax_http_request_free(clax_http_request_t *request);
void clax_http_response_init(clax_http_response_t *response, char *tempdir, size_t max_size);
void clax_http_response_free(clax_http_response_t *response);
int clax_http_parse(http_parser *parser, clax_http_request_t *request, const char *buf, ssize_t len);

#endif
