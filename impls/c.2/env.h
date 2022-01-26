#ifndef _MAL_ENV_H
#define _MAL_ENV_H

#include "libs/hashmap/hashmap.h"
#include "types.h"

typedef MalType*(*mal_fn)(list);
typedef struct Env_s Env;

struct Env_s {

  struct Env_s* outer;
  hashmap data;

};

Env* env_make(Env* outer, list binds, list exprs, MalType* more_symbol);
Env* env_set(Env* current, MalType* symbol, MalType* value);
Env* env_set_C_fn(Env* current, char* symbol_name, mal_fn fn);
MalType* env_get(Env* current, MalType* symbol);
Env* env_find(Env* current, MalType* symbol);
#endif
