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
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl_cache.h"

#include "clax.h"
#include "clax_ctx.h"
#include "clax_errors.h"
#include "clax_options.h"
#include "clax_http.h"
#include "clax_log.h"
#include "clax_util.h"
#include "clax_platform.h"

#define DEBUG_LEVEL 0

#define DEV_RANDOM_THRESHOLD        32

opt options;

void clax_exit(int code)
{
    if (options._log_file) {
        clax_log("Closing log file '%s'", options.log_file);
        fclose(options._log_file);

        clax_log("Exit=%d", code);
    }

    exit(code);
}

void clax_abort()
{
    clax_exit(255);
}

void term(int dummy)
{
    clax_abort();
}

int clax_recv(void *ctx,  unsigned char *buf, size_t len)
{
    int ret;
    int fd = fileno(stdin);

    ret = (int)read(fd, buf, len);

#ifdef MVS
    if (isatty(fd)) {
        clax_etoa((char *)buf, len);
    }

#endif

    /*clax_log("recv (%d)=%d from %d", fd, ret, len);*/

    return ret;
}

int clax_send(void *ctx, const unsigned char *buf, size_t len)
{
    int ret;
    int fd = fileno(stdout);

    ret = (int)write(fd, buf, len);

    /*clax_log("send (%d)=%d from %d", fd, ret, len);*/

    return ret;
}

int clax_recv_ssl(void *ctx, unsigned char *buf, size_t len)
{
    int ret;
    mbedtls_ssl_context *ssl = ctx;

    ret = mbedtls_ssl_read(ssl, buf, len);

    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
        return EAGAIN;

    if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY || ret == MBEDTLS_ERR_NET_CONN_RESET) {
        return 0;
    }

    /*clax_log("recv (ssl)=%d from %d", ret, len);*/

    return ret;
}

int clax_send_ssl(void *ctx, const unsigned char *buf, size_t len)
{
    int ret;
    mbedtls_ssl_context *ssl = ctx;

    while ((ret = mbedtls_ssl_write(ssl, buf, len)) <= 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            clax_log("failed\n  ! mbedtls_ssl_write returned %d", ret);
            return ret;
        }
    }

    /*clax_log("send (ssl)=%d from %d", ret, len);*/

    return ret;
}

int dev_random_entropy_poll(void *data, unsigned char *output,
                             size_t len, size_t *olen)
{
    FILE *file;
    size_t ret, left = len;
    unsigned char *p = output;
    ((void) data);

    clax_log("Polling entropy file");

    *olen = 0;

    file = fopen(options.entropy_file, "rb");
    if (file == NULL) {
        clax_log("Entropy failed");
        return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    }

    while (left > 0) {
        /* /dev/random can return much less than requested. If so, try again */
        ret = fread(p, 1, left, file);
        if (ret == 0 && ferror(file)) {
            fclose(file);
            clax_log("Entropy failed");
            return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
        }

        p += ret;
        left -= ret;
        sleep(1);
    }
    fclose(file);
    *olen = len;

    clax_log("Entropy=%d", len);

    return 0;
}

