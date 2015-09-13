#include "u/u.h"

#include "util.h"

#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

int main(void)
{
    START_TESTING

    RUN_SUITE(index)
    RUN_SUITE(basic_auth)
    RUN_SUITE(tree)
    RUN_SUITE(command)

    DONE_TESTING
}
