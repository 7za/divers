#include <stdlib.h>



void f1()
{
	void *p = malloc(4);

	free(p);
}

void f2()
{
	void *p = calloc(4, 4);

	free(p);
}


void f3()
{
	void *p = malloc(2);

	p = realloc(p, 6);
	free(p);
}
