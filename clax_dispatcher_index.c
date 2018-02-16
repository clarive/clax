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

#include "clax_version.h"
#include "clax_http.h"

void clax_dispatch_index(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *response)
{
    const char *body = "{"
        "\"message\":\"Hello, world!\""
        ",\"version\":\"" CLAX_VERSION "\""
        ",\"os\":\"" CLAX_OS "\""
        ",\"arch\":\"" CLAX_ARCH "\""
        ",\"backend\":\"libuv\""
        "}";

    clax_http_response_status(clax_ctx, response, 200);
    clax_http_response_header(clax_ctx, response, "Content-Type", "application/json");
    clax_http_response_header(clax_ctx, response, "X-Clax-Root", clax_ctx->options->root);
    clax_http_response_body(clax_ctx, response, (const unsigned char *)body, strlen(body));
}
