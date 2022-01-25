#ifndef _MAL_LINKED_LIST_H
#define _MAL_LINKED_LIST_H

#include "../iterator/iterator.h"

/* simplify references to void pointers */
typedef void* gptr;

/* a function pointer that converts a gptr to a char* */
typedef char*(*char_fn)(gptr);

/* linked list is constructed of pairs */
typedef struct pair_s {
  gptr data;
  struct pair_s* next;
} pair;

/* a list is just a pointer to the pair at the head of the list */
typedef pair *list;

/* an empty list is NULL */

/* interface */
list list_make(gptr data_ptr);
int list_empty(list lst);
list list_cons(list lst, gptr data_ptr);
gptr list_first(list lst);
list list_rest(list lst);
int list_count(list lst);
gptr list_nth(list lst, int n);
list list_reverse(list lst); /* note: destructive */
list list_copy(list lst);
list list_concatenate(list lst1, list lst2);
int list_findf(list lst, char* keystring, char_fn fn);
iterator list_iterator_make(list lst);

#endif
