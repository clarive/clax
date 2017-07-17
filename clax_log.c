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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>

#include "clax_platform.h"

int clax_log_(const char *file, int line, const char *func, char *fmt, ...)
{
    int ret;
    va_list args;
    char timestr[255];
    time_t epoch;
    struct tm *timeinfo;
    char *p = NULL;

    time(&epoch);
    timeinfo = localtime(&epoch);
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", timeinfo);

    va_start(args, fmt);

    ret = vasprintf(&p, fmt, args);
    if (ret != -1) {
        ret = fprintf(stderr, "%s:%d:%s:%d:%s(): %s\n", timestr, getpid(), file, line, func, p);
    }

    va_end(args);
    free(p);

    return ret;
}
