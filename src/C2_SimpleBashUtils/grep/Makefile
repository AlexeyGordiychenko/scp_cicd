CC=gcc
CFLAGS=-Wall -Werror -Wextra

all: s21_grep

s21_grep: s21_grep.c s21_grep.h
	sleep 20
	$(CC) $(CFLAGS) s21_grep.c s21_grep.h -o s21_grep

clean:
	rm s21_grep || true

rebuild: clean all
