#ifndef _MAL_HASHMAP_H
#define _MAL_HASHMAP_H

#include "../iterator/iterator.h"

/* simplify references to void pointers */
typedef void* gptr;

/* a pointer to a function that compares two values */
typedef int(*compare_fn)(gptr, gptr);

/* a hashmap entry has a key/value pair and a link to the next entry */
typedef struct entry_s* entry;

struct entry_s {
  gptr key;
  gptr val;
  entry next;
};

/* a hashmap is just a header and a list with key/value pairs */
typedef struct hashmap_s* hashmap;

struct hashmap_s {
  int count;
  compare_fn cmp_fn;
  entry head;
};

hashmap hashmap_make(compare_fn cmp_fn);
hashmap hashmap_put(hashmap map, gptr key, gptr val);
gptr hashmap_get(hashmap map, gptr key);
int hashmap_empty(hashmap map);
hashmap hashmap_delete(hashmap map, gptr key);
hashmap hashmap_update(hashmap map, gptr key, gptr val);
hashmap hashmap_copy(hashmap map);
int hashmap_count(hashmap map);
iterator hashmap_iterator_make(hashmap map);
int cmp_chars(gptr key1, gptr key2);
#endif
