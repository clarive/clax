#ifndef POPEN2_H
#define POPEN2_H

#include <stdlib.h>

typedef struct {
    pid_t pid;
    int out, in;
} popen2_t;

int popen2(const char *cmdline, popen2_t *child);
int pclose2(popen2_t *child);

#endif
