#ifndef stat_h
#define stat_h

#include <stdlib.h>
#include <string.h>
#include "smap.h"

struct FuncStat {
	int count;
	int sum_ex_time;
	struct SMap* trace;
};

typedef struct SMap GStat;

struct FuncStat*
create_func_stat() {
	struct FuncStat * fs = (struct FuncStat*)malloc(sizeof(struct FuncStat));
	fs->count = 0;
	fs->sum_ex_time = 0.0;
	fs->trace = smap_create(2);

	return fs;
}

void
stat_trace(struct FuncStat* fs, const char *pre_str, size_t pl) {
	struct SNode* sn = smap_insert(fs->trace, pre_str, pl);
	sn->value++;
}

GStat*
create_gs(int size) {
	return smap_create(size);
}

void
stat(GStat* map, const char *str, size_t l, const char *pre_str, size_t pl, int t) {
	struct SNode* sn = smap_insert(map, str, l);
	struct FuncStat* fs;
	if (!sn->value) {
		sn->value = (int)create_func_stat();
	}
	fs = (struct FuncStat*)sn->value;
	fs->count++;
	fs->sum_ex_time += t;
	if (pre_str) {
		stat_trace(fs, pre_str, pl);
	}
}

void
stat_print(GStat* map) {
	struct SMapIterator* iterator = smap_create_iterator(map);
	struct SNode* sn;
	while (sn = smap_next(map, iterator)){
		struct FuncStat* fs = (struct FuncStat*)sn->value;
		printf("%s\t%s\t%d\t%d\n", sn->key, sn->key, fs->count, fs->sum_ex_time);
		struct SMapIterator* it = smap_create_iterator(fs->trace);
		struct SNode* ssn;
		while (ssn = smap_next(fs->trace, it)){
			printf("\t%s\t%d\n", ssn->key, ssn->value);
		}
		smap_free_iterator(fs->trace, it);
	}
	smap_free_iterator(map, iterator);
}

#endif
