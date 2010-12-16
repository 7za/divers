#ifndef __HEAP_POOL_H
#define __HEAP_POOL_H

#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>


#define thread_scope __thread
#ifdef HEAP_POOL_UP
# undef  thread_scope
# define thread_scope
#endif

#define __HEAP_POOL_ALLOC_STEP	(5)
#define __HEAP_POOL_ALLOC_CHUNK_STEP	(10)

static thread_scope struct heap_pool *__hp_list;
static thread_scope struct heap_pool *__hp_last;
static thread_scope size_t __hp_nmemb;

#ifndef unlikely 
#define unlikely(x)	(__builtin_expect((x), 0))
#endif

#ifndef likely 
#define likely(x)	(__builtin_expect((x), 1))
#endif

struct heap_pool {
	size_t   hp_nmemb;
	size_t   hp_size;

	uint8_t  *hp_mem;
	uint8_t  *hp_curr;
	size_t	 *hp_index;
};

__attribute__((constructor))
static void __hp_private_constructor(void)
{
	__hp_nmemb = __HEAP_POOL_ALLOC_STEP;
	__hp_list  = calloc(__HEAP_POOL_ALLOC_STEP, sizeof(*__hp_list));
	__hp_last  = __hp_list;
}

__attribute__((destructor))
static void __hp_private_destructor(void)
{
	if(likely(__hp_list != NULL)) {
		free(__hp_list);
		__hp_list = NULL;
		__hp_last = NULL;
	}
}

static inline size_t __hp_nb_pool(void)
{
	return (size_t) (__hp_last - __hp_list);
}

static inline int __hp_pool_has_chunk_left(struct heap_pool const *const p)
{
	return (size_t)(p->hp_curr - p->hp_mem) < p->hp_nmemb * p->hp_size;
}

struct heap_pool * hp_create_pool(size_t nb, size_t size)
{
	if(__hp_nb_pool() >= __hp_nmemb) {
		void *tmp;
		tmp = realloc(__hp_list, __hp_nmemb + __HEAP_POOL_ALLOC_STEP);
		if(__builtin_expect(tmp == NULL, 0)) {
			return ((struct heap_pool*)-ENOMEM);
		}
		__hp_list = tmp;
		__hp_last = __hp_list + __hp_nmemb; 
	}

	__hp_last->hp_nmemb = nb;
	__hp_last->hp_size  = size;
	__hp_last->hp_curr  = (__hp_last->hp_mem = calloc(nb, size));
	return __hp_last++;
}

static void* __hp_realloc_mem_chunk(struct heap_pool const *const p) 
{
	void *ret;
	ret = realloc(p->hp_mem,
		p->hp_nmemb + p->hp_size * __HEAP_POOL_ALLOC_CHUNK_STEP);

	return ret;
}

static void* __hp_realloc_mem_desc(struct heap_pool const *const p)
{
	void *ret;
	size_t nsize = 
		(p->hp_nmemb + __HEAP_POOL_ALLOC_CHUNK_STEP) * sizeof(size_t); 
	ret = realloc(p->hp_index, nsize);
	return ret;
}

static int __hp_grow_pool_mem(struct heap_pool *const hp)
{
	void *tmp;
	size_t i, max;
	size_t *walker;
	tmp = __hp_realloc_mem_chunk(hp);
	if(!tmp) 
		return -ENOMEM;
	hp->hp_mem = tmp;

	tmp = __hp_realloc_mem_desc;
	if(!tmp)
		return -ENOMEM;
	hp->hp_index = tmp;
	walker = hp->hp_index + hp->hp_nmemb; 
	max    = hp->hp_nmemb + __HEAP_POOL_ALLOC_CHUNK_STEP;
	for(i = hp->hp_nmemb; 
			i < hp->hp_nmemb +  __HEAP_POOL_ALLOC_CHUNK_STEP; 
			++i, ++walker) {
		*walker = i; 
	}
	hp->hp_nmemb = max;
	return 0;

}

void* hp_alloc_mem(struct heap_pool *const hp)
{
	if(__builtin_expect(hp == NULL, 0)) {
		return ((void*)-EINVAL);
	}
	if(!__hp_pool_has_chunk_left(hp)) {
		int ret = __hp_grow_pool_mem(hp);
		if(ret) {
			return (void*)ret;
		}
	}
	return hp->hp_curr++;	
}


void hp_free_mem(struct heap_pool *const hp, void *_mem)
{
	const uint8_t *mem = (uint8_t*)_mem;
	const uint8_t *max_bound = &hp->hp_mem[hp->hp_size*hp->hp_nmemb];

	if(!(mem>=hp->hp_mem && mem <= max_bound)) {
		return;
	}




}


#endif /* __HEAP_POOL_H */
