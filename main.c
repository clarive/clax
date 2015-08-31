#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#if defined(_WIN32)
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

#include "clax_http.h"
#include "clax_log.h"

#define HTTP_RESPONSE \
    "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n" \
    "Hello from clax!\r\n"

#define DEBUG_LEVEL 0

#define DEV_RANDOM_THRESHOLD        32

clax_http_request_t request;
clax_http_response_t response;

typedef struct {
    char entropy_file[255];
    char log_file[255];

    char no_ssl;
    char cert_file[255];
    char key_file[255];

    /* private */
    FILE *_log_file;
} opt;

opt options;

typedef int (*recv_cb_t)(void *ctx, unsigned char *buf, size_t len);
typedef int (*send_cb_t)(void *ctx, const unsigned char *buf, size_t len);

void usage()
{
    fprintf(stderr,
            "usage: clax [-n] [-l log_file] [-e entropy_file]\n\n"
            "Options:\n\n"
            "   common\n"
            "   ------\n"
            "   -l <log_file>      path to log file\n"
            "\n"
            "   ssl\n"
            "   ---\n"
            "   -n                 do not use ssl\n"
            "   -e <entropy_file>  path to entropy file\n"
            "   -t <cert_file>     path to cert file (CA included)\n"
            "   -p <key_file>      path to private key file\n"
            "\n"
            );

    exit(255);
}

void _exit(int code)
{
    if (options._log_file) {
        clax_log("Closing log file '%s'", options.log_file);
        fclose(options._log_file);
    }

    clax_log("Exit=%d", code);

    exit(code);
}

void abort()
{
    _exit(255);
}

void term(int dummy)
{
    abort();
}

void clax_parse_options(opt *options, int argc, char **argv)
{
    int c;

    memset(options, 0, sizeof(opt));

#ifdef MVS
    if (argc > 1)
        __argvtoascii_a(argc, (char **)argv);
#endif

    opterr = 0;
    while ((c = getopt(argc, argv, "hnl:e:t:p:")) != -1) {
        switch (c) {
        case 'l':
            strncpy(options->log_file, optarg, sizeof(options->log_file));
            break;
        case 'n':
            options->no_ssl = 1;
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
        case '?':
        case 'h':
        default:
            usage();
        }
    }

    if (!options->no_ssl) {
        if (!strlen(options->cert_file) || !strlen(options->key_file)) {
            fprintf(stderr,
                    "Error: cert_file and key_file are required\n\n"
                   );

            usage();
        }
    }

    return;
}

int clax_recv(void *ctx,  unsigned char *buf, size_t len)
{
    int ret;
    int fd = fileno(stdin);

    ret = (int)read(fd, buf, len);

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

    *olen = 0;

    file = fopen(options.entropy_file, "rb");
    if (file == NULL)
        return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;

    while (left > 0) {
        /* /dev/random can return much less than requested. If so, try again */
        ret = fread(p, 1, left, file);
        if (ret == 0 && ferror(file)) {
            fclose(file);
            return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
        }

        p += ret;
        left -= ret;
        sleep(1);
    }
    fclose(file);
    *olen = len;

    return 0;
}

#define TRY if ((
#define GOTO ) < 0) {goto error;}

int clax_chunked(char *buf, size_t len, va_list a_list_)
{
    char obuf[255];
    int olen;
    send_cb_t send_cb;
    void *ctx;
    va_list a_list;

    va_copy(a_list, a_list_);

    send_cb = va_arg(a_list, void *);

    ctx = va_arg(a_list, void *);

    if (len) {
        olen = sprintf(obuf, "%x\r\n", len);
        TRY send_cb(ctx, obuf, olen) GOTO
        TRY send_cb(ctx, buf, len) GOTO
        TRY send_cb(ctx, "\r\n", 2) GOTO
    }
    else {
        TRY send_cb(ctx, "0\r\n\r\n", 5) GOTO
    }

    return 0;

error:
    return -1;
}

int clax_loop_read_parse(void *ctx, recv_cb_t recv_cb, clax_http_request_t *request)
{
    int ret = 0;
    int len = 0;
    unsigned char buf[1024];

    do {
        memset(buf, 0, sizeof(buf));
        ret = recv_cb(ctx, buf, sizeof(buf));

        if (ret == EAGAIN) {
            continue;
        }

        if (ret < 0) {
            clax_log("Reading failed!");
            return -1;
        }
        else if (ret == 0) {
            clax_log("Connection closed");
            return -1;
        }

        ret = clax_http_parse(request, buf, ret);

        if (ret < 0) {
            clax_log("Parsing failed!");
            return -1;
        }
        else if (ret == 1) {
            break;
        } else if (ret == 0) {
            clax_log("Waiting for more data...");
        }
    } while (1);

    return 0;
}

