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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "u/u.h"

#include "u_util.h"
#include "clax_util.h"

SUITE_START(clax_util)

TEST_START(default_values)
{
    clax_kv_list_t list;

    clax_kv_list_init(&list);

    ASSERT_EQ(list.size, 0);

    clax_kv_list_free(&list);
}
TEST_END

TEST_START(pushes_kv)
{
    clax_kv_list_t list;

    clax_kv_list_init(&list);

    clax_kv_list_push(&list, "foo", "bar");
    ASSERT_EQ(list.size, 1);

    clax_kv_list_push(&list, "bar", "baz");
    ASSERT_EQ(list.size, 2);

    clax_kv_list_push(&list, NULL, NULL);
    ASSERT_EQ(list.size, 3);

    clax_kv_list_free(&list);
}
TEST_END

TEST_START(pushes_buf_kv)
{
    clax_kv_list_t list;

    clax_kv_list_init(&list);

    clax_kv_list_pushn(&list, "foo", 3, "bar", 3);
    ASSERT_EQ(list.size, 1);
    ASSERT_STR_EQ(clax_kv_list_find(&list, "foo"), "bar");

    clax_kv_list_pushn(&list, "", 0, "bar", 3);
    ASSERT_EQ(list.size, 2);
    ASSERT_STR_EQ(clax_kv_list_find(&list, ""), "bar");

    clax_kv_list_pushn(&list, "baz", 3, "", 0);
    ASSERT_EQ(list.size, 3);
    ASSERT_STR_EQ(clax_kv_list_find(&list, "baz"), "");

    clax_kv_list_pushn(&list, "", 0, "", 0);
    ASSERT_EQ(list.size, 4);
    size_t start = 3;
    ASSERT_STR_EQ(clax_kv_list_find_all(&list, "", &start), "");

    clax_kv_list_free(&list);
}
TEST_END

TEST_START(finds_all_kv)
{
    size_t start = 0;
    clax_kv_list_t list;

    clax_kv_list_init(&list);

    ASSERT(clax_kv_list_find_all(&list, "foo", &start) == NULL);
    ASSERT_EQ(start, 0);

    start = 100;
    ASSERT(clax_kv_list_find_all(&list, "foo", &start) == NULL);
    ASSERT_EQ(start, 100);

    clax_kv_list_push(&list, "foo", "bar");
    clax_kv_list_push(&list, "bar", "baz");
    clax_kv_list_push(&list, "bar", "zab");

    start = 0;
    ASSERT_STR_EQ(clax_kv_list_find_all(&list, "foo", &start), "bar");
    ASSERT_EQ(start, 1);
    ASSERT(clax_kv_list_find_all(&list, "foo", &start) == NULL);
    ASSERT(clax_kv_list_find_all(&list, "foo", &start) == NULL);

    start = 0;
    ASSERT_STR_EQ(clax_kv_list_find_all(&list, "bar", &start), "baz");
    ASSERT_STR_EQ(clax_kv_list_find_all(&list, "bar", &start), "zab");
    ASSERT(clax_kv_list_find_all(&list, "bar", &start) == NULL);
    ASSERT(clax_kv_list_find_all(&list, "bar", &start) == NULL);

    clax_kv_list_free(&list);
}
TEST_END

TEST_START(finds_kv)
{
    clax_kv_list_t list;

    clax_kv_list_init(&list);

    ASSERT(clax_kv_list_find(&list, "foo") == NULL);

    clax_kv_list_push(&list, "foo", "bar");
    clax_kv_list_push(&list, "bar", "baz");
    clax_kv_list_push(&list, "bar", "zab");

    ASSERT_STR_EQ(clax_kv_list_find(&list, "foo"), "bar");
    ASSERT_STR_EQ(clax_kv_list_find(&list, "bar"), "baz");
    ASSERT_STR_EQ(clax_kv_list_find(&list, "bar"), "baz");

    ASSERT(clax_kv_list_find(&list, "unknown") == NULL);

    clax_kv_list_free(&list);
}
TEST_END

TEST_START(at_kv)
{
    clax_kv_list_t list;
    clax_kv_list_item_t *item;

    clax_kv_list_init(&list);

    ASSERT(clax_kv_list_at(&list, 0) == NULL);
    ASSERT(clax_kv_list_at(&list, 10) == NULL);

    clax_kv_list_push(&list, "foo", "bar");
    clax_kv_list_push(&list, "bar", "baz");
    clax_kv_list_push(&list, "bar", "zab");

    item = clax_kv_list_at(&list, 0);
    ASSERT_STR_EQ(item->key, "foo");
    ASSERT_STR_EQ(item->val, "bar");

    item = clax_kv_list_at(&list, 1);
    ASSERT_STR_EQ(item->key, "bar");
    ASSERT_STR_EQ(item->val, "baz");

    item = clax_kv_list_at(&list, 2);
    ASSERT_STR_EQ(item->key, "bar");
    ASSERT_STR_EQ(item->val, "zab");

    ASSERT(clax_kv_list_at(&list, 3) == NULL);

    clax_kv_list_free(&list);
}
TEST_END

