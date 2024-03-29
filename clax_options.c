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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>
#include <unistd.h>
#include <getopt.h>

#ifdef _WIN32
# include <windows.h>
#endif

#include "contrib/inih/ini.h"
#include "clax_log.h"
#include "clax_errors.h"
#include "clax_options.h"
#include "clax_util.h"
#include "clax_platform.h"
#include "clax.h"

extern char *optarg;
extern int optopt;
extern int optind;
extern int opterr;

void clax_options_init(opt *options)
{
    memset(options, 0, sizeof(opt));
}

void clax_options_free(opt *options)
{
    free(options->basic_auth_username);
    free(options->basic_auth_password);
}

void clax_usage()
{
    fprintf(stdout,
            "usage: clax [options]\n"
            "\n"
            "   -l <log_file>     path to log file (REQUIRED)\n"
            "   -c <config_file>  path to configuration file (defaults to clax.ini\n"
            "                         in binary location directory)\n"
            "\n"
            );
}

int clax_config_handler(void *ctx, const char *section, const char *name, const char *value)
{
    opt *options = ctx;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    if (MATCH("", "root")) {
        options->root[0] = 0;

        if (clax_strcatdir(options->root, sizeof_struct_member(opt, root), value) < 0) {
            return -1;
        }
        clax_san_path(options->root);
    } else if (MATCH("", "chdir")) {
        if (strcmp(value, "yes") == 0) {
            options->chdir = 1;
        }
    } else if (MATCH("ssl", "enabled")) {
        if (strcmp(value, "yes") == 0) {
            options->ssl = 1;
        }
    } else if (MATCH("ssl", "verify")) {
        if (strcmp(value, "no") == 0) {
            options->no_ssl_verify = 1;
        }
    } else if (MATCH("ssl", "cert_file")) {
        options->cert_file[0] = 0;

        if (clax_strcatfile(options->cert_file, sizeof_struct_member(opt, cert_file), value) < 0) {
            return -1;
        }
        clax_san_path(options->cert_file);
    } else if (MATCH("ssl", "key_file")) {
        options->key_file[0] = 0;

        if (clax_strcatfile(options->key_file, sizeof_struct_member(opt, key_file), value) < 0) {
            return -1;
        }
        clax_san_path(options->key_file);
    } else if (MATCH("ssl", "entropy_file")) {
        options->entropy_file[0] = 0;

        if (clax_strcatfile(options->entropy_file, sizeof_struct_member(opt, entropy_file), value) < 0) {
            return -1;
        }
        clax_san_path(options->entropy_file);
    } else if (MATCH("http_basic_auth", "username")) {
        options->basic_auth_username = clax_strdup(value);
    } else if (MATCH("http_basic_auth", "password")) {
        options->basic_auth_password = clax_strdup(value);
    } else if (MATCH("bind", "host")) {
        options->bind_host = clax_strdup(value);
    } else if (MATCH("bind", "port")) {
        options->bind_port = atoi(clax_strdup(value));
    } else {
        return 0;
    }

    return 1;
}

int clax_parse_options(opt *options, int argc, char **argv)
{
    int c;

    optarg = NULL;
    opterr = optopt = 0;
    optind = 1;

    while ((c = getopt(argc, argv, "h:l:c:")) != -1) {
        switch (c) {
        case 'c':
            if (clax_strcat(options->config_file, sizeof_struct_member(opt, config_file), optarg) < 0) {
                return -1;
            }
            clax_san_path(options->config_file);
            break;
        case 'l':
            if (clax_strcat(options->log_file, sizeof_struct_member(opt, log_file), optarg) < 0) {
                return -1;
            }
            clax_san_path(options->log_file);
            break;
        case '?':
        case 'h':
        default:
            return CLAX_ERROR_INVALID_CMD_ARGS;
        }
    }

    if (strlen(options->log_file) == 0) {
        return CLAX_ERROR_LOG_REQUIRED;
    }
    else {
        if (options->log_file[0]) {
            options->_log_file = fopen(options->log_file, "a");
            if (options->_log_file == NULL) {
                return CLAX_ERROR_LOG_CANTOPEN;
            }

            dup2(fileno(options->_log_file), STDERR_FILENO);

            if (clax_log("Started") < 0) {
                return CLAX_ERROR_LOG_CANTWRITE;
            }
        }
    }

    if (clax_detect_root(options->root, sizeof_struct_member(opt, root), argv) == NULL) {
        clax_log("Can't detect root directory: %s", strerror(errno));
        return -1;
    }

    clax_san_path(options->root);

    clax_log("Detected root directory: %s", options->root);

    if (options->chdir) {
        clax_log("Changing directory to '%s'", options->root);
        if (clax_chdir(options->root) < 0) {
            clax_log("Error: cannot chdir to '%s': %s", options->root, strerror(errno));
            return -1;
        }
    }

    if (strlen(options->config_file) == 0) {
        if (clax_strcatfile(options->config_file, sizeof_struct_member(opt, config_file), options->root) != 0) {
            return -1;
        }

        if (clax_strcatfile(options->config_file, sizeof_struct_member(opt, config_file), "clax.ini") != 0) {
            return -1;
        }

        if (clax_is_path_f(options->config_file)) {
            clax_log("Detected configuration file '%s'", options->config_file);
        }
        else {
            options->config_file[0] = 0;
        }
    }

    if (strlen(options->config_file)) {
        clax_log("Reading configuration file '%s'", options->config_file);

        char *old_root = clax_strdup(options->root);

        if (ini_parse(options->config_file, clax_config_handler, options) < 0) {
            clax_log("Error: Can't parse ini file");

            free(old_root);

            return -1;
        }

        if (strcmp(options->root, old_root) != 0) {
            clax_log("Root directory changed to '%s'", options->root);

            if (options->chdir) {
                clax_log("Changing directory to '%s'", options->root);
                if (clax_chdir(options->root) < 0) {
                    clax_log("Error: cannot chdir to '%s': %s", options->root, strerror(errno));

                    free(old_root);

                    return -1;
                }
            }
        }

        free(old_root);
    }

    if (options->ssl) {
        if (!strlen(options->cert_file) || !strlen(options->key_file)) {
            clax_log("Error: cert_file and key_file are required");
            return -1;
        }
    }

    if (options->bind_host) {
        options->standalone = 1;
        if (!options->bind_port) {
            options->bind_port = DEFAULT_PORT;
        }
    }

    return 0;
}
