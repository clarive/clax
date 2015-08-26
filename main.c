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
#include "mbedtls/error.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl_cache.h"

#include "clax_http.h"

#define HTTP_RESPONSE \
    "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n" \
    "Hello from clax!\r\n"

#define DEBUG_LEVEL 0

#define DEV_RANDOM_THRESHOLD        32

clax_http_request_t request;
clax_http_response_t response;

void clax_log_(const char *file, int line, const char *func_, char *fmt, ...)
{
    int size;
    char *cp;
    va_list args;
    char func[1024];

    strcpy(func, func_);

#ifdef MVS
    char func_a[1024];
    __toascii_a((char * )func_a, func);
    strcpy(func, func_a);
#endif

    va_start(args, fmt);
    size = vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);

    va_start(args, fmt);
    cp = (char *)calloc(size, sizeof(char));
    if (cp != NULL && vsnprintf(cp, size, fmt, args) > 0) {
        fprintf(stderr, "%s:%d:%s(): %s\n", file, line, func, cp);

    }
    va_end(args);
    free(cp);
}

#define clax_log(...) clax_log_(__FILE__, __LINE__, __func__, __VA_ARGS__)

typedef struct {
    char entropy_file[255];
    char log_file[255];
    char no_ssl;

    FILE *_log_file;
} opt;

opt options;

void usage()
{
    fprintf(stderr,
            "usage: clax [-n] [-l log_file] [-e entropy_file]\n\n"
            "Options:\n\n"
            "   -n                 do not use ssl\n"
            "   -e <entropy_file>  path to entropy file\n"
            "   -l <log_file>      path to log file\n"
            "\n"
            );

    abort();
}

void abort()
{
    if (options._log_file) {
        fclose(options._log_file);
    }

    exit(255);
}

void clax_parse_options(opt *options, int argc, char **argv)
{
    int c;

#ifdef MVS
    if (argc > 1)
        __argvtoascii_a(argc, (char **)argv);
#endif

    opterr = 0;
    while ((c = getopt (argc, argv, "hnl:e:")) != -1) {
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
        case '?':
        case 'h':
            usage();
            break;
        default:
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

    clax_log("recv (%d)=%d from %d", fd, ret, len);

    return ret;
}

int clax_send( void *ctx, const unsigned char *buf, size_t len )
{
    int ret;
    int fd = fileno(stdout);

    ret = (int)write(fd, buf, len);

    clax_log("send (%d)=%d from %d", fd, ret, len);

    return ret;
}

int clax_recv_ssl(void *ctx, unsigned char *buf, size_t len)
{
    int ret;
    mbedtls_ssl_context *ssl = ctx;

    ret = mbedtls_ssl_read(ssl, buf, len);

    if ( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
        return EAGAIN;

    clax_log("recv (ssl)=%d from %d", ret, len);

    return ret;
}

int clax_send_ssl( void *ctx, const unsigned char *buf, size_t len )
{
    int ret;
    mbedtls_ssl_context *ssl = ctx;

    while ((ret = mbedtls_ssl_write(ssl, buf, len)) <= 0 ) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            clax_log("failed\n  ! mbedtls_ssl_write returned %d", ret );
            return ret;
        }
    }

    clax_log("send (ssl)=%d from %d", ret, len);

    return ret;
}

int dev_random_entropy_poll( void *data, unsigned char *output,
                             size_t len, size_t *olen )
{
    FILE *file;
    size_t ret, left = len;
    unsigned char *p = output;
    ((void) data);

    *olen = 0;

    file = fopen( options.entropy_file, "rb" );
    if ( file == NULL )
        return( MBEDTLS_ERR_ENTROPY_SOURCE_FAILED );

    while ( left > 0 ) {
        /* /dev/random can return much less than requested. If so, try again */
        ret = fread( p, 1, left, file );
        if ( ret == 0 && ferror( file ) ) {
            fclose( file );
            return( MBEDTLS_ERR_ENTROPY_SOURCE_FAILED );
        }

        p += ret;
        left -= ret;
        sleep( 1 );
    }
    fclose( file );
    *olen = len;

    return( 0 );
}

