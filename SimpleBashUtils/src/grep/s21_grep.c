#define _GNU_SOURCE
#include "s21_grep.h"

#define MAX_PATTERN_LENGTH 1024
#define MAX_STRING_LENGTH 1024

struct option long_options[] = {};

int main(int argc, char *argv[]) {
  int fail = 1;
  if (argc > 2) {
    struct flags flag = read_flags(argc, argv);
    fail = Grep(argc, argv, flag);
  }
  return fail;
}
struct flags read_flags(int argc, char *argv[]) {
  struct flags flag = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int cur_flag;
  while ((cur_flag = getopt_long(argc, argv, "e:ivclnf:hso", long_options,
                                 NULL)) != -1) {
    switch (cur_flag) {
      case 'e':
        flag.e = 1;
        flag.regex_flag |= REG_EXTENDED;
        break;
      case 'i':
        flag.regex_flag |= REG_ICASE;
        break;
      case 'v':
        flag.v = 1;
        break;
      case 'c':
        flag.c = 1;
        break;
      case 'l':
        flag.l = 1;
        break;
      case 'n':
        flag.n = 1;
        break;
      case 'h':
        flag.h = 1;
        break;
      case 'f':
        flag.f = 1;
        break;
      case 'o':
        flag.o = 1;
        break;
      case 's':
        flag.s = 1;
        break;
      default:
        fprintf(stderr, "Unknown option: %c\n", cur_flag);
    }
  }
  if (flag.f == 1)
    for (int i = 0; i < argc; i++) {
      if (strcmp(argv[i], "-f") == 0) {
        flag.f = i + 1;
      }
    }
  return flag;
}

void remove_newline(char *str) {
  char *pos;
  if (str[0] == '\n') return;
  pos = strchr(str, '\n');
  if (pos != NULL) {
    *pos = '\0';
  }
}

void read_patterns_from_file(const char *file_name, regex_t **patterns,
                             size_t *pattern_count) {
  FILE *file = fopen(file_name, "r");
  if (!file) {
    perror("Failed to open pattern file");
    exit(EXIT_FAILURE);
  }

  char line[MAX_PATTERN_LENGTH];
  while (fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\n")] = '\0';  // Удалить символ новой строки
    if (strlen(line) > 0) {
      regex_t *new_pattern = malloc(sizeof(regex_t));
      if (regcomp(new_pattern, line, REG_EXTENDED | REG_NEWLINE)) {
        fprintf(stderr, "Could not compile regex: %s\n", line);
        free(new_pattern);
      } else {
        patterns[*pattern_count] = new_pattern;
        (*pattern_count)++;
      }
    }
  }
  fclose(file);
}

int match_line(const char *line, regex_t **patterns, size_t pattern_count) {
  int fl = 0;
  for (size_t i = 0; i < pattern_count; i++) {
    if (!fl && !regexec(patterns[i], line, 0, NULL, 0)) {
      fl = 1;
    }
  }
  return fl;
}

int check_e(int argc, char *argv[]) {
  regex_t preg2;
  regmatch_t match;
  regcomp(&preg2, "^-e[a-zA-Z]{1,}$", REG_EXTENDED);
  int fl = 0;
  for (int i = 0; i < argc - 1; i++) {
    if (!fl && !regexec(&preg2, argv[i], 1, &match, 0)) {
      fl = 1;
    }
  }
  regfree(&preg2);
  return fl;
}

char *flag_e(int argc, char *argv[]) {
  size_t totalLength = 0;
  for (int i = 0; i < argc; ++i) {
    totalLength += strlen(argv[i]) + 3;
  }
  char *pattern = 0;
  pattern = (char *)calloc(totalLength, sizeof(char));
  pattern[0] = '\0';

  int fl = 0;
  regex_t preg;
  regcomp(&preg, "^-.*e$", 0);
  int minus_lenght = 0;
  for (int i = 0; i < argc - 1; i++) {
    if (!regexec(&preg, argv[i], 0, NULL, 0) && fl == 0) {
      if (pattern[0] != '\0') {
        strcat(pattern, "|");
      }
      minus_lenght += strlen(argv[i + 1]) + 1;
      strcat(pattern, argv[i + 1]);
      fl = 1;
    } else {
      fl = 0;
    }
  }
  if (check_e(argc, argv) == 1) {
    regex_t preg2;
    regmatch_t match;
    regcomp(&preg2, "^-e.*$", 0);
    char *buf = malloc(sizeof(char) * (totalLength - minus_lenght));
    buf[0] = '\0';
    int q = 0;
    for (int i = 0; i < argc - 1; i++) {
      if (!regexec(&preg2, argv[i], 1, &match, 0)) {
        if (pattern[0] != '\0') {
          strcat(pattern, "|");
        }
        for (int j = 2; j < match.rm_eo; j++) {
          buf[q++] = argv[i][j];
        }
        strcat(pattern, buf);
      }
      q = 0;
    }
    free(buf);
    regfree(&preg2);
  }
  char *result = (char *)malloc(totalLength + 3);
  result[0] = '\0';
  strcat(result, "(");
  strcat(result, pattern);
  strcat(result, ")");
  free(pattern);
  regfree(&preg);
  return result;
}

