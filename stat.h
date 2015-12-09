#ifndef stat_h
#define stat_h

#include <stdlib.h>
#include <string.h>
#include "smap.h"

struct FuncStat {
	int tag;
	int count;
	int total_count;
	int ex_time;
	int sum_ex_time;
	struct SNode* sn;
	struct SMap* trace;
};

struct CallStat {
	int tag;
	int count;
	int total_count;
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
stat_trace(struct FuncStat* fs, const char *pre_str, size_t pl, int tag) {
	struct SNode* sn = smap_insert(fs->trace, pre_str, pl);
	struct CallStat* ct;
	if (!sn->value) {
		sn->value = (int)malloc(sizeof(struct CallStat));
	}
	ct = (struct CallStat*)sn->value;
	ct->count += 1;
	ct->total_count += 1;
	if (tag != ct->tag) {
		ct->tag = tag;
		ct->count = 1;
	}
}

GStat*
create_gs(int size) {
	return smap_create(size);
}

void
stat(GStat* map, const char *str, size_t l, const char *pre_str, size_t pl, int t, int tag) {
	struct SNode* sn = smap_insert(map, str, l);
	struct FuncStat* fs = (struct FuncStat*)sn->value;
	if (!sn->value) {
		fs = create_func_stat();
		fs->sn = sn;
		sn->value = (int)fs;
	}
	fs->count++;
	fs->total_count++;
	fs->ex_time +=t;
	fs->sum_ex_time += t;
	if (fs->tag != tag) {
		fs->count = 1;
		fs->ex_time = t;
		fs->tag = tag;
	}
	if (pre_str) {
		stat_trace(fs, pre_str, pl, tag);
	}
}

void
_sort(struct FuncStat ** fs, int left, int right) {
	int i = left;
	int j = right + 1;
	struct FuncStat *meta = fs[left];
	struct FuncStat *tmp;
	do {
		while(i < j && fs[--j]->sum_ex_time < meta->sum_ex_time);
		tmp = fs[i];
		fs[i] = fs[j];
		fs[j] = tmp;
		while( i < j && fs[++i]->sum_ex_time > meta->sum_ex_time);
		tmp = fs[i];
		fs[i] = fs[j];
		fs[j] = tmp;
	}while(i < j);
	if (left < i - 1) _sort(fs, left, i - 1);
	if (right > j + 1) _sort(fs, j + 1, right);
}

struct FuncStat **
sort(GStat* map) {
	struct FuncStat ** ret = (struct FuncStat **)malloc(map->nuse * (sizeof(struct FuncStat *)));
	struct SMapIterator* iterator = smap_create_iterator(map);
	struct SNode* sn;
	int i = 0;
	while (sn = smap_next(map, iterator)){
		ret[i++] = (struct FuncStat*)sn->value;
	}
	smap_free_iterator(map, iterator);
	_sort(ret, 0, map->nuse-1);

	return ret;
}

void
stat_print(GStat* map) {
	struct SMapIterator* iterator = smap_create_iterator(map);
	struct SNode* sn;
	struct CallStat* ct;
	struct FuncStat ** afs = sort(map);
	int i;
	for (i = 0; i < map->nuse; ++i) {
		struct FuncStat* fs = afs[i];
		sn = fs->sn;
		printf("%s\t%s\t%d\t%d", sn->key, sn->key, fs->count, fs->sum_ex_time);
		struct SMapIterator* it = smap_create_iterator(fs->trace);
		struct SNode* ssn;
		while (ssn = smap_next(fs->trace, it)){
			ct = (struct CallStat*)ssn->value;
			printf("\t{%s\t%d}", ssn->key, ct->total_count);
		}
		printf("\n");
		smap_free_iterator(fs->trace, it);
	}
	free(afs);
	//smap_free_iterator(map, iterator);
}

#endif
