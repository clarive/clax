#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ftw.h>

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
        fprintf(stderr, "Command failed=%d", ret);
        return -1;
    }

    write(ctx.in, request, strlen(request));

    int offset = 0;
    while ((ret = read(ctx.out, obuf + offset, olen - offset)) > 0) {
        offset += ret;
    }
    obuf[offset] = 0;

    if (pclose2(&ctx) != 0) {
        fprintf(stderr, "Command failed\n");
        return -1;
    }

    return offset;
}

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = remove(fpath);

    if (rv)
        perror(fpath);

    return rv;
}

char *mktmpdir()
{
    char template[] = "/tmp/clax.tests.tmpdir.XXXXXX";
    char *tmpdir = mkdtemp(template);
    mkdir(tmpdir, 0755);

    return strdup(tmpdir);
}

int rmrf(char *path)
{
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}
