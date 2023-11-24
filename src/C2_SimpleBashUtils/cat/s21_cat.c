#include "s21_cat.h"

int main(int argc, char *argv[]) {
  Flags flags = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  parse_flags(argc, argv, &flags);
  if (!flags.err_fl) {
    if (flags.hflag) {
      print_help(argv[0]);
    } else {
      process_files(argc, argv, &flags);
    }
  } else {
    print_errors(flags, argv[0]);
  }
  bool res = flags.err_file || flags.err_fl;
  return res;
}

void parse_flags(int argc, char *argv[], Flags *flags) {
  static struct option long_options[] = {
      {"number-nonblank", no_argument, 0, 'b'},
      {"number", no_argument, 0, 'n'},
      {"squeeze-blank", no_argument, 0, 's'},
      {"help", no_argument, 0, 0},
      {0, 0, 0, 0}};

  int opt, long_index = 0;
  while ((opt = getopt_long(argc, argv, "benstvTE", long_options,
                            &long_index)) != -1) {
    switch (opt) {
      case 'n':
        flags->nflag = 1;
        break;
      case 'e':
        flags->eflag = flags->vflag = 1;  // e implies v, same as vE
        break;
      case 'E':
        flags->eflag = 1;
        break;
      case 'v':
        flags->vflag = 1;
        break;
      case 'b':
        flags->bflag = flags->nflag = 1;  // b implies n
        break;
      case 's':
        flags->sflag = 1;
        break;
      case 't':
        flags->tflag = flags->vflag = 1;  // t implies v, same as vT
        break;
      case 'T':
        flags->tflag = 1;
        break;
      case 0:
        flags->hflag = 1;
        break;
      default:
        flags->err_fl = true;
    }
  }
}

void process_files(int argc, char *argv[], Flags *flags) {
  char *line = NULL;
  size_t len = 0, line_num = 1;
  ssize_t read;
  bool prev_blank_line = false, no_new_line_at_the_EOF = false;

  for (int i = optind; i < argc; i++) {
    FILE *fp = fopen(argv[i], "r");
    if (fp == NULL) {
      flags->err_file = true;
      print_errors(*flags, argv[i]);
    } else {
      while ((read = getline(&line, &len, fp)) != -1) {
        output_line(line, read, &line_num, *flags, &prev_blank_line,
                    &no_new_line_at_the_EOF);
      }

      /* reading last char of the file coz valgrind doesn't
      like if I try to get it from the 'line' */
      no_new_line_at_the_EOF = last_character(fp) != '\n';
      fclose(fp);
    }
  }
  free(line);
}

void output_line(char *line, ssize_t read, size_t *line_num, Flags flags,
                 bool *prev_blank_line, bool *no_new_line_at_the_EOF) {
  bool blank_line = line[0] == '\n';
  if (flags.sflag) {
    if (blank_line && *prev_blank_line && !*no_new_line_at_the_EOF) {
      return;
    }
    *prev_blank_line = blank_line && !*no_new_line_at_the_EOF;
  }

  if (!*no_new_line_at_the_EOF) {
    if ((flags.bflag && !blank_line) || (flags.nflag && !flags.bflag)) {
      printf("%6ld\t", (*line_num)++);
    }
  }
  *no_new_line_at_the_EOF = 0;

  for (int i = 0; i < read; i++) {
    if (flags.tflag && line[i] == '\t') {
      printf("^I");
    } else if (flags.eflag && line[i] == '\n') {
      printf("$\n");
    } else if (flags.vflag) {
      output_vflag(line[i]);
    } else {
      printf("%c", line[i]);
    }
  }
}

void output_vflag(char ch) {
  if (ch < -128 + 32) {
    printf("M-^%c", ch + 128 + 64);
  } else if (ch < 0) {
    printf("M-%c", ch + 128);
  } else if (ch < 32 && ch != 9 && ch != 10) {
    printf("^%c", ch + 64);
  } else if (ch < 127) {
    printf("%c", ch);
  } else {
    printf("^?");
  }
}

char last_character(FILE *fp) {
  fseek(fp, -1, SEEK_END);
  char ch;
  fread(&ch, 1, 1, fp);
  return ch;
}

void print_errors(Flags flags, char *arg) {
  if (flags.err_file) {
    fprintf(stderr, "%s: No such file or directory\n", arg);
  }
  if (flags.err_fl) {
    fprintf(stderr, "Try '%s --help' for more information.\n", arg);
  }
}

void print_help(char *name) {
  printf(
      "Usage: %s [OPTION]... [FILE]...\n"
      "Concatenate FILE(s) to standard output.\n\n"
      "  -b, --number-nonblank    number nonempty output lines, overrides -n\n"
      "  -e                       equivalent to -vE\n"
      "  -E, --show-ends          display $ at end of each line\n"
      "  -n, --number             number all output lines\n"
      "  -s, --squeeze-blank      suppress repeated empty output lines\n"
      "  -t                       equivalent to -vT\n"
      "  -T, --show-tabs          display TAB characters as ^I\n"
      "  -v, --show-nonprinting   use ^ and M- notation, except for LFD and "
      "TAB\n\n"
      "  -h  --help               display this help and exit\n",
      name);
}