TEST_START(iter_kv)
{
    clax_kv_list_t list;
    clax_kv_list_item_t *item;
    size_t iter = 0;

    clax_kv_list_init(&list);

    ASSERT(clax_kv_list_next(&list, &iter) == NULL);
    ASSERT(clax_kv_list_next(&list, &iter) == NULL);

    clax_kv_list_push(&list, "foo", "bar");
    clax_kv_list_push(&list, "bar", "baz");
    clax_kv_list_push(&list, "bar", "zab");

    item = clax_kv_list_next(&list, &iter);
    ASSERT_EQ(iter, 1);
    ASSERT_STR_EQ(item->key, "foo");
    ASSERT_STR_EQ(item->val, "bar");

    item = clax_kv_list_next(&list, &iter);
    ASSERT_EQ(iter, 2);
    ASSERT_STR_EQ(item->key, "bar");
    ASSERT_STR_EQ(item->val, "baz");

    item = clax_kv_list_next(&list, &iter);
    ASSERT_EQ(iter, 3);
    ASSERT_STR_EQ(item->key, "bar");
    ASSERT_STR_EQ(item->val, "zab");

    ASSERT(clax_kv_list_next(&list, &iter) == NULL);
    ASSERT(clax_kv_list_next(&list, &iter) == NULL);

    clax_kv_list_free(&list);
}
TEST_END

TEST_START(sets_new_val)
{
    clax_kv_list_t list;

    clax_kv_list_init(&list);

    clax_kv_list_set(&list, "foo", "bar");
    ASSERT_STR_EQ(clax_kv_list_find(&list, "foo"), "bar");

    clax_kv_list_set(&list, "foo", "baz");
    ASSERT_STR_EQ(clax_kv_list_find(&list, "foo"), "baz");

    clax_kv_list_free(&list);
}
TEST_END

TEST_START(clax_buf2str_allocates_str_from_buffer)
{
    char *p = clax_buf2str("foo", 3);

    ASSERT_EQ(strlen(p), 3);

    free(p);
}
TEST_END

TEST_START(clax_buf_append_appends_buffer)
{
    unsigned char *buf = NULL;
    size_t len = 0;

    clax_buf_append(&buf, &len, "foo", 3);
    ASSERT_EQ(len, 3);
    ASSERT_BUF_EQ(buf, "foo", 3);

    clax_buf_append(&buf, &len, "bar", 3);
    ASSERT_EQ(len, 6);
    ASSERT_BUF_EQ(buf, "foobar", 6);

    free(buf);
}
TEST_END

TEST_START(clax_san_path_fixes_path)
{
    char buf[1024];

    strcpy(buf, "nothing");
    clax_san_path(buf);
    ASSERT_STR_EQ(buf, "nothing");

    strcpy(buf, "/");
    clax_san_path(buf);
    ASSERT_STR_EQ(buf, "/");
    ASSERT_EQ(strlen(buf), 1);

    strcpy(buf, "//");
    clax_san_path(buf);
    ASSERT_STR_EQ(buf, "/");
    ASSERT_EQ(strlen(buf), 1);

    strcpy(buf, "//////.///../////..///");
    clax_san_path(buf);
    ASSERT_STR_EQ(buf, "/");
    ASSERT_EQ(strlen(buf), 1);

    strcpy(buf, "/./");
    clax_san_path(buf);
    ASSERT_STR_EQ(buf, "/");
    ASSERT_EQ(strlen(buf), 1);

    strcpy(buf, "/../");
    clax_san_path(buf);
    ASSERT_STR_EQ(buf, "/");

    strcpy(buf, "/foo/bar/");
    clax_san_path(buf);
    ASSERT_STR_EQ(buf, "/foo/bar/");
    ASSERT_EQ(strlen(buf), 9);

    strcpy(buf, "/foo/bar/../baz");
    clax_san_path(buf);
    ASSERT_STR_EQ(buf, "/foo/bar/baz");
    ASSERT_EQ(strlen(buf), 12);

    strcpy(buf, "C:\\foo\\\\bar//baz");
    clax_san_path(buf);
    ASSERT_STR_EQ(buf, "C:/foo/bar/baz");
    ASSERT_EQ(strlen(buf), 14);

    strcpy(buf, "C:\\clax\\clax.log");
    clax_san_path(buf);
    ASSERT_STR_EQ(buf, "C:/clax/clax.log");
    ASSERT_EQ(strlen(buf), 16);
}
TEST_END

