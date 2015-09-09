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

#include "clax.h"
#include "clax_dispatcher.h"
#include "u.h"
#include "u_util.h"

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

TEST_START(clax_dispatch_saves_upload_to_file)
{
    opt options;
    clax_ctx_t clax_ctx;
    clax_http_request_t request;
    clax_http_response_t response;

    memset(&clax_ctx, 0, sizeof(clax_ctx_t));
    memset(&options, 0, sizeof(opt));
    clax_http_request_init(&request);
    clax_http_response_init(&response);

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
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

    clax_big_buf_append(&multipart->bbuf, (const unsigned char *)"foobar", 6);

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
    chdir(cwd);
    rmdir(tmp_dirname);

    clax_http_request_free(&request);
    clax_http_response_free(&response);
}
TEST_END

TEST_START(clax_dispatch_saves_upload_to_file_with_another_name)
{
    opt options;
    clax_ctx_t clax_ctx;
    clax_http_request_t request;
    clax_http_response_t response;

    memset(&clax_ctx, 0, sizeof(clax_ctx_t));
    memset(&options, 0, sizeof(opt));
    clax_http_request_init(&request);
    clax_http_response_init(&response);

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    char template[] = "/tmp/tmpdir.XXXXXX";
    char *tmp_dirname = mkdtemp(template);
    mkdir(tmp_dirname, 0755);
    chdir(tmp_dirname);

    strcpy(options.root, tmp_dirname);
    strcat(options.root, "/");
    clax_ctx.options = &options;

    request.method = HTTP_POST;
    strcpy(request.path_info, "/upload");
    clax_kv_list_push(&request.query_params, "name", "another-name");
    strcpy(request.multipart_boundary, "---boundary");

    clax_http_multipart_t *multipart = clax_http_multipart_list_push(&request.multiparts);

    clax_kv_list_push(&multipart->headers, "Content-Disposition", "form-data; name=\"file\"; filename=\"foobar\"");

    clax_big_buf_append(&multipart->bbuf, (const unsigned char *)"foobar", 6);

    clax_dispatch(&clax_ctx, &request, &response);

    ASSERT_EQ(response.status_code, 200)

    char fpath[255] = {0};
    strcpy(fpath, options.root);
    strcat(fpath, "another-name");
    char content[255];
    size_t ret = slurp_file(fpath, content, sizeof(content));
    ASSERT_EQ((int)ret, 6);
    ASSERT_STRN_EQ(content, "foobar", 6);

    unlink(fpath);
    chdir(cwd);
    rmdir(tmp_dirname);

    clax_http_request_free(&request);
    clax_http_response_free(&response);
}
TEST_END

TEST_START(clax_dispatch_saves_upload_to_another_dir)
{
    opt options;
    clax_ctx_t clax_ctx;
    clax_http_request_t request;
    clax_http_response_t response;

    memset(&clax_ctx, 0, sizeof(clax_ctx_t));
    memset(&options, 0, sizeof(opt));
    clax_http_request_init(&request);
    clax_http_response_init(&response);

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    char template[] = "/tmp/tmpdir.XXXXXX";
    char *tmp_dirname = mkdtemp(template);
    mkdir(tmp_dirname, 0755);
    chdir(tmp_dirname);
    mkdir("subdir", 0755);

    strcpy(options.root, tmp_dirname);
    strcat(options.root, "/");
    clax_ctx.options = &options;

    request.method = HTTP_POST;
    strcpy(request.path_info, "/upload");
    clax_kv_list_push(&request.query_params, "dir", "subdir");
    strcpy(request.multipart_boundary, "---boundary");

    clax_http_multipart_t *multipart = clax_http_multipart_list_push(&request.multiparts);

    clax_kv_list_push(&multipart->headers, "Content-Disposition", "form-data; name=\"file\"; filename=\"foobar\"");

    clax_big_buf_append(&multipart->bbuf, (const unsigned char *)"foobar", 6);

    clax_dispatch(&clax_ctx, &request, &response);

    ASSERT_EQ(response.status_code, 200)

    char fpath[255] = {0};
    strcpy(fpath, options.root);
    strcat(fpath, "subdir/foobar");
    char content[255];
    size_t ret = slurp_file(fpath, content, sizeof(content));
    ASSERT_EQ((int)ret, 6);
    ASSERT_STRN_EQ(content, "foobar", 6);

    unlink(fpath);
    rmdir("subdir");
    chdir(cwd);
    rmdir(tmp_dirname);

    clax_http_request_free(&request);
    clax_http_response_free(&response);
}
TEST_END

