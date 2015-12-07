#include <lua.h>
#include <lauxlib.h>
#include <string.h>
#include <stdlib.h>
#include "stat.h"
#include <stdint.h>

#define MAX_CALL_STACK 1024
#define bool int
#define FALSE 0
#define TRUE 1

struct Call {
	uint64_t start_time;
	size_t len;
	bool is_tail_call;
	char *name;
};

struct CallStack {
	uint64_t suspend_t;
	uint64_t last_leave_t;
	GStat* gs;
	lua_State* l;
	int depth;
	struct Call call_data[MAX_CALL_STACK];
};

static struct CallStack * GCS = NULL;
static GStat* GS = NULL;

static uint64_t
gettime() {
	uint64_t t;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	t = (uint64_t)tv.tv_sec * 100;
	t += tv.tv_usec / 10000;
	return t;
}

static void
call_stack_init(struct CallStack * cs, lua_State* L) {
	cs->depth = 0;
	int i;
	for (i = 0; i < MAX_CALL_STACK; ++i)
	{
		cs->call_data[i].name = NULL;
	}
	free(cs->gs);
	cs->gs = GS;
	cs->l = L;
}

static void
suspend_l(struct CallStack * cs) {
	cs->last_leave_t = gettime();
}

static void
resume_l(struct CallStack * cs) {
	if(cs->last_leave_t >0){
		cs->suspend_t += (gettime() - cs->last_leave_t);
		cs->last_leave_t = 0;
	}
}

static void
clear_call_stack(struct CallStack * cs) {
	cs->depth = 0;
	int i;
	for (i = 0; i < MAX_CALL_STACK; ++i)
	{
		if (cs->call_data[i].name)
			free(cs->call_data[i].name);
		cs->call_data[i].name = NULL;
	}
}

static void
record_call(struct CallStack * cs, const char * name, size_t sz, bool is_tail_call) {
	struct Call *cl = &(cs->call_data[cs->depth++]);
	cl->is_tail_call = is_tail_call;
	cl->name = malloc(sz + 1);
	memcpy(cl->name, name, sz);
	cl->name[sz] = '\0';
	cl->len = sz;
	cl->start_time = gettime() - cs->suspend_t;
}

static void
pop_call(struct CallStack * cs) {
	if (cs->depth == 0) {
		return;
	}
	struct Call *cl = &(cs->call_data[cs->depth - 1]);
	cs->depth -= 1;
	char* pre = NULL;
	size_t pl = 0;
	if (cs->depth > 0) {
		struct Call* pcl = &(cs->call_data[cs->depth - 1]);
		pre = pcl->name;
		pl = pcl->len;
	}
	stat(cs->gs, cl->name, cl->len, pre, pl, gettime() - cl->start_time - cs->suspend_t);
	if (cl->is_tail_call && cs->depth > 0)
		pop_call(cs);
}

static lua_State*
getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "thread expected");
  return co;
}

static void
link_cs2l(lua_State*L, struct CallStack* cs) {
	lua_getglobal(L, "_GCS");
	lua_pushlightuserdata(L, cs);
	lua_seti(L, -2, (int)L);
	lua_pop(L, 1);
}

static void
unlink_cs2l(lua_State*L, struct CallStack* cs) {
	lua_getglobal(L, "_GCS");
	lua_pushlightuserdata(L, cs);
	lua_pushnil(L);
	lua_settable(L, -3);
	lua_pop(L, 1);
}

static struct CallStack*
find_cs(lua_State*L) {
	static struct CallStack* cs = NULL;
	lua_getglobal(L, "_GCS");
	lua_geti(L, -1, (int)L);
	if (!lua_islightuserdata(L, -1)) {
		lua_pop(L, 2);
		return NULL;
	}
	cs = (struct CallStack*)lua_topointer(L, -1);
	lua_pop(L, 2);
	return cs;
}

static void
release_cs(lua_State *co, struct CallStack* cs) {
	unlink_cs2l(co, cs);
	clear_call_stack(cs);
}

