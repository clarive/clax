#ifndef _UTIL_H
#define _UTIL_H

int util_parse_http_response(char *buf, size_t len);
int execute(char *command, char *request, char *obuf, size_t olen);
char *mktmpdir();
int rmrf(char *path);

#endif
