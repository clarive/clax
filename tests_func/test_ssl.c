#include <errno.h>
#include "u/u.h"

#include "util.h"

SUITE_START(ssl)

TEST_START(rejects not ssl request)
{
    int rcount;
    char output[1024];
    char cmd[1024];

    char *fpath = write_tmp_file_a("[ssl]\nenabled=yes\ncert_file=ssl/server-with-ca.pem\nkey_file=ssl/server.key\n");

    sprintf(cmd, CMD " -c %s -l " DEVNULL, fpath);

    rcount = execute(cmd, "GET /\r\nContent-Length: 0\r\n\r\n", output, sizeof(output));

    fprintf(stderr, "%s\n", output);

    ASSERT_NULL(strstr(output, "200 OK"));

    rmrf(fpath);
}
TEST_END

SUITE_END
