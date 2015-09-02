#include <stdlib.h>
#include <stdio.h>
#include "u.h"

int main(int argc, char *argv[])
{
    RUN_TEST(clax_http_parse_returns_0_when_need_more)
    RUN_TEST(clax_http_parse_returns_error_when_error)
    RUN_TEST(clax_http_parse_returns_ok_chunks)
    RUN_TEST(clax_http_parse_returns_ok)
    RUN_TEST(clax_http_parse_saves_request)
    RUN_TEST(clax_http_parse_saves_body)
    RUN_TEST(clax_http_parse_parses_form_body)
    RUN_TEST(clax_http_parse_parses_form_body_with_decoding)
    RUN_TEST(clax_http_parse_parses_multipart_body)
    RUN_TEST(clax_http_url_decode_decodes_string_inplace)
    RUN_TEST(clax_http_parse_returns_done_when_100_continue)

    RUN_TEST(clax_http_status_message_returns_message)
    RUN_TEST(clax_http_status_message_returns_unknown_message)

    RUN_TEST(clax_dispatch_sets_404_on_unknown_path)
    RUN_TEST(clax_dispatch_saves_upload_string_to_file)
    RUN_TEST(clax_dispatch_saves_upload_file_to_file)

    RUN_TEST(clax_command_runs_command)
    RUN_TEST(clax_command_runs_command_vaargs)
    RUN_TEST(clax_command_runs_command_with_error)

    RUN_TEST(clax_http_extract_kv_extracts_val)

    DONE_TESTING
}
