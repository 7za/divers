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


int main(int argc, char *argv[])
{
	if(argv[1][0] == '1')
		f1();
	else
		f2();
	return 0;
}

