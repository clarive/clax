#ifndef CLAX_COMMAND_H
#define CLAX_COMMAND_H

#include "clax_http.h"

int clax_command(char *command, clax_http_chunk_cb_t chunk_cb, va_list a_list);

#endif
