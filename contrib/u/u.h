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

#define COLOR_GREEN "\x1b[0;32m"
#define COLOR_RED "\x1b[0;31m"
#define COLOR_YELLOW "\x1b[0;33m"
#define COLOR_OFF "\x1b[0m"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "slre/slre.h"

int u_tests;
int u_tests_failed;
int u_tests_asserts;
int u_tests_asserts_failed;
int u_warnings;
int u_ok;

#define U_COLOR(color)            \
    do {                          \
        if (getenv("U_COLOR")) {  \
            printf(color);        \
        }                         \
    } while (0);

#define U_COLOR_OFF               \
    do {                          \
        if (getenv("U_COLOR")) {  \
            printf(COLOR_OFF);    \
        }                         \
    } while (0);

#define NOT_OK                                                         \
        u_local_tests_failed++;                                        \
        printf("    # %s:%d\n", __FILE__, __LINE__);                   \
        U_COLOR(COLOR_RED)                                             \
        printf("    not ok %d\n" COLOR_OFF, u_local_tests);            \
        U_COLOR_OFF

#define _ASSERT(code)     \
    do {                  \
        u_local_tests++;  \
        if (code) {       \
            u_ok = 1;     \
        }                 \
        else {            \
            u_ok = 0;     \
            NOT_OK        \
        }                 \
    } while (0);

