#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>

void clax_log_(const char *file, int line, const char *func_, char *fmt, ...)
{
    int size;
    char *cp;
    va_list args;
    char func[1024];
    time_t epoch;
    struct tm *timeinfo;

    strcpy(func, func_);

#ifdef MVS
    char func_a[1024];
    __toascii_a((char * )func_a, func);
    strcpy(func, func_a);
#endif

    va_start(args, fmt);
    size = vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);

    time(&epoch);
    timeinfo = localtime(&epoch);

    va_start(args, fmt);
    cp = (char *)calloc(size, sizeof(char));
    if (cp != NULL && vsnprintf(cp, size, fmt, args) > 0) {
        char timestr[255];
        strftime(timestr, sizeof(timestr), "%Y-%m-%d %T", timeinfo);

        fprintf(stderr, "%s:%d:%s:%d:%s(): %s\n", timestr, getpid(), file, line, func, cp);

    }
    va_end(args);
    free(cp);
}
