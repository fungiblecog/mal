#include <stddef.h>
#include <assert.h>
#include <gc.h>
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

inline iterator iterator_copy(iterator iter) {
  assert(iter != NULL);

  iterator new_iter = GC_MALLOC(sizeof(*new_iter));

  new_iter->value = iter->value;
  new_iter->next_fn = iter->next_fn;
  new_iter->current = iter->current;
  new_iter->source = iter->source;
  new_iter->data = iter->data;

  return new_iter;
}
