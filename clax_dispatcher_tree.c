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
#include <stdlib.h>
#include <stdarg.h>
#include <libgen.h> /* basename */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h> /* stat */
#include <unistd.h>
#include <time.h>
#include <utime.h>
#include <errno.h>
#include <dirent.h> /* DIR */

#ifdef MVS
#include "scandir.h"
#endif

#ifdef _WIN32
# include "scandir.h"
# include <windows.h>
#endif

#include "clax.h"
#include "clax_http.h"
#include "clax_log.h"
#include "clax_big_buf.h"
#include "clax_crc32.h"
#include "clax_dispatcher.h"
#include "clax_dispatcher_tree.h"
#include "clax_util.h"

char *clax_last_modified(const char *path, char *buf, size_t max_len)
{
    struct stat st;
    struct tm last_modified_time;
    struct tm *last_modified_time_p;

    if (stat(path, &st) != 0) {
        return NULL;
    }

    if (!clax_is_path_d(path) && !clax_is_path_f(path)) {
        return NULL;
    }

    last_modified_time_p = gmtime_r(&st.st_mtime, &last_modified_time);

    strftime(buf, max_len, "%a, %d %b %Y %H:%M:%S GMT", last_modified_time_p);

    return buf;
}

char *clax_file_size(const char *path, char *buf, size_t max_len)
{
    FILE *fh = fopen(path, "rb");
    size_t size;

    if (fh == NULL) {
        return NULL;
    }

    fseek(fh, 0L, SEEK_END);
    size = ftell(fh);
    fseek(fh, 0L, SEEK_SET);

    fclose(fh);

    snprintf(buf, max_len, "%d", (int)size);

    return buf;
}

void clax_dispatch_tree(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    if (req->method == HTTP_HEAD || req->method == HTTP_GET) {
        clax_dispatch_download(clax_ctx, req, res);
    } else if (req->method == HTTP_POST) {
        clax_dispatch_upload(clax_ctx, req, res);
    } else if (req->method == HTTP_DELETE) {
        clax_dispatch_delete(clax_ctx, req, res);
    } else {
        clax_dispatch_method_not_allowed(clax_ctx, req, res);
    }
}

