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

#include "u/u.h"

#include "clax_ctx.h"
#include "clax_dispatcher_command.h"
#include "u_util.h"

char content[1024] = {0};

int chunk_cb(char *buf, size_t len, va_list a_list_)
{
    strcpy(content, buf);

    return 0;
}

SUITE_START(clax_dispatcher_command)

TEST_START(returns_bad_request_when_wrong_params)
{
    opt options;
    clax_ctx_t clax_ctx;
    clax_http_request_t request;
    clax_http_response_t response;

    memset(&clax_ctx, 0, sizeof(clax_ctx_t));
    clax_options_init(&options);
    clax_http_request_init(&request, NULL);
    clax_http_response_init(&response, NULL, 0);

    request.method = HTTP_POST;
    strcpy(request.path_info, "/command");
    clax_kv_list_push(&request.body_params, "boo", "bar");

    clax_dispatch_command(&clax_ctx, &request, &response);

    ASSERT_EQ(response.status_code, 400)

    clax_http_request_free(&request);
    clax_http_response_free(&response);
    clax_options_free(&options);
}
TEST_END

TEST_START(returns ok)
{
    opt options;
    clax_ctx_t clax_ctx;
    clax_http_request_t request;
    clax_http_response_t response;

    memset(&clax_ctx, 0, sizeof(clax_ctx_t));
    clax_options_init(&options);
    clax_http_request_init(&request, NULL);
    clax_http_response_init(&response, NULL, 0);

    request.method = HTTP_POST;
    strcpy(request.path_info, "/command");
    clax_kv_list_push(&request.body_params, "command", "echo 'bar'");

    clax_dispatch_command(&clax_ctx, &request, &response);

    ASSERT_EQ(response.status_code, 200)
    ASSERT_STR_EQ(clax_kv_list_find(&response.headers, "Trailer"), "X-Clax-Exit, X-Clax-Status");
    ASSERT_MATCHES(clax_kv_list_find(&response.headers, "X-Clax-PID"), "^\\d+$");

    clax_http_request_free(&request);
    clax_http_response_free(&response);
    clax_options_free(&options);
}
TEST_END

TEST_START(returns trailing headers)
{
    opt options;
    clax_ctx_t clax_ctx;
    clax_http_request_t request;
    clax_http_response_t response;

    memset(&clax_ctx, 0, sizeof(clax_ctx_t));
    clax_options_init(&options);
    clax_http_request_init(&request, NULL);
    clax_http_response_init(&response, NULL, 0);

    request.method = HTTP_POST;
    strcpy(request.path_info, "/command");
    clax_kv_list_push(&request.body_params, "command", "echo 'bar'");

    clax_dispatch_command(&clax_ctx, &request, &response);

    content[0] = 0;
    response.body_cb(response.body_cb_ctx, chunk_cb);
    ASSERT_STR_EQ(content, "X-Clax-Exit: 0\r\nX-Clax-Status: success");

    clax_http_request_free(&request);
    clax_http_response_free(&response);
    clax_options_free(&options);
}
TEST_END

TEST_START(returns trailing headers when timeout)
{
    opt options;
    clax_ctx_t clax_ctx;
    clax_http_request_t request;
    clax_http_response_t response;

    memset(&clax_ctx, 0, sizeof(clax_ctx_t));
    clax_options_init(&options);
    clax_http_request_init(&request, NULL);
    clax_http_response_init(&response, NULL, 0);

    request.method = HTTP_POST;
    strcpy(request.path_info, "/command");
    clax_kv_list_push(&request.body_params, "command", "echo foo; sleep 1; echo bar; sleep 1");
    clax_kv_list_push(&request.body_params, "timeout", "1");

    clax_dispatch_command(&clax_ctx, &request, &response);

    content[0] = 0;
    response.body_cb(response.body_cb_ctx, chunk_cb);
    ASSERT_STR_EQ(content, "X-Clax-Exit: 255\r\nX-Clax-Status: timeout");

    clax_http_request_free(&request);
    clax_http_response_free(&response);
    clax_options_free(&options);
}
TEST_END

TEST_START(returns trailing headers when error)
{
    opt options;
    clax_ctx_t clax_ctx;
    clax_http_request_t request;
    clax_http_response_t response;

    memset(&clax_ctx, 0, sizeof(clax_ctx_t));
    clax_options_init(&options);
    clax_http_request_init(&request, NULL);
    clax_http_response_init(&response, NULL, 0);

    request.method = HTTP_POST;
    strcpy(request.path_info, "/command");
    clax_kv_list_push(&request.body_params, "command", "exit 3");

    clax_dispatch_command(&clax_ctx, &request, &response);

    content[0] = 0;
    response.body_cb(response.body_cb_ctx, chunk_cb);
    ASSERT_STR_EQ(content, "X-Clax-Exit: 3\r\nX-Clax-Status: error");

    clax_http_request_free(&request);
    clax_http_response_free(&response);
    clax_options_free(&options);
}
TEST_END

SUITE_END
