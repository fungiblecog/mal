#ifndef _MAL_VECTOR_H
#define _MAL_VECTOR_H

#include "../iterator/iterator.h"

struct vector_s {

  /* allocated size */
  int size;
  /* number of actual elements */
  int count;
  /* pointer to storage array */
  gptr *data;
};

typedef struct vector_s *vector;

/* interface */
vector vector_make(void);
vector vector_push(vector vec, gptr data_ptr);
vector vector_pop(vector vec);
gptr vector_get(vector vec, int n);
vector vector_set(vector vec, int index, gptr data_ptr);
int vector_empty(vector vec);
int vector_count(vector vec);
vector vector_copy(vector vec);
iterator vector_iterator_make(vector vec);

#endif
