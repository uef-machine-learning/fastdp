#if ! defined(__CBSORT_H)
#define __CBSORT_H

/* ----------------------------------------------------------------- */

#include "cb.h"

typedef enum  { DATA_DESCENDING=1, FREQ_DESCENDING=2,
                DATA_ASCENDING =3, FREQ_ASCENDING =4,
                VECTOR_MEAN=5 } SORT_MODE;

void  SortCodebook(CODEBOOK* CB, int  Mode);

/* ----------------------------------------------------------------- */

#endif /* __CBSORT_H */

