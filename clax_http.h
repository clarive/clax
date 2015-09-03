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

#ifndef CLAX_HTTP_H
#define CLAX_HTTP_H

#define MAX_PARAMS 255
#define MAX_HEADERS 13
#define MAX_ELEMENT_SIZE 2048
#define MAX_MULTIPARTS 5
#define MAX_CHUNKS 16

#include <stdarg.h>

#include "http_parser/http_parser.h"
#include "multipart_parser.h"
#include "clax.h"
#include "clax_util.h"

typedef struct {
    char key[MAX_ELEMENT_SIZE];
    char val[MAX_ELEMENT_SIZE];
} clax_http_kv_t;

typedef struct {
  clax_http_kv_t headers[MAX_HEADERS];
  size_t headers_num;
  unsigned char *part;
  FILE *part_fh;
  char part_fpath[1024];
  size_t part_len;
} clax_http_multipart_t;

typedef int (*clax_http_chunk_cb_t)(char *buf, size_t len, va_list a_list);

typedef struct {
  enum http_method method;
  char url[MAX_ELEMENT_SIZE];
  char path_info[MAX_ELEMENT_SIZE];

  clax_kv_list_t headers;
  size_t content_length;

  unsigned char *body;
  size_t body_len;

  clax_kv_list_t body_params;

  multipart_parser *multipart_parser;
  multipart_parser_settings multipart_callbacks;
  char multipart_boundary[70];
  clax_http_multipart_t multiparts[MAX_MULTIPARTS];
  size_t multiparts_num;

    /* Flags */
  char headers_done;
  char message_done;
  char is_complete;
  char continue_expected;

  clax_ctx_t *clax_ctx;
} clax_http_request_t;

typedef struct {
  unsigned int status_code;
  clax_kv_list_t headers;
  unsigned char body[MAX_ELEMENT_SIZE];
  size_t body_len;
  void (*body_cb)(void *ctx, clax_http_chunk_cb_t chunk_cb, ...);
  void *body_cb_ctx;
} clax_http_response_t;

typedef int (*recv_cb_t)(void *ctx, unsigned char *buf, size_t len);
typedef int (*send_cb_t)(void *ctx, const unsigned char *buf, size_t len);

/* Public */

void clax_http_request_init(clax_http_request_t *request);
void clax_http_request_free(clax_http_request_t *request);
void clax_http_response_init(clax_http_response_t *response);
void clax_http_response_free(clax_http_response_t *response);

int clax_http_dispatch(clax_ctx_t *clax_ctx, send_cb_t send_cb, recv_cb_t recv_cb, void *ctx);
const char *clax_http_extract_kv(const char *str, const char *key, size_t *len);
const char *clax_http_header_get(clax_http_kv_t *headers, size_t size, char *name);
int clax_http_chunked(char *buf, size_t len, va_list a_list_);

/* Private */

int clax_http_write_response(void *ctx, send_cb_t send_cb, clax_http_response_t *response);
int clax_http_read_parse(void *ctx, recv_cb_t recv_cb, http_parser *parser, clax_http_request_t *request);
int clax_http_parse(http_parser *parser, clax_http_request_t *request, const char *buf, size_t len);
const char *clax_http_status_message(int code);
void clax_http_url_decode(char *str);
#endif
