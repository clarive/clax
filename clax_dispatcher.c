#include <string.h>
#include <stdarg.h>
#include "clax_http.h"
#include "clax_command.h"

void clax_command_cb(int (*chunk_cb)(char *buf, size_t len, va_list a_list), va_list a_list)
{
    clax_command("date", chunk_cb, a_list);
}

void clax_dispatch(clax_http_request_t *req, clax_http_response_t *res)
{
    char *path_info = req->path_info;

    memset(res, 0, sizeof(clax_http_response_t));

    if (strcmp(path_info, "/ping") == 0) {
        res->status_code = 200;
        res->content_type = "application/json";
        memcpy(res->body, "{\"message\":\"pong\"}", 20);
        res->body_len = 20;
    }
    else if (req->method == HTTP_POST && strcmp(path_info, "/command") == 0) {
        res->status_code = 200;
        res->transfer_encoding = "chunked";

        /*clax_command("tail -f /var/log/messages", res->body_cb);*/

        /*res->body_cb_ctx;*/
        res->body_cb = clax_command_cb;
    }
    else {
        res->content_type = "text/plain";
        res->status_code = 404;
        memcpy(res->body, "Not found", 9);
        res->body_len = 9;
    }
}
