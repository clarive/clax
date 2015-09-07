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

int u_tests;
int u_tests_failed;
int u_rv;

#define NOT_OK                                                              \
        u_tests_failed++;                                                   \
        printf(COLOR_RED "not ok %d\n" COLOR_OFF, u_tests + u_local_tests); \
        printf("# %s:%d\n", __FILE__, __LINE__);

#define ASSERT(code)                                    \
    u_local_tests++;                                    \
    if (code) {                                         \
        u_rv = 1;                                       \
                                                        \
        printf("ok %d\n", u_tests + u_local_tests);     \
    }                                                   \
    else {                                              \
        u_rv = 0;                                       \
        NOT_OK \
    }

#define ASSERT_EQ(got, exp)                 \
    do {                                    \
        int i = (int)got;                   \
        ASSERT(got == exp)                  \
                                            \
        if (!u_rv) {                        \
            printf("# " #got ":\n");        \
            printf("#   got: %d\n", i);     \
            printf("#   exp: %d\n", exp);   \
        }                                   \
    } while (0);

#define ASSERT_NOT_EQ(got, exp)                 \
    do {                                    \
        int i = (int)got;                   \
        ASSERT(got != exp)                  \
                                            \
        if (!u_rv) {                        \
            printf("# " #got ":\n");        \
            printf("#   got: %d\n", i);     \
            printf("#   exp: != %d\n", exp);   \
        }                                   \
    } while (0);

#define ASSERT_NULL(got)                 \
    do {                                    \
        ASSERT(got == NULL)                  \
                                            \
        if (!u_rv) {                        \
            printf("# " #got ":\n");        \
            printf("#   exp: NULL\n");   \
        }                                   \
    } while (0);

#define ASSERT_NOT_NULL(got)                 \
    do {                                    \
        ASSERT(got != NULL)                  \
                                            \
        if (!u_rv) {                        \
            printf("# " #got ":\n");        \
            printf("#   exp: NOT NULL\n");   \
        }                                   \
    } while (0);

#define ASSERT_STR_EQ(got, exp)                 \
    do {                                        \
        char *p = (char *)got;                  \
        if (p != NULL) {                        \
            ASSERT(strcmp(p, exp) == 0)         \
                                                \
            if (!u_rv) {                        \
                printf("# " #got ":\n");        \
                printf("#   got: '%s'\n", p);   \
                printf("#   exp: '%s'\n", exp); \
            }                                   \
        } else {                                \
            u_local_tests++;                    \
            NOT_OK                              \
            printf("# " #got ":\n");            \
            printf("#   got: NULL\n");          \
            printf("#   exp: '%s'\n", exp);     \
        }                                       \
    } while (0);

#define ASSERT_STRN_EQ(got, exp, len)           \
    do {                                        \
        char *p = (char *)got;                  \
        if (p != NULL) {                        \
            ASSERT(strncmp(p, exp, len) == 0)   \
                                                \
            if (!u_rv) {                        \
                printf("# " #got ":\n");        \
                printf("#   got: '%s'\n", p);   \
                printf("#   exp: '%s'\n", exp); \
            }                                   \
        } else {                                \
            u_local_tests++;                    \
            NOT_OK                              \
            printf("# " #got ":\n");            \
            printf("#   got: NULL\n");          \
            printf("#   exp: '%s'\n", exp);     \
        }                                       \
    } while (0);

#define ASSERT_BUF_EQ(got, exp, len)    \
    ASSERT(memcmp(got, exp, len) == 0)  \
                                        \
    if (!u_rv) {                        \
        printf("# " #got ":\n");        \
        printf("#   got: ");            \
        for (int i = 0; i < len; i++) { \
            printf("%02x ", got[i]);    \
        }                               \
        printf("\n");                   \
        printf("#   exp: ");            \
        for (int i = 0; i < len; i++) { \
            printf("%02x ", exp[i]);    \
        }                               \
        printf("\n");                   \
    }

#define TEST_START(name)                    \
    void name() {                           \
        int u_local_tests = 0;              \
                                            \
        printf("# %s\n", #name);

#define TEST_END                            \
        if (!u_local_tests) {               \
            printf(COLOR_YELLOW "# no tests\n" COLOR_OFF);         \
        }                                   \
        else {                              \
            u_tests += u_local_tests;       \
        }                                   \
    }

#define RUN_TEST(name)                      \
    name();

#define DONE_TESTING                                             \
    printf("1..%d\n", u_tests);                                  \
                                                                 \
    if (u_tests_failed) {                                        \
        printf(COLOR_RED "FAILED tests\n" COLOR_OFF);                                \
        printf(COLOR_RED "Failed %d/%d tests\n" COLOR_OFF, u_tests_failed, u_tests); \
                                                                 \
        exit(255);                                               \
    } else {                                                     \
        printf(COLOR_GREEN "SUCCESS\n" COLOR_OFF);                                     \
    }                                                            \
                                                                 \
    exit(0);

#endif
