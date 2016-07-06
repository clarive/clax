/*
 *  Copyright (C) 2015, Clarive Software, All Rights Reserved
 *
 *  This file is part of clax.
 *
 *  Clax is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Clax is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Clax.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/wait.h>
#endif

#include "clax_util.h"
#include "clax_log.h"
#include "popen2.h"

#define READ 0
#define WRITE 1

#if defined(_WIN32)

#define PRINT_LAST_ERROR(msg)                                   \
    LPVOID lpMsgBuf;                                            \
    DWORD dw = GetLastError();                                  \
                                                                \
    FormatMessage(                                              \
        FORMAT_MESSAGE_ALLOCATE_BUFFER |                        \
        FORMAT_MESSAGE_FROM_SYSTEM |                            \
        FORMAT_MESSAGE_IGNORE_INSERTS,                          \
        NULL,                                                   \
        dw,                                                     \
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),              \
        (LPTSTR) &lpMsgBuf,                                     \
        0, NULL );                                              \
                                                                \
    /* lpMsgBuf already has a \n */                             \
    fprintf(stderr, msg ": %s", lpMsgBuf);                      \
                                                                \
    LocalFree(lpMsgBuf);

int popen2(const char *cmdline, char **env, popen2_t *child)
{
    SECURITY_ATTRIBUTES saAttr;

    HANDLE pipe_in_write = NULL;
    HANDLE pipe_out_read = NULL;

    HANDLE pipe_in_read = NULL;
    HANDLE pipe_out_write = NULL;

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&pipe_out_read, &pipe_out_write, &saAttr, 0))
        return -1;

    if (!SetHandleInformation(pipe_out_read, HANDLE_FLAG_INHERIT, 0))
        return -1;

    if (!CreatePipe(&pipe_in_read, &pipe_in_write, &saAttr, 0))
        return -1;

    if (!SetHandleInformation(pipe_in_write, HANDLE_FLAG_INHERIT, 0))
        return -1;

    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE;

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = pipe_out_write;
    siStartInfo.hStdOutput = pipe_out_write;
    siStartInfo.hStdInput = pipe_in_read;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    char *cmd_start = "cmd.exe /C \"";
    char *cmd_end = "\"";
    char *cmd_line_exe = malloc(strlen(cmd_start) + strlen(cmdline) + strlen(cmd_end) + 1);
    strcpy(cmd_line_exe, cmd_start);
    strcat(cmd_line_exe, cmdline);
    strcat(cmd_line_exe, cmd_end);

    char *env_text = NULL;
    size_t env_text_len = 0;
    if (env) {
        int i = 0;
        while (env[i] != NULL) {
            char *p = env[i];

            clax_buf_append(&env_text, &env_text_len, p, strlen(p));
            clax_buf_append(&env_text, &env_text_len, "\0", 1);

            i++;
        }

        clax_buf_append(&env_text, &env_text_len, "\0", 1);
    }

    bSuccess = CreateProcess(NULL,
       cmd_line_exe,
       NULL,
       NULL,
       TRUE,
       CREATE_NO_WINDOW,
       env_text,
       NULL,
       &siStartInfo,
       &piProcInfo);

    free(env_text);
    free(cmd_line_exe);

    if (!bSuccess) {
        /*PRINT_LAST_ERROR("CreateProcess failed");*/

        return -1;
    }

    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    CloseHandle(pipe_out_write);
    CloseHandle(pipe_in_read);

    int in = _open_osfhandle((intptr_t)pipe_in_write, _O_WRONLY | _O_BINARY);
    int out = _open_osfhandle((intptr_t)pipe_out_read, _O_RDONLY | _O_BINARY);

    child->pid = piProcInfo.dwProcessId;
    child->in = in;
    child->out = out;

    return 0;
}

int pclose2(popen2_t *child)
{
    /* Do nothing when nothing was created, otherwise directories are get locked, windows, heh? */
    if (!child->pid)
        return 255;

    _close(child->in);
    _close(child->out);

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, child->pid);

    unsigned long int exit_code = 255;

    DWORD ret = WaitForSingleObject(hProcess, INFINITE);

    if (ret == WAIT_OBJECT_0) {
        int done = 0;
        while (!done) {
            if (GetExitCodeProcess(hProcess, &exit_code) == FALSE) {
                /*PRINT_LAST_ERROR("GetExitCodeProcess failed");*/

                exit_code = 255;
                done++;
            }
            else if (exit_code == STILL_ACTIVE) {
                continue;
            }
            else {
                done++;
            }
        }
    }

    CloseHandle(hProcess);

    return exit_code;
}

#else

int popen2(const char *cmdline, char **env, popen2_t *child)
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
        setsid();
        setpgid(0, 0);

        close(pipe_stdin[WRITE]);
        dup2(pipe_stdin[READ], READ);

        close(pipe_stdout[READ]);
        dup2(pipe_stdout[WRITE], WRITE);

        execle("/bin/sh", "sh", "-c", cmdline, (char*)0, env);
        exit(99);
    }

    setpgid(pid, 0);

    close(pipe_stdin[READ]);
    close(pipe_stdout[WRITE]);

    child->pid = pid;
    child->in = pipe_stdin[WRITE];
    child->out = pipe_stdout[READ];

    fcntl(child->out, F_SETFL, (fcntl(child->out, F_GETFL, 0) | O_NONBLOCK));

    return 0;
}

int pclose2(popen2_t *child)
{
    pid_t pid;
    int pstat;

    do {
        pid = waitpid(child->pid, &pstat, 0);
    } while (pid == -1 && errno == EINTR);

    return (pid == -1 ? -1 : pstat / 256);
}

#endif
