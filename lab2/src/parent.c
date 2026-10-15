#define _POSIX_C_SOURCE 200809L

#include "utils.h"

#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

static char *read_filename(const char *prompt)
{
    write_err(prompt);

    char   *line = NULL;
    ssize_t len  = read_line(STDIN_FILENO, &line);

    if (len <= 0) {
        free(line);
        return NULL;
    }

    if (line[len - 1] == '\n') {
        line[len - 1] = '\0';
    } else {
        char *tmp = realloc(line, (size_t)len + 1);
        if (!tmp) {
            free(line);
            return NULL;
        }
        line      = tmp;
        line[len] = '\0';
    }

    return line;
}

static void safe_close(int *fd)
{
    if (*fd >= 0) {
        close(*fd);
        *fd = -1;
    }
}

int main(void)
{
    print_pid("Parent PID: ");

    char *fname1 = read_filename("Enter filename for child1: ");
    if (!fname1) {
        write_err("parent: failed to read first filename\n");
        return 1;
    }

    char *fname2 = read_filename("Enter filename for child2: ");
    if (!fname2) {
        write_err("parent: failed to read second filename\n");
        free(fname1);
        return 1;
    }

    int file_fd1 = open(fname1, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd1 < 0) {
        write_err("parent: open file1 failed\n");
        free(fname1); free(fname2);
        return 1;
    }

    int file_fd2 = open(fname2, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd2 < 0) {
        write_err("parent: open file2 failed\n");
        close(file_fd1); free(fname1); free(fname2);
        return 1;
    }

    free(fname1);
    free(fname2);

    int pipe1[2] = {-1, -1};
    int pipe2[2] = {-1, -1};

    if (pipe(pipe1) < 0) {
        write_err("parent: pipe1 failed\n");
        close(file_fd1); close(file_fd2);
        return 1;
    }

    if (pipe(pipe2) < 0) {
        write_err("parent: pipe2 failed\n");
        close(pipe1[0]); close(pipe1[1]);
        close(file_fd1); close(file_fd2);
        return 1;
    }

    pid_t pid1 = fork();
    if (pid1 < 0) {
        write_err("parent: fork child1 failed\n");
        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        close(file_fd1); close(file_fd2);
        return 1;
    }

    if (pid1 == 0) {
        if (dup2(pipe1[0], STDIN_FILENO) < 0 || dup2(file_fd1, STDOUT_FILENO) < 0) {
            write_err("child1: dup2 failed\n");
            _exit(1);
        }
        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        close(file_fd1); close(file_fd2);
        execl("./child", "child", NULL);
        write_err("child1: execl failed\n");
        _exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        write_err("parent: fork child2 failed\n");
        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        close(file_fd1); close(file_fd2);
        waitpid(pid1, NULL, 0);
        return 1;
    }

    if (pid2 == 0) {
        if (dup2(pipe2[0], STDIN_FILENO) < 0 || dup2(file_fd2, STDOUT_FILENO) < 0) {
            write_err("child2: dup2 failed\n");
            _exit(1);
        }
        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        close(file_fd1); close(file_fd2);
        execl("./child", "child", NULL);
        write_err("child2: execl failed\n");
        _exit(1);
    }

    close(pipe1[0]); pipe1[0] = -1;
    close(pipe2[0]); pipe2[0] = -1;
    close(file_fd1); file_fd1 = -1;
    close(file_fd2); file_fd2 = -1;

    write_err("Enter lines (Ctrl+D to finish):\n");

    int line_num = 0;
    int rc       = 0;

    for (;;) {
        char   *line = NULL;
        ssize_t len  = read_line(STDIN_FILENO, &line);

        if (len < 0) {
            write_err("parent: read error\n");
            rc = 1;
            break;
        }

        if (len == 0)
            break;

        line_num++;
        int target = (line_num % 2 != 0) ? pipe1[1] : pipe2[1];

        if (write_all(target, line, (size_t)len) < 0) {
            write_err("parent: write to pipe failed\n");
            free(line);
            rc = 1;
            break;
        }

        free(line);
    }

    safe_close(&pipe1[1]);
    safe_close(&pipe2[1]);

    int status1 = 0, status2 = 0;
    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);

    if (WIFEXITED(status1) && WEXITSTATUS(status1) != 0)
        rc = 1;
    if (WIFEXITED(status2) && WEXITSTATUS(status2) != 0)
        rc = 1;

    write_err("Parent: done\n");
    return rc;
}
