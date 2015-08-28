#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include "popen2.h"

#define READ 0
#define WRITE 1

int popen2(const char *cmdline, popen2_t *child)
{
    pid_t pid;
    int pipe_stdin[2], pipe_stdout[2];

    if (pipe(pipe_stdin))
        return -1;
    if (pipe(pipe_stdout))
        return -1;

    pid = fork();
    if (pid < 0)
        return -1;

    if (pid == 0) {
        close(pipe_stdin[WRITE]);
        dup2(pipe_stdin[READ], READ);

        close(pipe_stdout[READ]);
        dup2(pipe_stdout[WRITE], WRITE);

        execl("/bin/sh", "sh", "-c", cmdline, 0);
        exit(99);
    }

    close(pipe_stdin[READ]);
    close(pipe_stdout[WRITE]);

    child->pid = pid;
    child->in = pipe_stdin[WRITE];
    child->out = pipe_stdout[READ];

    return 0;
}

int pclose2(popen2_t *child)
{
    pid_t pid;
    int pstat;

    do {
        pid = waitpid(child->pid, &pstat, 0);
    } while (pid == -1 && errno == EINTR);

    return (pid == -1 ? -1 : pstat);
}