void clax_loop_ssl(clax_ctx_t *clax_ctx)
{
    int ret = 0;
    char pers[] = "clax_server";

#ifdef MVS
    clax_etoa(pers, strlen(pers));
#endif

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt srvcert;
    mbedtls_pk_context pkey;
    mbedtls_ssl_cache_context cache;

    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_ssl_cache_init(&cache);
    mbedtls_x509_crt_init(&srvcert);
    mbedtls_pk_init(&pkey);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold(DEBUG_LEVEL);
#endif

    clax_log("Loading the server cert and key...");

    unsigned char *file = NULL;
    size_t file_len = 0;

    clax_log("Loading '%s'...", options.cert_file);
    file = clax_slurp_alloc(options.cert_file, &file_len);

    if (file == NULL) {
        clax_log("Can't load cert_file '%s': %s", options.cert_file, strerror(errno));
        goto exit;
    }

#ifdef MVS
    clax_etoa((char *)file, file_len);
#endif

    clax_log("Parsing '%s'...", options.cert_file);
    ret = mbedtls_x509_crt_parse(&srvcert, (const unsigned char *)file, file_len);
    free(file);

    if (ret != 0) {
        clax_log("failed\n  !  mbedtls_x509_crt_parse returned %d", ret);
        goto exit;
    }

    clax_log("Loading '%s'...", options.key_file);
    file = clax_slurp_alloc(options.key_file, &file_len);
    if (file == NULL) {
        clax_log("Can't load key_file: %s", options.key_file);
        goto exit;
    }

#ifdef MVS
    clax_etoa((char *)file, file_len);
#endif

    clax_log("Parsing '%s'...", options.key_file);
    ret = mbedtls_pk_parse_key(&pkey, (const unsigned char *)file, file_len, NULL, 0);
    free(file);

    if (ret != 0) {
        clax_log("failed\n  !  mbedtls_pk_parse_key returned %d", ret);
        goto exit;
    }

    clax_log("ok");

    if (options.entropy_file[0]) {
        clax_log("Using '%s' as entropy file...", options.entropy_file);

        if ((ret = mbedtls_entropy_add_source(&entropy, dev_random_entropy_poll,
                                                NULL, DEV_RANDOM_THRESHOLD,
                                                MBEDTLS_ENTROPY_SOURCE_STRONG)) != 0) {
            clax_log("failed\n  ! mbedtls_entropy_add_source returned -0x%04x", -ret);
            goto exit;
        }

        clax_log("ok");
    }

    clax_log("Seeding the random number generator...");

    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                       (const unsigned char *)pers,
                                       strlen(pers))) != 0) {
        clax_log("failed\n  ! mbedtls_ctr_drbg_seed returned %d", ret);
        goto exit;
    }

    clax_log("ok");

    clax_log("Setting up the SSL data....");

    if ((ret = mbedtls_ssl_config_defaults(&conf,
                MBEDTLS_SSL_IS_SERVER,
                MBEDTLS_SSL_TRANSPORT_STREAM,
                MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        clax_log("failed\n  ! mbedtls_ssl_config_defaults returned %d", ret);
        goto exit;
    }

    if (!options.no_ssl_verify) {
        mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    }

    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

    mbedtls_ssl_conf_session_cache(&conf, &cache,
                                    mbedtls_ssl_cache_get,
                                    mbedtls_ssl_cache_set);

    mbedtls_ssl_conf_ca_chain(&conf, srvcert.next, NULL);
    if ((ret = mbedtls_ssl_conf_own_cert(&conf, &srvcert, &pkey)) != 0) {
        clax_log(" failed\n  ! mbedtls_ssl_conf_own_cert returned %d", ret);
        goto exit;
    }

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0) {
        clax_log(" failed\n  ! mbedtls_ssl_setup returned %d", ret);
        goto exit;
    }

    clax_log("ok");

    mbedtls_ssl_session_reset(&ssl);

    mbedtls_ssl_set_bio(&ssl, NULL, clax_send, clax_recv, NULL);

    clax_log("ok");

    clax_log("Performing the SSL/TLS handshake...");

    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            clax_log("failed\n  ! mbedtls_ssl_handshake returned %d", ret);
            goto exit;
        }
    }

    clax_log("ok");

    clax_http_dispatch(clax_ctx, clax_send_ssl, clax_recv_ssl, &ssl);

    clax_log("Closing the connection...");

    while ((ret = mbedtls_ssl_close_notify(&ssl)) < 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
                ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            clax_log("failed\n  ! mbedtls_ssl_close_notify returned %d", ret);
            goto exit;
        }
    }

    clax_log("ok");

    ret = 0;
    goto exit;

exit:
    fflush(stdout);

#ifdef MBEDTLS_ERROR_C
    if (ret != 0) {
        char error_buf[100];
        mbedtls_strerror(ret, error_buf, 100);
#ifdef MVS
        clax_atoe(error_buf, strlen(error_buf));
#endif
        clax_log("Last error was: %d - %s", ret, error_buf);
    }
#endif

    mbedtls_x509_crt_free(&srvcert);
    mbedtls_pk_free(&pkey);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ssl_cache_free(&cache);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
}

int main(int argc, char **argv)
{
    clax_ctx_t clax_ctx;

#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
#endif

    int is_interactive = isatty(fileno(stdin));

    signal(SIGINT, term);
    setbuf(stdout, NULL);

    clax_options_init(&options);
    clax_ctx_init(&clax_ctx);
    clax_ctx.options = &options;

    int ok = clax_parse_options(&options, argc, argv);
    if (ok < 0) {
        const char *error = clax_strerror(ok);
        size_t len = strlen(error);

        if (is_interactive) {
            fprintf(stdout, "Error: %s\n", error);

            if (ok != -1) {
                fprintf(stdout, "\n");
                clax_usage();
            }

            clax_exit(255);
        }
        else {
            char buf[1024];
            char *b = buf;

            snprintf(buf, sizeof(buf),
                "HTTP/1.1 500 System Error\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: %d\r\n"
                "\r\n"
                "%s\n",
                (int)(len + 1),
                error
            );

#ifdef MVS
            b = clax_etoa_alloc(buf, strlen(buf));
#endif

            fprintf(stdout, "%s", b);

#ifdef MVS
            free(b);
#endif
            goto cleanup;
        }
    }

    clax_log("Option: root=%s", options.root);
    clax_log("Option: entropy_file=%s", options.entropy_file);
    clax_log("Option: log_file=%s", options.log_file);
    clax_log("Option: ssl=%d", options.ssl);

    if (!options.ssl) {
        clax_http_dispatch(&clax_ctx, clax_send, clax_recv, NULL);
    } else {
        clax_loop_ssl(&clax_ctx);
    }

cleanup:
    fflush(stdout);
    fclose(stdout);

    clax_ctx_free(&clax_ctx);
    clax_options_free(&options);

    clax_exit(0);
}
