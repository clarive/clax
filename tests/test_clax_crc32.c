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

#include "contrib/u/u.h"
#include "u_util.h"

#include "clax_crc32.h"

SUITE_START(clax_crc32)

TEST_START(calculates crc32)
{
    unsigned long crc32 = clax_crc32_init();

    crc32 = clax_crc32_calc(crc32, (unsigned char *)"he", 2);
    crc32 = clax_crc32_calc(crc32, (unsigned char *)"ll", 2);
    crc32 = clax_crc32_calc(crc32, (unsigned char *)"o", 1);

    crc32 = clax_crc32_finalize(crc32);

    ASSERT_EQ(crc32, 907060870);
}
TEST_END

SUITE_END
