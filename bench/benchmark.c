#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ftw.h>

#ifdef WIN32

#include <windows.h>
double get_time()
{
    LARGE_INTEGER t, f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&f);
    return (double)t.QuadPart/(double)f.QuadPart;
}

#else

#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

double get_time()
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec*1e-6;
}

#endif

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = remove(fpath);

    if (rv)
        perror(fpath);

    return rv;
}

int rmrf(char *path)
{
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

int execute(char *command, char *request, size_t len)
{
    FILE *fp;

    fp = popen(command, "w");

    if (!fp){
        fprintf(stderr, "Could not open pipe for output.\n");
        return -1;
    }

    fwrite(request, 1, len, fp);

    if (fclose(fp) != 0) {
        fprintf(stderr, "Command failed\n");
        return -1;
    }

    return 0;
}

void benchmark(char *name, char *command, char *request, int iter)
{
    size_t req_len = strlen(request);
    int step = iter / 10;

    printf("> %s (%d)...\n\n", name, iter);

    double start = get_time();

    for (int i = 0; i < iter; i++) {
        if (execute(command, request, req_len) < 0)
            break;

        if (i && i % step == 0) {
            printf("    %5d done\n", i);
        }
    }

    printf("    %5d finish\n", iter);

    double end = get_time();

    printf("\n    elapsed: %f\n\n", end - start);
}

void benchmark_simple_request(char *tmpdir, int iter)
{
    benchmark("Simple Request", "../clax -n -r . -l /dev/null > /dev/null", "GET /\r\n\r\n", iter);
}

void benchmark_file_upload(char *tmpdir, int iter)
{
    char request[1024];

    char body[] = "--------------------------0e6bb0a28f620f98\r\n"
                  "Content-Disposition: form-data; name=\"file\"; filename=\"foobar\"\r\n"
                  "\r\n"
                  "hello from file!\r\n"
                  "--------------------------0e6bb0a28f620f98--\r\n";

    sprintf(request,
            "POST /upload HTTP/1.1\r\n"
            "Content-Type: multipart/form-data; boundary=------------------------0e6bb0a28f620f98\r\n"
            "Content-Length: %d\r\n"
            "\r\n"
            "%s", (int)strlen(body), body);

    char command[1024];
    sprintf(command, "../clax -n -r %s -l /dev/null > /dev/null", tmpdir);

    benchmark("File Upload", command, request, iter);
}

void benchmark_file_download(char *tmpdir, int iter)
{
    benchmark("File Download", "../clax -n -r . -l /dev/null > /dev/null", "GET /download?file=benchmark.c\r\n\r\n", iter);
}

void benchmark_command_execution(char *tmpdir, int iter)
{
    char request[1024];
    char body[] = "command=echo 'foo'";

    sprintf(request, 
            "POST /command\r\n"
            "Content-Type: application/x-www-form-urlencoded\r\n"
            "Content-Length: %d\r\n"
            "\r\n"
            "%s", (int)strlen(body), body);

    benchmark("Command Execution", "../clax -n -r . -l /dev/null > /dev/null", request, iter);
}

int main()
{
    printf("\nBenchmarking, grab a coffee...\n\n");

    char template[] = "/tmp/clax.bench.tmpdir.XXXXXX";
    char *tmp_dirname = mkdtemp(template);
    mkdir(tmp_dirname, 0755);

    double start = get_time();

    benchmark_simple_request(tmp_dirname, 1000);
    benchmark_file_upload(tmp_dirname, 1000);
    benchmark_file_download(tmp_dirname, 1000);
    benchmark_command_execution(tmp_dirname, 10);

    double end = get_time();

    rmrf(tmp_dirname);

    printf("\nTotal time elapsed: %f\n", end - start);

    printf("\nNOTE: If you finished your coffee before now, try increasing iterations number\n\n");

    return 0;
}
