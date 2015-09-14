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
#include <stdio.h>

#include "u/u.h"

#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

int main(int argc, char *argv[])
{
    START_TESTING

    RUN_SUITE(clax_big_buf)
    RUN_SUITE(clax_command)
    RUN_SUITE(clax_dispatch)
    RUN_SUITE(clax_http)
    RUN_SUITE(clax_http_multipart_list)
    RUN_SUITE(clax_options)
    RUN_SUITE(clax_util)

    DONE_TESTING
}
