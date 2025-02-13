#ifndef STRUCT_H
#define STRUCT_H

#include <ctype.h>
#include <getopt.h>
#include <limits.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct flags {
  int regex_flag;
  int v;
  int c;
  int l;
  int n;
  char *echar;
  int h;
  int o;
  int f;
  int e;
  int s;
};

struct flags read_flags(int argc, char *argv[]);
void remove_newline(char *str);
void read_patterns_from_file(const char *file_name, regex_t **patterns,
                             size_t *pattern_count);
int match_line(const char *line, regex_t **patterns, size_t pattern_count);
int check_e(int argc, char *argv[]);
char *flag_e(int argc, char *argv[]);
int Grep(int argc, char *argv[], struct flags flag);
void GrepFile(char *argv[], FILE *file, struct flags *flag, regex_t *regex[],
              int count, char *filename, int *count_for_c, size_t regex_count);
void GrepFile_o(FILE *file, struct flags *flag, regex_t *regex[],
                int count_files, char *filename, size_t regex_count);
void GrepFile_n(FILE *file, struct flags *flag, regex_t *regex[],
                int count_files, char *filename, size_t regex_count);
void GrepFile_l(FILE *file, struct flags *flag, regex_t *regex[],
                int count_files, char *filename, int *count_for_c,
                size_t regex_count);
void GrepFile_simple(FILE *file, struct flags *flag, regex_t *regex[],
                     int count, char *filename, size_t regex_count);
void GrepFile_c(FILE *file, struct flags *flag, regex_t *regex[],
                int count_files, char *filename, size_t regex_count);
void GrepFile_v(FILE *file, struct flags *flag, regex_t *regex[], int count,
                char *filename, size_t regex_count);

#endif