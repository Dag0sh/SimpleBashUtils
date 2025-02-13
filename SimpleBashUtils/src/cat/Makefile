NAME      := s21_cat

CC        := gcc
CCFLAGS   := -Wall -Werror -Wextra
SRCS      := s21_cat.c
TESTFILE  := s21_cat_tests.sh

RM        := rm -f
BASH      := bash

MAKEFLAGS += --no-print-directory

all: $(NAME)

$(NAME): $(SRCS)
	$(CC) $(CCFLAGS) $< -o $@

test: $(NAME)
	$(BASH) $(TESTFILE)

clean:
	$(RM) $(NAME)

rebuild:
	$(MAKE) clean
	$(MAKE) all