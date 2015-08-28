#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include "popen2.h"
#include "clax_http.h"

int clax_command(char *command, clax_http_chunk_cb_t chunk_cb, va_list a_list_)
{
    char buf[1024];
    int ret;
    popen2_t kid;
    va_list a_list;

    va_copy(a_list, a_list_);

    ret = popen2(command, &kid);
    if (ret < 0) {
        return -1;
    }

    do {
        ret = read(kid.out, buf, sizeof(buf));

        if (ret > 0) {
            chunk_cb(buf, ret, a_list);
        }
    } while (ret > 0 || (ret == -1 && errno == EAGAIN));

    return pclose2(&kid);
}
