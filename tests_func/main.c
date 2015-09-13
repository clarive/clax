#include "u/u.h"

#include "util.h"

#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

int main(void)
{
    RUN_TEST(test_index)

    RUN_TEST(test_basic_auth)
    RUN_TEST(test_basic_auth_header_missing)
    RUN_TEST(test_basic_auth_header_invalid)
    RUN_TEST(test_basic_auth_header_invalid_base64)
    RUN_TEST(test_basic_auth_no_colon)
    RUN_TEST(test_basic_auth_wrong_username)
    RUN_TEST(test_basic_auth_wrong_password)

    RUN_TEST(test_tree_upload)
    RUN_TEST(test_tree_upload_with_different_name)
    RUN_TEST(test_tree_download)
    RUN_TEST(test_tree_download_not_found)

    RUN_TEST(test_command)
    RUN_TEST(test_command_failure)

    DONE_TESTING
}
