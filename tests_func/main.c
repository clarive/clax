#include "u.h"
#include "util.h"

#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

int main(void)
{
    RUN_TEST(test_index)

    RUN_TEST(test_tree_upload)
    RUN_TEST(test_tree_upload_with_different_name)
    RUN_TEST(test_tree_download)
    RUN_TEST(test_tree_download_not_found)

    RUN_TEST(test_command)
    RUN_TEST(test_command_failure)

    DONE_TESTING
}
