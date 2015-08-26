#include <string.h>
#include "clax_http.h"

int clax_dispatch(clax_http_request_t *req, clax_http_response_t *res)
{
    char *path_info = req->path_info;

    if (strncmp(path_info, "/ping", 5) == 0) {
        res->status_code = 200;
        res->content_type = "application/json";
        memcpy(res->body, "{\"message\":\"pong\"}", 20);
        res->body_len = 20;

        return 1;
    }
    else {
        res->content_type = "text/plain";
        res->status_code = 404;
        memcpy(res->body, "Not found", 9);
        res->body_len = 9;

        return -1;
    }
}
