#include <stdio.h>
#include <string.h>
#include <gc.h>
#include <assert.h>
#include "vector.h"

#define INITIAL_SIZE 8

vector vector_make()
{

  /* make an empty vector */
  vector vec = GC_MALLOC(sizeof(*vec));

  vec->size = INITIAL_SIZE;
  vec->count = 0;
  vec->data = GC_MALLOC(INITIAL_SIZE*sizeof(gptr));

  return vec;
}

static vector vector_resize(vector vec, int new_size)
{
  gptr *data = GC_REALLOC(vec->data, sizeof(gptr) * new_size);
    if (data) {
      vec->data = data;
      vec->size = new_size;
    }
    return vec;
}

vector vector_push(vector vec, gptr data_ptr)
{
  assert(vec != NULL);

  /* reallocate space when required */
  if (vec->count == vec->size) {
    vec = vector_resize(vec, 2 * vec->size);
  }

  /* increment count and add item to end of array */
  vec->data[vec->count] = data_ptr;
  vec->count++;

  return vec;
}

vector vector_pop(vector vec)
{
  assert(vec != NULL);

  if (vec->count == 0) { return vec; }

  /* blank the cell */
  vec->data[vec->count] = NULL;
  vec->count--;
  return vec;
}

gptr vector_get(vector vec, int n)
{
  assert(vec!= NULL);

  if( (n < 0) || (n >= vec->count) ) {
      return NULL;

  } else {
    return vec->data[n];
  }
}

vector vector_set(vector vec, int n, gptr data_ptr)
{
  assert(vec!= NULL);

  /* index out of bounds */
  if ( (n < 0) || (n >= vec->count) ) {
    return vec;

  } else {
    vec->data[n] = data_ptr;
    return vec;
  }
}

int vector_empty(vector vec)
{
  assert(vec != NULL);

  return (vec->count == 0);
}

int vector_count(vector vec)
{
  assert(vec!= NULL);

  return vec->count;
}

vector vector_copy(vector vec)
{
  assert(vec!= NULL);

  /* create a new vector */
  vector copy = vector_make();
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

static iterator vector_next_fn(iterator iter) {
  assert(iter != NULL);

  /* check for end of the array */
  gptr curr = iter->current + sizeof(gptr);

  if (curr == iter->data) { return NULL; }

  iterator new_iter = iterator_copy(iter);

  /* increment the current pointer */
  new_iter->current = curr;

  /* set the next value */
  char* val = *(char**)curr;
  new_iter->value = val;

  return new_iter;
}

iterator vector_iterator_make(vector vec)
{
  assert(vec != NULL);

  /* empty vector returns a NULL iterator */
  if (vector_empty(vec)) { return NULL; }

  /* create an iterator */
  iterator iter = GC_MALLOC(sizeof(*iter));

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
