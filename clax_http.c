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

#include "clax_ctx.h"
#include "clax_http.h"
#include "clax_http_util.h"
#include "clax_http_parser.h"
#include "clax_dispatcher.h"
#include "clax_log.h"
#include "clax_http.h"
#include "clax_util.h"
#include "clax_platform.h"

int clax_http_write_response(void *ctx, clax_http_response_t *response);

void clax_http_dispatch_done_cb(clax_ctx_t *clax_ctx, clax_http_request_t *request, clax_http_response_t *response)
{
    clax_log("Dispatching done");

    if (request->continue_expected) {
        clax_http_write_response_headers(clax_ctx, response);

        request->continue_expected = 0;

        if (response->status_code == 100) {
            clax_log("100 continue");

            memset(response, 0, sizeof(clax_http_response_t));

            clax_kv_list_init(&response->headers);

            response->data = clax_ctx;
        }
        else {
            clax_log("Reject 100-continue");

            clax_http_write_response(clax_ctx, &clax_ctx->response);
        }
    }
    else {
        clax_http_write_response(clax_ctx, &clax_ctx->response);
    }
}

void clax_http_request_headers_done_cb(clax_http_request_t *request)
{
    clax_ctx_t *clax_ctx = request->data;

    clax_log("Request headers parsing done");

    if (clax_ctx->options->basic_auth_username && clax_ctx->options->basic_auth_password) {
        clax_log("Basic authorization required");

        int ok = clax_http_check_basic_auth(
                    clax_kv_list_find(&clax_ctx->request.headers, "Authorization"),
                    clax_ctx->options->basic_auth_username,
                    clax_ctx->options->basic_auth_password
                    );

        if (ok) {
            clax_log("User '%s'", clax_ctx->options->basic_auth_username);
        }
        else {
            clax_log("Basic authorization failed");

            clax_dispatch_not_authorized(clax_ctx, &clax_ctx->request, &clax_ctx->response);

            return;
        }
    }

    if (request->continue_expected) {
        clax_dispatch(clax_ctx, &clax_ctx->request, &clax_ctx->response);
    }
}

void clax_http_request_done_cb(clax_http_request_t *request)
{
    clax_ctx_t *clax_ctx = request->data;
    clax_http_response_t *response = &clax_ctx->response;

    clax_log("Request parsing done");

    clax_dispatch(clax_ctx, request, response);
}

void clax_http_response_done_cb(clax_http_response_t *response)
{
    clax_log("Response writing done");
}

int clax_http_dispatch(clax_ctx_t *clax_ctx, const char *buf, ssize_t len)
{
    clax_ctx->request.headers_done_cb = clax_http_request_headers_done_cb;
    clax_ctx->request.done_cb = clax_http_request_done_cb;

    int rv = clax_http_parse(&clax_ctx->parser, &clax_ctx->request, buf, len);

    if (rv < 0) {
        clax_log("Bad request");

        if (!&clax_ctx->response.finalized) {
            clax_dispatch_bad_request(clax_ctx, &clax_ctx->request, &clax_ctx->response, NULL);
        }

        return 0;
    }

//    if (clax_http_is_proxy(&request)) {
//        clax_http_dispatch_proxy(clax_ctx, &parser, &request, &response, recv_cb, send_cb, ctx);
//    }
//    else {
//        if (request.continue_expected) {
//            clax_dispatch(clax_ctx, &request, &response);
//
//            if (!response.status_code)
//                response.status_code = 100;
//
//            clax_log("Writing 100 Continue response...");
//            TRY clax_http_write_response(ctx, send_cb, &response) GOTO;
//            clax_log("ok");
//
//            if (response.status_code != 100) {
//                clax_log("100 Continue cancelled");
//                goto cleanup;
//            }
//
//            request.continue_expected = 0;
//            memset(&response, 0, sizeof(clax_http_response_t));
//
//            clax_log("Continuing reading & parsing request...");
//            TRY clax_http_read_parse(ctx, recv_cb, &parser, &request) GOTO;
//            clax_log("ok");
//        }
//
//        clax_log("Dispatching request...");
//        clax_dispatch(clax_ctx, &request, &response);
//        clax_log("ok");
//
//        clax_log("Writing response...");
//        TRY clax_http_write_response(ctx, send_cb, &response) GOTO;
//        clax_log("ok");
//    }

    return 0;
}

