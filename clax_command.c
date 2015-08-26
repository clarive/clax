#include <stdio.h>
#include <stdarg.h>

int clax_command(char *command, int (*chunk_cb)(char *buf, size_t len, va_list a_list), ...)
{
    FILE *fp;
    char buf[1024];
    size_t ret;
    va_list a_list;
    va_start(a_list, chunk_cb);

    fp = popen(command, "r");
    if (fp == NULL) {
        return -1;
    }

    while ((ret = fread(buf, 1, sizeof(buf), fp)) > 0) {
        chunk_cb(buf, ret, a_list);
    }

    chunk_cb(NULL, 0, a_list);

    va_end(a_list);

    pclose(fp);

    return 0;
}
