#ifndef _U_H
#define _U_H

#ifdef MVS
#include "../../arch/zos/libascii/_Ascii_a.h"
#endif

#define COLOR_GREEN "\e[0;32m"
#define COLOR_RED "\e[0;31m"
#define COLOR_YELLOW "\e[0;33m"
#define COLOR_OFF "\e[0m"

int u_tests;
int u_tests_failed;
int u_rv;

#define ASSERT(code)                        \
    u_local_tests++;                        \
    if (code) {                             \
        u_rv = 1;                           \
                                            \
        printf("ok %d\n", u_tests + u_local_tests);     \
    }                                       \
    else {                                  \
        u_rv = 0;                           \
        u_tests_failed++;                   \
        printf(COLOR_RED "not ok %d\n" COLOR_OFF, u_tests + u_local_tests); \
        printf("# %s:%d\n", __FILE__, __LINE__); \
    }

#define ASSERT_EQ(got, exp)             \
    ASSERT(got == exp)                  \
                                        \
    if (!u_rv) {                        \
        printf("# " #got ":\n");        \
        printf("#   got: %d\n", got);  \
        printf("#   exp: %d\n", exp);  \
    }

#define ASSERT_STR_EQ(got, exp)             \
    if (got != NULL) {                      \
        ASSERT(strcmp(got, exp) == 0)       \
                                            \
        if (!u_rv) {                        \
            printf("# " #got ":\n");        \
            printf("#   got: '%s'\n", got); \
            printf("#   exp: '%s'\n", exp); \
        }                                   \
    } else {                                \
        u_local_tests++;                        \
        u_tests_failed++; \
                            \
        printf(COLOR_RED "not ok %d\n" COLOR_OFF, u_tests + u_local_tests); \
        printf("# %s:%d\n", __FILE__, __LINE__); \
                                                    \
        printf("# " #got ":\n");            \
        printf("#   got: NULL\n", got);     \
        printf("#   exp: '%s'\n", exp);     \
    }

#define ASSERT_STRN_EQ(got, exp, len)         \
    ASSERT(strncmp(got, exp, len) == 0)       \
                                              \
    if (!u_rv) {                              \
        printf("# " #got ":\n");              \
        printf("#   got: '%.*s'\n", len, got);\
        printf("#   exp: '%s'\n", exp);       \
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
