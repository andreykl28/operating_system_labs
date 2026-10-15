#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <unistd.h>

ssize_t read_line(int fd, char **out);
int     write_all(int fd, const char *buf, size_t n);
void    write_err(const char *msg);
void    print_pid(const char *label);

#endif
