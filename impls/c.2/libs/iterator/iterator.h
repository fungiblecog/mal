#ifndef _MAL_ITERATOR_H
#define _MAL_ITERATOR_H

/* simplify references to void pointers */
typedef void* gptr;

typedef struct iterator_s* iterator;

/* a function that takes an iterator and returns the next in sequence */
typedef iterator (*iter_fn)(iterator iter);

/* the structure of an iterator */
struct iterator_s {
  /* the current value pointed at (mandatory) */
  gptr value;
  /* a function to get the next iterator (mandatory) */
  iter_fn next_fn;

  /* a reference to the current position (optional) */
  gptr current;
  /* a reference to the source data collection (optional) */
  gptr source;
  /* arbitrary place to store any data needed by the iterator (optional) */
  gptr data;
};

/* any collection can implement the iterator interface.
   each collection must provide:
   a) a constructor function for the first element and
   b) a function that takes an iterator and returns the next (NULL at the end)
*/

/* public iterator interface */
iterator iterator_next(iterator iter);
gptr iterator_value(iterator iter);
iterator iterator_copy(iterator iter);

#endif
