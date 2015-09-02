#ifndef CLAX_H
#define CLAX_H

typedef struct {
    char entropy_file[255];
    char log_file[255];
    char root[255];

    char no_ssl;
    char cert_file[255];
    char key_file[255];

    /* Private */
    FILE *_log_file;
} opt;

typedef struct {
    opt* options;
} clax_ctx_t;

#endif