int Grep(int argc, char *argv[], struct flags flag) {
  char **pattern = &argv[1];
  char **argv_cpy = malloc(argc * sizeof(char *));
  for (int i = 0; i < argc; i++) {
    argv_cpy[i] = malloc(strlen(argv[i]) + 1);
    strcpy(argv_cpy[i], argv[i]);
  }
  if (flag.e > 0) {
    char *res = flag_e(argc, argv);
    strcpy(*pattern, res);
    free(res);
  }
  char **end = &argv[argc];
  for (; pattern != end && pattern[0][0] == '-'; ++pattern) {
    ;
  }
  int fl = 0;
  if (pattern == end) {
    fprintf(stderr, "no pattern\n");
    fl = 1;
  }
  regex_t regex;
  if (regcomp(&regex, *pattern, flag.regex_flag)) {
    fprintf(stderr, "no pattern\n");
    fl = 1;
  }
  int count = 0;
  for (char **filename = pattern + 1; filename != end; ++filename) {
    if (**filename != '-' && strstr(*pattern, *filename) == NULL &&
        strcmp(*filename, argv_cpy[flag.f]) != 0)
      count++;
  }
  if (flag.h == 1) {
    count = 1;
  }
  int count_for_c = 0;
  regex_t *regex_arr[1024];
  regex_arr[0] = &regex;
  size_t regex_count = 1;
  for (char **filename = pattern + 1; filename != end; ++filename) {
    if (**filename != '-' || strstr(*pattern, *filename) == NULL) {
      FILE *file = fopen(*filename, "rb");
      if (file == NULL) {
        if (flag.s == 0 && strcmp(*pattern, *filename) != 0) {
          fprintf(stderr, "grep: %s: No such file or directory\n", *filename);
        }
      } else {
        GrepFile(argv_cpy, file, &flag, regex_arr, count, *filename,
                 &count_for_c, regex_count);
        fclose(file);
      }
    }
  }
  for (int i = 0; i < argc; i++) {
    free(argv_cpy[i]);
  }
  regfree(&regex);
  free(argv_cpy);
  return fl;
}

void GrepFile(char *argv[], FILE *file, struct flags *flag, regex_t *regex[],
              int count, char *filename, int *count_for_c, size_t regex_count) {
  if (flag->f > 0 && strcmp(argv[flag->f], filename) != 0) {
    read_patterns_from_file(argv[flag->f], regex, &regex_count);
  }
  if (strcmp(argv[flag->f], filename) != 0) {
    GrepFile_l(file, flag, regex, count, filename, count_for_c, regex_count);
    GrepFile_v(file, flag, regex, count, filename, regex_count);
    GrepFile_c(file, flag, regex, count, filename, regex_count);
    GrepFile_o(file, flag, regex, count, filename, regex_count);
    GrepFile_n(file, flag, regex, count, filename, regex_count);
    GrepFile_simple(file, flag, regex, count, filename, regex_count);
  }
  for (size_t i = 1; i < regex_count; ++i) {
    regfree(regex[i]);
    free(regex[i]);
  }
}

void GrepFile_o(FILE *file, struct flags *flag, regex_t *regex[],
                int count_files, char *filename, size_t regex_count) {
  if (flag->o == 1 && flag->l == 0) {
    regmatch_t match;
    int count = 0;
    char *line = 0;
    size_t length = 0;
    while (getline(&line, &length, file) > 0) {
      const char *p = line;
      count++;
      for (size_t i = 0; i < regex_count; i++) {
        while (regexec(regex[i], p, 1, &match, 0) == 0) {
          if (count_files > 1) {
            printf("%s:", filename);
          }
          if (flag->n == 1) {
            printf("%d:", count);
          }
          for (int j = match.rm_so; j < match.rm_eo; j++) {
            putchar(p[j]);
          }
          putchar('\n');
          p += match.rm_eo;
          if (match.rm_so == match.rm_eo) {
            p++;
          }
        }
      }
    }
    free(line);
  }
}

