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

#include <string.h>
#include <stdlib.h>

#include "clax_util.h"

#if defined(_WIN32)

#include <windows.h>

char *strndup(const char *str, size_t max_len)
{
    size_t len = MIN(strlen(str), max_len);

    char *p = malloc(len + 1);
    strncpy(p, str, len);
    p[len] = 0;
    return p;
}

unsigned int sleep(unsigned int seconds)
{
    Sleep(seconds * 1000);
    return 0;
}
#endif
