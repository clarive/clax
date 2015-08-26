#ifndef CLAX_COMMAND_H
#define CLAX_COMMAND_H

int clax_command(char *command, int (*chunk_cb)(char *buf, size_t len, va_list a_list), ...);

#endif