static int
is_co_dead(lua_State* co) {
	switch (lua_status(co)) {
      case LUA_YIELD:
        return 0;
      case LUA_OK: {
        lua_Debug ar;
        if (lua_getstack(co, 0, &ar) > 0)  /* does it have frames? */
          	return 0;
        else if (lua_gettop(co) == 0)
            return 1;
        else
          	return 0;
      }
      default:  /* some error occurred */
        return 1;
    }
    return 0;
}

static void
check_cs(lua_State *L) {
	if (GCS && GCS->l == L){
		return;
	}
	if (GCS){
		if (is_co_dead(GCS->l)) {
			release_cs(GCS->l, GCS);
		}
		suspend_l(GCS);
	}
	GCS = find_cs(L);
	if (!GCS) {
		GCS = malloc(sizeof(*GCS));
		call_stack_init(GCS, L);
		link_cs2l(L, GCS);
	}
	resume_l(GCS);
}

static void
monitor(lua_State *L, lua_Debug *ar) {
	char msg[1024];
	lua_getinfo(L, "nS", ar);
	if (ar->what[0] == 'C') return;
	check_cs(L);
	struct CallStack * cs = GCS;
	size_t sz;
	switch (ar->event){
		case LUA_HOOKCALL:
			if (ar->name != NULL){
				sz = snprintf(msg, 1024, "%s", ar->name);
			}else if (ar->linedefined >= 0){
				sz = snprintf(msg, 1024, "%s:%d", ar->short_src, ar->linedefined);
			}else {
				sz = snprintf(msg, 1024, "unknow");
			}
			record_call(cs, msg, sz, FALSE);
			break;
		case LUA_HOOKTAILCALL:
			if (ar->name != NULL){
				sz = snprintf(msg, 1024, "%s", ar->name);
			}else if (ar->linedefined >= 0){
				sz = snprintf(msg, 1024, "%s:%d", ar->short_src, ar->linedefined);
			}else {
				sz = snprintf(msg, 1024, "unknow");
			}
			record_call(cs, msg, sz, TRUE);
			break;
		case LUA_HOOKRET:
			if (ar->name != NULL) {
				sz = snprintf(msg, 1024, "Ret->%s", ar->name);
			}else if(ar->linedefined >= 0) {
				sz = snprintf(msg, 1024, "Ret->%s:%d", ar->short_src, ar->linedefined);
			}else {
				sz = snprintf(msg, 1024, "Ret");
			}
			pop_call(cs);
			break;
		default:
			if (ar->name != NULL) {
				sz = snprintf(msg, 1024, "haha");
			}else {
				sz = snprintf(msg, 1024, "haha");
			}
			
			break;
	}
	//printf("%s\n", msg);
}

static int
lstat(lua_State *L) {
	GCS = malloc(sizeof(*GCS));
	GS = create_gs(2);
	lua_newtable(L);
	lua_setglobal(L, "_GCS");
	link_cs2l(L, GCS);
	call_stack_init(GCS, L);
	lua_sethook(L, monitor, LUA_MASKCALL | LUA_MASKRET, 0);
	return 0;
}

static int llink_co(lua_State *L) {
	lua_State * co = getco(L);
	lua_sethook(co, monitor, LUA_MASKCALL | LUA_MASKRET, 0);
	return 0;
}

static int lrelease_co(lua_State *L) {
	lua_State * co = getco(L);
	lua_sethook(co, NULL, 0, 0);
	struct CallStack* cs = find_cs(co);
	if (cs) {
		release_cs(co, cs);
	}
	return 0;
}

static int ldump(lua_State *L) {
	if (GCS){
		stat_print(GCS->gs);
	}
	
	return 0;
}

static int lun_stat(lua_State *L) {
	lua_sethook(L, NULL, 0, 0);
	return 0;
}

int
luaopen_stat(lua_State *L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{ "stat", lstat },
		{ "un_stat", lun_stat},
		//{ "link_co", llink_co },
		{ "release_co", lrelease_co},
		{ "dump", ldump},
		{ NULL, NULL },
	};
	luaL_newlib(L, l);
	
	return 1;
}