int send_cb_wrapper(clax_ctx_send_cb_t send_cb, void *ctx, const unsigned char *buf, size_t len)
{
    const unsigned char *b = buf;
    int ret;

#ifdef MVS
    if (!isatty(fileno(stdin))) {
        b = (const unsigned char *)clax_etoa_alloc((const char *)buf, len);
    }
#endif

    ret = send_cb(ctx, b, len);

#ifdef MVS
    free((void *)b);
#endif

    return ret;
}

int clax_http_response_status(clax_ctx_t *ctx, clax_http_response_t *response, int status)
{
    response->status_code = status;

    return 0;
}

char *clax_http_request_header(clax_ctx_t *ctx, clax_http_request_t *request, const char *header)
{
    return clax_kv_list_find(&request->headers, (char *)header);
}

int clax_http_response_header(clax_ctx_t *ctx, clax_http_response_t *response, const char *header, const char *value)
{
    clax_kv_list_push(&response->headers, (char *)header, (char *)value);

    return 0;
}

int clax_http_response_body(clax_ctx_t *clax_ctx, clax_http_response_t *response, const unsigned char *buf, size_t len)
{
    clax_buf_append(&response->body_buf, &response->body_len, buf, len);

    clax_http_dispatch_done_cb(clax_ctx, &clax_ctx->request, response);

    return 0;
}

int clax_http_response_body_str(clax_ctx_t *clax_ctx, clax_http_response_t *response, const char *str)
{
    clax_buf_append(&response->body_buf, &response->body_len, (unsigned const char *)str, strlen(str));

    clax_http_dispatch_done_cb(clax_ctx, &clax_ctx->request, response);

    return 0;
}

int clax_http_response_body_handle(clax_ctx_t *clax_ctx, clax_http_response_t *response, uv_file handle)
{
    response->body_handle = handle;

    clax_http_dispatch_done_cb(clax_ctx, &clax_ctx->request, response);

    return 0;
}

int clax_http_write_response_headers(clax_ctx_t *ctx, clax_http_response_t *response)
{
    char buf[5];
    sprintf(buf, "%d ", (int)response->status_code);

    clax_ctx_t *clax_ctx = response->data;
    clax_ctx_send_cb_t send_cb = clax_ctx->send_cb;

    const char *status_message = clax_http_status_message(response->status_code);

    send_cb_wrapper(send_cb, ctx, (const unsigned char *)"HTTP/1.1 ", 9);
    send_cb_wrapper(send_cb, ctx, (const unsigned char *)buf, MIN(strlen(buf), sizeof(buf)));
    send_cb_wrapper(send_cb, ctx, (const unsigned char *)status_message, strlen(status_message));
    send_cb_wrapper(send_cb, ctx, (const unsigned char *)"\r\n", 2);

    size_t header_iter = 0;
    clax_kv_list_item_t *header;
    while ((header = clax_kv_list_next(&response->headers, &header_iter)) != NULL) {
        send_cb_wrapper(send_cb, ctx, (const unsigned char *)header->key, strlen(header->key));
        send_cb_wrapper(send_cb, ctx, (const unsigned char *)": ", 2);
        send_cb_wrapper(send_cb, ctx, (const unsigned char *)header->val, strlen(header->val));
        send_cb_wrapper(send_cb, ctx, (const unsigned char *)"\r\n", 2);
    }

    send_cb_wrapper(send_cb, ctx, (const unsigned char *)"\r\n", 2);

    return 0;
}

void clax_http_finalize_response(void *ctx, clax_http_response_t *response)
{
    clax_log("Response finalized");
    response->finalized = 1;
}

