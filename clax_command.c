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
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include "popen2.h"
#include "clax_log.h"
#include "clax_http.h"
#include "clax_command.h"

volatile int alarm_fired = 0;

void clax_command_timeout()
{
    signal(SIGALRM, SIG_IGN);

    alarm_fired = 1;
}

void clax_kill_kid(popen2_t *kid)
{
    int ret;

    clax_log("Killing pgroup=%d", -kid->pid);
    ret = kill(-kid->pid, SIGTERM);
    if (ret != 0)
        clax_log("Killing pgroup failed");

    clax_log("Killing pid=%d", kid->pid);
    ret = kill(kid->pid, SIGTERM);
    if (ret != 0)
        kill(kid->pid, SIGKILL);
}

int clax_command(command_ctx_t *ctx, clax_http_chunk_cb_t chunk_cb, va_list a_list_)
{
    char buf[1024];
    int ret;
    popen2_t kid;
    va_list a_list;

    char *command = ctx->command;
    int timeout = ctx->timeout;

    va_copy(a_list, a_list_);

    clax_log("Running command '%s'", command);

    ret = popen2(command, &kid);
    if (ret < 0) {
        clax_log("Command failed=%d", ret);
        return -1;
    }

    clax_log("Command started, pid=%d", kid.pid);

    fcntl(kid.out, F_SETFL, (fcntl(kid.out, F_GETFL, 0) | O_NONBLOCK));

    if (timeout) {
        clax_log("Setting command timeout=%d", timeout);

        alarm_fired = 0;
        signal(SIGALRM, clax_command_timeout);
        alarm(timeout);
    }

    while(1) {
        ret = read(kid.out, buf, sizeof(buf));

        if (ret == 0)
            break;

        if (alarm_fired) {
            clax_log("Command timeout reached=%d", timeout);

            clax_kill_kid(&kid);

            break;
        }

        if (ret < 0) {
            if (errno == EAGAIN) {

                /* 0.2s */
                usleep(200000);

                continue;
            }

            break;
        }

        if (kill(kid.pid, 0) != 0) {
            clax_log("Command unexpectedty exited");
            break;
        }

        ret = chunk_cb(buf, ret, a_list);
        if (ret < 0) {
            clax_log("Returning chunk failed. Client must be disconnected");
        }
    };

    ret = pclose2(&kid);
    clax_log("Command finished, exit_code=%d", ret);

    return ret;
}
