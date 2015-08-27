#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

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