#define ASSERT(code)                            \
    do {                                        \
        int u_val = code;                       \
        _ASSERT(!!u_val)                        \
        if (!u_ok) {                            \
            printf("    # " #code ":\n");       \
            printf("    #   exp: TRUE\n");      \
            printf("    #   got: %d\n", u_val); \
        }                                       \
    } while (0);

#define ASSERT_EQ(got, exp)                      \
    do {                                         \
        int u_val = (int)got;                    \
        _ASSERT(u_val == exp)                    \
                                                 \
        if (!u_ok) {                             \
            printf("    # " #got ":\n");         \
            printf("    #   exp: %d\n", exp);    \
            printf("    #   got: %d\n", u_val);  \
        }                                        \
    } while (0);

#define ASSERT_NOT_EQ(got, exp)                   \
    do {                                          \
        int u_val = (int)got;                     \
        _ASSERT(u_val != exp)                     \
                                                  \
        if (!u_ok) {                              \
            printf("    # " #got ":\n");          \
            printf("    #   exp: != %d\n", exp);  \
            printf("    #   got: %d\n", u_val);   \
        }                                         \
    } while (0);

#define ASSERT_NULL(got)                    \
    do {                                    \
        void *u_val = (void *)got;          \
        _ASSERT(u_val == NULL)              \
                                            \
        if (!u_ok) {                        \
            printf("    # " #got ":\n");    \
            printf("    #   exp: NULL\n");  \
        }                                   \
    } while (0);

#define ASSERT_NOT_NULL(got)                    \
    do {                                        \
        void *u_val = got;                      \
        _ASSERT(u_val != NULL)                  \
                                                \
        if (!u_ok) {                            \
            printf("    # " #got ":\n");        \
            printf("    #   exp: NOT NULL\n");  \
        }                                       \
    } while (0);

#define ASSERT_STR_EQ(got, exp)                       \
    do {                                              \
        char *u_val = (char *)got;                    \
        if (u_val != NULL) {                          \
            _ASSERT(strcmp(u_val, exp) == 0)          \
                                                      \
            if (!u_ok) {                              \
                printf("    # " #got ":\n");          \
                printf("    #   exp: '%s'\n", exp);   \
                printf("    #   got: '%s'\n", u_val); \
            }                                         \
        } else {                                      \
            u_local_tests++;                          \
            NOT_OK                                    \
            printf("    # " #got ":\n");              \
            printf("    #   exp: '%s'\n", exp);       \
            printf("    #   got: NULL\n");            \
        }                                             \
    } while (0);

#define ASSERT_STRN_EQ(got, exp, len)                 \
    do {                                              \
        char *u_val = (char *)got;                    \
        if (u_val != NULL) {                          \
            _ASSERT(strncmp(u_val, exp, len) == 0)    \
                                                      \
            if (!u_ok) {                              \
                printf("    # " #got ":\n");          \
                printf("    #   exp: '%s'\n", exp);   \
                printf("    #   got: '%s'\n", u_val); \
            }                                         \
        } else {                                      \
            u_local_tests++;                          \
            NOT_OK                                    \
            printf("    # " #got ":\n");              \
            printf("    #   exp: '%s'\n", exp);       \
            printf("    #   got: NULL\n");            \
        }                                             \
    } while (0);

#define ASSERT_BUF_EQ(got, exp, len)                  \
    do {                                              \
        unsigned char *u_val = (unsigned char *)got;  \
        _ASSERT(memcmp(u_val, exp, len) == 0)         \
                                                      \
        if (!u_ok) {                                  \
            printf("    # " #got ":\n");              \
            printf("    #   exp: ");                  \
            for (int i = 0; i < len; i++) {           \
                printf("%02x ", exp[i]);              \
            }                                         \
            printf("\n");                             \
            printf("    #   got: ");                  \
            for (int i = 0; i < len; i++) {           \
                printf("%02x ", u_val[i]);            \
            }                                         \
            printf("\n");                             \
        }                                             \
    } while (0);

#define ASSERT_MATCHES(got, re)                                    \
    do {                                                           \
        char *u_val = (char *)got;                                 \
        _ASSERT(slre_match(re, u_val, strlen(u_val), NULL, 0, 0))  \
                                                                   \
        if (!u_ok) {                                               \
            printf("    # " #got ":\n");                           \
            printf("    # not matches " # re "\n");                \
        }                                                          \
    } while (0);

#define SUITE_START(name)              \
    void suite_##name() {              \
        int u_local_tests = 0;         \
        int u_local_tests_failed = 0;  \
        char *u_suite_name = #name;    \
                                       \
        printf("# %s\n", #name);

#define SUITE_END \
    }

#define TEST_START(name)                            \
    do {                                            \
        u_local_tests = 0;                          \
        u_local_tests_failed = 0;                   \
                                                    \
        printf("# %s: %s\n", u_suite_name, #name);

#define TEST_END \
        if (!u_local_tests) {                                      \
            u_warnings++;                                          \
            U_COLOR(COLOR_YELLOW)                                  \
            printf("# no tests\n");                                \
            U_COLOR_OFF                                            \
        }                                                          \
        else {                                                     \
            u_tests++;                                             \
            u_tests_asserts += u_local_tests;                      \
                                                                   \
            if (u_local_tests_failed) {                            \
                u_tests_asserts_failed += u_local_tests_failed;    \
                u_tests_failed++;                                  \
                U_COLOR(COLOR_RED)                                 \
                printf("not ok %d (%d/%d)\n"                       \
                        , u_tests                                  \
                        , u_local_tests_failed                     \
                        , u_local_tests);                          \
                U_COLOR_OFF                                        \
                printf("# %s:%d\n", __FILE__, __LINE__);           \
            } else {                                               \
                printf("ok %d\n", u_tests);                        \
            }                                                      \
        }                                                          \
    } while (0);

#define RUN_SUITE(name) \
    suite_##name();

#ifdef _WIN32
# define DEVNULL "nul"
#else
# define DEVNULL "/dev/null"
#endif

#define START_TESTING \
    FILE *u_devnull = fopen(DEVNULL, "wb"); \
    dup2(fileno(u_devnull), 2);

#define DONE_TESTING                                               \
    fclose(u_devnull);                                             \
    printf("1..%d (%d)\n", u_tests, u_tests_asserts);              \
                                                                   \
    if (u_tests_failed) {                                          \
        U_COLOR(COLOR_RED)                                         \
        printf("FAILED tests\n");                                  \
        printf("Failed %d/%d (%d/%d) tests\n"                      \
                , u_tests_failed                                   \
                , u_tests                                          \
                , u_tests_asserts_failed                           \
                , u_tests_asserts);                                \
        U_COLOR_OFF                                                \
                                                                   \
        exit(255);                                                 \
    } else {                                                       \
        if (u_warnings) {                                          \
            U_COLOR(COLOR_YELLOW)                                  \
            printf("SUCCESS (warnings)\n");                        \
            U_COLOR_OFF                                            \
        } else {                                                   \
            U_COLOR(COLOR_GREEN)                                   \
            printf("SUCCESS\n");                                   \
            U_COLOR_OFF                                            \
        }                                                          \
    }                                                              \
                                                                   \
    exit(0);

#endif
