#include <stdlib.h>
#include <stdio.h>
#include "u.h"

int main(int argc, char *argv[])
{
    RUN_TEST(clax_http_parse_returns_0_when_need_more);
    RUN_TEST(clax_http_parse_returns_error_when_error);
    RUN_TEST(clax_http_parse_returns_ok_chunks);
    RUN_TEST(clax_http_parse_returns_ok);
    RUN_TEST(clax_http_parse_saves_message);

    DONE_TESTING;
}
