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

#include "contrib/u/u.h"

#include "clax_dispatcher_tree.h"
#include "clax_util.h"
#include "clax_platform.h"
#include "u_util.h"

SUITE_START(clax_dispatcher_tree)

/*TEST_START(saves_upload_to_file)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char cwd[1024];*/
    /*getcwd(cwd, sizeof(cwd));*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*chdir(tmp_dirname);*/

    /*request.method = HTTP_POST;*/
    /*strcpy(request.path_info, "/tree/");*/
    /*strcpy(request.multipart_boundary, "---boundary");*/

    /*clax_http_multipart_t *multipart = clax_http_multipart_list_push(&request.multiparts);*/
    /*clax_kv_list_push(&multipart->headers, "Content-Disposition", "form-data; name=\"file\"; filename=\"foobar\"");*/
    /*clax_big_buf_append(&multipart->bbuf, (const unsigned char *)"upload_to_file", 14);*/

    /*clax_dispatch_upload(&clax_ctx, &request, &response);*/

    /*ASSERT_EQ(response.status_code, 200)*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*char content[255];*/
    /*size_t ret = slurp_file("foobar", content, sizeof(content));*/
    /*ASSERT_EQ((int)ret, 14);*/
    /*ASSERT_STRN_EQ(content, "upload_to_file", 14);*/

    /*chdir(cwd);*/
    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

/*TEST_START(saves_upload_to_file_with_another_name)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char cwd[1024];*/
    /*getcwd(cwd, sizeof(cwd));*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*chdir(tmp_dirname);*/

    /*request.method = HTTP_POST;*/
    /*strcpy(request.path_info, "/tree/");*/
    /*clax_kv_list_push(&request.query_params, "name", "another-name");*/
    /*strcpy(request.multipart_boundary, "---boundary");*/

    /*clax_http_multipart_t *multipart = clax_http_multipart_list_push(&request.multiparts);*/

    /*clax_kv_list_push(&multipart->headers, "Content-Disposition", "form-data; name=\"file\"; filename=\"foobar\"");*/

    /*clax_big_buf_append(&multipart->bbuf, (const unsigned char *)"foobar", 6);*/

    /*clax_dispatch_upload(&clax_ctx, &request, &response);*/

    /*ASSERT_EQ(response.status_code, 200)*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*char content[255];*/
    /*size_t ret = slurp_file("another-name", content, sizeof(content));*/
    /*ASSERT_EQ((int)ret, 6);*/
    /*ASSERT_STRN_EQ(content, "foobar", 6);*/

    /*chdir(cwd);*/
    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

/*TEST_START(saves_upload_to_another_dir)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char cwd[1024];*/
    /*getcwd(cwd, sizeof(cwd));*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*chdir(tmp_dirname);*/
    /*clax_mkdir("subdir", 0755);*/

    /*request.method = HTTP_POST;*/
    /*strcpy(request.path_info, "/tree/subdir");*/
    /*strcpy(request.multipart_boundary, "---boundary");*/

    /*clax_http_multipart_t *multipart = clax_http_multipart_list_push(&request.multiparts);*/

    /*clax_kv_list_push(&multipart->headers, "Content-Disposition", "form-data; name=\"file\"; filename=\"foobar\"");*/

    /*clax_big_buf_append(&multipart->bbuf, (const unsigned char *)"foobar", 6);*/

    /*clax_dispatch_upload(&clax_ctx, &request, &response);*/

    /*ASSERT_EQ(response.status_code, 200)*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*char content[255];*/
    /*size_t ret = slurp_file("subdir/foobar", content, sizeof(content));*/
    /*ASSERT_EQ((int)ret, 6);*/
    /*ASSERT_STRN_EQ(content, "foobar", 6);*/

    /*chdir(cwd);*/
    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