int clax_http_push_response(void *ctx, clax_http_response_t *response, const unsigned char *buf, size_t len)
{
    clax_ctx_t *clax_ctx = ctx;
    clax_ctx_send_cb_t send_cb = clax_ctx->send_cb;

    if (len) {
        char obuf[255];
        int olen;

        olen = snprintf(obuf, sizeof(obuf), "%x\r\n", (int)len);

        send_cb_wrapper(send_cb, ctx, (const unsigned char *)obuf, olen);
        send_cb_wrapper(send_cb, ctx, (const unsigned char *)buf, len);
        send_cb_wrapper(send_cb, ctx, (const unsigned char *)"\r\n", 2);
    }
    else {
        send_cb_wrapper(send_cb, ctx, (const unsigned char *)"0\r\n", 3);

        /* Trailing headers */
        if (buf && strlen((const char *)buf)) {
            send_cb_wrapper(send_cb, ctx, (const unsigned char *)buf, strlen((const char *)buf));
            send_cb_wrapper(send_cb, ctx, (const unsigned char *)"\r\n", 2);
        }

        send_cb_wrapper(send_cb, ctx, (const unsigned char *)"\r\n", 2);

        clax_http_finalize_response(clax_ctx, response);
    }

    return 0;
}

typedef struct {
    uv_fs_t req;
    uv_buf_t buf;
    void *data;
} clax_uv_read_req_t;

int clax_http_read_response_handle_chunk(clax_ctx_t *clax_ctx, clax_http_response_t *response);

void clax_http_write_response_handle_(uv_fs_t *req)
{
    clax_uv_read_req_t *clax_req = (clax_uv_read_req_t *)req;
    clax_ctx_t *clax_ctx = clax_req->data;

    if (req->result == 0) {
        clax_log("Finished reading file");

        uv_fs_t close_req;
        uv_fs_close(uv_default_loop(), &close_req, clax_ctx->response.body_handle, NULL);

        clax_http_push_response(clax_ctx, &clax_ctx->response, NULL, 0);

        free(clax_req->buf.base);
        uv_fs_req_cleanup(req);
    }
    else if (req->result > 0) {
        clax_http_push_response(clax_ctx, &clax_ctx->response, (unsigned char *)clax_req->buf.base, req->result);

        free(clax_req->buf.base);
        uv_fs_req_cleanup(req);

        int r = clax_http_read_response_handle_chunk(clax_ctx, &clax_ctx->response);

        if (r < 0) {
            clax_log("Error reading body handle");

            clax_http_response_done_cb(&clax_ctx->response);
        }
    }
    else {
        clax_log("File read error: %s", uv_strerror(req->result));

        clax_http_push_response(clax_ctx, &clax_ctx->response, NULL, 0);

        if (clax_req->buf.base) {
            free(clax_req->buf.base);
        }
        uv_fs_req_cleanup(req);
    }
}

int clax_http_read_response_handle_chunk(clax_ctx_t *clax_ctx, clax_http_response_t *response)
{
    clax_uv_read_req_t *read_req = malloc(sizeof(clax_uv_read_req_t));
    read_req->data = clax_ctx;

    read_req->buf.base = (char *)malloc(1024);
    read_req->buf.len = 1024;

    return uv_fs_read(uv_default_loop(), (uv_fs_t *)read_req, response->body_handle, &read_req->buf, 1, -1, clax_http_write_response_handle_);
}

