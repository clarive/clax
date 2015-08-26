#ifndef _U_H
#define _U_H

#ifdef MVS
#include "../../arch/zos/libascii/_Ascii_a.h"
#endif

int u_tests;
int u_tests_failed;
int u_rv;

#define ASSERT(code)                        \
    u_local_tests++;                        \
    if (code) {                             \
        u_rv = 1;                           \
                                            \
        printf("ok %d\n", u_tests + u_local_tests + 1);     \
    }                                       \
    else {                                  \
        u_rv = 0;                           \
        u_tests_failed++;                   \
        printf("not ok %d\n", u_tests + u_local_tests + 1); \
    }

#define ASSERT_EQ(got, exp)             \
    ASSERT(got == exp)                  \
                                        \
    if (!u_rv) {                        \
        printf("# " #got ":\n");        \
        printf("#   got: '%d'\n", got);  \
        printf("#   exp: '%d'\n", exp);  \
    }

#define ASSERT_STR_EQ(got, exp)             \
    ASSERT(strcmp(got, exp) == 0)           \
                                            \
    if (!u_rv) {                            \
        printf("# " #got ":\n");            \
        printf("#   got: '%s'\n", got);      \
        printf("#   exp: '%s'\n", exp);      \
    }

#define TEST_START(name)                    \
    void name() {                           \
        int u_local_tests = 0;              \
                                            \
        printf("# %s\n", #name);

#define TEST_END                            \
        if (!u_local_tests) {               \
            printf("# no tests\n");         \
        }                                   \
        else {                              \
            u_tests += u_local_tests;       \
        }                                   \
    }

#define RUN_TEST(name)                      \
    name()

#define DONE_TESTING                                             \
    printf("1..%d\n", u_tests);                                  \
                                                                 \
    if (u_tests_failed) {                                        \
        printf("FAILED tests\n");                                \
        printf("Failed %d/%d tests\n", u_tests_failed, u_tests); \
                                                                 \
        exit(255);                                               \
    } else {                                                     \
        printf("SUCCESS\n");                                     \
    }                                                            \
                                                                 \
    exit(0);

#endif