void clax_dispatch_download(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    char *path = req->path_info + strlen("/tree/");

    char size_buf[255];
    char base_buf[255];
    char last_modified_buf[30];

    if (!clax_is_path_d(path) && !clax_is_path_f(path)) {
        clax_dispatch_not_found(clax_ctx, req, res);
        return;
    }

    clax_last_modified((const char *)path, last_modified_buf, sizeof(last_modified_buf));
    clax_kv_list_push(&res->headers, "Last-Modified", last_modified_buf);

    if (clax_is_path_d(path)) {
        res->status_code = 200;

        char *content_type = "application/vnd.clarive-clax.folder";
        const char *accept = clax_kv_list_find(&req->headers, "Accept");
        int is_html = 0;

        if (accept && strstr(accept, "text/html")) {
            is_html = 1;
            content_type = "text/html; charset=utf-8";
        }

        clax_kv_list_push(&res->headers, "Content-Type", content_type);

        if (req->method == HTTP_GET) {
            if (is_html) {
                clax_big_buf_append_str(&res->body, "<html>\n");
                clax_big_buf_append_str(&res->body, "<head><title>Index of /</title></head>\n");
                clax_big_buf_append_str(&res->body, "<body bgcolor=\"white\">\n");
                clax_big_buf_append_str(&res->body, "<h1>Index of /</h1><hr>");
                clax_big_buf_append_str(&res->body, "<table>\n");
                clax_big_buf_append_str(&res->body, "<tr><td><a href=\"../\">../</a></td><td>Last Modified</td><td>Size</td></tr>\n");
            }
            else {
                clax_big_buf_append_str(&res->body, "[");
            }

            struct dirent **namelist;
            int n;

            n = scandir(path, &namelist, 0, alphasort);

            if (n >= 0) {
                for (int i = 0; i < n; i++) {
                    char *name = namelist[i]->d_name;

                    if (strcmp(name, ".") == 0)
                        continue;
                    if (strcmp(name, "..") == 0)
                        continue;

                    size_t fullpath_maxlen = strlen(path) + strlen(name) + 2;
                    char *fullpath = clax_str_alloc(fullpath_maxlen);

                    clax_strcatfile(fullpath, fullpath_maxlen, path);
                    clax_strcatfile(fullpath, fullpath_maxlen, name);

                    char *line;
                    char last_modified_buf[30];
                    clax_last_modified(fullpath, last_modified_buf, sizeof(last_modified_buf));

                    if (clax_is_path_d(fullpath)) {
                        clax_strcatdir(fullpath, fullpath_maxlen, "/");

                        if (is_html) {
                            line = clax_sprintf_alloc("<tr><td><a href=\"%s/\">%s/</a></td><td><pre>%s</pre></td><td><pre>-</pre></td></tr>\n",
                                    name, name, last_modified_buf);
                        }
                        else {
                            line = clax_sprintf_alloc(
                                    "{"
                                        "\"name\":\"%s\","
                                        "\"path\":\"%s\","
                                        "\"dir\":true,"
                                        "\"last_modified\":\"%s\""
                                    "}", name, fullpath, last_modified_buf);
                        }
                    }
                    else if (clax_is_path_f(fullpath)) {
                        char size_buf[30];
                        clax_file_size(fullpath, size_buf, sizeof(size_buf));

                        if (is_html) {
                            line = clax_sprintf_alloc("<tr><td><a href=\"%s\">%s</a></td><td><pre>%s</pre></td><td><pre>%s</pre></td></tr>\n",
                                    name, name, last_modified_buf, size_buf);
                        }
                        else {
                            line = clax_sprintf_alloc(
                                    "{"
                                        "\"name\":\"%s\","
                                        "\"path\":\"%s\","
                                        "\"dir\":false,"
                                        "\"last_modified\":\"%s\","
                                        "\"size\":\"%s\""
                                    "}", name, fullpath, last_modified_buf, size_buf);
                        }
                    }

                    clax_big_buf_append_str(&res->body, line);
                    free(line);

                    free(fullpath);

                    free(namelist[n]);
                }

                free(namelist);
            }

            if (is_html) {
                clax_big_buf_append_str(&res->body, "</table>\n");
                clax_big_buf_append_str(&res->body, "<hr></body>\n");
                clax_big_buf_append_str(&res->body, "</html>\n");
            }
            else {
                clax_big_buf_append_str(&res->body, "]");
            }
        }

        return;
    }

    unsigned long crc32 = clax_crc32_calc_file(path);
    char crc32_hex[9] = {0};
    snprintf(crc32_hex, sizeof(crc32_hex), "%lx", crc32);

    clax_file_size(path, size_buf, sizeof(size_buf));

    FILE *fh = fopen(path, "rb");

    if (fh == NULL) {
        clax_dispatch_not_found(clax_ctx, req, res);
        return;
    }

    char *base = basename(path);
    strcpy(base_buf, "attachment; filename=\"");
    strcat(base_buf, base);
    strcat(base_buf, "\"");

    res->status_code = 200;
    clax_kv_list_push(&res->headers, "Content-Type", "application/octet-stream");
    clax_kv_list_push(&res->headers, "Content-Disposition", base_buf);
    clax_kv_list_push(&res->headers, "Pragma", "no-cache");
    clax_kv_list_push(&res->headers, "Content-Length", size_buf);

    clax_kv_list_push(&res->headers, "X-Clax-CRC32", crc32_hex);

    if (req->method == HTTP_GET)
        res->body_fh = fh;
    else
        fclose(fh);
}

