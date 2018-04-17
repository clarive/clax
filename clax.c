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

#include "contrib/mbedtls/mbedtls/entropy.h"
#include "contrib/mbedtls/mbedtls/ctr_drbg.h"
#include "contrib/mbedtls/mbedtls/certs.h"
#include "contrib/mbedtls/mbedtls/x509.h"
#include "contrib/mbedtls/mbedtls/ssl.h"
#include "contrib/mbedtls/mbedtls/net.h"
#include "contrib/mbedtls/mbedtls/error.h"
#include "contrib/mbedtls/mbedtls/debug.h"
#include "contrib/mbedtls/mbedtls/ssl_cache.h"

#include "contrib/libuv/include/uv.h"

#include "clax_ctx.h"
#include "clax_errors.h"
#include "clax_options.h"
#include "clax_http.h"
#include "clax_http_parser.h"
#include "clax_log.h"
#include "clax_util.h"
#include "clax_platform.h"
#include "clax.h"

#define DEBUG_LEVEL 0
#define DEV_RANDOM_THRESHOLD        32

opt options;

int clax_argc;
char **clax_argv;

mbedtls_entropy_context ssl_entropy;
mbedtls_ctr_drbg_context ssl_ctr_drbg;
mbedtls_ssl_config ssl_conf;
mbedtls_x509_crt ssl_srvcert;
mbedtls_pk_context ssl_pkey;
mbedtls_ssl_cache_context ssl_cache;

void clax_ssl_free();
int clax_conn_write_cb(clax_ctx_t *clax_ctx, const unsigned char *buf, size_t size);

void clax_cleanup()
{
    if (options.ssl) {
        clax_ssl_free();
    }

    if (options._log_file) {
        clax_log("Closing log file '%s'", options.log_file);
        fclose(options._log_file);
    }

    fflush(stdout);
    fclose(stdout);

    clax_options_free(&options);

    uv_stop(uv_default_loop());
    uv_loop_close(uv_default_loop());
}

