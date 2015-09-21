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

#ifndef CLAX_PLATFORM_H
#define CLAX_PLATFORM_H

#ifdef MVS
#endif

#if defined(_WIN32)

unsigned int sleep(unsigned int seconds);

#if defined(__MINGW64_VERSION_MAJOR)
# if defined(_USE_32BIT_TIME_T)
#  define gmtime_s _gmtime32_s
# else
#  define gmtime_s _gmtime64_s
# endif
#elif defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
# define gmtime_r(tp, tm) (gmtime((tp)))
# define localtime_r(tp, tm) (localtime((tp)))
#endif

#if defined(_MSC_VER) || defined(__MINGW64_VERSION_MAJOR)
# define gmtime_r(tp, tm) ((gmtime_s((tm), (tp)) == 0) ? (tm) : NULL)
# define localtime_r(tp, tm) ((localtime_s((tm), (tp)) == 0) ? (tm) : NULL)
#endif

#endif

#endif
