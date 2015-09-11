#include "u.h"
#include "util.h"

TEST_START(test_index)
{
    int rcount;
    char output[1024];

    rcount = execute("../clax -n -r . -l /dev/null", "GET /\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))
}
TEST_END
