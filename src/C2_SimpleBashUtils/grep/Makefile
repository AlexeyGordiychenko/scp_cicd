CC=gcc
CFLAGS=-Wall -Werror -Wextra

all: scp_grep

scp_grep: scp_grep.c scp_grep.h
	$(CC) $(CFLAGS) scp_grep.c scp_grep.h -o scp_grep

clean:
	rm scp_grep || true

rebuild: clean all
