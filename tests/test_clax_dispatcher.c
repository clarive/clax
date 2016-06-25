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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "u/u.h"

#include "clax.h"
#include "clax_dispatcher.h"
#include "clax_util.h"
#include "clax_platform.h"
#include "u_util.h"

int clax_dispatcher_match_(const char *path_info, const char *path)
{
    return clax_dispatcher_match(path_info, strlen(path_info), path, strlen(path));
}

SUITE_START(clax_dispatch)

TEST_START(sets_404_on_unknown_path)
{
    clax_http_request_t request;
    clax_http_response_t response;

    clax_http_request_init(&request, NULL);
    clax_http_response_init(&response, NULL, 0);

    strcpy(request.path_info, "/unknown-path");

    clax_dispatch(NULL, &request, &response);

    ASSERT_EQ(response.status_code, 404)
    ASSERT_STR_EQ(clax_kv_list_find(&response.headers, "Content-Type"), "text/plain")

    unsigned char buf[255];
    clax_big_buf_read(&response.body, buf, sizeof(buf), 0);
    ASSERT_BUF_EQ(buf, "Not found", 9)

    clax_http_request_free(&request);
    clax_http_response_free(&response);
}
TEST_END

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

    clax_dispatch(&clax_ctx, &request, &response);

    ASSERT_EQ(response.status_code, 400)

    clax_http_request_free(&request);
    clax_http_response_free(&response);
    clax_options_free(&options);
}
TEST_END

TEST_START(match_matches_paths)
{
    ASSERT_EQ(clax_dispatcher_match_("/foo", "/bar"), 0)
    ASSERT_EQ(clax_dispatcher_match_("/foo", "/foo"), 4)
    ASSERT_EQ(clax_dispatcher_match_("/foo/", "/foo/"), 5)

    ASSERT_EQ(clax_dispatcher_match_("/foobar", "^/foo/"), 0)
    ASSERT_EQ(clax_dispatcher_match_("/foo/", "^/foo/"), 5)
    ASSERT_EQ(clax_dispatcher_match_("/foo/bar", "^/foo/"), 5)
}
TEST_END

SUITE_END
