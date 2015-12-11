#include <stdio.h>
#include "stat.h"

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
	struct SNode* sn;
};

static struct FuncStat*
create_func_stat() {
	struct FuncStat * fs = (struct FuncStat*)malloc(sizeof(struct FuncStat));
	fs->count = 0;
	fs->sum_ex_time = 0.0;
	fs->trace = smap_create(2);
	fs->tag = 0;
	fs->total_count = 0;
	fs->ex_time = 0;
	return fs;
}

static void
stat_trace(struct FuncStat* fs, const char *pre_str, size_t pl, int tag) {
	struct SNode* sn = smap_insert(fs->trace, pre_str, pl);
	struct CallStat* ct = (struct CallStat*)sn->value;
	if (!sn->value) {
		ct = (struct CallStat*)malloc(sizeof(struct CallStat));
		ct->sn = sn;
		ct->count = 0;
		ct->tag = 0;
		ct->total_count = 0;
		sn->value = (int)ct;
	}
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

static void
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

static void
_sort2(struct FuncStat ** fs, int left, int right) {
	int i = left;
	int j = right + 1;
	struct FuncStat *meta = fs[left];
	struct FuncStat *tmp;
	do {
		while(i < j && fs[--j]->ex_time <= meta->ex_time);
		tmp = fs[i];
		fs[i] = fs[j];
		fs[j] = tmp;
		while( i < j && fs[++i]->ex_time >= meta->ex_time);
		tmp = fs[i];
		fs[i] = fs[j];
		fs[j] = tmp;
	}while(i < j);
	if (left < i - 1) _sort(fs, left, i - 1);
	if (right > j + 1) _sort(fs, j + 1, right);
}

static int
sort(GStat* map, struct FuncStat **afs, int tag) {
	struct SMapIterator* iterator = smap_create_iterator(map);
	struct SNode* sn;
	struct FuncStat * fs;
	int i = 0;
	while (sn = smap_next(map, iterator)){
		fs = (struct FuncStat*)sn->value;
		if (tag == 0 || fs->tag == tag) {
			afs[i++] = fs;
		}
	}
	smap_free_iterator(map, iterator);
	if (tag == 0) {
		_sort(afs, 0, i-1);
	}else {
		_sort2(afs, 0, i-1);
	}

	return i;
}

static void
_sort_call_stat(struct CallStat** cs, int left, int right) {
	int i = left;
	int j = right + 1;
	struct CallStat* meta = cs[left];
	struct CallStat* tmp;
	do {
		while(i < j && cs[--j]->total_count < meta->total_count);
		tmp = cs[i];
		cs[i] = cs[j];
		cs[j] = tmp;
		while( i < j && cs[++i]->total_count > meta->total_count);
		tmp = cs[i];
		cs[i] = cs[j];
		cs[j] = tmp;
	}while(i < j);
	if (left < i - 1) _sort_call_stat(cs, left, i - 1);
	if (right > j + 1) _sort_call_stat(cs, j + 1, right);
}

static void
_sort_call_stat2(struct CallStat** cs, int left, int right) {
	int i = left;
	int j = right + 1;
	struct CallStat* meta = cs[left];
	struct CallStat* tmp;
	do {
		while(i < j && cs[--j]->count < meta->count);
		tmp = cs[i];
		cs[i] = cs[j];
		cs[j] = tmp;
		while( i < j && cs[++i]->count > meta->count);
		tmp = cs[i];
		cs[i] = cs[j];
		cs[j] = tmp;
	}while(i < j);
	if (left < i - 1) _sort_call_stat(cs, left, i - 1);
	if (right > j + 1) _sort_call_stat(cs, j + 1, right);
}

static int
sort_call_stat(struct SMap* map, struct CallStat**acs, int tag) {
	struct SMapIterator* iterator = smap_create_iterator(map);
	struct SNode* sn;
	struct CallStat* cs;
	int i = 0;
	while (sn = smap_next(map, iterator)){
		cs = (struct CallStat*)sn->value;
		if (tag == 0 || tag == cs->tag) {
			acs[i++] = cs;
		}
	}
	smap_free_iterator(map, iterator);
	if (tag == 0) {
		_sort_call_stat(acs, 0, i-1);
	}else {
		_sort_call_stat2(acs, 0, i-1);
	}
	
	return i;
}

void
stat_print(GStat* map, int tag) {
	struct SMapIterator* iterator = smap_create_iterator(map);
	struct SNode* sn;
	struct CallStat* ct;
	struct FuncStat ** afs = (struct FuncStat **)malloc(map->nuse * sizeof(struct FuncStat *));
	int fc = sort(map, afs, tag);
	int i, j;
	for (i = 0; i < fc; ++i) {
		struct FuncStat* fs = afs[i];
		sn = fs->sn;
		printf("%s\t%s\t%d\t%d", sn->key, sn->key, tag == 0 ? fs->total_count : fs->count, tag == 0 ? fs->sum_ex_time : fs->ex_time);
		struct SNode* ssn;
		struct CallStat** acs = (struct CallStat**)malloc(fs->trace->nuse * sizeof(struct CallStat*));
		int cc = sort_call_stat(fs->trace, acs, tag);
		for (j = 0; j < cc; ++j) {
			ct = acs[j];
			ssn = ct->sn;
			printf("\t{%s\t%d}", ssn->key, tag == 0 ? ct->total_count : ct->count);
		}
		free(acs);
		printf("\n");
	}
	free(afs);
	//smap_free_iterator(map, iterator);
}
