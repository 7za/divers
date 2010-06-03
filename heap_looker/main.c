#include <stdio.h>
#include <stdlib.h>
#include "heap_looker.h"


int poet()
{
  f1();
  f2();
  f3();
  return 4;
}

int main()
{
	heap_looker_init(stdout);
	poet();
	void *p = malloc(4);
    strdup("coucou");
	free(p);
	heap_looker_exit();
}