int clax_loop_write_response(void *ctx, send_cb_t send_cb, clax_http_response_t *response)
{
    int ret = 0;
    int len = 0;
    unsigned char buf[1024];

    const char *status_message = clax_http_status_message(response->status_code);

    TRY send_cb(ctx, "HTTP/1.1 ", 9) GOTO
    TRY send_cb(ctx, status_message, strlen(status_message)) GOTO
    TRY send_cb(ctx, "\r\n", 2) GOTO

    if (response->content_type) {
        TRY send_cb(ctx, "Content-Type: ", 14) GOTO;
        TRY send_cb(ctx, response->content_type, strlen(response->content_type)) GOTO;
        TRY send_cb(ctx, "\r\n", 2) GOTO;
    }

    if (response->transfer_encoding) {
        TRY send_cb(ctx, "Transfer-Encoding: ", 19) GOTO;
        TRY send_cb(ctx, response->transfer_encoding, strlen(response->transfer_encoding)) GOTO;
        TRY send_cb(ctx, "\r\n", 2) GOTO;
    }

    if (response->body_len) {
        char buf[255];

        TRY send_cb(ctx, "Content-Length: ", 16) GOTO;
        sprintf(buf, "%d\r\n\r\n", response->body_len);
        TRY send_cb(ctx, buf, strlen(buf)) GOTO;

        TRY send_cb(ctx, response->body, response->body_len) GOTO;
    } else if (response->body_cb) {
        TRY send_cb(ctx, "\r\n", 2) GOTO;

        /* TODO: error handling */
        response->body_cb(response->body_cb_ctx, clax_chunked, send_cb, ctx);
    }

    return 0;

error:
    return -1;
}

int clax_loop(void *ctx, send_cb_t send_cb, recv_cb_t recv_cb) {
    clax_http_init();
    memset(&request, 0, sizeof(clax_http_request_t));
    memset(&response, 0, sizeof(clax_http_response_t));

    clax_log("Reading & parsing request...");
    TRY clax_loop_read_parse(ctx, recv_cb, &request) GOTO;
    clax_log("ok");

    clax_log("Dispatching request...");
    clax_dispatch(&request, &response);
    clax_log("ok");

    clax_log("Writing response...");
    TRY clax_loop_write_response(ctx, send_cb, &response) GOTO;
    clax_log("ok");

    return 1;

error:
    clax_log("failed!");

    return -1;
}

void clax_loop_ssl()
{
    int ret, len;
    unsigned char buf[1024];
    const char *pers = "clax_server";

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

    /*
     * 1. Load the certificates and private RSA key
     */
    clax_log("Loading the server cert. and key...");

    ret = mbedtls_x509_crt_parse_file(&srvcert, options.cert_file);
    if (ret != 0) {
        clax_log("failed\n  !  mbedtls_x509_crt_parse returned %d", ret);
        goto exit;
    }

    /*ret = mbedtls_x509_crt_parse_file(&srvcert, (const unsigned char *) mbedtls_test_cas_pem,*/
                                  /*mbedtls_test_cas_pem_len);*/
    /*if (ret != 0) {*/
        /*clax_log("failed\n  !  mbedtls_x509_crt_parse returned %d", ret);*/
        /*goto exit;*/
    /*}*/

    ret =  mbedtls_pk_parse_keyfile(&pkey, (const unsigned char *) options.key_file, NULL);
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
                                       (const unsigned char *) pers,
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

    /*
     * 5. Handshake
     */
    clax_log("Performing the SSL/TLS handshake...");

    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            clax_log("failed\n  ! mbedtls_ssl_handshake returned %d", ret);
            goto exit;
        }
    }

    clax_log("ok");

    clax_loop(&ssl, clax_send_ssl, clax_recv_ssl);

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
    memset(&options, 0, sizeof(opt));

    signal(SIGINT, term);

    clax_parse_options(&options, argc, argv);

    if (options.log_file[0]) {
        options._log_file = fopen(options.log_file, "w");
        if (options._log_file == NULL) {
            abort();
        }

        dup2(fileno(options._log_file), STDERR_FILENO);
    }

    setbuf(stdout, NULL);

    clax_log("Option: no_ssl=%d", options.no_ssl);
    clax_log("Option: entropy_file=%s", options.entropy_file);
    clax_log("Option: log_file=%s", options.log_file);

    if (options.no_ssl) {
        clax_loop(NULL, clax_send, clax_recv);
    } else {
        clax_loop_ssl();
    }

    fflush(stdout);
    fclose(stdout);

    _exit(0);
}
