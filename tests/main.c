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

#include <stdlib.h>
#include <stdio.h>
#include "u.h"

#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

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
    RUN_TEST(clax_http_chunked_writes_chunks)

    RUN_TEST(clax_http_status_message_returns_message)
    RUN_TEST(clax_http_status_message_returns_unknown_message)

    RUN_TEST(clax_dispatch_sets_404_on_unknown_path)
    RUN_TEST(clax_dispatch_saves_upload_string_to_file)
    RUN_TEST(clax_dispatch_saves_upload_file_to_file)
    RUN_TEST(clax_dispatch_returns_bad_request_when_wrong_params)

    RUN_TEST(clax_command_start_runs_command)
    RUN_TEST(clax_command_start_returns_error)
    RUN_TEST(clax_command_read_reads_output)
    RUN_TEST(clax_command_runs_command_vaargs)

    RUN_TEST(clax_http_extract_kv_extracts_val)

    DONE_TESTING
}
