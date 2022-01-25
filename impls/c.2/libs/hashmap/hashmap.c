#include <stdio.h>
#include <string.h>
#include <gc.h>
#include <assert.h>
#include "hashmap.h"

int cmp_chars(gptr key1, gptr key2) {

  char* keystring1 = (char*)key1;
  char* keystring2 = (char*)key2;

  return (strcmp(keystring1, keystring2) == 0);
}

hashmap hashmap_make(compare_fn cmp_fn) {

  hashmap map = GC_MALLOC(sizeof(*map));
  map->count = 0;
  map->head = NULL;
  map->cmp_fn = cmp_fn;

  return map;
}

hashmap hashmap_put(hashmap map, gptr key, gptr val) {
  assert(map != NULL);

  /* cannot add a NULL key */
  if (!key) { return map; }

  /* create a new key/val pair */
  entry pair = GC_MALLOC(sizeof(*pair));

  /* add key/val and point to existing head */
  pair->key = key;
  pair->val = val;
  pair->next = map->head;

  /* create new map */
  hashmap new_map = hashmap_make(map->cmp_fn);
  new_map->count = map->count + 1;
  new_map->head = pair;

  return new_map;
}

gptr hashmap_get(hashmap map, gptr key) {
  assert(map != NULL);

  entry pair = map->head;

  /* walk the internal list */
  while (pair) {

    if (map->cmp_fn(key, pair->key)) {
      /* find a matching key */
      return pair->val;
    }
    else {
      /* move to the next entry */
      pair = pair->next;
    }
  }
  /* not found */
  return NULL;
}

hashmap hashmap_update(hashmap map, gptr key, gptr val) {
  assert(map != NULL);

  hashmap new_map = hashmap_copy(map);
  entry pair = new_map->head;

  /* walk the internal list */
  while (pair) {

    if (new_map->cmp_fn(key, pair->key)) {
      /* update the data */
      pair->val = val;
      return new_map;
    }
    else {
      /* move to the next item */
      pair = pair->next;
    }
  }
  /* if key doesn't exist already add new key/value pair */
  return hashmap_put(new_map, key, val);
}

hashmap hashmap_copy(hashmap map) {
  assert(map != NULL);

  hashmap copy = hashmap_make(map->cmp_fn);

  if (map->count == 0) { return copy; }

  entry pair = map->head;

  /* walk the internal list */
  while (pair) {
    copy = hashmap_put(copy, (gptr)pair->key, (gptr)pair->val);
    pair = pair->next;
  }

  if (map->count == 1) { return copy; }

  /* reverse the list of keys/values */
  entry prev = NULL, next = NULL, current = copy->head;

  while (current) {

    /* stash current value of next pointer --> */
    next = current->next;

    /* reverse the next pointer on current pair <-- */
    current->next = prev;

    /* move on to next pair and repeat --> */
    prev = current;
    current = next;
  }
  /* new head of list is in prev when current = NULL */
  copy->head = prev;
  return copy;
}

int hashmap_count(hashmap map) {
  assert(map != NULL);
  return map->count;
}

int hashmap_empty(hashmap map) {
  assert(map != NULL);
  return (map->count == 0);
}

hashmap hashmap_delete(hashmap map, gptr key) {
  assert(map != NULL);

  hashmap new_map = hashmap_make(map->cmp_fn);
  entry pair = map->head;

  /* walk the internal list */
  while (pair) {

    if (map->cmp_fn(key, pair->key)) {
      /* find a matching key - skip to next */
      pair = pair->next;
    }
    else {
      /* add to new hashmap */
      new_map = hashmap_put(new_map, pair->key, pair->val);
      pair = pair->next;
    }
  }
  return new_map;
}

static iterator hashmap_next_fn(iterator iter) {
  assert(iter != NULL);

  /* iter->current points to a hashmap entry */
  entry current = iter->current;

  iterator new_iter = iterator_copy(iter);

  /* if we're looking for a key, advance the next pointer */
  if (iter->data == 0) {
    current = current->next;

    /* check for the end of the data */
    if (!current) { return NULL; }

    /* get the next key */
    new_iter->current = current;
    new_iter->value = current->key;

    /* next item is a val */
    new_iter->data = (gptr)1;
  }
  /* if we're looking for a value no need to advance the pointer */
  else {
    /* return the current val */
    new_iter->value = current->val;

    /* next item is a key */
    new_iter->data = (gptr)0;
  }
  return new_iter;
}

iterator hashmap_iterator_make(hashmap map) {
  assert(map != NULL);

  /* empty hashmap returns a NULL iterator */
  if (hashmap_empty(map)) { return NULL; }

  /* create an iterator */
  iterator iter = GC_MALLOC(sizeof(*iter));

  /* install the next function for a hashmap */
  iter->next_fn = hashmap_next_fn;

  /* save the data source - not required for this data structure */
  /* iter->source = map; */

  /* set the entry for the current element */
  iter->current = map->head;

  /* set the current value to the first key */
  entry curr = iter->current;
  if (!curr) {
    iter->value = NULL;
  }
  else {
    iter->value = curr->key;
  }
  /* use data as a flag that indicates next key (0) or val (1) */
  iter->data = (gptr)1;

  return iter;
}
