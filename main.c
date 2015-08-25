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

#define HTTP_RESPONSE \
    "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n" \
    "<h2>mbed TLS Test Server</h2>\r\n" \
    "<p>Successful connection using: %s</p>\r\n"

#define DEBUG_LEVEL 0

#define DEV_RANDOM_THRESHOLD        32

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
    int fd = 0;

    ret = (int)read(fd, buf, len);

    clax_log("recv (%d)=%d from %d", fd, ret, len);

    return ret;
}

int clax_send( void *ctx, const unsigned char *buf, size_t len )
{
    int ret;
    int fd = 1;

    ret = (int)write(fd, buf, len);

    clax_log("send (%d)=%d from %d", fd, ret, len);

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

void clax_loop() {
    int ret = 0, len = 0;
    unsigned char buf[1024];

    memset(buf, 0, sizeof(buf));

    clax_log("Reading request...");
    do {
        ret = clax_recv(NULL, buf, sizeof(buf) - len - 1);

        if (ret < 0) {
            clax_log("failed!");
            abort();
        } else if (ret == 0) {
            break;
        } else if (len >= sizeof(buf)) {
            break;
        }

        len += ret;
    } while (1);

    clax_log("ok");

    len = sprintf((char *)buf, HTTP_RESPONSE);

    clax_log("Writing response...");

    ret = clax_send(NULL, buf, len);

    if (ret < 0) {
        clax_log("failed!");
        abort();
    }

    clax_log("ok");
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

    /*
     * 6. Read the HTTP Request
     */
    clax_log("  < Read from client:" );

    do {
        len = sizeof( buf ) - 1;
        memset( buf, 0, sizeof( buf ) );
        ret = mbedtls_ssl_read( &ssl, buf, len );

        if ( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
            continue;

        if ( ret <= 0 ) {
            switch( ret ) {
            case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
                clax_log("connection was closed gracefully\n" );
                break;

            default:
                clax_log("mbedtls_ssl_read returned -0x%x\n", -ret );
                break;
            }

            break;
        }

        len = ret;
        clax_log("%d bytes read\n\n%s", len, (char *) buf );

        if ( ret > 0 )
            break;
    } while ( 1 );

    /*
     * 7. Write the 200 Response
     */
    clax_log("  > Write to client:" );

    len = sprintf( (char *) buf, HTTP_RESPONSE,
                   mbedtls_ssl_get_ciphersuite( &ssl ) );

    while ( ( ret = mbedtls_ssl_write( &ssl, buf, len ) ) <= 0 ) {
        if ( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE ) {
            clax_log("failed\n  ! mbedtls_ssl_write returned %d", ret );
            goto exit;
        }
    }

    len = ret;
    clax_log("%d bytes written\n\n%s\n", len, (char *) buf );

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

    clax_log("Option: no_ssl=%d", options.no_ssl);
    clax_log("Option: entropy_file=%s", options.entropy_file);
    clax_log("Option: log_file=%s", options.log_file);

    if (options.no_ssl) {
        clax_loop();
    } else {
        clax_loop_ssl();
    }
}