/*TEST_START(rejects_upload_if_crc_fails)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char cwd[1024];*/
    /*getcwd(cwd, sizeof(cwd));*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*chdir(tmp_dirname);*/

    /*request.method = HTTP_POST;*/
    /*strcpy(request.path_info, "/tree/");*/
    /*clax_kv_list_push(&request.query_params, "crc", "12345678");*/
    /*strcpy(request.multipart_boundary, "---boundary");*/

    /*clax_http_multipart_t *multipart = clax_http_multipart_list_push(&request.multiparts);*/

    /*clax_kv_list_push(&multipart->headers, "Content-Disposition", "form-data; name=\"file\"; filename=\"foobar\"");*/

    /*clax_big_buf_append(&multipart->bbuf, (const unsigned char *)"foobar", 6);*/

    /*clax_dispatch_upload(&clax_ctx, &request, &response);*/

    /*ASSERT_EQ(response.status_code, 400)*/

    /*ASSERT_EQ(access("boobar", F_OK), -1);*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*chdir(cwd);*/
    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

/*TEST_START(accepts_upload_with_correct_crc)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char cwd[1024];*/
    /*getcwd(cwd, sizeof(cwd));*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*chdir(tmp_dirname);*/

    /*request.method = HTTP_POST;*/
    /*strcpy(request.path_info, "/tree/");*/
    /*clax_kv_list_push(&request.query_params, "crc", "9ef61f95");*/
    /*strcpy(request.multipart_boundary, "---boundary");*/

    /*clax_http_multipart_t *multipart = clax_http_multipart_list_push(&request.multiparts);*/

    /*clax_kv_list_push(&multipart->headers, "Content-Disposition", "form-data; name=\"file\"; filename=\"foobar\"");*/

    /*clax_big_buf_append(&multipart->bbuf, (const unsigned char *)"\x66\x6F\x6F\x62\x61\x72", 6);*/

    /*clax_dispatch_upload(&clax_ctx, &request, &response);*/

    /*ASSERT_EQ(response.status_code, 200)*/
    /*ASSERT_EQ(access("foobar", F_OK), 0);*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*chdir(cwd);*/
    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

/*TEST_START(accepts_upload_with_correct_crc padded)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char cwd[1024];*/
    /*getcwd(cwd, sizeof(cwd));*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*chdir(tmp_dirname);*/

    /*request.method = HTTP_POST;*/
    /*strcpy(request.path_info, "/tree/");*/
    /*clax_kv_list_push(&request.query_params, "crc", "0f5cb862");*/
    /*strcpy(request.multipart_boundary, "---boundary");*/

    /*clax_http_multipart_t *multipart = clax_http_multipart_list_push(&request.multiparts);*/

    /*clax_kv_list_push(&multipart->headers, "Content-Disposition", "form-data; name=\"file\"; filename=\"foobar\"");*/

    /*clax_big_buf_append(&multipart->bbuf, (const unsigned char *)"RLXK0tyT", 8);*/

    /*clax_dispatch_upload(&clax_ctx, &request, &response);*/

    /*ASSERT_EQ(response.status_code, 200)*/
    /*ASSERT_EQ(access("foobar", F_OK), 0);*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*chdir(cwd);*/
    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

/*TEST_START(saves_upload_with_passed_time)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char cwd[1024];*/
    /*getcwd(cwd, sizeof(cwd));*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*chdir(tmp_dirname);*/

    /*request.method = HTTP_POST;*/
    /*strcpy(request.path_info, "/tree/");*/
    /*clax_kv_list_push(&request.query_params, "time", "1234567890");*/
    /*strcpy(request.multipart_boundary, "---boundary");*/

    /*clax_http_multipart_t *multipart = clax_http_multipart_list_push(&request.multiparts);*/
    /*clax_kv_list_push(&multipart->headers, "Content-Disposition", "form-data; name=\"file\"; filename=\"foobar\"");*/
    /*clax_big_buf_append(&multipart->bbuf, (const unsigned char *)"time", 4);*/

    /*clax_dispatch_upload(&clax_ctx, &request, &response);*/

    /*ASSERT_EQ(response.status_code, 200)*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*struct stat st;*/

    /*ASSERT_EQ(stat("foobar", &st), 0);*/

    /*ASSERT_EQ(st.st_mtime, 1234567890);*/

    /*chdir(cwd);*/
    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

