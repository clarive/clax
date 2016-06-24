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

TEST_START(parses_config)
{
    opt options;

    clax_options_init(&options);

    char *argv[] = {"clax", "-l", "/dev/null", "-c", "config.ini"};
    int ret = clax_parse_options(&options, sizeof_array(argv), argv);

    ASSERT_EQ(ret, 0)
    ASSERT_STR_EQ(options.config_file, "config.ini")
    ASSERT_EQ(options.ssl, 1);
    ASSERT_EQ(options.no_ssl_verify, 1)
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

    char *argv[] = {"clax", "-l", "/dev/null", "-c", "unknown.ini"};
    int ret = clax_parse_options(&options, sizeof_array(argv), argv);

    ASSERT_EQ(ret, -1)

    clax_options_free(&options);
}
TEST_END

TEST_START(parses_log_file)
{
    opt options;

    clax_options_init(&options);

    char *argv[] = {"clax", "-l", "/dev/null"};
    int ret = clax_parse_options(&options, sizeof_array(argv), argv);

    ASSERT_EQ(ret, 0)
    ASSERT_STR_EQ(options.log_file, "/dev/null")

    clax_options_free(&options);
}
TEST_END

SUITE_END
