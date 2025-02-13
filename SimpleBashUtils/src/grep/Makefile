CC = gcc
FLAGS = -Wall -Werror -Wextra -std=c11
OBJFILES = s21_grep.o
EXEC = s21_grep

all: $(EXEC)

s21_grep.o: s21_grep.c s21_grep.h
	$(CC) $(CFLAGS) -c s21_grep.c


clean:
	rm -f $(OBJFILES) $(EXEC)

rebuild: clean all

.PHONY: clean rebuild