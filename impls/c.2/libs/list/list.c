#include <stdio.h>
#include <string.h>
#include <gc.h>
#include <assert.h>
#include "list.h"

/* create a new 1 element list. note an empty list is just NULL */
list list_make(gptr data_ptr) {
  return list_cons(NULL, data_ptr);
}

/* returns true if list is empty (NULL) */
int list_empty(list lst) {
  return (!lst);
}

/* return a list with an element added at the head */
list list_cons(list lst, gptr val) {

  pair* head = GC_malloc(sizeof(pair));
  head->data = val;
  head->next = lst;

  return head;
}

/* return the first element in the list (the head) */
gptr list_first(list lst) {
  return (lst ? lst->data : NULL);
}

/* return a list of all emements except the head */
list list_rest(list lst) {
  return (lst ? lst->next : NULL);
}

/* return a count of the elements in the list */
int list_count(list lst) {

  /* handle empty case */
  if (!lst) { return 0; }

  int ctr = 1;

  /* walk the list counting elements */
  while(lst->next) {
    ctr++;
    lst = lst->next;
  }
  return ctr;
}

/* return the nth element counting from the head */
gptr list_nth(list lst, int n) {

  /* handle empty case */
  if(!lst) { return NULL; }

  int idx = 0;

  /* walk the list to get nth element */
  while (lst) {

    /* found */
    if (n == idx) {
      return lst->data;
    }
    /* keep walking */
    idx++;
    lst = lst->next;
  }
  /* reached end of list before n */
  return NULL;
}

/* return the list reversed *destructive* */
list list_reverse(list lst) {

  /* list is not empty and has more than one element */
  if (lst && lst->next) {

    pair *prev = NULL, *next = NULL, *current = lst;

    while(current) {

      /* stash current value of next pointer --> */
      next = current->next;

      /* reverse the next pointer on current pair <-- */
      current->next = prev;

      /* move on to next pair and repeat --> */
      prev = current;
      current = next;
    }
    /* new head of list is in prev when current = NULL */
    lst = prev;
  }
  return lst;
}

/* return a new list with identical contents */
list list_copy(list lst) {

  /* handle empty case */
  if(!lst) { return NULL; }

  list copy = NULL;

  /* walk the list */
  while(lst) {

    /* push the elements to the new list */
    copy = list_cons(copy, lst->data);
    lst = lst->next;
  }
  /* note that the copy is backwards */
  return list_reverse(copy);
}

/* return a list which is the input lists joined together */
list list_concatenate(list lst_1, list lst_2) {

  /* make a copy of the second list */
  list lst_cat = NULL;
  lst_cat = list_copy(lst_2);

  /* push the reverse of the first list onto the copy of the second list */
  lst_1 = list_reverse(lst_1);

  list lst_iter = lst_1;
  while (lst_iter) {

    gptr val = lst_iter->data;
    lst_cat = list_cons(lst_cat, val);
    lst_iter = lst_iter->next;
  }

  /* restore the original first list */
  lst_1 = list_reverse(lst_1);

  /* return the concatenated list */
  return lst_cat;
}
/* return the position of the element matching
   keystring when fn is applied to the data */
//int list_findf(list lst, char* keystring, char_fn fn) {
int list_find(list lst, gptr val, cmp_fn fn) {

  /* handle empty cases */
  if (!lst || !val) { return -1; }

  list current = lst;
  long ctr = 0;
  /* walk the list */
  while (current) {

    if (fn(val, current->data)) {
      /* return the 0-indexed position of the first match */
      return ctr;
    }
    else {
      /* skip to the next item in the list */
      current = current->next;
      ctr++;
    }
  }
  /* not found */
  return -1;
}

static iterator list_next_fn(iterator iter) {
  assert(iter != NULL);

  /* iter->current points to the current pair */
  list current = iter->current;

  /* advance the pointer */
  current = current->next;

  /* check for end of the list */
  if (!current) { return NULL; }

  iterator new_iter = iterator_copy(iter);

  /* set the next pair */
  new_iter->current = current;

  /* set the next value */
  new_iter->value = current->data;

  return new_iter;
}


iterator list_iterator_make(list lst)
{
  /* empty list returns a NULL iterator */
  if (!lst) { return NULL; }

  /* create an iterator */
  iterator iter = GC_MALLOC(sizeof(*iter));

  /* install the next function for a list */
  iter->next_fn = list_next_fn;

  /* source not needed */
  /* iter->source = lst; */

  /* set current to a pointer to the head of the list */
  iter->current = lst;

  /* set value to the value of list head */
  iter->value = lst->data;

  /* data not needed */
  /* iter->data = */

  return iter;
}
