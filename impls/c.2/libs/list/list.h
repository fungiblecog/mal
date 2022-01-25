#ifndef _MAL_LINKED_LIST_H
#define _MAL_LINKED_LIST_H

#include "../iterator/iterator.h"

/* simplify references to void pointers */
typedef void* gptr;

/* a function pointer that converts a gptr to a char* */
typedef int (*cmp_fn)(gptr, gptr);

/* typedef for structs */
typedef struct pair_s pair;
typedef pair* list;

/* linked list is constructed of pairs */
struct pair_s {
  gptr data;
  pair* next;
};

/* Note: an empty list is NULL */

/* interface */
list list_make(gptr val);
int list_count(list lst);
int list_empty(list lst);
list list_cons(list lst, gptr val);
gptr list_nth(list lst, int n);
gptr list_first(list lst);
list list_rest(list lst);
list list_reverse(list lst); /* note: destructive */
list list_copy(list lst);
list list_concatenate(list lst1, list lst2);
int list_find(list lst, gptr key, cmp_fn fn);
iterator list_iterator_make(list lst);
#endif
