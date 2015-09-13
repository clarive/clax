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

#include "clax_http_multipart.h"

TEST_START(clax_http_multipart_list_default_values)
{
    clax_http_multipart_list_t list;

    clax_http_multipart_list_init(&list, "/tmp");

    ASSERT_EQ(list.size, 0);

    clax_http_multipart_list_free(&list);
}
TEST_END

TEST_START(clax_http_multipart_list_pushes_multipart)
{
    clax_http_multipart_list_t list;

    clax_http_multipart_list_init(&list, "/tmp");

    clax_http_multipart_list_push(&list);
    ASSERT_EQ(list.size, 1);

    clax_http_multipart_list_push(&list);
    ASSERT_EQ(list.size, 2);

    clax_http_multipart_list_push(&list);
    ASSERT_EQ(list.size, 3);

    clax_http_multipart_list_free(&list);
}
TEST_END

TEST_START(clax_http_multipart_list_returns_item_at_position)
{
    clax_http_multipart_list_t list;

    clax_http_multipart_list_init(&list, "/tmp");

    ASSERT_NULL(clax_http_multipart_list_at(&list, 0));

    clax_http_multipart_list_push(&list);
    ASSERT_NOT_NULL(clax_http_multipart_list_at(&list, 0));
    ASSERT_NULL(clax_http_multipart_list_at(&list, 1));

    clax_http_multipart_list_push(&list);
    ASSERT_NOT_NULL(clax_http_multipart_list_at(&list, 0));
    ASSERT_NOT_NULL(clax_http_multipart_list_at(&list, 1));
    ASSERT_NULL(clax_http_multipart_list_at(&list, 2));

    clax_http_multipart_list_free(&list);
}
TEST_END

TEST_START(clax_http_multipart_list_returns_last_item)
{
    clax_http_multipart_list_t list;

    clax_http_multipart_list_init(&list, "/tmp");

    ASSERT_NULL(clax_http_multipart_list_last(&list));

    clax_http_multipart_list_push(&list);
    ASSERT_NOT_NULL(clax_http_multipart_list_last(&list));

    clax_http_multipart_list_free(&list);
}
TEST_END