void clax_dispatch_upload(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    char *subdir = req->path_info + strlen("/tree/");

    if (strlen(subdir)) {
        clax_mkdir_p(subdir);

        if (!clax_is_path_d(subdir)) {
            clax_log("Output directory '%s' does not exist", subdir);

            clax_dispatch_bad_request(clax_ctx, req, res, "Output directory does not exist");
            return;
        }
    }

    if (req->continue_expected) {
        return;
    }

    if (strlen(req->multipart_boundary)) {
        int i;
        for (i = 0; i < req->multiparts.size; i++) {
            clax_http_multipart_t *multipart = clax_http_multipart_list_at(&req->multiparts, i);

            const char *content_disposition = clax_kv_list_find(&multipart->headers, "Content-Disposition");
            if (!content_disposition)
                continue;

            char prefix[] = "form-data; ";
            if (strncmp(content_disposition, prefix, strlen(prefix)) != 0)
                continue;

            const char *kv = content_disposition + strlen(prefix);
            size_t name_len;
            size_t filename_len;
            const char *name = clax_http_extract_kv(kv, "name", &name_len);
            const char *filename = clax_http_extract_kv(kv, "filename", &filename_len);

            if (!name || !filename || (strncmp(name, "file", name_len) != 0))
                continue;

            char *new_name = clax_kv_list_find(&req->query_params, "name");
            char *crc32 = clax_kv_list_find(&req->query_params, "crc");
            char *time = clax_kv_list_find(&req->query_params, "time");

            if (crc32 && strlen(crc32) != 8) {
                clax_dispatch_bad_request(clax_ctx, req, res, "Invalid CRC32");
                return;
            }

            char *fpath;

            if (new_name && strlen(new_name)) {
                fpath = clax_strjoin("/", subdir, new_name, NULL);
            }
            else {
                char *p = clax_strndup(filename, filename_len);
                fpath = clax_strjoin("/", subdir, p, NULL);
                free(p);
            }

            clax_san_path(fpath);

            int ret = clax_big_buf_write_file(&multipart->bbuf, fpath);

            if (ret < 0) {
                clax_log("Saving file failed: %s\n", fpath);
                clax_dispatch_system_error(clax_ctx, req, res, "Saving file failed");
            }
            else {
                if (crc32 && strlen(crc32)) {
                    unsigned long got_crc32 = clax_htol(crc32);
                    unsigned long exp_crc32 = clax_crc32_calc_file(fpath);

                    if (got_crc32 != exp_crc32) {
                        clax_log("CRC mismatch %u != %u (%s)", exp_crc32, got_crc32, crc32);
                        clax_dispatch_bad_request(clax_ctx, req, res, "CRC32 mismatch");

                        remove(fpath);
                        free(fpath);

                        return;
                    } else {
                        clax_log("CRC ok %u != %u (%s)", exp_crc32, got_crc32, crc32);
                    }
                }

                if (time && strlen(time)) {
                    int mtime = atol(time);

                    struct utimbuf t;
                    t.actime = mtime;
                    t.modtime = mtime;
                    int ok = utime(fpath, &t);

                    if (ok < 0)
                        clax_log("utime on file '%s' failed: %s", fpath, strerror(errno));
                }

                clax_dispatch_success(clax_ctx, req, res);
            }

            free(fpath);

            break;
        }
    }
    else {
        char *dirname = clax_kv_list_find(&req->body_params, "dirname");
        if (dirname && strlen(dirname)) {
            clax_san_path(dirname);

            char *fullpath = clax_strjoin("/", subdir, dirname, NULL);
            if (clax_mkdir_p(fullpath) < 0) {
                clax_dispatch_system_error(clax_ctx, req, res, "Can't create directory");
            }
            else {
                clax_dispatch_success(clax_ctx, req, res);
            }
            free(fullpath);
        }
    }

    if (!res->status_code) {
        clax_dispatch_bad_request(clax_ctx, req, res, NULL);
    }
}

void clax_dispatch_delete(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    char *path = req->path_info + strlen("/tree/");
    char *error = NULL;

    int is_file = clax_is_path_f(path);
    if (is_file || clax_is_path_d(path)) {
        if (is_file) {
            if (unlink(path) < 0) {
                error = clax_sprintf_alloc("Can't delete file: %s", strerror(errno));
            }
        }
        else {
            char *recursive = clax_kv_list_find(&req->query_params, "recursive");

            if (recursive && strlen(recursive) && strcmp(recursive, "1") == 0) {
                if (clax_rmpath_r(path) < 0) {
                    error = clax_sprintf_alloc("Can't remove directory recursively: %s", strerror(errno));
                }
            }
            else if (rmdir(path) < 0) {
                error = clax_sprintf_alloc("Can't remove directory: %s", strerror(errno));
            }
        }

        if (error) {
            clax_dispatch_system_error(clax_ctx, req, res, error);

            free(error);
        }
        else {
            clax_dispatch_success(clax_ctx, req, res);
        }
    }
    else {
        clax_dispatch_not_found(clax_ctx, req, res);
    }
}

