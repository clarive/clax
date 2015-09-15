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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "clax_big_buf.h"
#include "clax_util.h"
#include "u/u.h"
#include "u_util.h"

SUITE_START(clax_big_buf)

TEST_START(default_values)
{
    clax_big_buf_t bbuf;

    char *tmp_dirname = clax_mktmpdir_alloc();

    clax_big_buf_init(&bbuf, tmp_dirname, 1024);

    ASSERT_EQ(bbuf.len, 0)

    clax_big_buf_free(&bbuf);

    rmrf(tmp_dirname);
}
TEST_END

TEST_START(creates_file_when_max_size)
{
    clax_big_buf_t bbuf;

    char *tmp_dirname = clax_mktmpdir_alloc();

    clax_big_buf_init(&bbuf, tmp_dirname, 1024);

    ASSERT_EQ(is_dir_empty(tmp_dirname), 1);

    for (int i = 0; i < 1024; i++) {
        clax_big_buf_append(&bbuf, (const unsigned char *)"1", 1);
    }

    ASSERT_EQ(is_dir_empty(tmp_dirname), 1);

    clax_big_buf_append(&bbuf, (const unsigned char *)"1", 1);

    ASSERT_EQ(is_dir_empty(tmp_dirname), 0);

    clax_big_buf_free(&bbuf);

    rmrf(tmp_dirname);
}
TEST_END

TEST_START(deletes_file_on_free)
{
    clax_big_buf_t bbuf;

    char *tmp_dirname = clax_mktmpdir_alloc();

    clax_big_buf_init(&bbuf, tmp_dirname, 1024);

    for (int i = 0; i < 1025; i++) {
        clax_big_buf_append(&bbuf, (const unsigned char *)"1", 1);
    }

    ASSERT_EQ(is_dir_empty(tmp_dirname), 0);

    clax_big_buf_free(&bbuf);

    ASSERT_EQ(is_dir_empty(tmp_dirname), 1);

    rmrf(tmp_dirname);
}
TEST_END

TEST_START(writes_memory_to_file)
{
    clax_big_buf_t bbuf;

    char *tmp_dirname = clax_mktmpdir_alloc();

    clax_big_buf_init(&bbuf, tmp_dirname, 1024);

    clax_big_buf_append(&bbuf, (const unsigned char *)"123", 3);

    char *fpath = catfile(tmp_dirname, "file");

    clax_big_buf_write_file(&bbuf, fpath);

    ASSERT_EQ(is_dir_empty(tmp_dirname), 0);

    clax_big_buf_free(&bbuf);

    ASSERT_EQ(is_dir_empty(tmp_dirname), 0);

    char content[1024];
    slurp_file(fpath, content, sizeof(content));
    ASSERT_BUF_EQ(content, "123", 3);

    rmrf(tmp_dirname);
}
TEST_END

TEST_START(renames_file)
{
    clax_big_buf_t bbuf;

    char *tmp_dirname = clax_mktmpdir_alloc();

    clax_big_buf_init(&bbuf, tmp_dirname, 2);

    clax_big_buf_append(&bbuf, (const unsigned char *)"123", 3);

    ASSERT_EQ(is_dir_empty(tmp_dirname), 0);

    char *fpath = catfile(tmp_dirname, "file");

    clax_big_buf_write_file(&bbuf, fpath);

    clax_big_buf_free(&bbuf);

    ASSERT_EQ(is_dir_empty(tmp_dirname), 0);

    char content[1024];
    slurp_file(fpath, content, sizeof(content));
    ASSERT_BUF_EQ(content, "123", 3);

    rmrf(tmp_dirname);
}
TEST_END

TEST_START(read_from_memory)
{
    clax_big_buf_t bbuf;

    char *tmp_dirname = clax_mktmpdir_alloc();

    clax_big_buf_init(&bbuf, tmp_dirname, 1024);

    clax_big_buf_append(&bbuf, (const unsigned char *)"123", 3);

    unsigned char buf[3];
    int rcount = 0;
    rcount = clax_big_buf_read(&bbuf, buf, 1, 0);
    ASSERT_EQ(rcount, 1);
    ASSERT_BUF_EQ(buf, "1", 1);

    rcount = clax_big_buf_read(&bbuf, buf, 1, 1);
    ASSERT_EQ(rcount, 1);
    ASSERT_BUF_EQ(buf, "2", 1);

    rcount = clax_big_buf_read(&bbuf, buf, 1, 2);
    ASSERT_EQ(rcount, 1);
    ASSERT_BUF_EQ(buf, "3", 1);

    rcount = clax_big_buf_read(&bbuf, buf, 1, 3);
    ASSERT_EQ(rcount, 0);

    clax_big_buf_free(&bbuf);

    rmrf(tmp_dirname);
}
TEST_END

TEST_START(read_from_memory_more_than_available)
{
    clax_big_buf_t bbuf;

    char *tmp_dirname = clax_mktmpdir_alloc();

    clax_big_buf_init(&bbuf, tmp_dirname, 1024);

    clax_big_buf_append(&bbuf, (const unsigned char *)"123", 3);

    unsigned char buf[1024];
    int rcount = 0;
    rcount = clax_big_buf_read(&bbuf, buf, sizeof(buf), 0);

    ASSERT_EQ(rcount, 3);
    ASSERT_BUF_EQ(buf, "123", 3);

    clax_big_buf_free(&bbuf);

    rmrf(tmp_dirname);
}
TEST_END

TEST_START(read_from_file)
{
    clax_big_buf_t bbuf;

    char *tmp_dirname = clax_mktmpdir_alloc();

    clax_big_buf_init(&bbuf, tmp_dirname, 2);

    clax_big_buf_append(&bbuf, (const unsigned char *)"123", 3);

    clax_big_buf_close(&bbuf);

    unsigned char buf[3];
    int rcount = 0;

    rcount = clax_big_buf_read(&bbuf, buf, 1, 0);
    ASSERT_EQ(rcount, 1);
    ASSERT_BUF_EQ(buf, "1", 1);

    rcount = clax_big_buf_read(&bbuf, buf, 1, 1);
    ASSERT_EQ(rcount, 1);
    ASSERT_BUF_EQ(buf, "2", 1);

    rcount = clax_big_buf_read(&bbuf, buf, 1, 2);
    ASSERT_EQ(rcount, 1);
    ASSERT_BUF_EQ(buf, "3", 1);

    rcount = clax_big_buf_read(&bbuf, buf, 1, 3);
    ASSERT_EQ(rcount, 0);

    clax_big_buf_free(&bbuf);

    rmrf(tmp_dirname);
}
TEST_END

SUITE_END
