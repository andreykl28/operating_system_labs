CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -std=c11 -pedantic -Isrc

.PHONY: all clean

all: parent child

parent: src/parent.c src/utils.c src/utils.h
	$(CC) $(CFLAGS) -o parent src/parent.c src/utils.c

child: src/child.c src/utils.c src/utils.h
	$(CC) $(CFLAGS) -o child src/child.c src/utils.c

clean:
	rm -f parent child out1.txt out2.txt