void clax_exit(int code)
{
    clax_cleanup();

    clax_log("Exit=%d", code);

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

int clax_recv(void *ctx, unsigned char *buf, size_t len)
{
    clax_ctx_t *clax_ctx = ctx;

    int have = MIN(len, clax_ctx->ssl.len);

    memcpy(buf, clax_ctx->ssl.buf, clax_ctx->ssl.len);
    clax_ctx->ssl.buf += len;
    clax_ctx->ssl.len -= len;

    if (have == 0) {
        return MBEDTLS_ERR_SSL_WANT_READ;
    }

    return have;
}

int clax_send(void *ctx, const unsigned char *buf, size_t len)
{
    clax_ctx_t *clax_ctx = ctx;

    int r = clax_conn_write_cb(clax_ctx, buf, len);

    if (r < 0) {
        clax_log("Error: send failed: %s", uv_strerror(r));
        return -1;
    }

    return len;
}

int clax_recv_ssl(void *ctx, unsigned char *buf, size_t len)
{
    int ret;

    clax_ctx_t *clax_ctx = ctx;
    mbedtls_ssl_context *ssl = (mbedtls_ssl_context *)&clax_ctx->ssl;

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
    clax_ctx_t *clax_ctx = ctx;
    mbedtls_ssl_context *ssl = (mbedtls_ssl_context *)&clax_ctx->ssl;

    while ((ret = mbedtls_ssl_write(ssl, buf, len)) <= 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            clax_log("failed\n  ! mbedtls_ssl_write returned %d", ret);
            return -1;
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

int clax_ssl_init()
{
    int ret = 0;
    char pers[] = "clax_server";

#ifdef MVS
    clax_etoa(pers, strlen(pers));
#endif

    mbedtls_ssl_config_init(&ssl_conf);
    mbedtls_ssl_cache_init(&ssl_cache);
    mbedtls_x509_crt_init(&ssl_srvcert);
    mbedtls_pk_init(&ssl_pkey);
    mbedtls_entropy_init(&ssl_entropy);
    mbedtls_ctr_drbg_init(&ssl_ctr_drbg);

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
    ret = mbedtls_x509_crt_parse(&ssl_srvcert, (const unsigned char *)file, file_len);
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
    ret = mbedtls_pk_parse_key(&ssl_pkey, (const unsigned char *)file, file_len, NULL, 0);
    free(file);

    if (ret != 0) {
        clax_log("failed\n  !  mbedtls_pk_parse_key returned %d", ret);
        goto exit;
    }

    if (options.entropy_file[0]) {
        clax_log("Using '%s' as entropy file...", options.entropy_file);

        if ((ret = mbedtls_entropy_add_source(&ssl_entropy, dev_random_entropy_poll,
                                                NULL, DEV_RANDOM_THRESHOLD,
                                                MBEDTLS_ENTROPY_SOURCE_STRONG)) != 0) {
            clax_log("failed\n  ! mbedtls_entropy_add_source returned -0x%04x", -ret);
            goto exit;
        }
    }

    clax_log("Seeding the random number generator...");

    if ((ret = mbedtls_ctr_drbg_seed(&ssl_ctr_drbg, mbedtls_entropy_func, &ssl_entropy,
                                       (const unsigned char *)pers,
                                       strlen(pers))) != 0) {
        clax_log("failed\n  ! mbedtls_ctr_drbg_seed returned %d", ret);
        goto exit;
    }

    clax_log("Setting up the SSL data....");

    if ((ret = mbedtls_ssl_config_defaults(&ssl_conf,
                MBEDTLS_SSL_IS_SERVER,
                MBEDTLS_SSL_TRANSPORT_STREAM,
                MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        clax_log("failed\n  ! mbedtls_ssl_config_defaults returned %d", ret);
        goto exit;
    }

    if (!options.no_ssl_verify) {
        mbedtls_ssl_conf_authmode(&ssl_conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    }

    mbedtls_ssl_conf_rng(&ssl_conf, mbedtls_ctr_drbg_random, &ssl_ctr_drbg);

    mbedtls_ssl_conf_session_cache(&ssl_conf, &ssl_cache,
                                    mbedtls_ssl_cache_get,
                                    mbedtls_ssl_cache_set);

    mbedtls_ssl_conf_ca_chain(&ssl_conf, ssl_srvcert.next, NULL);
    if ((ret = mbedtls_ssl_conf_own_cert(&ssl_conf, &ssl_srvcert, &ssl_pkey)) != 0) {
        clax_log(" failed\n  ! mbedtls_ssl_conf_own_cert returned %d", ret);
        goto exit;
    }

    return 0;

exit:
    return -1;
}

void clax_ssl_free()
{
    clax_log("Freeing ssl");

    mbedtls_x509_crt_free(&ssl_srvcert);
    mbedtls_pk_free(&ssl_pkey);
    mbedtls_ssl_config_free(&ssl_conf);
    mbedtls_ssl_cache_free(&ssl_cache);
    mbedtls_ctr_drbg_free(&ssl_ctr_drbg);
    mbedtls_entropy_free(&ssl_entropy);
}

int clax_ssl_init_ctx(clax_ctx_t *clax_ctx)
{
    clax_log("Initializing ssl ctx");

    mbedtls_ssl_context *ssl = (mbedtls_ssl_context *)&clax_ctx->ssl;

    if (mbedtls_ssl_setup(ssl, &ssl_conf) != 0) {
        clax_log("Error: ssl ctx setup failed");
        return -1;
    }

    mbedtls_ssl_session_reset(ssl);

    mbedtls_ssl_set_bio(ssl, clax_ctx, clax_send, clax_recv, NULL);

    return 0;
}

void clax_ssl_free_ctx(clax_ctx_t *clax_ctx)
{
    clax_log("Freeing ssl ctx");

    mbedtls_ssl_context *ssl = (mbedtls_ssl_context *)&clax_ctx->ssl;

    mbedtls_ssl_free(ssl);
}

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
    clax_ctx_t *clax_ctx;
} write_req_t;

void free_write_req(uv_write_t *req)
{
    write_req_t *wr = (write_req_t *)req;
    free(wr->buf.base);
    free(wr);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
}

void clax_conn_close_cb(uv_handle_t *handle)
{
    clax_log("Freeing connection handle");

    if (handle)
        free(handle);
}

static void clax_conn_shutdown_cb(uv_shutdown_t* req, int status)
{
    clax_log("Closing handle");

    uv_close((uv_handle_t *)req->handle, clax_conn_close_cb);
    free(req);
}

int clax_conn_done_cb(clax_ctx_t *clax_ctx)
{
    clax_log("Closing connection");

    uv_handle_t *rh = clax_ctx->rh;
    uv_handle_t *wh = clax_ctx->wh;

    if (options.ssl) {
        /*mbedtls_ssl_context *ssl = (mbedtls_ssl_context *)&clax_ctx->ssl;*/
        /*int ret;*/
        /*while ((ret = mbedtls_ssl_close_notify(ssl)) < 0) {*/
            /*if (ret != MBEDTLS_ERR_SSL_WANT_READ &&*/
                    /*ret != MBEDTLS_ERR_SSL_WANT_WRITE) {*/
                /*clax_log("failed\n  ! mbedtls_ssl_close_notify returned %d", ret);*/
            /*}*/
        /*}*/
    }

    if (wh && rh != wh) {
        uv_read_stop((uv_stream_t *)wh);

        clax_log("Closing write handle");

        uv_close(wh, clax_conn_close_cb);
    }

    if (rh) {
        uv_read_stop((uv_stream_t *)rh);

        clax_log("Shutting down io handle");

        /*uv_shutdown_t *sreq = malloc(sizeof *sreq);*/
        /*uv_shutdown(sreq, (uv_stream_t *)rh, clax_conn_shutdown_cb);*/
        uv_close((uv_handle_t *)rh, clax_conn_close_cb);
    }

    clax_ctx->rh = NULL;
    clax_ctx->wh = NULL;

    if (options.ssl)
        clax_ssl_free_ctx(clax_ctx);

    clax_ctx_free(clax_ctx);

    return 0;
}

void clax_conn_write_finished_cb(uv_write_t *req, int status)
{
    write_req_t *write_req = (write_req_t *)req;
    clax_ctx_t *clax_ctx = write_req->clax_ctx;

    if (write_req->buf.base) {
        clax_ctx->response.to_write -= write_req->buf.len;
    }

    if (status) {
        clax_log("Error: write error: %s %s", uv_err_name(status), uv_strerror(status));
    }

    if (clax_ctx->response.finalized) {
        if (clax_ctx->response.to_write <= 0) {
            clax_conn_done_cb(clax_ctx);
        }
    }

    free_write_req(req);
}

int clax_conn_write_cb(clax_ctx_t *clax_ctx, const unsigned char *buf, size_t size) {
    write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));

    if (req) {
        if (buf && size) {
            req->clax_ctx = clax_ctx;

            req->buf.base = malloc(size);
            req->buf.len = size;
            memcpy(req->buf.base, buf, size);

            clax_ctx->response.to_write += size;

            int r = uv_write((uv_write_t *)req, (uv_stream_t *)clax_ctx->wh, &req->buf, 1, clax_conn_write_finished_cb);

            if (r) {
                free(req->buf.base);
                free(req);

                clax_log("Error: write error");
                clax_conn_done_cb(clax_ctx);
            }
        }
        else {
            free(req);
        }
    }

    if (clax_ctx->response.finalized && clax_ctx->response.to_write == 0) {
        clax_conn_done_cb(clax_ctx);
    }

    return 0;
}

void clax_conn_uv_read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    clax_ctx_t *clax_ctx = (clax_ctx_t *)client->data;

    char *base = buf->base;

    if (nread > 0) {
        if (options.ssl) {
            mbedtls_ssl_context *ssl = (mbedtls_ssl_context *)&clax_ctx->ssl;

            clax_ctx->ssl.buf = buf->base;
            clax_ctx->ssl.len = nread;

            if (!clax_ctx->ssl_handshake_done) {
                clax_log("Performing the SSL/TLS handshake...");

                int r = mbedtls_ssl_handshake(ssl);

                if (r == 0) {
                    clax_log("SSL/TLS handshake done");

                    clax_ctx->ssl_handshake_done++;
                }
                else if (r == MBEDTLS_ERR_SSL_WANT_READ || r == MBEDTLS_ERR_SSL_WANT_WRITE) {
                    return;
                }
                else {
                    clax_log("SSL/TLS handshake failed");

                    clax_conn_done_cb(clax_ctx);

                    return;
                }
            }

            if (!clax_ctx->ssl_handshake_done) {
                return;
            }

            uv_buf_t decoded_buf;
            decoded_buf.base = (char *)malloc(1024);
            decoded_buf.len = 1024;

            int ret = mbedtls_ssl_read(ssl, (unsigned char *)decoded_buf.base, decoded_buf.len);

            if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
                return;

            if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY || ret == MBEDTLS_ERR_NET_CONN_RESET) {
                clax_conn_done_cb(clax_ctx);
                return;
            }

            nread = ret;
            base = decoded_buf.base;
        }

        int ok = clax_http_dispatch(clax_ctx, base, nread);

        if (ok < 0) {
            clax_conn_done_cb(clax_ctx);
        }
    }
    else if (nread < 0) {
        if (nread != UV_EOF)
            clax_log("Read error %s\n", uv_err_name(nread));

        clax_conn_done_cb(clax_ctx);
    }

    if (buf && buf->base)
        free(buf->base);
}

void clax_conn_uv_new_cb(uv_stream_t *server, int status) {
    if (status < 0) {
        clax_log("New connection error %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(uv_default_loop(), client);

    clax_ctx_t *clax_ctx = clax_ctx_alloc();
    clax_ctx_init(clax_ctx, &options);

    if (options.ssl)
        clax_ssl_init_ctx(clax_ctx);

    clax_ctx->rh = client;
    clax_ctx->wh = client;
    if (options.ssl) {
        clax_ctx->send_cb = clax_send_ssl;
    }
    else {
        clax_ctx->send_cb = clax_conn_write_cb;
    }

    client->data = clax_ctx;

    int r = uv_accept(server, (uv_stream_t *)client);
    if (r == 0) {
        clax_log("New tcp connection");

        uv_read_start((uv_stream_t *)client, alloc_buffer, clax_conn_uv_read_cb);
    }
    else {
        clax_log("Error: accept failed: %s", uv_strerror(r));

        if (options.ssl)
            clax_ssl_free_ctx(clax_ctx);

        clax_ctx_free(clax_ctx);

        uv_close((uv_handle_t *)client, clax_conn_close_cb);
    }
}

int clax_service_mode()
{
#ifdef _WIN32

    HANDLE hProcessToken = NULL;
    DWORD groupLength = 50;

    PTOKEN_GROUPS groupInfo = (PTOKEN_GROUPS)LocalAlloc(0, groupLength);

    SID_IDENTIFIER_AUTHORITY siaNt = SECURITY_NT_AUTHORITY;
    PSID InteractiveSid = NULL;
    PSID ServiceSid = NULL;
    DWORD i;

    BOOL exe = TRUE;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hProcessToken))
        goto ret;

    if (groupInfo == NULL)
        goto ret;

    if (!GetTokenInformation(hProcessToken, TokenGroups, groupInfo,
                groupLength, &groupLength))
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            goto ret;

        LocalFree(groupInfo);

        groupInfo = (PTOKEN_GROUPS)LocalAlloc(0, groupLength);

        if (groupInfo == NULL)
            goto ret;

        if (!GetTokenInformation(hProcessToken, TokenGroups, groupInfo,
                    groupLength, &groupLength))
        {
            goto ret;
        }
    }

    if (!AllocateAndInitializeSid(&siaNt, 1, SECURITY_INTERACTIVE_RID, 0, 0, 0, 0, 0, 0, 0, &InteractiveSid)) {
        goto ret;
    }

    if (!AllocateAndInitializeSid(&siaNt, 1, SECURITY_SERVICE_RID, 0, 0, 0, 0, 0, 0, 0, &ServiceSid)) {
        goto ret;
    }

    for (i = 0; i < groupInfo->GroupCount; i++) {
        SID_AND_ATTRIBUTES sanda = groupInfo->Groups[i];
        PSID Sid = sanda.Sid;

        if (EqualSid(Sid, InteractiveSid)) {
            goto ret;
        }
        else if (EqualSid(Sid, ServiceSid)) {
            exe = FALSE;
            goto ret;
        }
    }

    exe = FALSE;

ret:

    if (InteractiveSid)
        FreeSid(InteractiveSid);

    if (ServiceSid)
        FreeSid(ServiceSid);

    if (groupInfo)
        LocalFree(groupInfo);

    if (hProcessToken)
        CloseHandle(hProcessToken);

    return !exe;

#else

    return 0;

#endif
}