TEST_START(clax_strjoin_joins_strings)
{
    char *p;

    p = clax_strjoin("/", NULL);

    ASSERT_STR_EQ(p, "");

    free(p);

    p = clax_strjoin("/", "foo", "bar", "baz", NULL);

    ASSERT_STR_EQ(p, "foo/bar/baz");

    free(p);

    p = clax_strjoin("/", "foo", "bar", "baz", NULL);

    ASSERT_STR_EQ(p, "foo/bar/baz");

    free(p);

    p = clax_strjoin("/", "", "", "", NULL);

    ASSERT_STR_EQ(p, "");

    free(p);

    p = clax_strjoin("/", "/", "/", "/", NULL);

    ASSERT_STR_EQ(p, "/////");

    free(p);
}
TEST_END

TEST_START(htol converts hex to integer)
{
    ASSERT_EQ(clax_htol(""), 0);
    ASSERT_EQ(clax_htol("0"), 0);
    ASSERT_EQ(clax_htol("1"), 1);
    ASSERT_EQ(clax_htol("a"), 10);
    ASSERT_EQ(clax_htol("ff"), 255);
}
TEST_END

TEST_START(strdup duplicates a string)
{
    char *p;

    p = clax_strdup(NULL);
    ASSERT_NULL(p);

    p = clax_strdup("foobar");
    ASSERT_STR_EQ(p, "foobar");
    free(p);
}
TEST_END

TEST_START(strndup duplicates a string with max characters)
{
    char *p;

    p = clax_strndup(NULL, 6);
    ASSERT_NULL(p);

    p = clax_strndup("foobar", 6);
    ASSERT_STR_EQ(p, "foobar");
    free(p);

    p = clax_strndup("foobar", 10);
    ASSERT_STR_EQ(p, "foobar");
    free(p);

    p = clax_strndup("foobar", 3);
    ASSERT_STR_EQ(p, "foo");
    free(p);
}
TEST_END

TEST_START(generates random string)
{
    char *p;

    p = clax_randstr_alloc(0);
    ASSERT_EQ(strlen(p), 0);
    free(p);

    p = clax_randstr_alloc(6);
    ASSERT_EQ(strlen(p), 6);
    ASSERT_MATCHES(p, "[a-zA-Z0-9]+");
    free(p);
}
TEST_END

TEST_START(generates random string based on template)
{
    char *p;

    p = clax_randstr_template_alloc("X");
    ASSERT_EQ(strlen(p), 1);
    ASSERT_MATCHES(p, "[a-zA-Z0-9]+");
    free(p);

    p = clax_randstr_template_alloc("XXXX");
    ASSERT_EQ(strlen(p), 4);
    ASSERT_MATCHES(p, "[a-zA-Z0-9]+");
    free(p);

    p = clax_randstr_template_alloc("fooXXXbar");
    ASSERT_EQ(strlen(p), 9);
    ASSERT_MATCHES(p, "foo[a-zA-Z0-9]+bar");
    free(p);
}
TEST_END

TEST_START(generates random filename)
{
    char *p;

    p = clax_mktmpfile_alloc("tmp", NULL);
    ASSERT_EQ(strlen(p), 3 + 1 + 8);
    ASSERT_MATCHES(p, "^tmp/[a-zA-Z0-9]+$");
    free(p);

    p = clax_mktmpfile_alloc(".", NULL);
    ASSERT_EQ(strlen(p), 1 + 1 + 8);
    ASSERT_MATCHES(p, "^\\./[a-zA-Z0-9]+$");
    free(p);

    p = clax_mktmpfile_alloc("", NULL);
    ASSERT_EQ(strlen(p), 1 + 1 + 8);
    ASSERT_MATCHES(p, "^\\./[a-zA-Z0-9]+$");
    free(p);

    p = clax_mktmpfile_alloc(NULL, NULL);
    ASSERT_EQ(strlen(p), 1 + 1 + 8);
    ASSERT_MATCHES(p, "^\\./[a-zA-Z0-9]+$");
    free(p);
}
TEST_END

TEST_START(generates random filename with template)
{
    char *p;

    p = clax_mktmpfile_alloc("tmp", ".fileXXX");
    ASSERT_EQ(strlen(p), 3 + 1 + 8);
    ASSERT_MATCHES(p, "^tmp/\\.file[a-zA-Z0-9]+$");
    free(p);

    p = clax_mktmpfile_alloc("tmp", "aXbXc");
    ASSERT_EQ(strlen(p), 3 + 1 + 5);
    ASSERT_MATCHES(p, "^tmp/a[a-zA-Z0-9]b[a-zA-Z0-9]c$");
    free(p);
}
TEST_END