void GrepFile_n(FILE *file, struct flags *flag, regex_t *regex[],
                int count_files, char *filename, size_t regex_count) {
  if (flag->n == 1 && flag->l == 0) {
    int count = 0;
    char line[MAX_STRING_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
      count++;
      if (match_line(line, regex, regex_count)) {
        if (count_files > 1) {
          printf("%s:", filename);
        }
        printf("%d:%s", count, line);
        if (strchr(line, '\n') == NULL) printf("\n");
      }
    }
  }
}

void GrepFile_l(FILE *file, struct flags *flag, regex_t *regex[],
                int count_files, char *filename, int *count_for_c,
                size_t regex_count) {
  if (flag->l == 1) {
    char *line = 0;
    size_t length = 0;
    while (getline(&line, &length, file) > 0) {
      if (flag->v == 0) {
        if (match_line(line, regex, regex_count)) {
          if (flag->c > 0) {
            *count_for_c += 1;
            if (flag->c == 1 && count_files > 1) {
              fprintf(stdout, "%s:%d\n", filename, *count_for_c);
            } else {
              printf("%d\n", *count_for_c);
            }
          }
          printf("%s\n", filename);
          break;
        }
      } else {
        if (!match_line(line, regex, regex_count)) {
          if (flag->c > 0) {
            *count_for_c += 1;
            if (flag->c == 1 && count_files > 1) {
              fprintf(stdout, "%s:%d\n", filename, *count_for_c);
            } else {
              printf("%d\n", *count_for_c);
            }
          }
          printf("%s\n", filename);
          break;
        }
      }
    }
    if (*count_for_c == 0) {
      if (flag->c == 1 && count_files > 1) {
        fprintf(stdout, "%s:%d\n", filename, *count_for_c);
      } else if (flag->c == 1) {
        printf("%d\n", *count_for_c);
      }
    }
    *count_for_c = 0;
    free(line);
  }
}

void GrepFile_simple(FILE *file, struct flags *flag, regex_t *regex[],
                     int count, char *filename, size_t regex_count) {
  if (flag->c == 0 && flag->l == 0 && flag->n == 0 && flag->v == 0 &&
      flag->o == 0) {
    char *line = 0;
    size_t length = 0;
    while (getline(&line, &length, file) > 0) {
      if (match_line(line, regex, regex_count)) {
        if (!(strcmp(line, "\n") == 0 && flag->f == 0)) {
          if (count > 1) {
            printf("%s:", filename);
          }
          printf("%s", line);
          if (strchr(line, '\n') == NULL) printf("\n");
        }
      }
    }
    free(line);
  }
}

void GrepFile_c(FILE *file, struct flags *flag, regex_t *regex[],
                int count_files, char *filename, size_t regex_count) {
  if (flag->c == 1 && flag->v == 0 && flag->l == 0) {
    int count = 0;
    char *line = 0;
    size_t length = 0;
    while (getline(&line, &length, file) > 0) {
      if (match_line(line, regex, regex_count)) {
        if (strcmp(line, "\n") != 0) count++;
        ;
      }
    }
    if (flag->c == 1) {
      if (count_files > 1) {
        printf("%s:", filename);
      }
      printf("%d\n", count);
    }
    free(line);
  }
}

void GrepFile_v(FILE *file, struct flags *flag, regex_t *regex[], int count,
                char *filename, size_t regex_count) {
  if (flag->v == 1 && flag->l == 0) {
    int c = 0;
    int n = 0;
    char *line = 0;
    size_t length = 0;
    while (getline(&line, &length, file) > 0) {
      n++;
      if (!match_line(line, regex, regex_count)) {
        c++;
        if (count > 1 && flag->c == 0) {
          printf("%s:", filename);
        }
        if (flag->n == 1 && flag->c == 0) printf("%d:", n);
        if (flag->c == 0) printf("%s", line);
        if (strchr(line, '\n') == NULL && flag->c == 0) printf("\n");
      }
    }
    if (count > 1 && flag->c == 1) {
      printf("%s:", filename);
    }
    if (flag->c == 1) printf("%d\n", c);
    free(line);
  }
}