CC=gcc
CFLAGS=-Wall -Werror -Wextra

all: scp_cat

scp_cat: scp_cat.c scp_cat.h
	$(CC) $(CFLAGS) scp_cat.c scp_cat.h -o scp_cat

clean:
	rm scp_cat || true

rebuild: clean all
