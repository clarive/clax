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
    int timeout_fired;
    popen2_t kid;
} command_ctx_t;

void clax_command_init(command_ctx_t *command_ctx);
void clax_command_free(command_ctx_t *command_ctx);
int clax_command_init_env(command_ctx_t *command_ctx, char **env);
const char *clax_command_get_env(command_ctx_t *ctx, const char *key);
char *clax_command_set_env(command_ctx_t *command_ctx, const char *key, const char *val);
char *clax_command_set_env_pair(command_ctx_t *ctx, const char *pair);
char *clax_command_env_expand_a(command_ctx_t *ctx, const char *val);

int clax_command_start(command_ctx_t *ctx);
int clax_command_read_va(command_ctx_t *ctx, clax_http_chunk_cb_t chunk_cb, ...);
int clax_command_read(command_ctx_t *ctx, clax_http_chunk_cb_t chunk_cb, va_list a_list);
int clax_command_close(command_ctx_t *ctx);
int clax_command_is_running(command_ctx_t *ctx);

#endif
