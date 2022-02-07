#include <stdio.h>
#include <string.h>
#include <gc.h>
#include <assert.h>
#include "vector.h"

#define INITIAL_SIZE 8

Vector *vector_make(void)
{

  /* make an empty vector */
  Vector *vec = GC_MALLOC(sizeof(*vec));

  vec->size = INITIAL_SIZE;
  vec->count = 0;
  vec->data = GC_MALLOC(INITIAL_SIZE*sizeof(void *));

  return vec;
}

static Vector *vector_resize(Vector *vec, int new_size)
{
  void **data = GC_REALLOC(vec->data, sizeof(void *) * new_size);
  if (data) {
    vec->data = data;
    vec->size = new_size;
  }
  return vec;
}

Vector *vector_push(Vector *vec, void *data)
{
  assert(vec != NULL);

  /* reallocate space when required */
  if (vec->count == vec->size) {
    vec = vector_resize(vec, 2 * vec->size);
  }

  /* increment count and add item to end of array */
  vec->data[vec->count] = data;
  vec->count++;

  return vec;
}

Vector *vector_pop(Vector *vec)
{
  assert(vec != NULL);

  if (vec->count == 0) { return vec; }

  /* blank the cell */
  vec->data[vec->count] = NULL;
  vec->count--;
  return vec;
}

void *vector_get(Vector *vec, int idx)
{
  assert(vec!= NULL);

  if ((idx < 0) || (idx >= vec->count)) {
      return NULL;

  } else {
    return vec->data[idx];
  }
}

Vector *vector_set(Vector *vec, int idx, void *data)
{
  assert(vec!= NULL);

  /* index out of bounds */
  if ((idx < 0) || (idx >= vec->count)) {
    return vec;

  } else {
    vec->data[idx] = data;
    return vec;
  }
}

int vector_empty(Vector *vec)
{
  assert(vec != NULL);
  return (vec->count == 0);
}

int vector_count(Vector *vec)
{
  assert(vec!= NULL);
  return vec->count;
}

Vector *vector_copy(Vector *vec)
{
  assert(vec!= NULL);

  /* create a new vector */
  Vector *copy = vector_make();
  copy->size = vec->size;
  copy->count = vec->count;

  /* copy the rest of the data */
  for (int i = 0; i < vec->count; i++) {
    copy->data[i] = vec->data[i];
  }

  /* NOT WORKING */
  /* copy->data = memcpy(copy->data, vec->data, vec->size); */

  return copy;
}

static Iterator *vector_next_fn(Iterator *iter) {
  assert(iter != NULL);

  /* check for end of the array */
  void *curr = iter->current + sizeof(void *);

  if (curr == iter->data) { return NULL; }

  Iterator *new_iter = iterator_copy(iter);

  /* increment the current pointer */
  new_iter->current = curr;

  /* set the next value */
  char* val = *(char**)curr;
  new_iter->value = val;

  return new_iter;
}

Iterator *vector_iterator_make(Vector *vec)
{
  assert(vec != NULL);

  /* empty vector returns a NULL iterator */
  if (vector_empty(vec)) { return NULL; }

  /* create an iterator */
  Iterator *iter = GC_MALLOC(sizeof(*iter));

  /* install the next function for a vector */
  iter->next_fn = vector_next_fn;

  /* save the data source */
  iter->source = vec;

  /* set current to a pointer to the start of the array */
  iter->current = vec->data;

  /* set value to the value of the first array element */
  iter->value = vec->data[0];

  /* data holds a pointer one element past the end of the array */
  iter->data = &vec->data[vec->count];

  return iter;
}