#ifdef MVS

TEST_START(converts ascii to ebcdic)
{
    char buf[] = "\x68\x65\x6C\x6C\x6F\x20\x74\x68\x65\x72\x65\x21";

    int ret = clax_atoe(buf, strlen(buf));
    ASSERT_EQ(ret, 12);
    ASSERT_BUF_EQ(buf, "\x88\x85\x93\x93\x96\x40\xA3\x88\x85\x99\x85\x5A", 12);
}
TEST_END

TEST_START(converts ebcdic to ascii)
{
    char buf[] = "\x88\x85\x93\x93\x96\x40\xA3\x88\x85\x99\x85\x5A";

    int ret = clax_etoa(buf, strlen(buf));
    ASSERT_EQ(ret, 12);
    ASSERT_BUF_EQ(buf, "\x68\x65\x6C\x6C\x6F\x20\x74\x68\x65\x72\x65\x21", 12);
}
TEST_END

#endif

TEST_START(slurps file)
{
    char *tmp_dirname = clax_mktmpdir_alloc();
    char *filename = clax_strjoin("/", tmp_dirname, "file", NULL);

    FILE *fh = fopen(filename, "w");
    fprintf(fh, "%s", "\xab\xcd\xef");
    fclose(fh);

    size_t len = 0;

    unsigned char *p = clax_slurp_alloc(filename, &len);
    ASSERT_BUF_EQ(p, "\xab\xcd\xef\x00", 4);
    ASSERT_EQ(len, 4);

    free(p);

    rmrf(tmp_dirname);

    free(filename);
}
TEST_END

TEST_START(appends new string)
{
    char str[255] = "";

    int ok = clax_strcat(str, sizeof(str), "hello");

    ASSERT_EQ(ok, 0);
    ASSERT_STR_EQ(str, "hello");

    ok = clax_strcat(str, sizeof(str), " there");

    ASSERT_EQ(ok, 0);
    ASSERT_STR_EQ(str, "hello there");
}
TEST_END

TEST_START(appends empty string)
{
    char str[255] = "hello";

    int ok = clax_strcat(str, sizeof(str), "");

    ASSERT_EQ(ok, 0);
    ASSERT_STR_EQ(str, "hello");
}
TEST_END

TEST_START(returns error when string does not fit)
{
    char str[6] = "hello";

    int ok = clax_strcat(str, sizeof(str), " there");

    ASSERT_EQ(ok, -1);
    ASSERT_STR_EQ(str, "hello");
}
TEST_END

TEST_START(contatenates file paths)
{
    char str[255] = {0};
    int ok = -1;

    ok = clax_strcatfile(str, sizeof(str), "one");

    ASSERT_EQ(ok, 0);
    ASSERT_STR_EQ(str, "one");

    ok = clax_strcatfile(str, sizeof(str), "two");

    ASSERT_EQ(ok, 0);
    ASSERT_STR_EQ(str, "one/two");

    ok = clax_strcatfile(str, sizeof(str), "/three");

    ASSERT_EQ(ok, 0);
    ASSERT_STR_EQ(str, "one/two/three");
}
TEST_END

TEST_START(contatenates dir paths)
{
    char str[255] = {0};
    int ok = -1;

    ok = clax_strcatdir(str, sizeof(str), "one");

    ASSERT_EQ(ok, 0);
    ASSERT_STR_EQ(str, "one/");

    ok = clax_strcatdir(str, sizeof(str), "two");

    ASSERT_EQ(ok, 0);
    ASSERT_STR_EQ(str, "one/two/");

    ok = clax_strcatdir(str, sizeof(str), "/three/");

    ASSERT_EQ(ok, 0);
    ASSERT_STR_EQ(str, "one/two/three/");
}
TEST_END

TEST_START(creates intermediate directories)
{
    char *path = "/foo/bar/baz/";
    char *tempdir = clax_mktmpdir_alloc();

    char *fullpath = clax_strjoin("/", tempdir, path, NULL);

    clax_mkdir_p(fullpath);

    ASSERT_EQ(clax_is_path_d(fullpath), 1);

    clax_rmpath_r(tempdir);

    free(fullpath);
    free(tempdir);
}
TEST_END

#ifndef _WIN32
TEST_START(detects root if absolute)
{
    char root[1024] = {0};
    char *argv[1] = {"/hello/world"};

    clax_detect_exe_from_argv(root, sizeof(root), argv);

    ASSERT_STR_EQ(root, "/hello/world/");
}
TEST_END
#endif

SUITE_END
