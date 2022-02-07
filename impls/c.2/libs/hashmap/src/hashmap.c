#include <stdio.h>
#include <string.h>
#include <gc.h>
#include <assert.h>
#include "hashmap.h"

/* forward declarations */
static Hashmap *hashmap_copy(Hashmap *map);
static Hashmap *hashmap_put(Hashmap *map, void *key, void *val);

Hashmap *hashmap_make(compare_fn cmp_fn) {

  Hashmap *map = GC_MALLOC(sizeof(*map));
  map->count = 0;
  map->head = NULL;
  map->cmp_fn = cmp_fn;

  return map;
}

int hashmap_count(Hashmap *map) {
  assert(map != NULL);
  return map->count;
}

int hashmap_empty(Hashmap *map) {
  assert(map != NULL);
  return (map->count == 0);
}

Hashmap *hashmap_assoc(Hashmap *map, void *key, void *val) {
  assert(map != NULL);

  /* if key doesn't exist already add new key/value pair */
  if (!hashmap_get(map, key)) {
    return hashmap_put(map, key, val);
  }

  /* make a new map and update the existing key's value */
  Hashmap *new_map = hashmap_copy(map);
  HashEntry *pair = new_map->head;

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
  return new_map;
}

Hashmap *hashmap_dissoc(Hashmap *map, void *key) {
  assert(map != NULL);

  Hashmap *new_map = hashmap_make(map->cmp_fn);
  HashEntry *pair = map->head;

  int found = 0;
  /* walk the internal list */
  while (pair) {

    if (map->cmp_fn(key, pair->key)) {
      /* find a matching key - skip to next */
      pair = pair->next;
      found = 1;
    }
    else {
      /* add to new hashmap */
      new_map = hashmap_put(new_map, pair->key, pair->val);
      pair = pair->next;
    }
  }

  if (found) {
    return new_map;
  }
  else {
    /* don't make a copy if the map didn't change */
    return map;
  }
}

void *hashmap_get(Hashmap *map, void *key) {
  assert(map != NULL);

  HashEntry *pair = map->head;

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

static Iterator *hashmap_next_fn(Iterator *iter) {
  assert(iter != NULL);

  /* iter->current points to a hashmap entry */
  HashEntry *current = iter->current;

  Iterator *new_iter = iterator_copy(iter);

  /* if we're looking for a key, advance the next pointer */
  if (iter->data == 0) {
    current = current->next;

    /* check for the end of the data */
    if (!current) { return NULL; }

    /* get the next key */
    new_iter->current = current;
    new_iter->value = current->key;

    /* next item is a val */
    new_iter->data = (void *)1;
  }
  /* if we're looking for a value no need to advance the pointer */
  else {
    /* return the current val */
    new_iter->value = current->val;

    /* next item is a key */
    new_iter->data = (void *)0;
  }
  return new_iter;
}

Iterator *hashmap_iterator_make(Hashmap *map) {
  assert(map != NULL);

  /* empty hashmap returns a NULL iterator */
  if (hashmap_empty(map)) { return NULL; }

  /* create an iterator */
  Iterator *iter = GC_MALLOC(sizeof(*iter));

  /* install the next function for a hashmap */
  iter->next_fn = hashmap_next_fn;

  /* save the data source - not required for this data structure */
  /* iter->source = map; */

  /* set the entry for the current element */
  iter->current = map->head;

  /* set the current value to the first key */
  HashEntry *curr = iter->current;
  if (!curr) {
    iter->value = NULL;
  }
  else {
    iter->value = curr->key;
  }
  /* use data as a flag that indicates next key (0) or val (1) */
  iter->data = (void *)1;

  return iter;
}

static Hashmap *hashmap_copy(Hashmap *map) {
  assert(map != NULL);

  Hashmap *copy = hashmap_make(map->cmp_fn);

  if (map->count == 0) { return copy; }

  HashEntry *pair = map->head;

  /* walk the internal list */
  while (pair) {
    copy = hashmap_put(copy, pair->key, pair->val);
    pair = pair->next;
  }

  if (map->count == 1) { return copy; }

  /* reverse the list of keys/values */
  HashEntry *prev = NULL, *next = NULL, *current = copy->head;

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

static Hashmap *hashmap_put(Hashmap *map, void *key, void *val) {
  assert(map != NULL);

  /* cannot add a NULL key */
  if (!key) { return map; }

  /* create a new key/val pair */
  HashEntry *pair = GC_MALLOC(sizeof(*pair));

  /* add key/val and point to existing head */
  pair->key = key;
  pair->val = val;
  pair->next = map->head;

  /* create new map */
  Hashmap *new_map = hashmap_make(map->cmp_fn);
  new_map->count = map->count + 1;
  new_map->head = pair;

  return new_map;
}
