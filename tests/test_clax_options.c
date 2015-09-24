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

#include "u/u.h"

#include "clax_options.h"
#include "clax_util.h"
#include "u_util.h"

SUITE_START(clax_options)

TEST_START(not_enough_args)
{
    opt options;

    clax_options_init(&options);

    char *argv[] = {0};
    int ret = clax_parse_options(&options, 0, argv);

    ASSERT_EQ(ret, -1);

    clax_options_free(&options);
}
TEST_END

TEST_START(root_is_required)
{
    opt options;

    clax_options_init(&options);

    char *argv[] = {"clax", "-n"};
    int ret = clax_parse_options(&options, sizeof_array(argv), argv);

    ASSERT_EQ(ret, -1);

    char *argv2[] = {"clax", "-n", "-r", "unknown-root"};
    int ret2 = clax_parse_options(&options, sizeof_array(argv2), argv2);

    ASSERT_EQ(ret2, -1);

    clax_options_free(&options);
}
TEST_END

TEST_START(parses_basic_auth)
{
    opt options;

    clax_options_init(&options);

    char *argv[] = {"clax", "-n", "-r", ".", "-a", "foo:bar"};
    int ret = clax_parse_options(&options, sizeof_array(argv), argv);

    ASSERT_EQ(ret, 0);
    ASSERT_STR_EQ(options.basic_auth_username, "foo");
    ASSERT_STR_EQ(options.basic_auth_password, "bar");

    clax_options_free(&options);
}
TEST_END

TEST_START(parses_basic_auth_invalid)
{
    opt options;

    clax_options_init(&options);

    char *argv[] = {"clax", "-n", "-r", ".", "-a", "foobar"};
    int ret = clax_parse_options(&options, sizeof_array(argv), argv);

    ASSERT_EQ(ret, -1);

    clax_options_free(&options);
}
TEST_END

TEST_START(parses_no_ssl_options)
{
    opt options;

    clax_options_init(&options);

    char *argv[] = {"clax", "-n", "-r", "."};
    int ret = clax_parse_options(&options, sizeof_array(argv), argv);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(options.no_ssl, 1);

    clax_options_free(&options);
}
TEST_END

TEST_START(parses_ssl_options)
{
    opt options;

    clax_options_init(&options);

    char *argv[] = {"clax", "-r", ".", "-t", "ssl/server.crt", "-p", "ssl/server.key"};
    int ret = clax_parse_options(&options, sizeof_array(argv), argv);

    ASSERT_EQ(ret, 0)
    ASSERT_EQ(options.no_ssl, 0)
    ASSERT_EQ(options.no_ssl_verify, 0)
    ASSERT_STR_EQ(options.cert_file, "ssl/server.crt")
    ASSERT_STR_EQ(options.key_file, "ssl/server.key")

    clax_options_free(&options);
}
TEST_END

TEST_START(parses_ssl_options_require_cert_and_key)
{
    opt options;

    clax_options_init(&options);

    char *argv[] = {"clax", "-r", "."};
    int ret = clax_parse_options(&options, sizeof_array(argv), argv);

    ASSERT_EQ(ret, -1)

    clax_options_free(&options);
    clax_options_init(&options);

    char *argv2[] = {"clax", "-r", ".", "-t", "ssl/server.crt"};
    int ret2 = clax_parse_options(&options, sizeof_array(argv2), argv2);

    ASSERT_EQ(ret2, -1)

    clax_options_free(&options);
    clax_options_init(&options);

    char *argv3[] = {"clax", "-r", ".", "-p", "ssl/server.key"};
    int ret3 = clax_parse_options(&options, sizeof_array(argv3), argv3);

    ASSERT_EQ(ret3, -1)

    clax_options_free(&options);
}
TEST_END

TEST_START(parses_ssl_options_no_verify)
{
    opt options;

    clax_options_init(&options);

    char *argv[] = {"clax", "-r", ".", "-t", "ssl/server.crt", "-p", "ssl/server.key", "-k"};
    int ret = clax_parse_options(&options, sizeof_array(argv), argv);

    ASSERT_EQ(ret, 0)
    ASSERT_EQ(options.no_ssl_verify, 1)

    clax_options_free(&options);
}
TEST_END

TEST_START(parses_ssl_options_entropy_file)
{
    opt options;

    clax_options_init(&options);

    char *argv[] = {"clax", "-r", ".", "-t", "ssl/server.crt", "-p", "ssl/server.key", "-e", "ssl/entropy"};
    int ret = clax_parse_options(&options, sizeof_array(argv), argv);

    ASSERT_EQ(ret, 0)
    ASSERT_STR_EQ(options.entropy_file, "ssl/entropy")

    clax_options_free(&options);
}
TEST_END

TEST_START(parses_config)
{
    opt options;

    clax_options_init(&options);

    char *argv[] = {"clax", "-c", "config.ini"};
    int ret = clax_parse_options(&options, sizeof_array(argv), argv);

    ASSERT_EQ(ret, 0)
    ASSERT_STR_EQ(options.config_file, "config.ini")
    ASSERT_EQ(options.no_ssl, 0);
    ASSERT_STR_EQ(options.cert_file, "ssl/server.crt");
    ASSERT_STR_EQ(options.key_file, "ssl/server.key");
    ASSERT_STR_EQ(options.entropy_file, "ssl/entropy");
    ASSERT_STR_EQ(options.basic_auth_username, "clax");
    ASSERT_STR_EQ(options.basic_auth_password, "password");

    clax_options_free(&options);
}
TEST_END

TEST_START(returns_error_when_config_not_found)
{
    opt options;

    clax_options_init(&options);

    char *argv[] = {"clax", "-c", "unknown.ini"};
    int ret = clax_parse_options(&options, sizeof_array(argv), argv);

    ASSERT_EQ(ret, -1)

    clax_options_free(&options);
}
TEST_END

TEST_START(parses_log_file)
{
    opt options;

    clax_options_init(&options);

    char *argv[] = {"clax", "-r", ".", "-n", "-l", "/dev/null"};
    int ret = clax_parse_options(&options, sizeof_array(argv), argv);

    ASSERT_EQ(ret, 0)
    ASSERT_STR_EQ(options.log_file, "/dev/null")

    clax_options_free(&options);
}
TEST_END

SUITE_END
