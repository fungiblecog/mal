#include <stddef.h>
#include <assert.h>
#include <gc.h>
#include "iterator.h"

inline Iterator *iterator_next(Iterator *iter) {
  assert(iter != NULL);
  return (*iter->next_fn)(iter);
}

inline void *iterator_value(Iterator *iter) {
  assert(iter != NULL);
  return (iter->value);
}

inline Iterator *iterator_copy(Iterator *iter) {
  assert(iter != NULL);

  Iterator *new_iter = GC_MALLOC(sizeof(*new_iter));

  new_iter->next_fn = iter->next_fn;
  new_iter->current = iter->current;
  new_iter->value = iter->value;

  new_iter->source = iter->source;
  new_iter->data = iter->data;

  return new_iter;
}
