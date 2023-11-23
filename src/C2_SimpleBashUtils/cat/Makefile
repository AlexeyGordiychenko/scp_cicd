CC=gcc
CFLAGS=-Wall -Werror -Wextra

all: s21_cat

s21_cat: s21_cat.c s21_cat.h
	$(CC) $(CFLAGS) s21_cat.c s21_cat.h -o s21_cat

clean:
	rm s21_cat || true

rebuild: clean all
