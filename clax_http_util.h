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

#ifndef CLAX_HTTP_UTIL_H
#define CLAX_HTTP_UTIL_H

#include "clax_util.h"

const char *clax_http_extract_kv(const char *str, const char *key, size_t *len);
void clax_http_parse_urlencoded(clax_kv_list_t *params, const char *buf, size_t len);
const char *clax_http_status_message(int code);
size_t clax_http_url_decode(char *str);
int clax_http_check_basic_auth(char *header, char *username, char *password);

#endif
