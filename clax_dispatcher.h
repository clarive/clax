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

#ifndef CLAX_DISPATCHER_H
#define CLAX_DISPATCHER_H

#include "clax.h"
#include "clax_http.h"

void clax_dispatch(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res);
size_t clax_dispatcher_match(const char *path_info, size_t path_info_len, const char *path, size_t path_len);

void clax_dispatch_success(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res);
void clax_dispatch_bad_gateway(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res);
void clax_dispatch_not_found(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res);
void clax_dispatch_method_not_allowed(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res);
void clax_dispatch_system_error(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res, char *msg);
void clax_dispatch_bad_request(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res, char *msg);
void clax_dispatch_not_authorized(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res);

#endif
