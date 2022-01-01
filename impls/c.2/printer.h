#ifndef _PRINTER_H
#define _PRINTER_H

#include <stdarg.h>
#include "types.h"

#define UNREADABLY 0
#define READABLY 1

char* pr_str(MalType* mal_val, int readably);
char* pr_str_sequential(list lst, int readably, char* start_delimiter, char* end_delimiter, char* separator);
char* pr_str_list(list lst, int readably);
char* pr_str_vector(vector vec, int readably);
char* pr_str_hashmap(hashmap map, int readably);
char* escape_string(char* str);
char* snprintfbuf(long initial_size, char* fmt, ...);

#endif
