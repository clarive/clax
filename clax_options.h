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

#ifndef CLAX_OPTIONS_H
#define CLAX_OPTIONS_H

typedef struct {
    char root[255];
    char log_file[255];
    char access_log_file[255];
    char config_file[255];

    char *basic_auth_username;
    char *basic_auth_password;

    char ssl;
    char no_ssl_verify;
    char cert_file[255];
    char key_file[255];
    char entropy_file[255];

    /* Private */
    FILE *_log_file;
    FILE *_access_log_file;
} opt;

void clax_options_init(opt *options);
void clax_options_free(opt *options);
void clax_usage();
int clax_parse_options(opt *options, int argc, char **argv);

#endif
