#include "smap.h"


#define mod(s, size) \
	((int)((s)&(size - 1)))

struct SMap*
smap_create(int size){
	int i;
	struct SMap* map = (struct SMap*)malloc(sizeof(struct SMap));
	map->hash = (struct SNode**)malloc(sizeof(struct SNode*) * size);
	for (i = 0; i < size; ++i) {
		map->hash[i] = NULL;
	}
	map->size = size;
	map->nuse = 0;

	return map;
}

static struct SNode*
smap_create_node(const char *str, size_t l, int hash){
	struct SNode* node = (struct SNode*)malloc(sizeof(struct SNode) + (l + 1) * sizeof(char));
	memcpy(node->key, str, l * sizeof(char));
	node->key[l] = 0;
	node->len = l;
	node->hash = hash;
	node->value = 0;
	return node;
}

static int
s_hash(const char *str, size_t l, unsigned int seed) {
	unsigned int h = seed ^ ((unsigned int)l);
	size_t l1;
	size_t step = (l >> 5) + 1;
	for (l1 = l; l1 >= step; l1 -= step)
		h = h ^ ((h<<5) + (h>>2) + (int)(str[l1 - 1]));
	return h;
}

static int
smap_resize(struct SMap *map, int new_size){
	int i;
	if (new_size > map->size){
		map->hash = realloc(map->hash, new_size * sizeof(struct SNode*));
		for (i = map->size; i < new_size; ++i){
			map->hash[i] = NULL;
		}
	}
	for (i = 0; i < map->size; ++i){
		struct SNode *sn = map->hash[i];
		map->hash[i] = NULL;
		while(sn) {
			struct SNode* next = sn->next;
			unsigned h = mod(sn->hash, new_size);
			sn->next = map->hash[h];
			map->hash[h] = sn;
			sn = next;
		}
	}
	map->size = new_size;
	return 0;
}

struct SNode*
smap_insert(struct SMap *map, const char *str, size_t l) {
	unsigned int h = s_hash(str, l, map->seed);
	struct SNode **list = &map->hash[mod(h, map->size)];
	struct SNode *sn;
	for(sn = *list; sn != NULL; sn = sn->next){
		if (l == sn->len &&
			 (memcmp(str, sn->key, l * sizeof(char)) == 0)){
			return sn;
		}
	}
	if (map->nuse >= map->size) {
		smap_resize(map, map->size * 2);
		list = &map->hash[mod(h, map->size)];
	}
	sn = smap_create_node(str, l, h);
	sn->next = *list;
	*list = sn;
	map->nuse++;
	return sn;
}

struct SMapIterator*
smap_create_iterator(struct SMap *map) {
	struct SMapIterator * ret = (struct SMapIterator*) malloc(sizeof(struct SMapIterator));
	ret->idx = 0;
	ret->cur = map->hash[0];

	return ret;
}

void
smap_free_iterator(struct SMap *map, struct SMapIterator* iterator) {
	free(iterator);
}

struct SNode*
smap_next(struct SMap *map, struct SMapIterator* iterator) {
	struct SNode* ret = NULL;
	while(iterator->cur == NULL && ++iterator->idx < map->size)
	{
		iterator->cur = map->hash[iterator->idx];
	}
	if (iterator->cur) {
		ret = iterator->cur;
		iterator->cur = iterator->cur->next;
	}
	
	return ret;
}
