#include <stdio.h>
#include <string.h>
#include <gc.h>
#include <assert.h>
#include "hashmap.h"

hashmap hashmap_make() {

  hashmap map = GC_MALLOC(sizeof(*map));
  map->count = 0;
  map->head = NULL;

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
  hashmap new_map = hashmap_make();
  new_map->count = map->count + 1;
  new_map->head = pair;

  return new_map;
}

gptr hashmap_get(hashmap map, char* keystring) {
  assert(map != NULL);

  entry pair = map->head;
  /* walk the internal list */
  while(pair) {

    if (strcmp(keystring, (char*)pair->key) == 0) {
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

gptr hashmap_getf(hashmap map, gptr key, char_fn fn) {
  assert(map != NULL);

  entry pair = map->head;
  /* walk the internal list */
  while(pair) {

    /* apply fn to the keys to get strings */
    char* keystring = fn(key);
    char* item = fn(pair->key);

    if (strcmp(keystring, item) == 0) {
      /* found a matching key */
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

//TODO: make non-destructive */
hashmap hashmap_updatef(hashmap map, gptr key, gptr val, char_fn fn) {

  entry pair = map->head;

  /* walk the internal list */
  while(pair) {

    /* apply fn to the keys to get strings */
    char* keystring = fn(key);
    char* item = fn(pair->key);

    if (strcmp(keystring, item) == 0) {
      /* update the data */
      pair->val = val;
      return map;
    }
    else {
      /* move to the next item */
      pair = pair->next;
    }
  }
  /* if key doesn't exist already add new key/value pair */
  return hashmap_put(map, key, val);
}

hashmap hashmap_copy(hashmap map) {
  assert(map != NULL);

  hashmap copy = hashmap_make();

  if (map->count == 0) { return copy; }

  entry pair = map->head;

  /* walk the internal list */
  while(pair) {
    copy = hashmap_put(copy, (gptr)pair->key, (gptr)pair->val);
    pair = pair->next;
  }

  if (map->count == 1) { return copy; }

  /* reverse the list of keys/values */
  entry prev = NULL, next = NULL, current = copy->head;

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
  copy->head = prev;
  return copy;
}

int hashmap_count(hashmap map) {
  assert(map != NULL);
  return map->count;
}

static iterator hashmap_next_fn(iterator iter) {
  assert(iter != NULL);

  /* iter->current points to a hashmap entry */
  entry current = iter->current;

  /* if we're looking for a key, advance the next pointer */
  if (iter->data == 0) {
    current = current->next;

    /* check for the end of the data */
    if (!current) { return NULL; }

    /* get the next key */
    iter->current = current;
    iter->value = current->key;

    /* next item is a val */
    iter->data = (gptr)1;
  }
  /* if we're looking for a value no need to advance the pointer */
  else {
    /* return the current val */
    iter->value = current->val;

    /* next item is a key */
    iter->data = (gptr)0;
  }

  return iter;
}

iterator hashmap_iterator_make(hashmap map) {
  assert(map != NULL);

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
