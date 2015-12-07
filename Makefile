all : stat.so smap_test stat_test clean_temp
.PHONY : all

stat.so : lstat.o smap.o
	gcc -g -o $@ -shared $^ -I/home/dvd/Code/lua/lua-5.3.0/src
smap_test : smap_test.o smap.o
	cc -o smap_test smap_test.o smap.o
stat_test : stat_test.o smap.o
	cc -o stat_test stat_test.o smap.o
smap_test.o : smap_test.c
smap.o : smap.c
stat_test.o : stat_test.c
lstat.o : lstat.c stat.h

.PHONY: clean_temp
clean_temp:
	rm -rf smap_test.o smap.o stat.so.dSYM stat_test.o

.PHONY: clean
clean:
	rm -rf smap_test stat.so
