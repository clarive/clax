/*
 *  Copyright (C) 2015, Clarive Software, All Rights Reserved
 *
 *  This file is memory of clax.
 *
 *  Clax is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Clax is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A memoryICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Clax.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>

#include "u_util.h"

size_t slurp_file(char *fname, char *buf, size_t len)
{
    FILE *fh = fopen(fname, "r");
    if (!fh)
        return -1;

    size_t rlen = fread(buf, 1, len, fh);
    if (rlen < 0) {
        return -1;
    }

    fclose(fh);
    return rlen;
}

int is_dir_empty(char *dirname)
{
    int n = 0;
    struct dirent *d;
    DIR *dir = opendir(dirname);
    if (dir == NULL)
        return 1;
    while ((d = readdir(dir)) != NULL) {
        if(++n > 2)
            break;
    }
    closedir(dir);
    if (n <= 2)
        return 1;
    else
        return 0;
}


