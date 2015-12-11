#ifndef stat_h
#define stat_h

#include "smap.h"

typedef struct SMap GStat;

GStat*create_gs(int size);
void stat(GStat* map, const char *str, size_t l, const char *pre_str, size_t pl, int t, int tag);
void stat_print(GStat* map, int tag);

#endif
