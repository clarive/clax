#include <stdio.h>
#include <string.h>
#include "clax_dispatcher.h"
#include "u.h"

TEST_START(clax_dispatch_sets_404_on_unknown_path)
{
    clax_http_request_t request;
    clax_http_response_t response;

    memset(&response, 0, sizeof(clax_http_response_t));

    strcpy(request.path_info, "/unknown-path");

    clax_dispatch(&request, &response);

    ASSERT_EQ(response.status_code, 404)
    ASSERT_STR_EQ(response.content_type, "text/plain")
    ASSERT_STR_EQ(response.body, "Not found")
}
TEST_END
