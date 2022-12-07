
#ifndef UTIL_H_
#define UTIL_H_

#include <cstdlib> // exit
#include <cstdio> // exit

#define RAND_FLOAT()  (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))

#define safemalloc(n) safe_malloc(n, __LINE__)

#define terminal_error(errstr) __terminal_error(__FILE__, __LINE__, __func__,errstr)

/*#define STATCODE(a) a*/
#define STATCODE(a)

void __terminal_error(const char *file, const unsigned line, const char *func, const char *msg);
int rand_int(int max_int);
int rand_int_range(int min_int, int max_int);
void *safe_malloc(size_t n, unsigned int line);
int *rand_int_vector(int max_int, int n);
int count_lines(FILE *fp);


#endif
