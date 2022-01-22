#ifndef _MAL_HASHMAP_H
#define _MAL_HASHMAP_H

#include "../iterator/iterator.h"

/* simplify references to void pointers */
typedef void* gptr;

/* a function pointer that converts a gptr to a char* */
typedef char*(*char_fn)(gptr);

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
  entry head;
};

hashmap hashmap_make(void);
hashmap hashmap_put(hashmap map, gptr key, gptr val);
gptr hashmap_get(hashmap map, char* keystring);
gptr hashmap_getf(hashmap map, gptr key, char_fn fn);
int hashmap_empty(hashmap map);
hashmap hashmap_delete(hashmap map, char* keystring);
hashmap hashmap_deletef(hashmap map, gptr key, char_fn fn);
hashmap hashmap_updatef(hashmap map, gptr key, gptr val, char_fn fn);
hashmap hashmap_copy(hashmap map);
int hashmap_count(hashmap map);
iterator hashmap_iterator_make(hashmap map);

#endif
