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

#ifndef CLAX_CTX_H
#define CLAX_CTX_H

#include "contrib/mbedtls/mbedtls/ssl.h"

#include "clax_options.h"
#include "clax_http_parser.h"

typedef struct clax_ctx_t clax_ctx_t;

typedef int (*clax_ctx_send_cb_t)(clax_ctx_t *clax_ctx, const unsigned char *buf, size_t len);

typedef struct clax_ctx_ssl_t clax_ctx_ssl_t;

struct clax_ctx_ssl_t {
    struct mbedtls_ssl_context ssl;
    char *buf;
    int len;
};

struct clax_ctx_t {
    opt* options;
    http_parser parser;
    clax_http_request_t request;
    clax_http_response_t response;

    void *rh;
    void *wh;
    clax_ctx_send_cb_t send_cb;

    clax_ctx_ssl_t ssl;
    int ssl_handshake_done;
};

clax_ctx_t *clax_ctx_alloc();
void clax_ctx_init(clax_ctx_t *ctx, opt *options);
void clax_ctx_free(clax_ctx_t *ctx);

#endif
