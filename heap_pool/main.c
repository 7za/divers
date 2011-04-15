#include "heap_pool.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

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

	struct heap_pool_desc * p = heap_pool_create(NULL, 10000, 10000, 0);
	
	
	heap_pool_addr_t tab[10000];
	int i = 0;

struct timeval t1, t2, t3;
gettimeofday(&t1, 0);
	GETTIME(low1, high1);
	while( i < 10000 && !IS_ERR_OR_NULL(heap_pool_alloc(p, tab+i++)));
	GETTIME(low2, high2);
gettimeofday(&t2, 0);
	timersub(&t2, &t1, &t3);

	//printf("%ld %ld\n", t3.tv_sec, t3.tv_usec);
	printf("%lu %lu\n",high2 - high1, low2 - low1);

	printf("%d\n", i);

	heap_pool_free(p, tab[5]);
	heap_pool_free(p, tab[7]);
	heap_pool_free(p, tab[6]);

	{
		void *pp[1000];
		i = 0;
		gettimeofday(&t1, 0);
		GETTIME(low1, high1);

		while( i < 10000 &&  (pp[i++]=malloc(10000)));
		gettimeofday(&t2, 0);
		timersub(&t2, &t1, &t3);
		GETTIME(low2, high2);

		printf("%lu %lu\n",high2 - high1, low2 - low1);
		//printf("%ld %ld\n", t3.tv_sec, t3.tv_usec);

	}

	heap_pool_destroy(p, NULL);
}

