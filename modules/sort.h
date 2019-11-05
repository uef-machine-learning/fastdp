#if ! defined(__SORT_H)
#define __SORT_H

/* ----------------------------------------------------------------- */

void InsertSort(void* base,
                int   nelem,
                int   elemsize,
                void* info,
                int (*cmp)(const void *e1,
                           const void *e2,
                           const void *info));
void QuickSort(void* base,
               int   nelem,
               int   elemsize,
               void* info,
               int (*cmp)(const void *e1,
                          const void *e2,
                          const void *info));

/* ----------------------------------------------------------------- */

#endif /* __SORT_H */

