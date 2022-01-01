#include <stddef.h>
#include <assert.h>
#include "iterator.h"

/* public iterator interface */
inline iterator iterator_next(iterator iter) {
  assert(iter != NULL);
  return (*iter->next_fn)(iter);
}

inline gptr iterator_value(iterator iter) {
  assert(iter != NULL);
  return (iter->value);
}
