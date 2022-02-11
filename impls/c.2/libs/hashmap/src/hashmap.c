/*
    Copyright (C) 2020 Duncan Watts

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, version 3 or later.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gc.h>
#include <assert.h>

#include "hashmap.h"

/*
   for explanations see:
   http://blog.higher-order.net/2009/09/08/understanding-clojures-persistenthashmap-deftwice
*/

/* constant NodeTypes */

/* Leaf nodes store key/value pairs */
NodeType NT_LEAF = {leaf_get, leaf_assoc, leaf_dissoc, leaf_visit};

/* Bitmap indexed nodes hold pointers to up to 32 sub-nodes */
NodeType NT_BITMAP_INDEXED = {bitmap_indexed_get, bitmap_indexed_assoc, \
                              bitmap_indexed_dissoc, bitmap_indexed_visit};

/* Hash collision nodes replace leaf nodes when there is a collision */
NodeType NT_HASH_COLLISION = {hash_collision_get, hash_collision_assoc, \
                              hash_collision_dissoc, hash_collision_visit};

/*
   default hash implementation djb2 * this algorithm was
   first reported by Dan Bernstein * many years ago in comp.lang.c
*/
static hash_t hash_str(void *obj) {

  hash_t hash = 5381;

  int c;
  while ((c = *(char*)obj++)) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

/* default comparison operation */
static int equal_str(void *obj1, void *obj2) {

  return (strcmp((char *)obj1, (char *)obj2) == 0);
}

/* generic utilities */

/* count 1's in x efficiently */
#define popcount(x) __builtin_popcount(x)

/* uncomment this if on something other than gcc */
/* int popcount(unsigned x) { */
/*   int c = 0; */
/*   for (; x != 0; x &= x - 1) { c++; } */
/*   return c; */
/* } */

/* shift hash by 'level'. Each level is BITS_PER_LEVEL bits
   which is 5 by default giving [0-31] children */
int mask(hash_t hash, int level){
  return (hash >> (BITS_PER_LEVEL * level)) & 0x01f;
}

/* map a hash code [0-31] to a bit in the bitmap 2^[0-31] */
int bitpos(hash_t hash, int level){
  return 1 << mask(hash, level);
}

/* return the number of 1's in bitmap less than bit */
int bit_index(int bitmap, int bit){
  return popcount(bitmap & (bit - 1));
}

/* external interface */
Hashmap *hashmap_make(hash_fn hash, equal_fn eq_keys, equal_fn eq_vals) {

  Hashmap *map = GC_MALLOC(sizeof(Hashmap));

  map->count = 0;
  map->root = NULL;

  map->hash = hash ? hash : hash_str;
  map->eq_key = eq_keys ? eq_keys : equal_str;
  map->eq_val = eq_vals ? eq_vals : equal_str;

  return map;
}

Hashmap *hashmap_assoc(Hashmap* map, void *key, void *val) {

  /* result indicates if the map changed during the operation
   result = UNCHANGED - no change
   result = ADDED     - added a key/value
   result = UPDATED   - updated an existing value
   result = REMOVED   - removed a key/value
  */
  int result = UNCHANGED;
  Node *root = NULL;

  /* if there are no entries create a leaf node */
  if (!map->root) {

    LeafNode* leaf = new_leaf_node(key, val, map->hash(key));
    result = ADDED;
    root = (Node*)leaf;
  }
  /* otherwise call assoc on the root node */
  else {
      root = (map->root)->type->assoc(map->root, 0, key, val, map->hash(key), \
                                      map->eq_key, map->eq_val, &result);
  }

  /* if there was a change create a new hashmap */
  if (result != UNCHANGED) {

    Hashmap *new = GC_MALLOC(sizeof(Hashmap));
    memcpy(new, map, sizeof(*new));
    new->root = root;

    /* increase the count if there was an addition */
    if (result == ADDED) {
      new->count++;
    }

    return new;

  } else {

    return map;
  }
}

Hashmap *hashmap_dissoc(Hashmap *map, void *key) {

  /* result is a boolean that indicates if the map changed during the operation */
  int result = UNCHANGED;

  /* if there are no entries there's nothing to dissoc */
  if (!map->root) { return map; }

  /* otherwise call dissoc on the root node */
  Node *root = (map->root)->type->dissoc(map->root, 0, key, map->hash(key), \
                                         map->eq_key, map->eq_val, &result);

  /* if there was a change create a new hashmap */
  if (result != UNCHANGED) {

    Hashmap *new = GC_MALLOC(sizeof(Hashmap));
    memcpy(new, map, sizeof(*new));
    new->root = root;
    new->count--;
    return new;

  } else {

    return map;
  }
}

void *hashmap_get(Hashmap *map, void *key) {

  if (!map->root) { return NULL; }

  return (map->root)->type->get(map->root, 0, key, map->hash(key), \
                                map->eq_key, map->eq_val);
}

int hashmap_count(Hashmap *map) {
  return map->count;
}

int hashmap_empty(Hashmap *map) {
  return (map->count == 0);
}

void hashmap_visit(Hashmap *map, visit_fn fn, void** acc) {

  if (!map->root) { return; }

  (map->root)->type->visitor(map->root, fn, acc);
}

/* basic list structure for implementing an iterator */
typedef struct iter_list {
  void *data;
  struct iter_list *next;
} iter_list;

/* function to visit each node and create a list of key/val pairs */
static void hashmap_iter_visit(void *key, void *val, void **acc) {

  /* acc holds a pointer to the list being generated */
  iter_list *lst = *(struct iter_list **)acc;

  /* create a list node to hold the next value */
  iter_list *lst_val = GC_MALLOC(sizeof(iter_list));
  lst_val->data = val;
  lst_val->next = lst;

  /* create a list node to hold the next key */
  iter_list *lst_key = GC_MALLOC(sizeof(iter_list));
  lst_key->data = key;
  lst_key->next = lst_val;

  /* return the updated list in acc */
  *acc = lst_key;
}

/* function to advance the iterator */
static Iterator *hashmap_next_fn(Iterator *iter) {
  assert(iter != NULL);

  /* iter->current points to a hashmap entry */
  iter_list *current = iter->current;

  /* check for the end of the data */
  if (!current->next) { return NULL; }

  /* advance the pointer */
  current = current->next;

  /* create a new iterator */
  Iterator *new = iterator_copy(iter);

  /* set the iterator values */
  new->current = current;
  new->value = current->data;

  return new;
}

Iterator *hashmap_iterator_make(Hashmap *map) {
  assert(map != NULL);

  /* empty hashmap returns a NULL iterator */
  if (hashmap_empty(map)) { return NULL; }

  /* create an iterator */
  Iterator *iter = GC_MALLOC(sizeof(Iterator));

  /* install the next function for a hashmap */
  iter->next_fn = hashmap_next_fn;

  /* save the data source - not required for this data structure */
  /* iter->source = map; */

  /* create a list of key/val pairs */
  iter_list *lst = NULL;
  hashmap_visit(map, hashmap_iter_visit, (void **)&lst);

  /* if there are no key/val pairs */
  if (!lst) { return NULL; }

  /* set the entry for the current element */
  iter->current = lst;
  iter->value = lst->data;

  /* not using data */
  /* iter->data = (void *)1; */

  return iter;
}

/* internal implementation */
LeafNode *new_leaf_node(void *key, void *val, hash_t hash) {

  LeafNode *node = GC_MALLOC(sizeof(LeafNode));
  node->type = &NT_LEAF;
  node->key = key;
  node->val = val;
  node->hash = hash;

  return node;
}

BitmapIndexedNode *new_bitmap_indexed_node() {

  BitmapIndexedNode *node = GC_MALLOC(sizeof(BitmapIndexedNode));
  node->type = &NT_BITMAP_INDEXED;
  node->bitmap = 0;

  return node;
}

HashCollisionNode *new_hash_collision_node(void *key, void* val, hash_t hash) {

  HashCollisionNode *node = GC_MALLOC(sizeof(HashCollisionNode));
  node->type = &NT_HASH_COLLISION;
  node->key = key;
  node->val = val;
  node->hash = hash;

  node->next = NULL;
  return node;
}

void *leaf_get(Node *self, int level, void *key, hash_t hash, \
               equal_fn eq_key, equal_fn eq_val) {

  LeafNode* node = (LeafNode*)self;

  if (eq_key(node->key, key)) {
    /* found */
    return node->val;
  }
  else {
    /* not found */
    return NULL;
  }
}

void *bitmap_indexed_get(Node *self, int level, void *key, hash_t hash, \
                         equal_fn eq_key, equal_fn eq_val) {

  BitmapIndexedNode *node = (BitmapIndexedNode*)self;

  int bit = bitpos(hash, level);

  if(node->bitmap & bit) {
    int idx = bit_index(node->bitmap, bit);
    Node *child = node->children[idx];

    /* keep looking at next level */
    return child->type->get(child, (level + 1), key, hash, eq_key, eq_val);
  }
  /* not found */
  else {
    return NULL;
  }
}

void *hash_collision_get(Node *self, int level, void *key, hash_t hash, \
                         equal_fn eq_key, equal_fn eq_val) {

  HashCollisionNode *node = (HashCollisionNode*)self;

  /* found key in first node */
  if (eq_key(node->key, key)) {
    return node->val;
  }
  /* search through the linked list of nodes */
  else if (node->next) {
    return hash_collision_get((Node*)node->next, level, key, hash, eq_key, eq_val);
  }
  /* not found */
  else {
    return NULL;
  }
}

Node *leaf_assoc(Node *self, int level, void *key, void *val, hash_t hash, \
                 equal_fn eq_key, equal_fn eq_val, int *result) {

  LeafNode *node = (LeafNode*)self;

  /* if the hash doesn't match this leaf create a BitmapIndexedNode to
     hold the existing LeafNode and the new LeafNode and return it */
  if (node->hash != hash) {

    BitmapIndexedNode *parent = new_bitmap_indexed_node();

    /* calculate bit-positions of new nodes */
    int node_bit = bitpos(node->hash, level);
    int new_bit = bitpos(hash, level);

    /* can't put two leaves in the same bitmap if they
       have the same bit position */
    if (node_bit != new_bit) {

      /* allocate enough space for two nodes */
      parent->children = GC_MALLOC(sizeof(LeafNode) * 2);

      /* update the bitmap */
      parent->bitmap |= node_bit;
      parent->bitmap |= new_bit;

      /* work out the new indices */
      int node_idx = bit_index(parent->bitmap, node_bit);
      int new_idx = bit_index(parent->bitmap, new_bit);

      /* create new child node */
      LeafNode *new = new_leaf_node(key, val, hash);

      /* add the leaf nodes */
      parent->children[node_idx] = self;
      parent->children[new_idx] = (Node*)new;

      *result = ADDED;
      return (Node*)parent;
    }
    /* add a node at level + 1 */
    else {
      /* allocate enough space for single node */
      parent->children = GC_MALLOC(sizeof(BitmapIndexedNode));

      /* update the bitmap */
      parent->bitmap |= node_bit;

      /* work out the new index */
      int idx = bit_index(parent->bitmap, node_bit);

      /* add the original leaf node */
      parent->children[idx] = self;

      /* call assoc again on the new bitmap_indexed_node */
      return (Node*)(parent->type->assoc((Node*)parent, level, key, val, hash, \
                                         eq_key, eq_val, result));
    }
  }

  /* if the key/value pair already exists return the original node */
  else if (eq_key(node->key, key) && eq_val(node->val, val)) {

    *result = UNCHANGED;
    return self;
  }
 /* if the key exists with a different value return a copy with the new value */
  else if (eq_key(node->key, key)) {

    LeafNode *new = new_leaf_node(key, val, hash);
    *result = UPDATED;
    return (Node*)new;
  }
  /* if the key is different but has the same hash we have a collision
     so create a pair of linked HashCollisionNodes and return the first one */
  else {

    HashCollisionNode *original = new_hash_collision_node(node->key, node->val, node->hash);
    HashCollisionNode *new = new_hash_collision_node(key, val, hash);
    new->next = original;

    *result = ADDED;
    return (Node*)new;
  }
}

Node *bitmap_indexed_assoc(Node *self, int level, void *key, void *val, hash_t hash, \
                           equal_fn eq_key, equal_fn eq_val, int *result) {

  BitmapIndexedNode *node = (BitmapIndexedNode*)self;

  int bit = bitpos(hash, level);
  int idx = bit_index(node->bitmap, bit);

  /* if a child node already exists at this bitpos */
  if (node->bitmap & bit) {

    /* assoc at the existing child */
    Node *child = node->children[idx];
    Node *new = child->type->assoc(child, (level + 1), key, val, hash,  \
                                   eq_key, eq_val, result);

    /* change was made to the child node */
    if (*result != UNCHANGED) {

      /* copy the node */
      BitmapIndexedNode *copy = GC_MALLOC(sizeof(BitmapIndexedNode));
      memcpy(copy, node, sizeof(BitmapIndexedNode));

      /* count the number of 1s in the bitmap */
      int nodes = popcount(node->bitmap);

      /* allocate storage dynamically */
      copy->children = GC_MALLOC(sizeof(Node*) * nodes);

      /* copy existing child nodes before the new node index */
      for (int i=0; i < idx; i++) {
	copy->children[i] = node->children[i];
      }

      /* add the new child node at idx */
      copy->children[idx] = new;

      /* copy the existing child nodes after the new node index */
      for (int i=idx+1; i < nodes; i++) {
	copy->children[i] = node->children[i];
      }
      return (Node*)copy;
    }
    /* key not found or key/value pair already exists */
    else {
      return self;
    }
  }
  /* create a new child node */
  else {

    /* copy the node */
    BitmapIndexedNode *new = GC_MALLOC(sizeof(BitmapIndexedNode));
    memcpy(new, node, sizeof(BitmapIndexedNode));

    /* set the new node in the bitmap */
    new->bitmap |= bit;

    /* count the number of 1s in the bitmap */
    int nodes = popcount(new->bitmap);

    /* allocate storage dynamically */
    new->children = GC_MALLOC(sizeof(Node*) * nodes);

    /* copy existing child nodes before the new node index */
    for (int i=0; i < idx; i++) {
      new->children[i] = node->children[i];
    }

    /* create a new child node at idx */
    LeafNode* child= new_leaf_node(key, val, hash);
    new->children[idx] = (Node*)child;

    /* copy the existing child nodes after the new node index */
    for (int i=idx+1; i < nodes; i++) {
      new->children[i] = node->children[i-1];
    }
    *result = ADDED;
    return (Node*)new;
  }
}

Node *hash_collision_assoc(Node *self, int level, void *key, void *val, hash_t hash, \
                           equal_fn eq_key, equal_fn eq_val, int *result) {

  HashCollisionNode *node = (HashCollisionNode*)self;

  /* check if the key already exists */
  while (node) {

    if (eq_key(node->key, key)) {
      /* found - exit the loop */
      break;
    }
    /* keep looking */
    node = node->next;
  }

  /* not found */
  if (!node) {
    /* add a new HashCollisionNode to the head of the linked list */
    HashCollisionNode *new = new_hash_collision_node(key, val, hash);
    new->next = (HashCollisionNode*)self;

    *result = ADDED;
    return (Node*)new;
  }

  /* if the key/value pair already exists return the original node */
  if (eq_key(node->key, key) && eq_val(node->val, val)) {

    *result = UNCHANGED;
    return self;
  }

  /* if the key exists with a different value we replace it */

  /* create new node to replace current value */
  HashCollisionNode *new = new_hash_collision_node(key, val, hash);
  /* point it at the tail */
  new->next = node->next;

  /* if we modified the head of the list we're done */
  if ((Node*)node == (Node*)self) {
    *result = UPDATED;
    return (Node*)new;
  }

  /* if we're not at the head of the list then we need to
     copy all the nodes up to the one we changed */

  /* make a copy of the list up to the replaced node */
  HashCollisionNode *prev = new;
  HashCollisionNode *curr= (HashCollisionNode*)self;
  HashCollisionNode *copy = NULL;

  /* copy the nodes */
  while (curr != node) {

    copy = new_hash_collision_node(curr->key, curr->val, curr->hash);
    copy->next = prev;
    prev = copy;
    curr = curr->next;
  }
  *result = UPDATED;
  return (Node*)copy;
}

Node *leaf_dissoc(Node *self, int level, void *key, hash_t hash, \
                  equal_fn eq_key, equal_fn eq_val, int *result) {

  /* dissocing a leaf node creates a NULL Node */
  LeafNode* node = (LeafNode*)self;

  if (eq_key(node->key, key)) {
    *result = REMOVED;
    return NULL;
  }
  else {
    /* not found */
    *result = UNCHANGED;
    return self;
  }
}

Node *bitmap_indexed_dissoc(Node *self, int level, void *key, hash_t hash, \
                            equal_fn eq_key, equal_fn eq_val, int *result) {

  BitmapIndexedNode *node = (BitmapIndexedNode*)self;

  int bit = bitpos(hash, level);
  int idx = bit_index(node->bitmap, bit);

  /* a child node already exists */
  if (node->bitmap & bit) {

    /* dissoc a child node at the correct index */
    Node *child = node->children[idx];
    Node *new = child->type->dissoc(child, (level + 1), key, hash, \
                                    eq_key, eq_val, result);

    if (*result != UNCHANGED) {

      /* copy the node */
      BitmapIndexedNode *copy = GC_MALLOC(sizeof(BitmapIndexedNode));
      memcpy(copy, node, sizeof(BitmapIndexedNode));

      /* node is now empty */
      if (!new) {
        /* clear bit */
	copy->bitmap &= ~bit;

        /* count the number of 1s in the whole bitmap */
	int nodes = popcount(copy->bitmap);
        /* allocate storage dynamically */
	copy->children = GC_MALLOC(sizeof(Node*) * nodes);

	/* copy existing child nodes skipping empty node */
	for (int i=0; i < idx; i++) {
	  copy->children[i] = node->children[i];
	}
	for (int i=idx+1; i < nodes + 1; i++) {
	  copy->children[i-1] = node->children[i];
	}
      }
      else {
        /* count the number of 1s in the bitmap */
	int nodes = popcount(copy->bitmap);

        /* allocate storage dynamically */
	copy->children = GC_MALLOC(sizeof(Node*) * nodes);

	/* copy existing child nodes */
	for (int i=0; i < idx; i++) {
	  copy->children[i] = node->children[i];
	}

	copy->children[idx] = new;

	for (int i=idx+1; i < nodes; i++) {
	  copy->children[i] = node->children[i];
	}
      }
      return (Node*)copy;
    }
    else {
      return self;
    }
  }
  /* node doesn't exist in bitmap */
  else {
    /* not found */
    *result = UNCHANGED;
    return self;
  }
}

Node *hash_collision_dissoc(Node *self, int level, void *key, hash_t hash, \
                            equal_fn eq_key, equal_fn eq_val, int *result) {

  HashCollisionNode *node = (HashCollisionNode*)self;

  /* check if the key exists */
  while (node) {

    if (eq_key(node->key, key)) {
      /* found - exit the loop */
      break;
    }
    /* keep looking */
    node = node->next;
  }

  /* not found */
  if (!node) {
    *result = UNCHANGED;
    return (Node*)node;
  }

  /* if the key exists at the head of the list we return the rest */
  if ((Node*)node == (Node*)self) {
    *result = REMOVED;
    return (Node*)node->next;
  }

  /* make a copy of the list up to the removed node */
  HashCollisionNode *prev = node->next;
  HashCollisionNode *curr= (HashCollisionNode*)self;
  HashCollisionNode *copy = NULL;

  /* copy the nodes */
  while (curr != node) {

    copy = new_hash_collision_node(curr->key, curr->val, curr->hash);
    copy->next = prev;
    prev = copy;
    curr = curr->next;
  }

  *result = REMOVED;
  return (Node*)copy;
}

void leaf_visit(Node *self, visit_fn fn, void **acc) {

  LeafNode* node = (LeafNode*)self;
  fn(node->key, node->val, acc);
}

void bitmap_indexed_visit(Node *self, visit_fn fn, void **acc) {

  BitmapIndexedNode *node = (BitmapIndexedNode*)self;
  int nodes = popcount(node->bitmap);

  for (int i=0; i < nodes; i++) {
    Node *child = node->children[i];
    child->type->visitor(child, fn, acc);
  }
}

void hash_collision_visit(Node *self, visit_fn fn, void **acc) {

  HashCollisionNode *node = (HashCollisionNode*)self;

  while (node) {
    fn(node->key, node->val, acc);
    node = node->next;
  }
}
