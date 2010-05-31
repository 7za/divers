#include <stdio.h>


int poet()
{
  f1();
  f2();
  f3();
  return 4;
}

int main()
{
	heap_looker_init();
	poet();
	void *p = malloc(4);
	free(p);
	heap_looker_exit();
}
