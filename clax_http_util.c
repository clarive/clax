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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

#include "contrib/base64/base64.h"
#include "clax_util.h"
#include "clax_http_util.h"
#include "clax_platform.h"

enum {
    PARAM_MODE_KEY,
    PARAM_MODE_VAL
};

struct clax_http_status_message_s {
    int code;
    const char *message;
};

struct clax_http_status_message_s clax_http_messages[] = {
    {100, "Continue"},
    {200, "OK"},
    {204, "No Content"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {500, "Internal System Error"},
    {502, "Bad Gateway"}
};

const char *clax_http_status_message(int code)
{
    int i;
    size_t len = sizeof clax_http_messages / sizeof clax_http_messages[0];

    for (i = 0; i < len; i++) {
        if (clax_http_messages[i].code == code) {
            return clax_http_messages[i].message;
        }
    }

    return "Unknown";
}

void clax_http_parse_urlencoded(clax_kv_list_t *params, const char *buf, size_t len)
{
    const char *key = NULL;
    size_t key_len = 0;
    const char *val = NULL;
    size_t val_len = 0;
    char mode = PARAM_MODE_KEY;

    const char *b = buf;

    int j;
    for (j = 0; j < len; j++) {
        if (b[j] == '&') {
            mode = PARAM_MODE_KEY;

            if (key_len || val_len) {
                char *k = clax_buf2str(key, key_len);
                clax_http_url_decode(k);
                char *v = clax_buf2str(val, val_len);
                clax_http_url_decode(v);

                clax_kv_list_push(params, k, v);

                free(k);
                free(v);
            }

            key = NULL;
            key_len = 0;
            val = NULL;
            val_len = 0;
        }
        else if (b[j] == '=') {
            mode = PARAM_MODE_VAL;
        }
        else if (mode == PARAM_MODE_KEY) {
            if (key == NULL)
                key = b + j;

            key_len++;
        }
        else if (mode == PARAM_MODE_VAL) {
            if (val == NULL)
                val = b + j;

            val_len++;
        }
    }

    if (key_len || val_len) {
        char *k = clax_buf2str(key, key_len);
        clax_http_url_decode(k);
        char *v = clax_buf2str(val, val_len);
        clax_http_url_decode(v);

        clax_kv_list_push(params, k, v);

        free(k);
        free(v);
    }
}

size_t clax_http_url_decode(char *str)
{
    int code;
    char hex[3];
    size_t len;
    size_t new_len;

    if (str == NULL)
        return 0;

    new_len = len = strlen(str);

    char *p = str;

    /* we cannot use *p++ pattern here, since we can get \0 from %00 */
    while (p - str <= len) {
        if (*p == '+') {
            *p = ' ';
        }
        else if (*p == '%' && *(p + 1) && *(p + 2)) {
            hex[0] = *(p + 1);
            hex[1] = *(p + 2);
            hex[2] = 0;

            sscanf(hex, "%x", (unsigned int *)&code);
            new_len -= 2;

            *p = code;

#ifdef MVS
            clax_atoe(p, 1);
#endif

            if (*(p + 3)) {
                memmove(p + 1, p + 3, strlen(p + 3) + 1);
            }
            else {
                *(p + 1) = 0;
                break;
            }
        }

        p++;
    }

    return new_len;
}

int clax_http_check_basic_auth(char *header, char *username, char *password)
{
    int ok = 0;
    char *prefix = "Basic ";
    size_t prefix_len = strlen(prefix);
    char *auth = NULL;
    size_t auth_len = 0;
    char *sep;

    ok = header && strlen(header) > prefix_len && strncmp(header, prefix, prefix_len) == 0;

    if (ok)
        ok = base64_decode_alloc(header + prefix_len, strlen(header) - prefix_len, &auth, &auth_len) && auth != NULL;

    if (ok && auth) {
        auth[auth_len] = 0;
#ifdef MVS
        clax_atoe(auth, strlen(auth));
#endif
        sep = strstr(auth, ":");
        ok = sep != NULL;
    }

    if (ok) {
        size_t username_len = sep - auth;
        ok = strlen(username) == username_len
            && strncmp(username, auth, username_len) == 0;
    }

    if (ok) {
        size_t password_len = auth_len - (sep - auth) - 1;
        ok = strlen(password) == password_len
            && strncmp(sep + 1, password, password_len) == 0;
    }

    free(auth);

    return ok;
}

const char *clax_http_extract_kv(const char *str, const char *key, size_t *len)
{
    char *p;
    char *q;

    if (len)
        *len = 0;

    p = strstr(str, key);
    if (!p) return NULL;

    p += strlen(key);
    if (*p++ != '=') return NULL;
    if (*p++ != '"') return NULL;

    q = strchr(p, '"');
    if (!q) return NULL;

    if (len)
        *len = q - p;

    return p;
}
