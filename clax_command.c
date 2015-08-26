#include <stdio.h>

int clax_command(char *command, int (*chunk_cb)(char *buf, size_t len))
{
    FILE *fp;
    char buf[1024];
    size_t ret;

    fp = popen(command, "r");
    if (fp == NULL) {
        return -1;
    }

    while ((ret = fread(buf, 1, sizeof(buf), fp)) > 0) {
        chunk_cb(buf, ret);
    }

    chunk_cb(NULL, 0);

    pclose(fp);

    return 0;
}
