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

#include "clax.h"
#include "clax_dispatcher.h"
#include "u.h"

TEST_START(clax_dispatch_sets_404_on_unknown_path)
{
    clax_http_request_t request;
    clax_http_response_t response;

    clax_http_request_init(&request);
    clax_http_response_init(&response);

    strcpy(request.path_info, "/unknown-path");

    clax_dispatch(NULL, &request, &response);

    ASSERT_EQ(response.status_code, 404)
    ASSERT_STR_EQ(clax_kv_list_find(&response.headers, "Content-Type"), "text/plain")
    ASSERT_STRN_EQ(response.body, "Not found", 9)

    clax_http_request_free(&request);
    clax_http_response_free(&response);
}
TEST_END

size_t slurp_file(char *fname, char *buf, size_t len)
{
    FILE *fh = fopen(fname, "r");
    if (!fh)
        return -1;

    size_t rlen = fread(buf, 1, len, fh);
    if (rlen < 0) {
        return -1;
    }

    fclose(fh);
    return rlen;
}

TEST_START(clax_dispatch_saves_upload_string_to_file)
{
    opt options;
    clax_ctx_t clax_ctx;
    clax_http_request_t request;
    clax_http_response_t response;

    memset(&clax_ctx, 0, sizeof(clax_ctx_t));
    memset(&options, 0, sizeof(opt));
    clax_http_request_init(&request);
    clax_http_response_init(&response);

    char template[] = "/tmp/tmpdir.XXXXXX";
    char *tmp_dirname = mkdtemp(template);
    mkdir(tmp_dirname, 0755);
    chdir(tmp_dirname);

    strcpy(options.root, tmp_dirname);
    strcat(options.root, "/");
    clax_ctx.options = &options;

    request.method = HTTP_POST;
    strcpy(request.path_info, "/upload");
    strcpy(request.multipart_boundary, "---boundary");

    clax_http_multipart_t *multipart = clax_http_multipart_list_push(&request.multiparts);

    clax_kv_list_push(&multipart->headers, "Content-Disposition", "form-data; name=\"file\"; filename=\"foobar\"");

    unsigned char *part = malloc(6 + 1);
    strcpy((char *)part, "foobar");
    multipart->part = part;
    multipart->part_len = 6;

    clax_dispatch(&clax_ctx, &request, &response);

    ASSERT_EQ(response.status_code, 200)

    char fpath[255] = {0};
    strcpy(fpath, options.root);
    strcat(fpath, "foobar");
    char content[255];
    size_t ret = slurp_file(fpath, content, sizeof(content));
    ASSERT_EQ((int)ret, 6);
    ASSERT_STRN_EQ(content, "foobar", 6);

    unlink(fpath);
    rmdir(tmp_dirname);

    clax_http_request_free(&request);
    clax_http_response_free(&response);
}
TEST_END

TEST_START(clax_dispatch_saves_upload_file_to_file)
{
    opt options;
    clax_ctx_t clax_ctx;
    clax_http_request_t request;
    clax_http_response_t response;

    memset(&clax_ctx, 0, sizeof(clax_ctx_t));
    memset(&options, 0, sizeof(opt));
    clax_http_request_init(&request);
    clax_http_response_init(&response);

    char template[] = "/tmp/tmpdir.XXXXXX";
    char *tmp_dirname = mkdtemp(template);
    mkdir(tmp_dirname, 0777);
    chdir(tmp_dirname);

    strcpy(options.root, tmp_dirname);
    strcat(options.root, "/");
    clax_ctx.options = &options;

    request.method = HTTP_POST;
    strcpy(request.path_info, "/upload");
    strcpy(request.multipart_boundary, "---boundary");

    clax_http_multipart_t *multipart = clax_http_multipart_list_push(&request.multiparts);

    clax_kv_list_push(&multipart->headers, "Content-Disposition", "form-data; name=\"file\"; filename=\"foobar\"");

    char tpath[255] = {0};
    strcpy(tpath, options.root);
    strcat(tpath, ".upload.file");
    clax_dispatcher_write_file(tpath, (const unsigned char *)"hello", 5);

    strcpy(multipart->part_fpath, tpath);
    multipart->part_len = 5;

    clax_dispatch(&clax_ctx, &request, &response);

    ASSERT_EQ(response.status_code, 200)

    char fpath[255] = {0};
    strcpy(fpath, options.root);
    strcat(fpath, "foobar");
    char content[255] = {0};
    size_t ret = slurp_file(fpath, content, sizeof(content));
    ASSERT_EQ((int)ret, 5);
    ASSERT_STR_EQ(content, "hello");

    unlink(fpath);
    rmdir(tmp_dirname);

    clax_http_request_free(&request);
    clax_http_response_free(&response);
}
TEST_END

TEST_START(clax_dispatch_returns_bad_request_when_wrong_params)
{
    opt options;
    clax_ctx_t clax_ctx;
    clax_http_request_t request;
    clax_http_response_t response;

    memset(&clax_ctx, 0, sizeof(clax_ctx_t));
    memset(&options, 0, sizeof(opt));
    clax_http_request_init(&request);
    clax_http_response_init(&response);

    request.method = HTTP_POST;
    strcpy(request.path_info, "/command");
    clax_kv_list_push(&request.body_params, "boo", "bar");

    clax_dispatch(&clax_ctx, &request, &response);

    ASSERT_EQ(response.status_code, 400)

    clax_http_request_free(&request);
    clax_http_response_free(&response);
}
TEST_END