int clax_main(int argc, char **argv)
{
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
#endif

    int is_interactive = isatty(fileno(stdin));

    signal(SIGINT, term);
    setbuf(stderr, NULL);
    setbuf(stdout, NULL);

    clax_options_init(&options);

    int ok = clax_parse_options(&options, argc, argv);
    if (ok < 0) {
        const char *error = clax_strerror(ok);
        size_t len = strlen(error);

        clax_log("Options parsing error: %s", error);

        if (is_interactive) {
            fprintf(stdout, "Error: %s\n", error);

            if (ok != -1) {
                fprintf(stdout, "\n");
                clax_usage();
            }

            clax_abort();
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
            clax_abort();
        }
    }

    clax_log("Option: root=%s", options.root);
    clax_log("Option: entropy_file=%s", options.entropy_file);
    clax_log("Option: log_file=%s", options.log_file);
    clax_log("Option: ssl=%d", options.ssl);

    if (options.ssl) {
        clax_ssl_init();
    }

    if (options.standalone || clax_service_mode()) {
        if (!options.bind_host) {
            options.bind_host = DEFAULT_BIND;
            options.bind_port = DEFAULT_PORT;
        }

        clax_log("Option: listen=%s:%d", options.bind_host, options.bind_port);

        uv_tcp_t server;

        uv_tcp_init(uv_default_loop(), &server);

        struct sockaddr_in bind_addr;
        uv_ip4_addr(options.bind_host, options.bind_port, &bind_addr);

        int r = uv_tcp_bind(&server, (const struct sockaddr *)&bind_addr, 0);
        if (r) {
            clax_log("Error: bind failed: %s", uv_strerror(r));
            clax_abort();
            return 1;
        }

        r = uv_listen((uv_stream_t *)&server, 128, clax_conn_uv_new_cb);
        if (r) {
            clax_log("Error: listen failed: %s", uv_strerror(r));
            clax_abort();
            return 1;
        }

        clax_log("Listening on %s:%d\n", options.bind_host, options.bind_port);
        fprintf(stdout, "Listening on %s:%d\n", options.bind_host, options.bind_port);
    }
    else {
        clax_log("New stdin connection");

        uv_pipe_t *stdin_pipe = (uv_pipe_t *)malloc(sizeof(uv_pipe_t));
        uv_pipe_t *stdout_pipe = (uv_pipe_t *)malloc(sizeof(uv_pipe_t));

        uv_pipe_init(uv_default_loop(), stdin_pipe, 0);
        uv_pipe_open(stdin_pipe, fileno(stdin));

        uv_pipe_init(uv_default_loop(), stdout_pipe, 0);
        uv_pipe_open(stdout_pipe, fileno(stdout));

        clax_ctx_t *clax_ctx = clax_ctx_alloc();
        clax_ctx_init(clax_ctx, &options);

        clax_ctx->rh = stdin_pipe;
        clax_ctx->wh = stdout_pipe;
        clax_ctx->send_cb = clax_conn_write_cb;

        stdin_pipe->data = clax_ctx;
        stdout_pipe->data = clax_ctx;

        int r = uv_read_start((uv_stream_t*)stdin_pipe, alloc_buffer, clax_conn_uv_read_cb);

        if (r < 0) {
            clax_log("Error: read start failed: %s", uv_strerror(r));
            clax_abort();
        }
    }

    int r = uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    if (r < 0) {
        clax_log("Error: event loop failed: %s", uv_strerror(r));
        clax_abort();
    }

    /*if (!options.ssl) {*/
        /*clax_http_dispatch(&clax_ctx, clax_send, clax_recv, NULL);*/
    /*} else {*/
        /*clax_loop_ssl(&clax_ctx);*/
    /*}*/

    return 0;
}

