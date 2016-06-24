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

#include <limits.h>

#include "clax_errors.h"

const char *clax_errors[] = {
    "System error, check log file",
    "Invalid command line arguments",
    "Log file was not specified",
    "Cannot open log file",
    "Cannot write to log file"
};

const char *clax_strerror(int error)
{
    int index = error * -1 - 1;

    return clax_errors[index];
}
