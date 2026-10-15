#define _POSIX_C_SOURCE 200809L

#include "utils.h"

#include <string.h>

static void reverse_line(char *buf, size_t len)
{
    if (len == 0)
        return;

    size_t end = (buf[len - 1] == '\n') ? len - 1 : len;

    if (end <= 1)
        return;

    size_t left = 0, right = end - 1;
    while (left < right) {
        char tmp    = buf[left];
        buf[left]   = buf[right];
        buf[right]  = tmp;
        left++;
        right--;
    }
}

int main(void)
{
    print_pid("Child PID: ");

    for (;;) {
        char   *line = NULL;
        ssize_t len  = read_line(STDIN_FILENO, &line);

        if (len < 0) {
            write_err("child: read error\n");
            return 1;
        }

        if (len == 0)
            break;

        reverse_line(line, (size_t)len);

        if (write_all(STDOUT_FILENO, line, (size_t)len) < 0) {
            write_err("child: write error\n");
            free(line);
            return 1;
        }

        free(line);
    }

    return 0;
}