TEST_START(clax_dispatch_rejects_upload_if_crc_fails)
{
    opt options;
    clax_ctx_t clax_ctx;
    clax_http_request_t request;
    clax_http_response_t response;

    memset(&clax_ctx, 0, sizeof(clax_ctx_t));
    memset(&options, 0, sizeof(opt));
    clax_http_request_init(&request);
    clax_http_response_init(&response);

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    char template[] = "/tmp/tmpdir.XXXXXX";
    char *tmp_dirname = mkdtemp(template);
    mkdir(tmp_dirname, 0755);
    chdir(tmp_dirname);

    strcpy(options.root, tmp_dirname);
    strcat(options.root, "/");
    clax_ctx.options = &options;

    request.method = HTTP_POST;
    strcpy(request.path_info, "/upload");
    clax_kv_list_push(&request.query_params, "crc", "12345678");
    strcpy(request.multipart_boundary, "---boundary");

    clax_http_multipart_t *multipart = clax_http_multipart_list_push(&request.multiparts);

    clax_kv_list_push(&multipart->headers, "Content-Disposition", "form-data; name=\"file\"; filename=\"foobar\"");

    clax_big_buf_append(&multipart->bbuf, (const unsigned char *)"foobar", 6);

    clax_dispatch(&clax_ctx, &request, &response);

    ASSERT_EQ(response.status_code, 400)

    char fpath[255] = {0};
    strcpy(fpath, options.root);
    strcat(fpath, "foobar");
    ASSERT_EQ(access(fpath, F_OK), -1);

    chdir(cwd);
    rmdir(tmp_dirname);

    clax_http_request_free(&request);
    clax_http_response_free(&response);
}
TEST_END

TEST_START(clax_dispatch_accepts_upload_with_correct_crc)
{
    opt options;
    clax_ctx_t clax_ctx;
    clax_http_request_t request;
    clax_http_response_t response;

    memset(&clax_ctx, 0, sizeof(clax_ctx_t));
    memset(&options, 0, sizeof(opt));
    clax_http_request_init(&request);
    clax_http_response_init(&response);

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    char template[] = "/tmp/tmpdir.XXXXXX";
    char *tmp_dirname = mkdtemp(template);
    mkdir(tmp_dirname, 0755);
    chdir(tmp_dirname);

    strcpy(options.root, tmp_dirname);
    strcat(options.root, "/");
    clax_ctx.options = &options;

    request.method = HTTP_POST;
    strcpy(request.path_info, "/upload");
    clax_kv_list_push(&request.query_params, "crc", "9ef61f95");
    strcpy(request.multipart_boundary, "---boundary");

    clax_http_multipart_t *multipart = clax_http_multipart_list_push(&request.multiparts);

    clax_kv_list_push(&multipart->headers, "Content-Disposition", "form-data; name=\"file\"; filename=\"foobar\"");

    clax_big_buf_append(&multipart->bbuf, (const unsigned char *)"foobar", 6);

    clax_dispatch(&clax_ctx, &request, &response);

    ASSERT_EQ(response.status_code, 200)

    char fpath[255] = {0};
    strcpy(fpath, options.root);
    strcat(fpath, "foobar");
    ASSERT_EQ(access(fpath, F_OK), 0);

    chdir(cwd);
    rmdir(tmp_dirname);

    clax_http_request_free(&request);
    clax_http_response_free(&response);
}
TEST_END

TEST_START(clax_dispatch_serves_404_when_file_not_found)
{
    opt options;
    clax_ctx_t clax_ctx;
    clax_http_request_t request;
    clax_http_response_t response;

    memset(&clax_ctx, 0, sizeof(clax_ctx_t));
    memset(&options, 0, sizeof(opt));
    clax_http_request_init(&request);
    clax_http_response_init(&response);

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    char template[] = "/tmp/tmpdir.XXXXXX";
    char *tmp_dirname = mkdtemp(template);
    mkdir(tmp_dirname, 0755);
    chdir(tmp_dirname);

    strcpy(options.root, tmp_dirname);
    strcat(options.root, "/");
    clax_ctx.options = &options;

    request.method = HTTP_GET;
    strcpy(request.path_info, "/download");
    clax_kv_list_push(&request.query_params, "file", "foo");

    clax_dispatch(&clax_ctx, &request, &response);

    ASSERT_EQ(response.status_code, 404)

    chdir(cwd);
    rmdir(tmp_dirname);

    clax_http_request_free(&request);
    clax_http_response_free(&response);
}
TEST_END

TEST_START(clax_dispatch_serves_file_as_attachment)
{
    opt options;
    clax_ctx_t clax_ctx;
    clax_http_request_t request;
    clax_http_response_t response;

    memset(&clax_ctx, 0, sizeof(clax_ctx_t));
    memset(&options, 0, sizeof(opt));
    clax_http_request_init(&request);
    clax_http_response_init(&response);

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    char template[] = "/tmp/tmpdir.XXXXXX";
    char *tmp_dirname = mkdtemp(template);
    mkdir(tmp_dirname, 0755);
    chdir(tmp_dirname);

    strcpy(options.root, tmp_dirname);
    strcat(options.root, "/");
    clax_ctx.options = &options;

    char fpath[255];
    strcpy(fpath, clax_ctx.options->root);
    strcat(fpath, "foo");
    FILE *fp = fopen(fpath, "wb");
    char *buf = "hello";
    fwrite(buf, 1, strlen(buf), fp);
    fclose(fp);

    request.method = HTTP_GET;
    strcpy(request.path_info, "/download");
    clax_kv_list_push(&request.query_params, "file", "foo");

    clax_dispatch(&clax_ctx, &request, &response);

    ASSERT_EQ(response.status_code, 200)
    ASSERT(clax_kv_list_find(&response.headers, "Last-Modified"))

    chdir(cwd);
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
