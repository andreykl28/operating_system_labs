#define _POSIX_C_SOURCE 200809L

#include "utils.h"

#include <string.h>
#include <errno.h>

#define INITIAL_BUF 128

ssize_t read_line(int fd, char **out)
{
    size_t capacity = INITIAL_BUF;
    size_t length   = 0;

    char *buf = malloc(capacity);
    if (!buf)
        return -1;

    for (;;) {
        char    ch;
        ssize_t n = read(fd, &ch, 1);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            free(buf);
            return -1;
        }

        if (n == 0)
            break;

        if (length == capacity) {
            size_t new_cap = capacity * 2;
            char  *tmp     = realloc(buf, new_cap);
            if (!tmp) {
                free(buf);
                return -1;
            }
            buf      = tmp;
            capacity = new_cap;
        }

        buf[length++] = ch;

        if (ch == '\n')
            break;
    }

    if (length == 0) {
        free(buf);
        *out = NULL;
        return 0;
    }

    *out = buf;
    return (ssize_t)length;
}

int write_all(int fd, const char *buf, size_t n)
{
    size_t written = 0;
    while (written < n) {
        ssize_t w = write(fd, buf + written, n - written);
        if (w < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        written += (size_t)w;
    }
    return 0;
}

void write_err(const char *msg)
{
    if (msg)
        write_all(STDERR_FILENO, msg, strlen(msg));
}

void print_pid(const char *label)
{
    pid_t pid = getpid();
    char  buf[32];
    int   idx = 0;
    char  rev[32];
    int   ri  = 0;

    if (pid == 0) {
        buf[idx++] = '0';
    } else {
        pid_t tmp = pid;
        while (tmp > 0) {
            rev[ri++] = (char)('0' + tmp % 10);
            tmp /= 10;
        }
        for (int i = ri - 1; i >= 0; i--)
            buf[idx++] = rev[i];
    }

    write_err(label);
    write_all(STDERR_FILENO, buf, (size_t)idx);
    write_err("\n");
}
