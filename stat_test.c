#include <stdlib.h>
#include <stdio.h>
#include "stat.h"

int main() {
	GStat* map = smap_create(2);
	stat(map, "auto", 4, "guozi", 5, 2);
	stat(map, "auto", 4, "guozi", 5, 3);
	stat(map, "auto", 4, "guozi", 5, 4);
	stat(map, "auto", 4, "guozi", 5, 5);
	stat(map, "auto", 4, "guozi2", 6, 2);
	stat(map, "dvd", 3, "guozi2", 6, 2);
	stat(map, "dvd", 3, "guozi", 5, 2);
	stat(map, "dvd", 3, "guozi", 5, 3);
	stat(map, "dvd", 3, "guozi", 5, 4);
	stat(map, "dvd", 3, "guozi", 5, 5);
	stat(map, "dvd", 3, "guozi2", 6, 2);
	stat(map, "dvd", 3, "guozi2", 6, 2);
	stat(map, "dvd", 3, NULL, 0, 2);
	stat_print(map);
	return 0;
}