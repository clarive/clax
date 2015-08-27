#include <stdio.h>
#include <stdarg.h>

#include "clax_http.h"

int clax_command(char *command, clax_http_chunk_cb_t chunk_cb, va_list a_list_)
{
    FILE *fp;
    char buf[1024];
    size_t ret;
    va_list a_list;

    va_copy(a_list, a_list_);

    fp = popen(command, "r");
    if (fp == NULL) {
        return -1;
    }

    while ((ret = fread(buf, 1, sizeof(buf), fp)) > 0) {
        chunk_cb(buf, ret, a_list);
    }

    return pclose(fp) / 256;
}