/*TEST_START(creates directory)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char cwd[1024];*/
    /*getcwd(cwd, sizeof(cwd));*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*chdir(tmp_dirname);*/

    /*request.method = HTTP_POST;*/
    /*strcpy(request.path_info, "/tree/");*/
    /*clax_kv_list_push(&request.body_params, "dirname", "foo");*/

    /*clax_dispatch_upload(&clax_ctx, &request, &response);*/

    /*chdir(cwd);*/

    /*ASSERT_EQ(response.status_code, 200);*/

    /*char *resultpath = clax_strjoin("/", tmp_dirname, "foo", NULL);*/
    /*ASSERT_EQ(clax_is_path_d(resultpath), 1);*/
    /*free(resultpath);*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

/*TEST_START(returns ok when file exists)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char cwd[1024];*/
    /*getcwd(cwd, sizeof(cwd));*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*chdir(tmp_dirname);*/

    /*clax_touch("foo");*/

    /*request.method = HTTP_HEAD;*/
    /*strcpy(request.path_info, "/tree/foo");*/

    /*clax_dispatch_download(&clax_ctx, &request, &response);*/

    /*ASSERT_EQ(response.status_code, 200)*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*chdir(cwd);*/
    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

/*TEST_START(returns ok when directory exists)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char cwd[1024];*/
    /*getcwd(cwd, sizeof(cwd));*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*chdir(tmp_dirname);*/

    /*clax_mkdir("foo", 0755);*/

    /*request.method = HTTP_HEAD;*/
    /*strcpy(request.path_info, "/tree/foo");*/

    /*clax_dispatch_download(&clax_ctx, &request, &response);*/

    /*ASSERT_EQ(response.status_code, 200)*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*chdir(cwd);*/
    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

/*TEST_START(serves_404_when_file_not_found)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char cwd[1024];*/
    /*getcwd(cwd, sizeof(cwd));*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*chdir(tmp_dirname);*/

    /*request.method = HTTP_GET;*/
    /*strcpy(request.path_info, "/tree/foo");*/

    /*clax_dispatch_download(&clax_ctx, &request, &response);*/

    /*ASSERT_EQ(response.status_code, 404)*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*chdir(cwd);*/
    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

/*TEST_START(serves_file_as_attachment)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char cwd[1024];*/
    /*getcwd(cwd, sizeof(cwd));*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*chdir(tmp_dirname);*/

    /*FILE *fp = fopen("foo", "wb");*/
    /*char *buf = "hello";*/
    /*fwrite(buf, 1, strlen(buf), fp);*/
    /*fclose(fp);*/

    /*request.method = HTTP_GET;*/
    /*strcpy(request.path_info, "/tree/foo");*/

    /*clax_dispatch_download(&clax_ctx, &request, &response);*/

    /*ASSERT_EQ(response.status_code, 200)*/
    /*ASSERT_NOT_NULL(clax_kv_list_find(&response.headers, "Last-Modified"))*/

    /*char *content_length = clax_kv_list_find(&response.headers, "Content-Length");*/
    /*ASSERT_STR_EQ(content_length, "5");*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*chdir(cwd);*/
    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

/*TEST_START(serves file with crc32)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char cwd[1024];*/
    /*getcwd(cwd, sizeof(cwd));*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*chdir(tmp_dirname);*/

    /*FILE *fp = fopen("foo", "wb");*/
    /*char *buf = "hello";*/
    /*fwrite(buf, 1, strlen(buf), fp);*/
    /*fclose(fp);*/

    /*request.method = HTTP_GET;*/
    /*strcpy(request.path_info, "/tree/foo");*/

    /*clax_dispatch_download(&clax_ctx, &request, &response);*/

/*#ifdef MVS*/
    /*ASSERT_STR_EQ(clax_kv_list_find(&response.headers, "X-Clax-CRC32"), "2166a346")*/
/*#else*/
    /*ASSERT_STR_EQ(clax_kv_list_find(&response.headers, "X-Clax-CRC32"), "3610a686")*/
/*#endif*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*chdir(cwd);*/
    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

