#ifndef CLAX_COMMAND_H
#define CLAX_COMMAND_H

#include "clax_http.h"

typedef struct {
    char command[1024];
    int timeout;
} command_ctx_t;

int clax_command(command_ctx_t *ctx, clax_http_chunk_cb_t chunk_cb, va_list a_list);

#endif
