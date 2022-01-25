#ifndef _MAL_VECTOR_H
#define _MAL_VECTOR_H

#include "../iterator/iterator.h"

/* simplify references to void pointers */
typedef void* gptr;

/* typedefs for structs */
typedef struct vector_s *vector;

/* a vector is a header and an array of data */
struct vector_s {
  /* allocated size */
  int size;
  /* number of actual elements */
  int count;
  /* pointer to storage array */
  gptr *data;
};

/* interface */
vector vector_make(void);
int vector_count(vector vec);
int vector_empty(vector vec);
vector vector_push(vector vec, gptr val);
vector vector_pop(vector vec);
gptr vector_get(vector vec, int idx);
vector vector_set(vector vec, int idx, gptr val);
vector vector_copy(vector vec);
iterator vector_iterator_make(vector vec);
#endif
