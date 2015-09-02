#ifndef CLAX_DISPATCHER_H
#define CLAX_DISPATCHER_H

#include "clax.h"
#include "clax_http.h"

void clax_dispatch(clax_ctx_t *clax_ctx, clax_http_request_t *req, clax_http_response_t *res);

#endif
