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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <libgen.h> /* basename */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h> /* stat */
#include <unistd.h>
#include <time.h>
#include <utime.h>
#include <errno.h>

#include "clax_version.h"
#include "clax_http.h"
#include "clax_log.h"
#include "clax_command.h"
#include "clax_big_buf.h"
#include "clax_crc32.h"
#include "clax_dispatcher.h"
#include "clax_util.h"
#include "clax_platform.h"

void clax_dispatch_index(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res)
{
    res->status_code = 200;
    clax_kv_list_push(&res->headers, "Content-Type", "application/json");
    clax_kv_list_push(&res->headers, "X-Clax-Root", clax_ctx->options->root);
    clax_big_buf_append_str(&res->body, "{\"message\":\"Hello, world!\",\"version\":\"" CLAX_VERSION "\",\"os\":\"" CLAX_OS "\",\"arch\":\"" CLAX_ARCH "\"}");
}
