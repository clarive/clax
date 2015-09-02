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

void clax_log_(const char *file, int line, const char *func_, char *fmt, ...)
{
    int size;
    char *cp;
    va_list args;
    char func[1024];
    time_t epoch;
    struct tm *timeinfo;

    strcpy(func, func_);

#ifdef MVS
    char func_a[1024];
    __toascii_a((char * )func_a, func);
    strcpy(func, func_a);
#endif

    va_start(args, fmt);
    size = vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);

    time(&epoch);
    timeinfo = localtime(&epoch);

    va_start(args, fmt);
    cp = (char *)calloc(size, sizeof(char));
    if (cp != NULL && vsnprintf(cp, size, fmt, args) > 0) {
        char timestr[255];
        strftime(timestr, sizeof(timestr), "%Y-%m-%d %T", timeinfo);

        fprintf(stderr, "%s:%d:%s:%d:%s(): %s\n", timestr, getpid(), file, line, func, cp);

    }
    va_end(args);
    free(cp);
}
