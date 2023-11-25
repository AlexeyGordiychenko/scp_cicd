#include <getopt.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  bool iflag;
  bool vflag;
  bool cflag;
  bool lflag;
  bool nflag;
  bool hflag;
  bool sflag;
  bool oflag;
  bool helpflag;
  bool sfile;  // single file, only one file provided
  bool err_mem;
  bool err_fl;
  bool err_re;
  bool err_re_comp;
  bool err_file;
} Flags;

void parse_args(int argc, char *argv[], Flags *flags, int *re_count,
                regex_t **re);
void parse_flags(int argc, char *argv[], Flags *flags);
void parse_re(int argc, char *argv[], Flags *flags, int *re_count,
              regex_t **re);
int process_file(char *filename, Flags *flags, int re_count, regex_t *re);
void process_lines(FILE *fp, regex_t *regex, Flags *flags);
int find_matches(char *line, Flags *flags, int re_count, regex_t *re,
                 regmatch_t **matches);
void find_matches_oflag(char *line, int re_count, regex_t *re,
                        int *matches_count, regmatch_t **matches);
int cmp_matches(const void *a, const void *b);
int add_re(char *pattern, int *re_count, regex_t **re, int flags);
void free_re(int re_count, regex_t **re);
bool re_flag_handler(int fl, Flags *flags, int *re_count, regex_t **re,
                     int re_flags);
bool no_errors(Flags flags);
void print_errors(Flags flags, char *arg);
void output_line(char *filename, char *line, int line_len, int line_count,
                 bool *lflag_check, Flags flags, int matches_found,
                 regmatch_t *matches);
void output_line_cflag(char *filename, int matches_count, Flags flags);
void print_help(char *name);