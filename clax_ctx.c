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

#include <string.h> /* memset */
#include <stdlib.h>

#include "clax_ctx.h"
#include "clax_log.h"

#include "contrib/http-parser/http_parser.h" /* http_parser_init */

clax_ctx_t *clax_ctx_alloc()
{
    return malloc(sizeof(clax_ctx_t));
}

void clax_ctx_init(clax_ctx_t *ctx, opt *options)
{
    clax_log("Initializing clax ctx");

    memset(ctx, 0, sizeof(clax_ctx_t));
    memset(&ctx->parser, 0, sizeof(http_parser));

    http_parser_init(&ctx->parser, HTTP_REQUEST);

    ctx->options = options;

    clax_http_request_init(&ctx->request, ctx->options->root);
    clax_http_response_init(&ctx->response, ctx->options->root, 1024 * 1024);

    ctx->request.data = ctx;
    ctx->response.data = ctx;
}

void clax_ctx_free(clax_ctx_t *ctx)
{
    clax_log("Freeing clax ctx");

    clax_http_request_free(&ctx->request);
    clax_http_response_free(&ctx->response);

    free(ctx);
}
