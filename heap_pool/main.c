#include "heap_pool.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#if 0
int f1()
{
	heap_pool_addr_t addr;
	void *ret;
	struct heap_pool_desc * p = heap_pool_create("/plop", 1, 2097208, 0);

	if(IS_ERR(p)) {
		puts(strerror(-PTR_ERR(p)));
		return 0;
	}


	char *u = p->hpd_raw;
	printf("heap_pool & raw %p %p\n", p, u);
	ret = heap_pool_alloc(p, &addr);
	memset(*(void**)&addr, 'p', 20);
	printf("alloc = %p\n", ret);
	getchar();

	heap_pool_free(p, addr);

	ret = heap_pool_alloc(p, &addr);
	printf("alloc = %p\n", *(void**)&addr);

	ret = heap_pool_alloc(p, &addr);
	printf("alloc = %p\n", *(void**)&addr);

	heap_pool_free(p, addr);
	heap_pool_free(p, addr);
	heap_pool_free(p, addr);

	heap_pool_close(p);
	return 0;
}

int f2()
{

	heap_pool_addr_t addr;
	void *ret;
	struct heap_pool_desc * p;
	p = heap_pool_open("/plop");
	if(IS_ERR(p)) {
		puts(strerror(-PTR_ERR(p)));
		return 0;
	}

	char *u = p->hpd_raw;
	printf("heap_pool & raw%p %p\n", p, u);

	assert(heap_pool_raw_to_desc(u) == p);
	ret = heap_pool_alloc(p, &addr);
	printf("alloc = %p\n", *(void**)&addr);

	ret = heap_pool_alloc(p, &addr);
	printf("alloc = %p\n", *(void**)&addr);

	heap_pool_free(p, addr);
	heap_pool_free(p, addr);
	heap_pool_free(p, addr);



	heap_pool_destroy(heap_pool_raw_to_desc(u), "/plop"); 
	return 0;
}

#endif

#include <sys/time.h>
#define GETTIME(low,high) asm ("rdtsc" : "=a" (low), "=d" (high))


int main(int argc, char *argv[])
{
#if 0
	if(argv[1][0] == '1')
		f1();
	else
		f2();
	return 0;
#endif
	unsigned long high1, low1, high2, low2;
	struct timeval t1,t2;

#if 0
	int j;

	for(j = 0; j < 2000; j++) {
		int k;
		for(k = 1; k < 1200; ++k) {
			struct heap_pool_desc *desc = heap_pool_create(NULL, 10, k, j);
			if(!IS_ERR_OR_NULL(desc)) {
				heap_pool_destroy(desc, NULL);
			}
		}
	}

#endif

	struct heap_pool_desc * p = heap_pool_create(NULL, 10, 128, 0);
	
	if(IS_ERR_OR_NULL(p)) {return -1;}
	
	void *tab[100];
	int i = 0;
#if 0
	void *a,*b,*c, *d, *e;

	a = heap_pool_alloc(p);
	printf("a=%p\n", a);
	b = heap_pool_alloc(p);
	printf("b=%p\n", b);
	c = heap_pool_alloc(p);
	printf("c=%p\n", c);
	d = heap_pool_alloc(p);
	printf("d=%p\n", d);
	e = heap_pool_alloc(p);
	printf("e=%p\n", e);

	heap_pool_free(p, d);
	heap_pool_free(p, b);
#endif

	gettimeofday(&t1, 0);
	GETTIME(low1, high1);
	while( i < 10 &&   !IS_ERR_OR_NULL(((tab[i] = heap_pool_alloc(p))))) {
		++i;
	}
	GETTIME(low2, high2);
	gettimeofday(&t2, 0);
	//printf("alloc1 %lu %lu\n",high2 - high1, low2 - low1);
	timersub(&t2, &t1, &t2);
	printf("%lu %lu\n", t2.tv_sec, t2.tv_usec);




	{
		void *pp[10];
		i = 0;
		gettimeofday(&t1, 0);
		GETTIME(low1, high1);

		while( i < 10 &&  (pp[i++]=malloc(128)));
		GETTIME(low2, high2);
		gettimeofday(&t2, 0);

		//printf("alloc2 %lu %lu\n",high2 - high1, low2 - low1);
		timersub(&t2, &t1, &t2);
		printf("%lu %lu\n", t2.tv_sec, t2.tv_usec);

		gettimeofday(&t1, 0);
		GETTIME(low1, high1);
		while(i--)
			free(pp[i]);
		GETTIME(low2, high2);
		gettimeofday(&t2, 0);
		timersub(&t2, &t1, &t2);
		printf("%lu %lu\n", t2.tv_sec, t2.tv_usec);

		//printf("free2 %lu %lu\n",high2 - high1, low2 - low1);

	}
	i=0;
	gettimeofday(&t1, 0);
	GETTIME(low1, high1);
	while(i < 10) {
		heap_pool_free(p, tab[i ++]);
	}
	GETTIME(low2, high2);
	gettimeofday(&t2, 0);
	timersub(&t2, &t1, &t2);
	printf("%lu %lu\n", t2.tv_sec, t2.tv_usec);
	//printf("free1 %lu %lu\n",high2 - high1, low2 - low1);
	heap_pool_destroy(p, NULL);
}

