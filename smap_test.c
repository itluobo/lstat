#include <stdlib.h>
#include <stdio.h>
#include "smap.h"

int main() {
	struct SMap* map = smap_create(2);
	struct SNode* a = smap_insert(map, "aotu", 4);
	a->value = 9;
	struct SNode* b = smap_insert(map, "aoaotutu", 8);
	b->value = 10;
	a = smap_insert(map, "dvd", 3);
	a->value = 15;
	a = smap_insert(map, "wangjing", 8);
	a->value = 153;
	a = smap_insert(map, "dashu", 5);
	a->value = 12;
	a = smap_insert(map, "guoji", 5);
	a->value = 347;
	a = smap_insert(map, "hehe", 4);
	a->value = 34;
	
	struct SMapIterator* iterator = smap_create_iterator(map);
	printf("map size - > %d\n", map->size);
	while (a = smap_next(map, iterator)){
		printf("%s - > %d\n", a->key, a->value);
	}
	smap_free_iterator(map, iterator);
	return 0;
}
