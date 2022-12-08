
#include "util.h"

void __terminal_error(const char *file, const unsigned line, const char *func, const char *msg) {

  printf("[%s:%u:%s] %s\n", file, line, func, msg);
  fflush(stdout);
  /*std::raise(SIGINT);*/
  exit(1);
}


int rand_int(int max_int) { return std::rand() % (max_int); }
int rand_int_range(int min_int, int max_int) {
  return min_int + (std::rand() % (max_int - min_int + 1));
}

void *safe_malloc(size_t n, unsigned int line) {
  void *p = malloc(n);
  if (!p) {
    fprintf(stderr, "[%s:%u]Out of memory(%u bytes)\n", __FILE__, line, (unsigned int)n);
    exit(EXIT_FAILURE);
  }
  return p;
}

int *rand_int_vector(int max_int, int n) {
  int *int_v = (int *)safemalloc(sizeof(int) * n);
  for (int i = 0; i < n; i++) {
    int_v[i] = rand_int(max_int + 1);
  }
  return int_v;
}

int count_lines(FILE *fp) {
  int num_lines = 0;
  char ch;
  fseek(fp, 0L, SEEK_SET);
  while (!feof(fp)) {
    ch = fgetc(fp);
    if (ch == '\n') {
      num_lines++;
    }
  }
  fseek(fp, 0L, SEEK_SET);
  return num_lines;
}
