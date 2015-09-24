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
#include <stdarg.h>

#include "u/u.h"
#include "u_util.h"

#include "clax_crc32.h"
#include "clax_util.h"

SUITE_START(clax_crc32)

TEST_START(calculates crc32 from fd)
{
    char *tmp_dirname = clax_mktmpdir_alloc();
    char *filename = clax_strjoin("/", tmp_dirname, "file", NULL);

    FILE *fh = fopen(filename, "w");
    fprintf(fh, "%s", "\xab\xcd\xef");
    fclose(fh);

    fh = fopen(filename, "r");

    int crc32 = clax_crc32_calc_fd(fileno(fh));

    fclose(fh);

    ASSERT_EQ(crc32, 1686977913);

    free(filename);
    rmrf(tmp_dirname);
}
TEST_END

SUITE_END