/*TEST_START(serves file with padded crc32)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char cwd[1024];*/
    /*getcwd(cwd, sizeof(cwd));*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*chdir(tmp_dirname);*/

    /*FILE *fp = fopen("foo", "wb");*/
    /*char *buf = "RLXK0tyT";*/
    /*fwrite(buf, 1, strlen(buf), fp);*/
    /*fclose(fp);*/

    /*request.method = HTTP_GET;*/
    /*strcpy(request.path_info, "/tree/foo");*/

    /*clax_dispatch_download(&clax_ctx, &request, &response);*/

/*#ifdef MVS*/
    /*ASSERT_STR_EQ(clax_kv_list_find(&response.headers, "X-Clax-CRC32"), "2166a346")*/
/*#else*/
    /*ASSERT_STR_EQ(clax_kv_list_find(&response.headers, "X-Clax-CRC32"), "0f5cb862")*/
/*#endif*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*chdir(cwd);*/
    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

/*TEST_START(deletes file)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char cwd[1024];*/
    /*getcwd(cwd, sizeof(cwd));*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*chdir(tmp_dirname);*/

    /*FILE *fp = fopen("foobar", "wb");*/
    /*char *buf = "hello";*/
    /*fwrite(buf, 1, strlen(buf), fp);*/
    /*fclose(fp);*/

    /*request.method = HTTP_DELETE;*/
    /*strcpy(request.path_info, "/tree/foobar");*/

    /*clax_dispatch_delete(&clax_ctx, &request, &response);*/

    /*ASSERT_EQ(response.status_code, 200)*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*ASSERT_EQ(access("foobar", F_OK), -1);*/

    /*chdir(cwd);*/
    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

/*TEST_START(deletes directory)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*char *path = clax_sprintf_alloc("%s/%s", tmp_dirname, "empty-dir");*/

    /*char *path_info = clax_sprintf_alloc("/tree/%s", path);*/

    /*clax_mkdir_p(path);*/

    /*request.method = HTTP_DELETE;*/
    /*strcpy(request.path_info, path_info);*/

    /*clax_dispatch_delete(&clax_ctx, &request, &response);*/

    /*ASSERT_EQ(response.status_code, 200)*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*ASSERT_EQ(access(path, F_OK), -1);*/

    /*free(path);*/
    /*free(path_info);*/
    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

/*TEST_START(deletes directory recursively)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*char *path = clax_sprintf_alloc("%s/%s", tmp_dirname, "empty-dir");*/

    /*char *path_info = clax_sprintf_alloc("/tree/%s", tmp_dirname);*/
    /*printf("path_info=%s\n", path_info);*/

    /*clax_mkdir_p(path);*/

    /*request.method = HTTP_DELETE;*/
    /*strcpy(request.path_info, path_info);*/
    /*clax_kv_list_push(&request.query_params, "recursive", "1");*/

    /*clax_dispatch_delete(&clax_ctx, &request, &response);*/

    /*ASSERT_EQ(response.status_code, 200)*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*ASSERT_EQ(access(tmp_dirname, F_OK), -1);*/

    /*free(path);*/
    /*free(path_info);*/
    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

/*TEST_START(returns error when deleting unknown file)*/
/*{*/
    /*clax_ctx_t clax_ctx;*/
    /*clax_http_request_t request;*/
    /*clax_http_response_t response;*/

    /*clax_ctx_init(&clax_ctx);*/
    /*clax_http_request_init(&request, NULL);*/
    /*clax_http_response_init(&response, NULL, 0);*/

    /*char cwd[1024];*/
    /*getcwd(cwd, sizeof(cwd));*/

    /*char *tmp_dirname = clax_mktmpdir_alloc();*/
    /*chdir(tmp_dirname);*/

    /*request.method = HTTP_DELETE;*/
    /*strcpy(request.path_info, "/tree/foobar");*/

    /*clax_dispatch_delete(&clax_ctx, &request, &response);*/

    /*ASSERT_EQ(response.status_code, 404)*/

    /*clax_http_request_free(&request);*/
    /*clax_http_response_free(&response);*/
    /*clax_ctx_free(&clax_ctx);*/

    /*chdir(cwd);*/
    /*rmrf(tmp_dirname);*/
/*}*/
/*TEST_END*/

SUITE_END
