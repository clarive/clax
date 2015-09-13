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

#ifndef _U_H
#define _U_H

#ifdef MVS
#include "../../arch/zos/libascii/_Ascii_a.h"
#endif

#define COLOR_GREEN "\x1b[0;32m"
#define COLOR_RED "\x1b[0;31m"
#define COLOR_YELLOW "\x1b[0;33m"
#define COLOR_OFF "\x1b[0m"

#include <stdlib.h>
#include <stdio.h>

#include "slre/slre.h"

int u_tests;
int u_tests_failed;
int u_tests_asserts;
int u_tests_asserts_failed;
int u_ok;

#define NOT_OK                                                              \
        u_tests_asserts_failed++;                                                   \
        u_local_tests_failed++;  \
        printf("    # %s:%d\n", __FILE__, __LINE__);\
        printf(COLOR_RED "    not ok %d\n" COLOR_OFF, u_local_tests);

#define _ASSERT(code)                                        \
    do {                                                    \
        u_local_tests++;                                    \
        if (code) {                                         \
            u_ok = 1;                                       \
        }                                                   \
        else {                                              \
            u_ok = 0;                                       \
            NOT_OK                                          \
        }                                                   \
    } while (0);

#define ASSERT(code)                                        \
    do { \
        _ASSERT(code) \
        if (!u_ok) {                        \
            printf("    # " #code ":\n");        \
            printf("    #   exp: TRUE\n");   \
            printf("    #   got: %d\n", code); \
        } \
    } while (0);

#define ASSERT_EQ(got, exp)                 \
    do {                                    \
        int got_i = (int)got;               \
        _ASSERT(got_i == exp)                \
                                            \
        if (!u_ok) {                        \
            printf("    # " #got ":\n");        \
            printf("    #   exp: %d\n", exp);   \
            printf("    #   got: %d\n", got_i); \
        }                                   \
    } while (0);

#define ASSERT_NOT_EQ(got, exp)                 \
    do {                                    \
        int i = (int)got;                   \
        _ASSERT(got != exp)                  \
                                            \
        if (!u_ok) {                        \
            printf("    # " #got ":\n");        \
            printf("    #   exp: != %d\n", exp);   \
            printf("    #   got: %d\n", i);     \
        }                                   \
    } while (0);

#define ASSERT_NULL(got)                 \
    do {                                    \
        _ASSERT(got == NULL)                  \
                                            \
        if (!u_ok) {                        \
            printf("    # " #got ":\n");        \
            printf("    #   exp: NULL\n");   \
        }                                   \
    } while (0);

#define ASSERT_NOT_NULL(got)                 \
    do {                                    \
        _ASSERT(got != NULL)                  \
                                            \
        if (!u_ok) {                        \
            printf("    # " #got ":\n");        \
            printf("    #   exp: NOT NULL\n");   \
        }                                   \
    } while (0);

#define ASSERT_STR_EQ(got, exp)                 \
    do {                                        \
        char *__got = (char *)got;              \
        if (__got != NULL) {                    \
            _ASSERT(strcmp(__got, exp) == 0)     \
                                                \
            if (!u_ok) {                        \
                printf("    # " #got ":\n");        \
                printf("    #   exp: '%s'\n", exp); \
                printf("    #   got: '%s'\n", __got);   \
            }                                   \
        } else {                                \
            u_local_tests++;                    \
            NOT_OK                              \
            printf("    # " #got ":\n");            \
            printf("    #   exp: '%s'\n", exp);     \
            printf("    #   got: NULL\n");          \
        }                                       \
    } while (0);

#define ASSERT_STRN_EQ(got, exp, len)           \
    do {                                        \
        char *p = (char *)got;                  \
        if (p != NULL) {                        \
            _ASSERT(strncmp(p, exp, len) == 0)   \
                                                \
            if (!u_ok) {                        \
                printf("    # " #got ":\n");        \
                printf("    #   exp: '%s'\n", exp); \
                printf("    #   got: '%s'\n", p);   \
            }                                   \
        } else {                                \
            u_local_tests++;                    \
            NOT_OK                              \
            printf("    # " #got ":\n");            \
            printf("    #   exp: '%s'\n", exp);     \
            printf("    #   got: NULL\n");          \
        }                                       \
    } while (0);

#define ASSERT_BUF_EQ(got, exp, len)    \
    _ASSERT(memcmp(got, exp, len) == 0)  \
                                        \
    if (!u_ok) {                        \
        printf("    # " #got ":\n");        \
        printf("    #   exp: ");            \
        for (int i = 0; i < len; i++) { \
            printf("%02x ", exp[i]);    \
        }                               \
        printf("\n");                   \
        printf("    #   got: ");            \
        for (int i = 0; i < len; i++) { \
            printf("%02x ", got[i]);    \
        }                               \
        printf("\n");                   \
    }

#define ASSERT_MATCHES(got, re)    \
    _ASSERT(slre_match(re, got, strlen(got), NULL, 0, 0))  \
                                        \
    if (!u_ok) {                        \
        printf("    # " #got ":\n");        \
        printf("    # not matches " # re "\n");   \
    }

#define TEST_START(name)                    \
    void name() {                           \
        int u_local_tests = 0;              \
        int u_local_tests_failed = 0;              \
                                            \
        printf("# %s\n", #name);

#define TEST_END                            \
        if (!u_local_tests) {               \
            printf(COLOR_YELLOW "# no tests\n" COLOR_OFF);         \
        }                                   \
        else {                              \
            u_tests++; \
            u_tests_asserts += u_local_tests;       \
\
            if (u_local_tests_failed) { \
                u_tests_failed++;                                                   \
                printf(COLOR_RED "not ok %d (%d/%d)\n" COLOR_OFF, u_tests, u_local_tests_failed, u_local_tests); \
                printf("# %s:%d\n", __FILE__, __LINE__); \
            } else { \
                printf("ok %d\n", u_tests); \
            } \
        }                                   \
    }

#define RUN_TEST(name)                      \
    name();

#define DONE_TESTING                                             \
    printf("1..%d (%d)\n", u_tests, u_tests_asserts);                                  \
                                                                 \
    if (u_tests_failed) {                                        \
        printf(COLOR_RED "FAILED tests\n" COLOR_OFF);                                \
        printf(COLOR_RED "Failed %d/%d (%d/%d) tests\n" COLOR_OFF, u_tests_failed, u_tests, u_tests_asserts_failed, u_tests_asserts); \
                                                                 \
        exit(255);                                               \
    } else {                                                     \
        printf(COLOR_GREEN "SUCCESS\n" COLOR_OFF);                                     \
    }                                                            \
                                                                 \
    exit(0);

#endif