int clax_http_write_response(void *ctx, clax_http_response_t *response)
{
    clax_ctx_t *clax_ctx = ctx;

    if (response->finalized) {
        return 0;
    }

    /*clax_log("Writing response");*/

    if (response->body_handle) {
        clax_log("Serving body handle");

        if (clax_ctx->request.method == HTTP_HEAD) {
            clax_log("Closing file because of HEAD");

            uv_fs_t close_req;
            uv_fs_close(uv_default_loop(), &close_req, clax_ctx->response.body_handle, NULL);

            char body_len_buf[30];
            snprintf(body_len_buf, sizeof(body_len_buf), "%ld", clax_ctx->response.body_len);

            clax_http_response_header(ctx, response, "Content-Length", body_len_buf);

            clax_http_write_response_headers(ctx, response);

            clax_http_finalize_response(clax_ctx, response);
        }
        else {
            clax_http_response_header(ctx, response, "Transfer-Encoding", "chunked");

            clax_http_write_response_headers(ctx, response);

            int r = clax_http_read_response_handle_chunk(clax_ctx, &clax_ctx->response);

            if (r < 0) {
                clax_log("Error reading body handle");

                clax_http_response_done_cb(response);

                return -1;
            }
        }
    }
    else if (response->body_len > 0) {
        clax_log("Serving body data");

        clax_http_response_header(ctx, response, "Transfer-Encoding", "chunked");

        clax_http_write_response_headers(ctx, response);

        clax_http_push_response(clax_ctx, &clax_ctx->response, response->body_buf, response->body_len);
        clax_http_push_response(clax_ctx, &clax_ctx->response, NULL, 0);
    }
    else {
        clax_log("No body");

        clax_http_response_header(ctx, response, "Content-Length", "0");

        clax_http_write_response_headers(ctx, response);

        clax_http_finalize_response(clax_ctx, response);

        return 0;
    }

    return 0;
}

