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

#include "clax_ctx.h"
#include "clax_http_parser.h"

int clax_http_dispatch(clax_ctx_t *clax_ctx, const char *buf, ssize_t len);
void clax_http_dispatch_done_cb(clax_ctx_t *clax_ctx, clax_http_request_t *request, clax_http_response_t *response);

char *clax_http_request_header(clax_ctx_t *ctx, clax_http_request_t *request, const char *header);

int clax_http_response_status(clax_ctx_t *ctx, clax_http_response_t *response, int status);
int clax_http_response_header(clax_ctx_t *ctx, clax_http_response_t *response, const char *header, const char *value);
int clax_http_response_body(clax_ctx_t *ctx, clax_http_response_t *response, const unsigned char *buf, size_t len);
int clax_http_response_body_str(clax_ctx_t *clax_ctx, clax_http_response_t *response, const char *str);
int clax_http_response_body_handle(clax_ctx_t *clax_ctx, clax_http_response_t *response, uv_file handle);
int clax_http_push_response(void *ctx, clax_http_response_t *response, const unsigned char *buf, size_t len);
int clax_http_write_response_headers(clax_ctx_t *clax_ctx, clax_http_response_t *response);
#endif