#ifdef _WIN32
SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE ServiceStatusHandle;

DWORD service_init(DWORD argc, LPTSTR *argv, DWORD *specificError);

void WINAPI service_main(DWORD argc, LPTSTR *argv);
void WINAPI service_ctrl_handler(DWORD opcode);

DWORD service_init(DWORD argc, LPTSTR *argv, DWORD *specificError){
    *argv;
    argc;
    specificError;

    return 0;
}

void WINAPI service_main(DWORD argc, LPTSTR *argv){

    DWORD status;

    ServiceStatus.dwServiceType = SERVICE_WIN32;
    ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    ServiceStatus.dwWin32ExitCode = 0;
    ServiceStatus.dwServiceSpecificExitCode = 0;
    ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = 0;

    ServiceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, service_ctrl_handler);

    if (ServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) {
        clax_log("Service register error: %ld.\n", GetLastError());

        return;
    } else {
        clax_log("Service registered");
    }

    ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = 0;

    if (!SetServiceStatus(ServiceStatusHandle, &ServiceStatus)) {
        status = GetLastError();

        clax_log("Service status error: %ld", status);

        return;
    }
    else {
        clax_log("Service running");
    }

    unlink("clax.service.log");

    clax_main(clax_argc, clax_argv);

    return;
}

void WINAPI service_ctrl_handler(DWORD Opcode) {
    DWORD status;

    switch (Opcode) {
        case SERVICE_CONTROL_STOP:

            // Do whatever it takes to stop here...
            ServiceStatus.dwWin32ExitCode = 0;
            ServiceStatus.dwCurrentState = SERVICE_STOPPED;
            ServiceStatus.dwCheckPoint = 0;
            ServiceStatus.dwWaitHint = 0;

            if (!SetServiceStatus(ServiceStatusHandle, &ServiceStatus)){
                status = GetLastError();
                clax_log("[MY_SERVICE] SetServiceStatus() error: %ld", status);
            }

            clax_log("Leaving");

            clax_cleanup();

            return;
        case SERVICE_CONTROL_INTERROGATE:

            // Fall through to send current status.
            break;
        default:
            clax_log("Unrecognized opcode %ld", Opcode);
    }

    if (!SetServiceStatus(ServiceStatusHandle, &ServiceStatus)){
        status = GetLastError();
        clax_log("SetServiceStatus error %ld", status);

        return;
    }
    else {
        printf("SetServiceStatus() is OK.\n");
    }

    return;
}

#endif

int main(int argc, char **argv)
{
    if (clax_service_mode()) {
#ifdef _WIN32
        clax_argc = argc;
        clax_argv = argv;

        clax_options_init(&options);

        /* We have to parse the log file as soon as possible, otherwise the service crashes */
        int ok = clax_parse_options(&options, argc, argv);
        if (ok == 0 && options.log_file) {
            freopen(options.log_file, "w", stderr);
        }
        else {
            freopen("clax.service.log", "w", stderr);
        }

        clax_options_free(&options);

        SERVICE_TABLE_ENTRY service_dispatch_table[] = {
            {SERVICE_NAME, service_main}, {NULL, NULL}
        };

        if (!StartServiceCtrlDispatcher(service_dispatch_table)) {
            clax_log("Registering dispatch table failed: %ld", GetLastError());

            return -1;
        }
#endif
    }
    else {
        clax_main(argc, argv);
    }

    return 0;
}
