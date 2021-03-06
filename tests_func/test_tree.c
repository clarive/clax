#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h> /* stat */

#include "u/u.h"

#include "clax_util.h"
#include "util.h"

SUITE_START(tree)

TEST_START(upload)
{
    char output[1024];
    char request[1024];
    char config[1024];
    char body[] = "--------------------------0e6bb0a28f620f98\r\n"
                  "Content-Disposition: form-data; name=\"file\"; filename=\"foobar\"\r\n"
                  "\r\n"
                  "hello from file!\r\n"
                  "--------------------------0e6bb0a28f620f98--\r\n";

    sprintf(request,
            "POST /tree/ HTTP/1.1\r\n"
            "Content-Type: multipart/form-data; boundary=------------------------0e6bb0a28f620f98\r\n"
            "Content-Length: %d\r\n"
            "\r\n"
            "%s", (int)strlen(body), body);

    char *tmpdir = clax_mktmpdir_alloc();

    sprintf(config, "root=%s\nchdir=yes", tmpdir);
    char *fpath = write_tmp_file_a(config);

    char command[1024];
    sprintf(command, CMD " -c %s -l " DEVNULL, fpath);

    int rcount = execute(command, request, output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    struct stat st;
    char *new_fpath = clax_strjoin("/", tmpdir, "foobar", NULL);
    ASSERT(stat(new_fpath, &st) == 0 && (st.st_mode & S_IFREG));

    free(new_fpath);
    rmrf(tmpdir);

    rmrf(fpath);
}
TEST_END

TEST_START(upload_with_different_name)
{
    char output[1024];
    char request[1024];
    char config[1024];
    char body[] = "--------------------------0e6bb0a28f620f98\r\n"
                  "Content-Disposition: form-data; name=\"file\"; filename=\"foobar\"\r\n"
                  "\r\n"
                  "hello from file!\r\n"
                  "--------------------------0e6bb0a28f620f98--\r\n";

    sprintf(request,
            "POST /tree/?name=another-name HTTP/1.1\r\n"
            "Content-Type: multipart/form-data; boundary=------------------------0e6bb0a28f620f98\r\n"
            "Content-Length: %d\r\n"
            "\r\n"
            "%s", (int)strlen(body), body);

    char *tmpdir = clax_mktmpdir_alloc();

    sprintf(config, "root=%s\nchdir=yes", tmpdir);
    char *fpath = write_tmp_file_a(config);

    char command[1024];
    sprintf(command, CMD " -c %s -l " DEVNULL, fpath);

    int rcount = execute(command, request, output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_MATCHES(output, "200 OK")

    struct stat st;
    char *newname_fpath = clax_strjoin("/", tmpdir, "another-name", NULL);
    ASSERT(stat(newname_fpath, &st) == 0 && (st.st_mode & S_IFREG));

    free(newname_fpath);
    rmrf(tmpdir);

    rmrf(fpath);
}
TEST_END

TEST_START(download)
{
    char output[1024];

    int rcount = execute(CMD " -l" DEVNULL, "GET /tree/main.c\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_MATCHES(output, "200 OK")
    ASSERT_MATCHES(output, "Content-Disposition: attachment; filename=\"main.c\"")
    ASSERT_MATCHES(output, "Last-Modified: ")
}
TEST_END

TEST_START(download_not_found)
{
    char output[1024];

    int rcount = execute(CMD " -l" DEVNULL, "GET /tree/unlikely-to-exist\r\n\r\n", output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_MATCHES(output, "404 Not Found")
}
TEST_END

TEST_START(delete file)
{
    char output[1024];
    char request[1024];
    char config[1024];
    char body[] = "--------------------------0e6bb0a28f620f98\r\n"
                  "Content-Disposition: form-data; name=\"file\"; filename=\"foobar\"\r\n"
                  "\r\n"
                  "hello from file!\r\n"
                  "--------------------------0e6bb0a28f620f98--\r\n";

    sprintf(request,
            "POST /tree/ HTTP/1.1\r\n"
            "Content-Type: multipart/form-data; boundary=------------------------0e6bb0a28f620f98\r\n"
            "Content-Length: %d\r\n"
            "\r\n"
            "%s", (int)strlen(body), body);

    char *tmpdir = clax_mktmpdir_alloc();

    sprintf(config, "root=%s\nchdir=yes", tmpdir);
    char *config_fpath = write_tmp_file_a(config);

    char command[1024];
    sprintf(command, CMD " -c %s -l " DEVNULL, config_fpath);

    execute(command, request, output, sizeof(output));

    sprintf(request, "DELETE /tree/foobar HTTP/1.1\r\n\r\n");

    int rcount = execute(command, request, output, sizeof(output));

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    struct stat st;
    char *fpath = clax_strjoin("/", tmpdir, "foobar", NULL);
    ASSERT(stat(fpath, &st) != 0);

    free(fpath);
    rmrf(tmpdir);

    rmrf(config_fpath);
}
TEST_END

TEST_START(delete not found)
{
    char output[1024];

    int rcount = execute(CMD " -l" DEVNULL, "DELETE /tree/unlikely-to-exist\r\n\r\n", output, sizeof(output));
    output[rcount] = 0;

    ASSERT(rcount > 0);
    ASSERT(util_parse_http_response(output, rcount))

    ASSERT_MATCHES(output, "404 Not Found")
}
TEST_END

SUITE_END
