#ifndef _MAL_CORE_H
#define _MAL_CORE_H

#include "libs/hashmap/hashmap.h"
#include "types.h"

typedef struct ns_s ns;

struct ns_s {
  hashmap mappings;
};

ns* ns_make_core(void);

#endif
