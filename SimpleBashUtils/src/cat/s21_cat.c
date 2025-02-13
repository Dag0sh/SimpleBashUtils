#include <ctype.h>
#include <getopt.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#define cat_putc(c) \
  if (putchar(c) == EOF) break

#define cat_puts(s) \
  if (printf(s) == EOF) break

#define cat_printf(f, args...) \
  if (printf(f, args) == EOF) break

static enum Opts {
  flagb = 1u << 0,
  flage = 1u << 1,
  flagn = 1u << 2,
  flags = 1u << 3,
  flagt = 1u << 4,
  flagv = 1u << 5
} opts;

static struct option const long_options[] = {
    {.name = "number-nonblank", .has_arg = no_argument, .val = 'b'},
    {.name = "number", .has_arg = no_argument, .val = 'n'},
    {.name = "squeeze-blank", .has_arg = no_argument, .val = 's'}};

static void parse_flag(int argc, char** argv) {
  for (;;) {
    switch (getopt_long(argc, argv, "benstuvAET", long_options, NULL)) {
      case -1:
        return;
      case 'b':
        opts |= (flagb | flagn);
        break;
      case 'e':
        opts |= (flage | flagv);
        break;
      case 'n':
        if (!(opts & flagb)) opts |= flagn;
        break;
      case 's':
        opts |= flags;
        break;
      case 't':
        opts |= (flagt | flagv);
        break;
      case 'u':
        setbuf(stdout, NULL);
        break;
      case 'v':
        opts |= flagv;
        break;
      case 'A':
        opts |= (flage | flagt | flagv);
        break;
      case 'E':
        opts |= flage;
        break;
      case 'T':
        opts |= flagt;
        break;
      case '?':  // Неизвестный флаг
      default:
        fprintf(stderr, "s21_cat: illegal option -- %c\n", optopt);
        fprintf(stderr, "usage: %s [-benstuv] [file ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }
}

static void cat(const char* filename) {
  int c, prev_c = '\n';
  int count_str = 0;
  FILE* file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "s21_cat: %s: No such file or directory\n", filename);
    return;
  }
  int empty = 0;
  for (; (c = fgetc(file)) != EOF; prev_c = c) {
    if (prev_c == '\n') {
      if (opts & flags) {
        if (c == '\n') {
          if (empty) {
            continue;
          }
          empty = 1;
        } else {
          empty = 0;
        }
      }
      if ((opts & flagn) && (!(opts & flagb) || c != '\n')) {
        cat_printf("%6d\t", ++count_str);
      }
    }
    if (c == '\n') {
      if (opts & flage) {
        cat_putc('$');
      }
    } else if (c == '\t') {
      if (opts & flagt) {
        cat_puts("^I");
        continue;
      }
    } else if (opts & flagv) {
      if (!isascii(c) && !isprint(c)) {
        cat_puts("M-");
        c = toascii(c);
      }
      if (iscntrl(c)) {
        cat_printf("^%c", (c == '\177') ? '?' : c | 0100);
        continue;
      }
    }
    cat_putc(c);
  }
  fclose(file);
}

int main(int argc, char* argv[]) {
  parse_flag(argc, argv);
  for (; optind < argc; ++optind) {
    cat(argv[optind]);
  }
  return 0;
}