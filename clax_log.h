#ifndef CLAX_LOG_H
#define CLAX_LOG_H

#define clax_log(...) clax_log_(__FILE__, __LINE__, __func__, __VA_ARGS__)

void clax_log_(const char *file, int line, const char *func_, char *fmt, ...);

#endif
