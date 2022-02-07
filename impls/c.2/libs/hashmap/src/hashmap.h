#ifndef _MAL_HASHMAP_H
#define _MAL_HASHMAP_H

#include "../../iterator/iterator.h"

/* a pointer to a function that compares two values */
typedef int(*compare_fn)(void *, void *);

typedef struct HashEntry_s HashEntry;
typedef struct Hashmap_s Hashmap;

/* a hashmap entry has a key/value pair and a link to the next entry */
struct HashEntry_s {
  void *key;
  void *val;
  HashEntry *next;
};

/* a hashmap is just a header and a list with key/value pairs */
struct Hashmap_s {
  int count;
  compare_fn cmp_fn;
  HashEntry *head;
};

/* interface */
Hashmap *hashmap_make(compare_fn cmp_fn);
int hashmap_empty(Hashmap *map);
int hashmap_count(Hashmap *map);
Hashmap *hashmap_assoc(Hashmap *map, void *key, void *val);
Hashmap *hashmap_dissoc(Hashmap *map, void *key);
void *hashmap_get(Hashmap *map, void *key);
Iterator *hashmap_iterator_make(Hashmap *map);
#endif
