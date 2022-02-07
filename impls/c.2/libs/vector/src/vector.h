#ifndef _MAL_VECTOR_H
#define _MAL_VECTOR_H

#include "../../iterator/iterator.h"

/* a vector is a header and an array of data */
typedef struct {
  /* allocated size */
  int size;
  /* number of actual elements */
  int count;
  /* pointer to storage array */
  void **data;
} Vector;

/* interface */
Vector *vector_make(void);
int vector_count(Vector *vec);
int vector_empty(Vector *vec);
Vector *vector_push(Vector *vec, void *val);
Vector *vector_pop(Vector *vec);
void *vector_get(Vector *vec, int idx);
Vector *vector_set(Vector *vec, int idx, void *val);
Vector *vector_copy(Vector *vec);
Iterator *vector_iterator_make(Vector *vec);
#endif
