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

#include "../../include/hashmap.h"

#ifndef _PERSISTENT_HASHMAP_INTERNAL_H
#define _PERSISTENT_HASHMAP_INTERNAL_H

#define BITS_PER_LEVEL 5

#define UNCHANGED 0
#define ADDED 1
#define UPDATED 2
#define REMOVED 3

/*
   for explanations see:
   http://blog.higher-order.net/2009/09/08/understanding-clojures-persistenthashmap-deftwice
*/

/* Implementation details */
typedef struct NodeType NodeType;
typedef struct Node Node;
typedef struct LeafNode LeafNode;
typedef struct BitmapIndexedNode BitmapIndexedNode;
typedef struct HashCollisionNode HashCollisionNode;

typedef void *(*get_fn)(Node *self, int level, void *key, \
			hash_t hash, equal_fn eq_key, equal_fn eq_val);

typedef Node *(*assoc_fn)(Node *self, int level, void *key, void *val, \
			  hash_t hash, equal_fn eq_key, equal_fn eq_val, int *result);

typedef Node *(*dissoc_fn)(Node *self, int level, void *key, hash_t hash, \
			   equal_fn eq_key, equal_fn eq_val, int *result);

typedef void (*visitor_fn)(Node *self, visit_fn fn, void **acc);

/* hashmap holds a reference to the nodes, the node count and the functions to use for
   generating a hash and testing keys and values for equality */
struct Hashmap {
  hash_fn hash;
  equal_fn eq_key;
  equal_fn eq_val;
  int count;
  Node* root;
};

/* polymorphic dispatch */
struct NodeType {
  get_fn get;
  assoc_fn assoc;
  dissoc_fn dissoc;
  visitor_fn visitor;
};

/* generic node */
struct Node {
  NodeType *type;
};

/* holds a key/value pair */
struct LeafNode {
  NodeType *type;
  void *key;
  void *val;
  hash_t hash;
};

/* a linked list of LeafNodes with the same hash */
struct HashCollisionNode {
  NodeType *type;
  void *key;
  void *val;
  hash_t hash;

  HashCollisionNode *next;
};

/* a dynamically sized array of up to 32 child nodes */
struct BitmapIndexedNode {
  NodeType *type;
  int bitmap;
  Node **children;
};


/* forward references */
LeafNode *new_leaf_node(void *key, void *val, hash_t hash);

HashCollisionNode *new_hash_collision_node(void *key, void* val, hash_t hash);

BitmapIndexedNode *new_bitmap_indexed_node();

void *leaf_get(Node *self, int level, void *key, \
	       hash_t hash, equal_fn eq_key, equal_fn eq_val);

void *bitmap_indexed_get(Node *self, int level, void *key, \
			 hash_t hash, equal_fn eq_key, equal_fn eq_val);

void *hash_collision_get(Node *self, int level, void *key,	\
			 hash_t hash, equal_fn eq_key, equal_fn eq_val);

Node *leaf_assoc(Node *self, int level, void *key, void *val, \
		 hash_t hash, equal_fn eq_key, equal_fn eq_val, int *result);

Node *bitmap_indexed_assoc(Node *self, int level, void *key, void *val, \
			   hash_t hash, equal_fn eq_key, equal_fn eq_val, int *result);

Node *hash_collision_assoc(Node *self, int level, void *key, void *val, \
			   hash_t hash, equal_fn eq_key, equal_fn eq_val, int *result);

Node *leaf_dissoc(Node *self, int level, void *key, hash_t hash, \
		  equal_fn eq_key, equal_fn eq_val, int *result);

Node *bitmap_indexed_dissoc(Node *self, int level, void *key, hash_t hash, \
			    equal_fn eq_key, equal_fn eq_val, int *result);

Node *hash_collision_dissoc(Node *self, int level, void *key, hash_t hash, \
			    equal_fn eq_key, equal_fn eq_val, int *result);

void leaf_visit(Node *self, visit_fn fn, void **acc);

void bitmap_indexed_visit(Node *self, visit_fn fn, void **acc);

void hash_collision_visit(Node *self, visit_fn fn, void **acc);

#endif
