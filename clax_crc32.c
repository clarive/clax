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

/*
 * Based on CRC-32 version 1.04 by Craig Bruce, 05-Dec-1994
 */

#include <stdio.h>
#include <unistd.h>

#include "clax_crc32.h"
#include "clax_platform.h"

static unsigned long clax_crc32_table[256];

static void clax_crc32_gen(void)
{
    unsigned long poly;
    int i, j;

    poly = 0xEDB88320L;
    for (i = 0; i < 256; i++) {
        unsigned long crc = i;
        for (j = 8; j > 0; j--) {
            if (crc & 1) {
                crc = (crc >> 1) ^ poly;
            } else {
                crc >>= 1;
            }
        }

        clax_crc32_table[i] = crc;
    }
}

unsigned long clax_crc32_init()
{
    clax_crc32_gen();

    return 0xFFFFFFFF;
}

unsigned long clax_crc32_finalize(unsigned long crc)
{
    return crc ^ 0xFFFFFFFF;
}

unsigned long clax_crc32_calc(unsigned long crc, unsigned char *buf, size_t len)
{
    unsigned char *p = buf;

    do {
        crc = ((crc >> 8) & 0x00FFFFFF) ^ clax_crc32_table[(unsigned char)((crc & 0xff) ^ *(p++))];
    } while (--len);

    return crc;
}
