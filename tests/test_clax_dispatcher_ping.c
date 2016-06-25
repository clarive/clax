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

#include <stdio.h>

#include "u/u.h"

#include "clax_dispatcher_ping.h"
#include "u_util.h"

SUITE_START(clax_dispatcher_ping)

TEST_START(returns pong)
{
    clax_ctx_t clax_ctx;
    clax_http_request_t request;
    clax_http_response_t response;

    clax_ctx_init(&clax_ctx);
    clax_http_request_init(&request, NULL);
    clax_http_response_init(&response, NULL, 0);

    request.method = HTTP_GET;
    strcpy(request.path_info, "/");

    clax_dispatch_ping(&clax_ctx, &request, &response);

    ASSERT_EQ(response.status_code, 200)

    clax_http_request_free(&request);
    clax_http_response_free(&response);
    clax_ctx_free(&clax_ctx);
}
TEST_END

SUITE_END
