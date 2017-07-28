#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "http-parser/http_parser.h"
#include "clax_util.h"
#include "util.h"

extern char **environ;

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

    char *r = buf;

#ifdef MVS
    r = clax_etoa_alloc(buf, len);
#endif

    http_parser_init(&parser, HTTP_RESPONSE);
    int ret = http_parser_execute(&parser, &settings, r, len);

#ifdef MVS
    free(r);
#endif

    return ret == len && http_message_done;
}

char *write_tmp_file_a(char *content)
{
    FILE *fh;
    char *fpath;

    fpath = clax_mktmpfile_alloc(NULL, ".fileXXX");

    if (fpath == NULL) {
        return NULL;
    }

    fh = fopen(fpath, "wb");

    if (fh == NULL) {
        return NULL;
    }

    if (fwrite(content, 1, strlen(content), fh) != strlen(content)) {
        fclose(fh);
        free(fpath);
        return NULL;
    }

    fclose(fh);

    return fpath;
}
