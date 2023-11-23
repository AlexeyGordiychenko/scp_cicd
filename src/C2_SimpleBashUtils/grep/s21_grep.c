#include "s21_grep.h"

int main(int argc, char *argv[]) {
  Flags flags = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  regex_t *re = NULL;
  int re_count = 0;

  // exit code 0 - found, 1 - not found, 2 - error
  int exit_code = 1;

  // Parse command-line args
  parse_args(argc, argv, &flags, &re_count, &re);
  if (no_errors(flags)) {
    int not_found = 1;
    for (int i = optind; i < argc && !flags.err_mem; i++) {
      not_found = process_file(argv[i], &flags, re_count, re) && not_found;
      if (flags.err_mem || flags.err_file) {
        print_errors(flags, argv[i]);
        exit_code = 2;
      }
    }
    exit_code = (exit_code == 2) ? exit_code : not_found;
  } else {
    print_errors(flags, argv[0]);
    exit_code = 2;
  }

  free_re(re_count, &re);

  return exit_code;
}

void parse_args(int argc, char *argv[], Flags *flags, int *re_count,
                regex_t **re) {
  /*
  first iteration just to get flags
  second to process patterns
  */
  parse_flags(argc, argv, flags);
  if (!flags->err_fl) {
    if (flags->helpflag) {
      print_help(argv[0]);
    } else {
      parse_re(argc, argv, flags, re_count, re);
      flags->sfile = (optind == argc - 1);
    }
  }
}

void parse_flags(int argc, char *argv[], Flags *flags) {
  static struct option long_options[] = {{"help", no_argument, 0, 0},
                                         {0, 0, 0, 0}};

  int opt, long_index = 0;
  while ((opt = getopt_long(argc, argv, "e:ivclnhsf:o", long_options,
                            &long_index)) != -1) {
    switch (opt) {
      case 'e':
      case 'f':
        break;
      case 'i':
        flags->iflag = 1;
        break;
      case 'v':
        flags->vflag = 1;
        break;
      case 'c':
        flags->cflag = 1;
        break;
      case 'l':
        flags->lflag = 1;
        break;
      case 'n':
        flags->nflag = 1;
        break;
      case 'h':
        flags->hflag = 1;
        break;
      case 's':
        flags->sflag = 1;
        break;
      case 'o':
        flags->oflag = 1;
        break;
      case 0:
        flags->helpflag = 1;
        break;
      default:
        flags->err_fl = true;
        break;
    }
  }
}

void parse_re(int argc, char *argv[], Flags *flags, int *re_count,
              regex_t **re) {
  bool res = true;
  // flags for regcomp
  int re_flags = REG_NEWLINE;
  if (flags->iflag) re_flags |= REG_ICASE;

  // reset the variables after the first loop
  optind = 1;
  opterr = 0;
  int opt;
  while ((opt = getopt(argc, argv, "e:f:")) != -1 && res) {
    switch (opt) {
      case 'e':
      case 'f':
        res = re_flag_handler(opt, flags, re_count, re, re_flags);
        break;
      default:
        break;
    }
  }
  if (res && *re_count == 0) {
    if (optind < argc) {
      res = add_re(argv[optind], re_count, re, re_flags);
      optind++;
    }
  }
  flags->err_re_comp = !res && !flags->err_file;
  if (res && *re_count == 0) res = false;
  flags->err_re = !res && !flags->err_file && !flags->err_re_comp;
}

int process_file(char *filename, Flags *flags, int re_count, regex_t *re) {
  int matches_count = 0;
  flags->err_file = false;
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    flags->err_file = true;
  } else {
    int line_count = 0;
    bool lflag_check = true;
    char *line = NULL;
    size_t len = 0;
    ssize_t line_len;
    while ((line_len = getline(&line, &len, fp)) != -1 && !flags->err_mem &&
           lflag_check) {
      line_count++;
      regmatch_t *matches = NULL;
      int matches_found = find_matches(line, flags, re_count, re, &matches);
      if (!flags->err_mem && ((!flags->vflag && matches_found) ||
                              (flags->vflag && !matches_found++))) {
        matches_count++;
        output_line(filename, line, line_len, line_count, &lflag_check, *flags,
                    matches_found, matches);
        if (matches != NULL) free(matches);
      }
    }

    if (line != NULL) free(line);

    output_line_cflag(filename, matches_count, *flags);

    fclose(fp);
  }
  return matches_count <= 0;
}

int find_matches(char *line, Flags *flags, int re_count, regex_t *re,
                 regmatch_t **matches) {
  int matches_count = 0;
  if (flags->oflag && !flags->cflag && !flags->vflag && !flags->lflag) {
    find_matches_oflag(line, re_count, re, &matches_count, matches);
  } else {
    for (int i = 0; i < re_count && !matches_count; i++) {
      if (regexec(&re[i], line, 0, NULL, 0) == 0) matches_count++;
    }
  }
  if (matches_count == -1) flags->err_mem = true;
  return matches_count;
}

