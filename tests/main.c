#include <stdlib.h>
#include <stdio.h>
#include "u.h"

int main(int argc, char *argv[])
{
    RUN_TEST(clax_http_parse_returns_0_when_need_more);
    RUN_TEST(clax_http_parse_returns_error_when_error);
    RUN_TEST(clax_http_parse_returns_ok_chunks);
    RUN_TEST(clax_http_parse_returns_ok);
    RUN_TEST(clax_http_parse_saves_request);

    RUN_TEST(clax_http_status_message_returns_message);
    RUN_TEST(clax_http_status_message_returns_unknown_message);

    RUN_TEST(clax_dispatch_sets_404_on_unknown_path);

    RUN_TEST(clax_command_runs_command);

    DONE_TESTING;
}
