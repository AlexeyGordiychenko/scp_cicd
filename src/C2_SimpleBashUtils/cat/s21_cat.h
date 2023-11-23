#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  bool nflag;
  bool bflag;
  bool eflag;
  bool vflag;
  bool sflag;
  bool tflag;
  bool hflag;
  bool err_fl;
  bool err_file;
} Flags;

void parse_flags(int argc, char *argv[], Flags *flags);
void process_files(int argc, char *argv[], Flags *flags);
void output_line(char *line, ssize_t read, size_t *line_num, Flags flags,
                 bool *prev_blank_line, bool *no_new_line_at_the_EOF);
void output_vflag(char ch);
char last_character(FILE *fp);
void print_errors(Flags flags, char *arg);
void print_help(char *name);