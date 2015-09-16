#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ftw.h>
#include <errno.h>

#include "http-parser/http_parser.h"
#include "popen2.h"
#include "util.h"

int http_message_done = 0;
int util_message_complete_cb(http_parser *p)
{
    http_message_done = 1;

    return 0;
}

int util_parse_http_response(char *buf, size_t len)
{
    http_parser parser;
    http_parser_settings settings = {
      .on_message_complete = util_message_complete_cb
    };

    if (len < 0)
        return -1;

    http_message_done = 0;

    http_parser_init(&parser, HTTP_RESPONSE);
    int ret = http_parser_execute(&parser, &settings, buf, len);

    return ret == len && http_message_done;
}

int execute(char *command, char *request, char *obuf, size_t olen)
{
    int ret;
    popen2_t ctx;

    ret = popen2(command, &ctx);
    if (ret < 0) {
        fprintf(stderr, "Command '%s' failed=%d", command, ret);
        return -1;
    }

    int offset = 0;
    size_t request_len = strlen(request);
    int wcount = 0;

    while (1) {
        ret = write(ctx.in, request + offset, request_len - offset);
        wcount += ret;

        if (wcount == request_len)
            break;

        if (ret < 0 && errno == EAGAIN) {
            continue;
        }

        offset += ret;
    }

    offset = 0;
    while (1) {
        ret = read(ctx.out, obuf + offset, olen - offset);

        if (ret == 0)
            break;

        if (ret < 0 && errno == EAGAIN) {
            continue;
        }

        offset += ret;
    }
    obuf[offset] = 0;

    int exit_code = pclose2(&ctx);
    if (exit_code != 0) {
        fprintf(stderr, "Exit code=%d\n", exit_code);
        return -1;
    }

    return offset;
}
