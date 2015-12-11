#ifndef _SMAP_H
#define _SMAP_H

#include <stdlib.h>

struct SNode{
	struct SNode *next;
	int value;
	int hash;
	int len;
	char key[0];
};

struct SMap{
	int size;
	int seed;
	int nuse;
	struct SNode **hash;
};

struct SMapIterator{
	int idx;
	struct SNode *cur;
};

struct SMap* smap_create(int size);
struct SNode* smap_insert(struct SMap *map, const char *str, size_t l);
struct SMapIterator* smap_create_iterator(struct SMap *map);
void smap_free_iterator(struct SMap *map, struct SMapIterator* iterator);
struct SNode* smap_next(struct SMap *map, struct SMapIterator* iterator);

#endif
