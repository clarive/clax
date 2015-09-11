#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#include "inih/ini.h"
#include "clax_options.h"
#include "clax_util.h"

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

void usage()
{
    fprintf(stderr,
            "usage: clax [options]\n"
            "\n"
            "Options:\n"
            "\n"
            "   common\n"
            "   ------\n"
            "   -c <config_file>        path to configuration file\n"
            "   -z                      print default configuration\n"
            "   -r <root>               home directory (required, will chdir to it)\n"
            "   -l <log_file>           path to log file (default: stderr)\n"
            "   -a <username:password>  basic authentication credentials\n"
            "\n"
            "   ssl\n"
            "   ---\n"
            "   -n                      do not use ssl at all (default: on)\n"
            "   -k                      do not verify client certificate (default: on)\n"
            "   -t <cert_file>          path to cert file (required if ssl, CA included)\n"
            "   -p <key_file>           path to private key file (required if ssl)\n"
            "   -e <entropy_file>       path to entropy file (needed on some systems)\n"
            "\n"
            );

    exit(255);
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
        if (strcmp(value, "no") == 0) {
            options->no_ssl = 1;
        }
    } else if (MATCH("ssl", "cert_file")) {
        strncpy(options->cert_file, value, sizeof_struct_member(opt, cert_file));
    } else if (MATCH("ssl", "key_file")) {
        strncpy(options->key_file, value, sizeof_struct_member(opt, key_file));
    } else if (MATCH("ssl", "entropy_file")) {
        strncpy(options->entropy_file, value, sizeof_struct_member(opt, entropy_file));
    } else {
        return 0;
    }

    return 1;
}

void clax_parse_options(opt *options, int argc, char **argv)
{
    int c;

    if (argc < 2)
        usage();

#ifdef MVS
    if (argc > 1)
        __argvtoascii_a(argc, (char **)argv);
#endif

    opterr = 0;
    while ((c = getopt(argc, argv, "hnl:e:t:p:r:kc:za:")) != -1) {
        switch (c) {
        case 'c':
            strncpy(options->config_file, optarg, sizeof(options->config_file));
            break;
        case 'l':
            strncpy(options->log_file, optarg, sizeof(options->log_file));
            break;
        case 'r':
            /* -1 for the / if its needed */
            strncpy(options->root, optarg, sizeof(options->root) - 1);
            break;
        case 'n':
            options->no_ssl = 1;
            break;
        case 'k':
            options->no_ssl_verify = 1;
            break;
        case 'e':
            strncpy(options->entropy_file, optarg, sizeof(options->entropy_file));
            break;
        case 't':
            strncpy(options->cert_file, optarg, sizeof(options->cert_file));
            break;
        case 'p':
            strncpy(options->key_file, optarg, sizeof(options->key_file));
            break;
        case 'a': {
            char *sep;
            if ((sep = strstr(optarg, ":")) != NULL) {
                options->basic_auth_username = strndup(optarg, sep - optarg);
                options->basic_auth_password = strndup(sep + 1, strlen(optarg) - (sep - optarg));
            }
            else {
                fprintf(stderr, "Error: Invalid username:password pair\n\n");
                usage();
            }
            break;
                  }
        case 'z':
            fprintf(stderr,
                    "root = /opt/clarive/clax\n"
                    "log_file = /opt/clarive/logs/clax.log\n"
                    "\n"
                    "[ssl]\n"
                    "enabled = yes\n"
                    "cert_file = /opt/clarive/ssl/clax.crt\n"
                    "key_file = /opt/clarive/ssl/clax.key\n"
                    "#entropy_file = /opt/clarive/entropy\n"
                    "\n"
                    "[http_basic_auth]\n"
                    "enabled = yes\n"
                    "username = clax\n"
                    "password = password\n"
                   );

            exit(0);
            break;
        case '?':
        case 'h':
        default:
            usage();
        }
    }

    if (strlen(options->config_file)) {
        if (ini_parse(options->config_file, clax_config_handler, options) < 0) {
            fprintf(stderr, "Error: can't load '%s\n\n", options->config_file);

            usage();
        }
    }

    if (!options->no_ssl) {
        if (!strlen(options->cert_file) || !strlen(options->key_file)) {
            fprintf(stderr, "Error: cert_file and key_file are required\n\n");

            usage();
        }
    }

    if (!strlen(options->root)) {
        fprintf(stderr, "Error: root is required\n\n");

        usage();
    } else {
        DIR *dir = opendir(options->root);
        if (dir) {
            closedir(dir);

            if (chdir(options->root) < 0) {
                fprintf(stderr, "Error: cannot chdir to '%s'\n\n", options->root);

                usage();
            }

            char last = options->root[strlen(options->root) - 1];
            if (last != '/') {
                strcat(options->root, "/");
            }
        } else if (ENOENT == errno) {
            fprintf(stderr, "Error: provided root directory does not exist\n\n");

            usage();
        } else {
            fprintf(stderr, "Error: cannot open provided root directory\n\n");

            usage();
        }
    }

    return;
}
