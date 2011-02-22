#include "heap_pool.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>


int main()
{
	heap_pool_addr_t addr;
	void *ret;
	struct heap_pool_desc * p = heap_pool_create("/plop", 30, 20, 64);
	if(IS_ERR(p)) {
		puts(strerror(-PTR_ERR(p)));
		return 0;
	}
	ret = heap_pool_alloc(p, &addr);
	memset(*(void**)&addr, 'p', 20);
	printf("%p\n", ret);
	getchar();

	heap_pool_free(p, addr);

	ret = heap_pool_alloc(p, &addr);
	printf("%p\n", *(void**)&addr);

	heap_pool_free(p, addr);
	heap_pool_free(p, addr);
	heap_pool_free(p, addr);

	heap_pool_close(p);

	p = heap_pool_open("/plop");
	if(IS_ERR(p)) {
		puts(strerror(-PTR_ERR(p)));
		return 0;
	}

	char *u = p->hpd_raw;

	assert(heap_pool_raw_to_desc(u) == p);

	heap_pool_destroy(p, "/plop");

	p = heap_pool_create_if_needed("/plop", 30, 20, 64);

	p = heap_pool_create_if_needed("/plop", 30, 20, 64);

	u = p->hpd_raw;

	heap_pool_destroy(heap_pool_raw_to_desc(u), "/plop"); 

	return 0;
}

