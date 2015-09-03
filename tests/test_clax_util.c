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
#include <stdarg.h>

#include "clax_util.h"
#include "u.h"

TEST_START(clax_kv_list_default_values)
{
    clax_kv_list_t list;

    clax_kv_list_init(&list);

    ASSERT_EQ(list.size, 0);

    clax_kv_list_free(&list);
}
TEST_END

TEST_START(clax_kv_list_pushes_kv)
{
    clax_kv_list_t list;

    clax_kv_list_init(&list);

    clax_kv_list_push(&list, "foo", "bar");
    ASSERT_EQ(list.size, 1);

    clax_kv_list_push(&list, "bar", "baz");
    ASSERT_EQ(list.size, 2);

    clax_kv_list_free(&list);
}
TEST_END

TEST_START(clax_kv_list_finds_all_kv)
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

TEST_START(clax_kv_list_finds_kv)
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

TEST_START(clax_kv_list_at_kv)
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

TEST_START(clax_kv_list_iter_kv)
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

TEST_START(clax_kv_list_sets_new_val)
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
}
TEST_END