void find_matches_oflag(char *line, int re_count, regex_t *re,
                        int *matches_count, regmatch_t **matches) {
  int matches_size = 0;
  for (int i = 0; i < re_count && *matches_count >= 0; i++) {
    // Search for all matches within the line
    char *buf = line;
    regmatch_t match;
    int offset = 0;
    while (regexec(&re[i], buf, 1, &match, 0) == 0 && *matches_count >= 0) {
      // Merge overlapping matches
      bool merged = false;
      match.rm_so += offset;
      match.rm_eo += offset;
      for (int k = 0; k < (*matches_count) && !merged; k++) {
        if (match.rm_so < (*matches)[k].rm_eo &&
            match.rm_eo > (*matches)[k].rm_so) {
          (*matches)[k].rm_so = (match.rm_so < (*matches)[k].rm_so)
                                    ? match.rm_so
                                    : (*matches)[k].rm_so;
          (*matches)[k].rm_eo = (match.rm_eo > (*matches)[k].rm_eo)
                                    ? match.rm_eo
                                    : (*matches)[k].rm_eo;
          merged = true;
        }
      }
      if (!merged) {
        if (*matches_count < matches_size) {
          (*matches)[(*matches_count)++] = match;
        } else {
          // Handle case where there are more matches than matches_size
          matches_size = (matches_size == 0) ? 3 : matches_size * 2;
          *matches = realloc(*matches, sizeof(regmatch_t) * matches_size);
          if (*matches == NULL) {
            *matches_count = -1;
          } else {
            (*matches)[(*matches_count)++] = match;
          }
        }
      }

      // Advance buffer to next match
      buf += match.rm_eo - offset;
      offset = match.rm_eo;
    }
  }
  if (*matches_count > 0)
    qsort(*matches, *matches_count, sizeof(regmatch_t), cmp_matches);
}

int cmp_matches(const void *a, const void *b) {
  return ((*(regmatch_t *)a).rm_so - (*(regmatch_t *)b).rm_so);
}

int add_re(char *pattern, int *re_count, regex_t **re, int flags) {
  (*re_count)++;
  *re = realloc(*re, sizeof(regex_t) * *re_count);
  return regcomp(&((*re)[*re_count - 1]), pattern, flags) == 0;
}

void free_re(int re_count, regex_t **re) {
  if (re != NULL) {
    for (int i = 0; i < re_count; i++) {
      regfree(&((*re)[i]));
    }
    free(*re);
  }
}

bool re_flag_handler(int fl, Flags *flags, int *re_count, regex_t **re,
                     int re_flags) {
  bool res = true;

  if (fl == 'e') {
    res = add_re(optarg, re_count, re, re_flags);
  } else if (fl == 'f') {
    FILE *ffile = fopen(optarg, "r");
    if (ffile == NULL) {
      flags->err_file = true;
      res = false;
    } else {
      char *line = NULL;
      size_t len = 0;
      while (getline(&line, &len, ffile) != -1 && res) {
        line[strcspn(line, "\n")] = 0;
        res = add_re(line, re_count, re, re_flags);
      }
      fclose(ffile);
      if (line != NULL) free(line);
    }
  }
  return res;
}

bool no_errors(Flags flags) {
  return !(flags.err_fl || flags.err_re || flags.err_re_comp || flags.err_mem ||
           flags.err_file);
}

void print_errors(Flags flags, char *arg) {
  if (!flags.sflag) {
    if (flags.err_mem) {
      fprintf(stderr, "Error: out of memory.\n");
    }
    if (flags.err_re_comp) {
      fprintf(stderr, "Failed to compile pattern.\n");
    }
    if (flags.err_file) {
      fprintf(stderr, "%s: No such file or directory\n", arg);
    }
  }
  if (flags.err_fl || flags.err_re) {
    fprintf(stderr,
            "Usage: grep [OPTION]... PATTERNS [FILE]...\n"
            "Try '%s --help' for more information.\n",
            arg);
  }
}

void output_line(char *filename, char *line, int line_len, int line_count,
                 bool *lflag_check, Flags flags, int matches_found,
                 regmatch_t *matches) {
  if (flags.lflag) {
    printf("%s\n", filename);
    *lflag_check = false;
  } else {
    if (!flags.cflag) {
      if ((flags.oflag && !flags.vflag) || !flags.oflag) {
        for (int i = 0; i < matches_found; i++) {
          if (!flags.hflag && !flags.sfile) printf("%s:", filename);
          if (flags.nflag) printf("%d:", line_count);
          if (flags.oflag) {
            fwrite(line + matches[i].rm_so, 1,
                   matches[i].rm_eo - matches[i].rm_so, stdout);
            printf("\n");
          } else {
            printf("%s", line);
            if (line[line_len - 1] != '\n') printf("\n");
          }
        }
      }
    }
  }
}

void output_line_cflag(char *filename, int matches_count, Flags flags) {
  if (!flags.err_mem) {
    if (flags.cflag && !flags.lflag) {
      if (!flags.sfile && !flags.hflag) printf("%s:", filename);
      printf("%d\n", matches_count);
    }
  }
}

void print_help(char *name) {
  printf(
      "Usage: %s [OPTION]... PATTERNS [FILE]...\n"
      "Search for PATTERNS in each FILE.\n"
      "Example: grep -i 'hello world' menu.h main.c\n"
      "PATTERNS can contain multiple patterns separated by newlines.\n"
      "\n"
      "Pattern selection and interpretation:\n"
      "  -e,               use PATTERNS for matching\n"
      "  -f,               take PATTERNS from FILE\n"
      "  -i,               ignore case distinctions in patterns and "
      "\n"
      "Miscellaneous:\n"
      "  -s,               suppress error messages\n"
      "  -v,               select non-matching lines\n"
      "\n"
      "Output control:\n"
      "  -n,               print line number with output lines\n"
      "  -h,               suppress the file name prefix on output\n"
      "  -o,               show only nonempty parts of lines that "
      "match\n"
      "  -l,               print only names of FILEs with selected "
      "lines\n"
      "  -c,               print only a count of selected lines per "
      "FILE\n"
      "      --help        display this help and exit\n",
      name);
}