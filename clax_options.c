#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>

#ifdef _WIN32
# include <windows.h>
#endif

#include "inih/ini.h"
#include "clax.h"
#include "clax_log.h"
#include "clax_errors.h"
#include "clax_options.h"
#include "clax_util.h"
#include "clax_platform.h"

extern char *optarg;
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
        strncpy(options->root, value, sizeof_struct_member(opt, root));
    } else if (MATCH("", "log_file")) {
        strncpy(options->log_file, value, sizeof_struct_member(opt, log_file));
    } else if (MATCH("ssl", "enabled")) {
        if (strcmp(value, "yes") == 0) {
            options->ssl = 1;
        }
    } else if (MATCH("ssl", "verify")) {
        if (strcmp(value, "no") == 0) {
            options->no_ssl_verify = 1;
        }
    } else if (MATCH("ssl", "cert_file")) {
        strncpy(options->cert_file, value, sizeof_struct_member(opt, cert_file));
    } else if (MATCH("ssl", "key_file")) {
        strncpy(options->key_file, value, sizeof_struct_member(opt, key_file));
    } else if (MATCH("ssl", "entropy_file")) {
        strncpy(options->entropy_file, value, sizeof_struct_member(opt, entropy_file));
    } else if (MATCH("http_basic_auth", "username")) {
        options->basic_auth_username = clax_strdup(value);
    } else if (MATCH("http_basic_auth", "password")) {
        options->basic_auth_password = clax_strdup(value);
    } else {
        return 0;
    }

    return 1;
}

int clax_parse_options(opt *options, int argc, char **argv)
{
    int c;

    optarg = NULL;
    optind = opterr = optopt = 0;

    opterr = 0;
    while ((c = getopt(argc, argv, "h:l:c:")) != -1) {
        switch (c) {
        case 'c':
            strncpy(options->config_file, optarg, sizeof(options->config_file));
            break;
        case 'l':
            strncpy(options->log_file, optarg, sizeof(options->log_file));
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

    clax_log("Command line arguments:");
    for (int i = 0; i < argc; i++) {
        clax_log("%s", argv[i]);
    }

    if (clax_detect_root(options->root, sizeof_struct_member(opt, root), argv) == NULL) {
        clax_log("Can't detect root directory: %s", strerror(errno));
        return -1;
    }
    clax_log("Detected root directory: %s", options->root);

    clax_log("Changing directory to '%s'", options->root);
    if (clax_chdir(options->root) < 0) {
        clax_log("Error: cannot chdir to '%s': %s", options->root, strerror(errno));
        return -1;
    }

    if (strlen(options->config_file) == 0) {
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

        if (ini_parse(options->config_file, clax_config_handler, options) < 0) {
            clax_log("Error: Can't parse ini file");
            return -1;
        }

        char cwd[1024];
        getcwd(cwd, sizeof(cwd));

        if (strcmp(options->root, cwd) != 0) {
            clax_log("Root directory changed to '%s'", options->root);

            clax_log("Changing directory to '%s'", options->root);
            if (clax_chdir(options->root) < 0) {
                clax_log("Error: cannot chdir to '%s': %s", options->root, strerror(errno));
                return -1;
            }
        }
    }

    if (options->ssl) {
        if (!strlen(options->cert_file) || !strlen(options->key_file)) {
            clax_log("Error: cert_file and key_file are required");
            return -1;
        }
    }

    return 0;
}