//int clax_connect(int sockfd, struct addrinfo *addr, int timeout)
//{
//    fd_set fds;
//    struct timeval tv;
//
//#ifndef _WIN32
//    long flags;
//
//    if ((flags = fcntl(sockfd, F_GETFL, NULL)) < 0) {
//        clax_log("Error fcntl: %s", strerror(errno));
//        return -1;
//    }
//    flags |= O_NONBLOCK;
//    if (fcntl(sockfd, F_SETFL, flags) < 0) {
//        clax_log("Error fcntl: %s", strerror(errno));
//        return -1;
//    }
//#endif
//
//    if (!timeout) {
//        timeout = 15;
//    }
//
//    if (connect(sockfd, addr->ai_addr, addr->ai_addrlen) < 0) {
//        if (errno == EINPROGRESS) {
//            while (1) {
//                tv.tv_sec = timeout;
//                tv.tv_usec = 0;
//
//                FD_ZERO(&fds);
//                FD_SET(sockfd, &fds);
//
//                int rv = select(sockfd + 1, NULL, &fds, NULL, &tv);
//                if (rv < 0 && errno != EINTR) {
//                   clax_log("Error connecting: %s", strerror(errno));
//                   return -1;
//                }
//                else if (rv > 0) {
//                   int optval;
//                   socklen_t optlen = sizeof(optval);
//
//                   if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (void *)(&optval), &optlen) < 0) {
//                      clax_log("Socket options error: %s", strerror(errno));
//                      return -1;
//                   }
//
//                   if (optval) {
//                      clax_log("Socket error: %s", strerror(optval));
//                      return -1;
//                   }
//
//                   break;
//                }
//                else {
//                   clax_log("Connection timeout");
//                   return -1;
//                }
//            }
//
//#ifndef _WIN32
//            if ((flags = fcntl(sockfd, F_GETFL, NULL)) < 0) {
//                clax_log("Error fcntl: %s", strerror(errno));
//                return -1;
//            }
//            flags &= (~O_NONBLOCK);
//            if (fcntl(sockfd, F_SETFL, flags) < 0) {
//                clax_log("Error fcntl: %s", strerror(errno));
//                return -1;
//            }
//#endif
//        }
//        else {
//            return -1;
//        }
//    }
//
//    return 0;
//}
//
//int clax_http_connect_to_proxy(char *hostname, char *port, int timeout)
//{
//    struct addrinfo hints;
//    struct addrinfo *addrs, *addr;
//    int sockfd;
//    int rv;
//
//    memset(&hints, 0, sizeof(struct addrinfo));
//    hints.ai_family = AF_INET;
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_flags = 0;
//    hints.ai_protocol = IPPROTO_TCP;
//
//    if ((rv = getaddrinfo(hostname, port, &hints, &addrs)) != 0) {
//        clax_log("Can't resolve '%s:%s': %d - %s", hostname, port, rv, strerror(errno));
//        return -1;
//    }
//
//    for (addr = addrs; addr != NULL; addr = addr->ai_next) {
//        sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
//        if (sockfd == -1)
//            continue;
//
//        if (clax_connect(sockfd, addr, timeout) == 0) {
//            break;
//        }
//
//        close(sockfd);
//    }
//
//    if (addr == NULL) {
//        clax_log("Could not connect");
//        return -1;
//    }
//
//    freeaddrinfo(addrs);
//
//    return sockfd;
//}
//
//int clax_http_proxy_write_request_body(clax_http_request_t *req, int sockfd)
//{
//    if (req->body && !req->continue_expected) {
//        clax_log("Writing proxy body");
//        size_t offset = 0;
//        ssize_t wcount = 0;
//
//        while ((wcount = send(sockfd, req->body + offset, req->body_len - offset, 0)) > 0) {
//            offset += wcount;
//        }
//
//        if (wcount < 0) {
//            return -1;
//        }
//    }
//
//    return 0;
//}
//
//int clax_http_proxy_write_request(char *hostname, char *port, int sockfd, clax_http_request_t *req, char *hops)
//{
//    const char *method = http_method_str(req->method);
//    TRY send(sockfd, method, strlen(method), 0) GOTO;
//    TRY send(sockfd, " ", 1, 0) GOTO;
//    TRY send(sockfd, req->url, strlen(req->url), 0) GOTO;
//    TRY send(sockfd, " HTTP/1.1\r\n", 11, 0) GOTO;
//
//    clax_kv_list_item_t *header;
//    size_t header_iter = 0;
//
//    while ((header = clax_kv_list_next(&req->headers, &header_iter)) != NULL) {
//        if (strcmp(header->key, "X-Hops") == 0 && (hops == NULL || strlen(hops) == 0)) {
//            continue;
//        }
//
//        TRY send(sockfd, header->key, strlen(header->key), 0) GOTO;
//        TRY send(sockfd, ": ", 2, 0) GOTO;
//
//        if (strcmp(header->key, "Host") == 0) {
//            TRY send(sockfd, hostname, strlen(hostname), 0) GOTO;
//            TRY send(sockfd, ":", 1, 0) GOTO;
//            TRY send(sockfd, port, strlen(port), 0) GOTO;
//        }
//        else if (strcmp(header->key, "X-Hops") == 0 && hops) {
//            TRY send(sockfd, hops, strlen(hops), 0) GOTO;
//        }
//        else {
//            TRY send(sockfd, header->val, strlen(header->val), 0) GOTO;
//        }
//
//        TRY send(sockfd, "\r\n", 2, 0) GOTO;
//    }
//
//    TRY send(sockfd, "\r\n", 2, 0) GOTO;
//
//    TRY clax_http_proxy_write_request_body(req, sockfd) GOTO;
//
//    return 0;
//
//error:
//    return -1;
//}

