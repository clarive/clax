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
#include <unistd.h>

#include "clax_big_buf.h"
#include "u.h"
#include "u_util.h"

TEST_START(clax_big_buf_default_values)
{
    clax_big_buf_t bbuf;

    clax_big_buf_init(&bbuf, "/tmp", 1024);

    ASSERT_EQ(bbuf.len, 0);

    clax_big_buf_free(&bbuf);
}
TEST_END

TEST_START(clax_big_buf_creates_file_when_max_size)
{
    clax_big_buf_t bbuf;

    char template[] = "/tmp/tmpdir.XXXXXX";
    char *tmp_dirname = mkdtemp(template);

    mkdir(tmp_dirname, 0755);

    clax_big_buf_init(&bbuf, tmp_dirname, 1024);

    ASSERT_EQ(is_dir_empty(tmp_dirname), 1);

    for (int i = 0; i < 1024; i++) {
        clax_big_buf_append(&bbuf, "1", 1);
    }

    ASSERT_EQ(is_dir_empty(tmp_dirname), 1);

    clax_big_buf_append(&bbuf, "1", 1);

    ASSERT_EQ(is_dir_empty(tmp_dirname), 0);

    clax_big_buf_free(&bbuf);

    rmdir(tmp_dirname);
}
TEST_END

TEST_START(clax_big_buf_deletes_file_on_free)
{
    clax_big_buf_t bbuf;

    char template[] = "/tmp/tmpdir.XXXXXX";
    char *tmp_dirname = mkdtemp(template);

    mkdir(tmp_dirname, 0755);

    clax_big_buf_init(&bbuf, tmp_dirname, 1024);

    for (int i = 0; i < 1025; i++) {
        clax_big_buf_append(&bbuf, "1", 1);
    }

    ASSERT_EQ(is_dir_empty(tmp_dirname), 0);

    clax_big_buf_free(&bbuf);

    ASSERT_EQ(is_dir_empty(tmp_dirname), 1);

    rmdir(tmp_dirname);
}
TEST_END

TEST_START(clax_big_buf_writes_memory_to_file)
{
    clax_big_buf_t bbuf;

    char template[] = "/tmp/tmpdir.XXXXXX";
    char *tmp_dirname = mkdtemp(template);

    mkdir(tmp_dirname, 0755);

    clax_big_buf_init(&bbuf, tmp_dirname, 1024);

    clax_big_buf_append(&bbuf, "123", 3);

    char fpath[1024];
    strcpy(fpath, tmp_dirname);
    strcat(fpath, "/file");
    clax_big_buf_write_file(&bbuf, fpath);

    ASSERT_EQ(is_dir_empty(tmp_dirname), 0);

    clax_big_buf_free(&bbuf);

    ASSERT_EQ(is_dir_empty(tmp_dirname), 0);

    char content[1024];
    slurp_file(fpath, content, sizeof(content));
    ASSERT_BUF_EQ(content, "123", 3);

    rmdir(tmp_dirname);
}
TEST_END

TEST_START(clax_big_buf_renames_file)
{
    clax_big_buf_t bbuf;

    char template[] = "/tmp/tmpdir.XXXXXX";
    char *tmp_dirname = mkdtemp(template);

    mkdir(tmp_dirname, 0755);

    clax_big_buf_init(&bbuf, tmp_dirname, 2);

    clax_big_buf_append(&bbuf, "123", 3);

    ASSERT_EQ(is_dir_empty(tmp_dirname), 0);

    char fpath[1024];
    strcpy(fpath, tmp_dirname);
    strcat(fpath, "/file");
    clax_big_buf_write_file(&bbuf, fpath);

    clax_big_buf_free(&bbuf);

    ASSERT_EQ(is_dir_empty(tmp_dirname), 0);

    char content[1024];
    slurp_file(fpath, content, sizeof(content));
    ASSERT_BUF_EQ(content, "123", 3);

    rmdir(tmp_dirname);
}
TEST_END

TEST_START(clax_big_buf_read_from_memory)
{
    clax_big_buf_t bbuf;

    clax_big_buf_init(&bbuf, "/tmp/", 1024);

    clax_big_buf_append(&bbuf, "123", 3);

    char buf[3];
    int rcount = 0;
    rcount = clax_big_buf_read(&bbuf, buf, sizeof(buf));

    ASSERT_EQ(rcount, 3);
    ASSERT_BUF_EQ(buf, "123", 3);

    clax_big_buf_free(&bbuf);
}
TEST_END

TEST_START(clax_big_buf_read_from_file)
{
    clax_big_buf_t bbuf;

    char template[] = "/tmp/tmpdir.XXXXXX";
    char *tmp_dirname = mkdtemp(template);

    mkdir(tmp_dirname, 0755);

    clax_big_buf_init(&bbuf, tmp_dirname, 2);

    clax_big_buf_append(&bbuf, (const unsigned char *)"123", 3);

    clax_big_buf_close(&bbuf);

    char buf[3];
    int rcount = 0;
    rcount = clax_big_buf_read(&bbuf, buf, sizeof(buf));

    ASSERT_EQ(rcount, 3);
    ASSERT_BUF_EQ(buf, "123", 3);

    clax_big_buf_free(&bbuf);

    rmdir(tmp_dirname);
}
TEST_END