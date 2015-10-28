#ifndef _UTIL_H
#define _UTIL_H

int util_parse_http_response(char *buf, size_t len);
int execute(char *command, char *request, char *obuf, size_t olen);
char *mktmpdir();
int rmrf(char *path);

#ifdef _WIN32
# define CMD "..\\clax.exe"
#else
# define CMD "../clax"
#endif

#endif