//void clax_http_dispatch_proxy(clax_ctx_t *clax_ctx, http_parser *parser, clax_http_request_t *req, clax_http_response_t *res,
//        recv_cb_t recv_cb, send_cb_t send_cb, void *ctx)
//{
//    char *hostname = NULL;
//    char *port = NULL;
//    char *hops = clax_strdup(clax_kv_list_find(&req->headers, "X-Hops"));
//    char *hop_timeout = clax_kv_list_find(&req->headers, "X-Hop-Timeout");
//    char *sep;
//
//    if (hops == NULL || strlen(hops) == 0) {
//        clax_dispatch_bad_request(clax_ctx, req, res, "Invalid X-Hops header");
//        TRY clax_http_write_response(ctx, send_cb, res) GOTO;
//        return;
//    }
//
//    if ((sep = strstr(hops, ",")) != NULL) {
//        hostname = clax_strndup(hops, sep - hops);
//        strcpy(hops, sep + 1);
//    }
//    else {
//        hostname = clax_strdup(hops);
//        hops[0] = 0;
//    }
//
//    if (hostname == NULL || strlen(hostname) == 0) {
//        clax_dispatch_bad_request(clax_ctx, req, res, "Invalid X-Hops header");
//        TRY clax_http_write_response(ctx, send_cb, res) GOTO;
//
//        free(hops);
//        free(hostname);
//        free(port);
//
//        return;
//    }
//
//    if ((sep = strstr(hostname, ":")) != NULL) {
//        port = clax_strdup(sep + 1);
//        *sep = 0;
//    }
//    else {
//        port = clax_strdup("80");
//    }
//
//    if (hop_timeout == NULL)
//        hop_timeout = "";
//
//    clax_log("Connecting to proxy %s:%s (%d)", hostname, port, atoi(hop_timeout));
//
//#ifdef _WIN32
//    WSADATA wsaData;
//    WSAStartup(MAKEWORD(2, 2), &wsaData);
//#endif
//
//    int sockfd = clax_http_connect_to_proxy(hostname, port, atoi(hop_timeout));
//
//    if (sockfd < 0) {
//        clax_dispatch_bad_gateway(clax_ctx, req, res);
//        TRY clax_http_write_response(ctx, send_cb, res) GOTO;
//        return;
//    }
//
//    clax_log("Connected to proxy");
//
//    if (clax_http_proxy_write_request(hostname, port, sockfd, req, hops) < 0) {
//        clax_dispatch_bad_gateway(clax_ctx, req, res);
//        TRY clax_http_write_response(ctx, send_cb, res) GOTO;
//    }
//
//    clax_log("Proxy request written. Waiting for response...");
//
//#ifdef _WIN32
//    shutdown(sockfd, SD_SEND);
//#else
//    shutdown(sockfd, SHUT_WR);
//#endif
//
//    unsigned char buffer[1024];
//    memset(buffer, 0, sizeof(buffer) * sizeof(char));
//
//    while (1) {
//        int rcount = recv(sockfd, buffer, sizeof(buffer), 0);
//
//        if (rcount < 0) {
//            clax_log("Error during reading from proxy: %s", strerror(errno));
//            goto error;
//        }
//
//        if (rcount == 0) {
//            clax_log("Done reading from proxy");
//            break;
//        }
//
//        TRY send_cb_wrapper(send_cb, ctx, buffer, rcount) GOTO;
//
//        if (req->continue_expected) {
//            req->continue_expected = 0;
//
//            int ret = 0;
//            unsigned char buf[1024];
//
//            do {
//                memset(buf, 0, sizeof(buf));
//                ret = recv_cb(ctx, buf, sizeof(buf));
//
//                if (ret == EAGAIN) {
//                    clax_log("EAGAIN");
//                    continue;
//                }
//
//                if (ret < 0) {
//                    clax_log("Reading failed!");
//                    goto error;
//                }
//                else if (ret == 0) {
//                    clax_log("Connection closed");
//                }
//
//                TRY send(sockfd, buf, ret, 0) GOTO;
//
//                ret = clax_http_parse(parser, req, (char *)buf, ret);
//
//                if (ret < 0) {
//                    goto error;
//                }
//                else if (ret == 1) {
//                    clax_log("Request parsing done");
//                    break;
//                } else if (ret == 0) {
//                }
//            } while (1);
//
//            clax_log("DONE");
//        }
//    }
//
//    clax_log("Disconnecting from proxy");
//
//error:
//    if (sockfd > 0)
//        close(sockfd);
//
//    free(hops);
//    free(hostname);
//    free(port);
//
//#ifdef _WIN32
//    WSACleanup();
//#endif
//
//    return;
//}