int clax_loop(void *ctx, int (*send_cb)(void *ctx, const unsigned char *buf, size_t len),
        int (*recv_cb)(void *ctx, unsigned char *buf, size_t len)
        ) {
    int ret = 0;
    int len = 0;
    unsigned char buf[1024];

    clax_http_init();

    clax_log("Reading & parsing request...");
    do {
        memset(buf, 0, sizeof(buf));
        ret = recv_cb(ctx, buf, sizeof(buf));

        clax_log("ret=%d", ret);

        if (ret == EAGAIN) {
            clax_log("EAGAIN");
            continue;
        }

        if (ret < 0) {
            clax_log("Reading failed!");
            return -1;
        }

        ret = clax_http_parse(&request, buf, ret);

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

    clax_log("ok");

    clax_log("Dispatching response...");

    clax_dispatch(&request, &response);

    clax_log("Writing response...");

    if (response.status_code == 200) {
        send_cb(ctx, "HTTP/1.1 200 OK\r\n", 6 + 2);
    }
    else if (response.status_code == 404) {
        send_cb(ctx, "HTTP/1.1 404 Not Found\r\n", 13 + 2);
    }

    send_cb(ctx, "Content-Type: application/json\r\n", 30 + 2);
    send_cb(ctx, "\r\n", 2);
    send_cb(ctx, response.body, response.body_len);

    if (ret < 0) {
        clax_log("failed!");
        return -1;
    }

    clax_log("ok");

    return 1;
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

    mbedtls_ssl_init( &ssl );
    mbedtls_ssl_config_init( &conf );
    mbedtls_ssl_cache_init( &cache );
    mbedtls_x509_crt_init( &srvcert );
    mbedtls_pk_init( &pkey );
    mbedtls_entropy_init( &entropy );
    mbedtls_ctr_drbg_init( &ctr_drbg );

#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold( DEBUG_LEVEL );
#endif

    /*
     * 1. Load the certificates and private RSA key
     */
    clax_log("Loading the server cert. and key..." );

    /*
     * This demonstration program uses embedded test certificates.
     * Instead, you may want to use mbedtls_x509_crt_parse_file() to read the
     * server and CA certificates, as well as mbedtls_pk_parse_keyfile().
     */
    ret = mbedtls_x509_crt_parse( &srvcert, (const unsigned char *) mbedtls_test_srv_crt,
                                  mbedtls_test_srv_crt_len );
    if ( ret != 0 ) {
        clax_log("failed\n  !  mbedtls_x509_crt_parse returned %d", ret);
        goto exit;
    }

    ret = mbedtls_x509_crt_parse( &srvcert, (const unsigned char *) mbedtls_test_cas_pem,
                                  mbedtls_test_cas_pem_len );
    if ( ret != 0 ) {
        clax_log("failed\n  !  mbedtls_x509_crt_parse returned %d", ret);
        goto exit;
    }

    ret =  mbedtls_pk_parse_key( &pkey, (const unsigned char *) mbedtls_test_srv_key,
                                 mbedtls_test_srv_key_len, NULL, 0 );
    if ( ret != 0 ) {
        clax_log("failed\n  !  mbedtls_pk_parse_key returned %d", ret);
        goto exit;
    }

    clax_log("ok");

    if (options.entropy_file[0]) {
        clax_log("Using '%s' as entropy file...", options.entropy_file );

        if ( ( ret = mbedtls_entropy_add_source( &entropy, dev_random_entropy_poll,
                                                NULL, DEV_RANDOM_THRESHOLD,
                                                MBEDTLS_ENTROPY_SOURCE_STRONG ) ) != 0 ) {
            clax_log("failed\n  ! mbedtls_entropy_add_source returned -0x%04x", -ret);
            goto exit;
        }

        clax_log("ok");
    }

    clax_log("Seeding the random number generator...");

    if ( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                                       (const unsigned char *) pers,
                                       strlen( pers ) ) ) != 0 ) {
        clax_log("failed\n  ! mbedtls_ctr_drbg_seed returned %d", ret);
        goto exit;
    }

    clax_log("ok");

    /*
     * 4. Setup stuff
     */
    clax_log("Setting up the SSL data....");

    if ( ( ret = mbedtls_ssl_config_defaults( &conf,
                MBEDTLS_SSL_IS_SERVER,
                MBEDTLS_SSL_TRANSPORT_STREAM,
                MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 ) {
        clax_log("failed\n  ! mbedtls_ssl_config_defaults returned %d", ret);
        goto exit;
    }

    mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg );

    mbedtls_ssl_conf_session_cache( &conf, &cache,
                                    mbedtls_ssl_cache_get,
                                    mbedtls_ssl_cache_set );

    mbedtls_ssl_conf_ca_chain( &conf, srvcert.next, NULL );
    if ( ( ret = mbedtls_ssl_conf_own_cert( &conf, &srvcert, &pkey ) ) != 0 ) {
        clax_log(" failed\n  ! mbedtls_ssl_conf_own_cert returned %d", ret );
        goto exit;
    }

    if ( ( ret = mbedtls_ssl_setup( &ssl, &conf ) ) != 0 ) {
        clax_log(" failed\n  ! mbedtls_ssl_setup returned %d", ret );
        goto exit;
    }

    clax_log("ok" );

    mbedtls_ssl_session_reset( &ssl );

    mbedtls_ssl_set_bio( &ssl, NULL, clax_send, clax_recv, NULL );

    clax_log("ok" );

    /*
     * 5. Handshake
     */
    clax_log("Performing the SSL/TLS handshake..." );

    while ( ( ret = mbedtls_ssl_handshake( &ssl ) ) != 0 ) {
        if ( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE ) {
            clax_log("failed\n  ! mbedtls_ssl_handshake returned %d", ret );
            goto exit;
        }
    }

    clax_log("ok" );

    clax_loop(&ssl, clax_send_ssl, clax_recv_ssl);

    clax_log("Closing the connection..." );

    while ( ( ret = mbedtls_ssl_close_notify( &ssl ) ) < 0 ) {
        if ( ret != MBEDTLS_ERR_SSL_WANT_READ &&
                ret != MBEDTLS_ERR_SSL_WANT_WRITE ) {
            clax_log("failed\n  ! mbedtls_ssl_close_notify returned %d", ret );
            goto exit;
        }
    }

    clax_log("ok" );

    ret = 0;
    goto exit;

exit:
    fflush(stdout);

#ifdef MBEDTLS_ERROR_C
    if ( ret != 0 ) {
        char error_buf[100];
        mbedtls_strerror( ret, error_buf, 100 );
        clax_log("Last error was: %d - %s\n\n", ret, error_buf );
    }
#endif

    mbedtls_x509_crt_free( &srvcert );
    mbedtls_pk_free( &pkey );
    mbedtls_ssl_free( &ssl );
    mbedtls_ssl_config_free( &conf );
    mbedtls_ssl_cache_free( &cache );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
}

void _exit(int dummy)
{
    abort();
}

int main(int argc, char **argv)
{
    memset(&options, 0, sizeof(opt));

    signal(SIGINT, _exit);

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
}
