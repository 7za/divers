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
	void *ptr;
	heap_looker_init(stdout);
	poet();

	f1();

	heap_looker_exit();
}
