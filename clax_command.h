/*
 *  Copyright (C) 2015, Clarive Software, All Rights Reserved
 *
 *  This file is part of clax.
 *
 *  Clax is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Clax is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Clax.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CLAX_COMMAND_H
#define CLAX_COMMAND_H

#include "popen2.h"
#include "clax_http.h"

typedef struct {
    char **env;
    char command[1024];
    int timeout;
    popen2_t kid;
} command_ctx_t;

int clax_command_start(command_ctx_t *ctx);
int clax_command_read_va(command_ctx_t *ctx, clax_http_chunk_cb_t chunk_cb, ...);
int clax_command_read(command_ctx_t *ctx, clax_http_chunk_cb_t chunk_cb, va_list a_list);
int clax_command_close(command_ctx_t *ctx);
int clax_command_is_running(command_ctx_t *ctx);

#endif
