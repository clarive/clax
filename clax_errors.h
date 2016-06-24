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

#ifndef CLAX_ERRORS_H
#define CLAX_ERRORS_H

#define CLAX_ERROR_START -1

#define CLAX_ERROR_INVALID_CMD_ARGS (CLAX_ERROR_START - 1)
#define CLAX_ERROR_LOG_REQUIRED     (CLAX_ERROR_START - 2)
#define CLAX_ERROR_LOG_CANTOPEN     (CLAX_ERROR_START - 3)
#define CLAX_ERROR_LOG_CANTWRITE    (CLAX_ERROR_START - 4)

const char *clax_strerror(int error);

#endif
