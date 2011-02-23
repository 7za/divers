#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "heap_pool.h"

#ifndef __HEAP_POOL_ALLOC_STEP
# define __HEAP_POOL_ALLOC_STEP	(5)
#endif

#ifndef __HEAP_POOL_ALLOC_CHUNK_STEP
# define __HEAP_POOL_ALLOC_CHUNK_STEP	(10)
#endif

#ifdef DBG
# define HEAP_POOL_DEBUG(...)  printf(__VA_ARGS__);
#else
# define HEAP_POOL_DEBUG(...)  do{}while(0)
#endif


#if  defined(_BSD_SOURCE) || _XOPEN_SOURCE >= 500
# define heap_pool_getpagesize()	getpagesize()
#else
# define heap_pool_getpagesize()	sysconf(_SC_PAGESIZE)
#endif


#define __ALIGN(x, y) (((x) + (y) - 1) & ~((y) - 1))
#define __ALIGN_PTR(addr, align) ((void*)__ALIGN((unsigned long)(addr), (align)))

#ifdef   HEAP_POOL_OVERFLOW_DBG
typedef  uint32_t chunk_extra_t;	
# define HEAP_POOL_MAGIC 0xcacacaca
#else
typedef  uint8_t  chunk_extra_t;
# define HEAP_POOL_MAGIC 0xca 
#endif

#define HEAP_CHUNK_FREE (0)
#define HEAP_CHUNK_USED (1)


#define HEAP_POOL_GET_NTH_CHUNK(p, nth)	\
	(p+(p)->hpd_offfirstelem + nth *	\
	 	((p)->hpd_holesize + (p)->hpd_esize + sizeof(chunk_extra_t)))

static inline size_t heap_pool_nb_free(struct heap_pool_desc *p)
{
	return p->hpd_enum - p->hpd_firstfree;
}

void* heap_pool_alloc(struct heap_pool_desc *p, heap_pool_addr_t *ret)
{
	size_t nieme;
	if(p->hpd_firstfree == p->hpd_enum) {
		return  (void*)(unsigned long)-ENOMEM;
	}
	nieme = ((size_t*)p->hpd_raw)[p->hpd_firstfree];
	ret->index = p->hpd_firstfree;
	ret->addr  = HEAP_POOL_GET_NTH_CHUNK(p, nieme);
	++p->hpd_firstfree;
	return ret->addr;
}

void heap_pool_free(struct heap_pool_desc *p, heap_pool_addr_t addr)
{
	size_t tmp;
	size_t *array = (size_t*)p->hpd_raw;
	if(unlikely(p->hpd_firstfree == 0)) {
		return;
	}
	--p->hpd_firstfree;

	tmp = array[p->hpd_firstfree];
	array[p->hpd_firstfree] = array[addr.index]; 
}

void heap_pool_destroy(struct heap_pool_desc *desc, char name[])
{
	munmap(desc, desc->hpd_allocsize);
	shm_unlink(name);
}

void heap_pool_close(struct heap_pool_desc *desc)
{
	munmap(desc, desc->hpd_allocsize);
}

static inline int __heap_pool_get_stat(char name[], struct stat *st)
{
	int ret;
	char *ptr = name;
	char _name[PATH_MAX];
	if(name[0] == '/') {
		sprintf(_name, "/dev/shm%s", name);
		ptr = _name;
	}

	ret = stat(ptr, st);
	return ret;
}

static inline struct heap_pool_desc* 
__heap_pool_open(char name [], size_t const size)
{
	struct heap_pool_desc *desc;
	int fd = shm_open(name, O_RDWR, 0666);
	if(fd < 0)
		return ERR_PTR(-EINVAL);
	desc = mmap(NULL, size, PROT_READ | PROT_WRITE,
		MAP_SHARED, fd, 0);
	

	close(fd);
	return desc;
}

struct heap_pool_desc* heap_pool_open(char name[])
{
	struct stat st;
	int ret;

	ret = __heap_pool_get_stat(name, &st);
	/* dont exist */
	if(ret < 0) {
		return ERR_PTR(-errno);	
	}
	return __heap_pool_open(name, st.st_size);
}

struct heap_pool_desc* heap_pool_create_if_needed(char name[],
						size_t const nb,
						size_t const size,
						size_t const align)
{
	struct stat stat;
	int ret;

	ret = __heap_pool_get_stat(name, &stat);
	/* dont exist */
	if(ret < 0) {
		return heap_pool_create(name, nb, size, align);
	}
	return __heap_pool_open(name, stat.st_size);
}

struct heap_pool_desc* heap_pool_create(char name[],
					size_t const nb,
					size_t const size,
					size_t const align)
{
	size_t allocsize;
	size_t chunksize;
	size_t holesize = 0;
	size_t pagesize;
	struct heap_pool_desc *ret = NULL;
	int fd;
	size_t  *walker, i;
	uint8_t *ptr;

	if(align & 0x1) {
		return ERR_PTR(-EINVAL);
	}

	pagesize  = (size_t)heap_pool_getpagesize();
	chunksize = size + sizeof(chunk_extra_t);
	if(align)
		holesize  = __ALIGN(chunksize, align) - chunksize;
	allocsize = sizeof(*ret) + align + nb*(sizeof(size_t) + chunksize);
	allocsize = __ALIGN(allocsize, pagesize);

	HEAP_POOL_DEBUG("allocsize is %zu\n", allocsize);
	HEAP_POOL_DEBUG("holesize is %zu\n", holesize);

	fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0666);
	if(unlikely(fd < 0)) {
		return ERR_PTR(-errno);
	}
	ftruncate(fd, allocsize);

	ret = mmap(NULL, allocsize, PROT_READ | PROT_WRITE, 
		MAP_SHARED, fd, 0);
	if(ret == MAP_FAILED) {
		goto mapfailed;
	}
	close(fd);
	ret->hpd_next  = NULL;
	ret->hpd_esize = size;
	ret->hpd_enum  = 
		((allocsize - align - offsetof(struct heap_pool_desc, hpd_raw)) /
		 (sizeof(size_t) + holesize + chunksize));

	HEAP_POOL_DEBUG("numelem is %zu\n", ret->hpd_enum);

	ret->hpd_holesize  = holesize;
	ret->hpd_allocsize = allocsize;
	ret->hpd_firstfree = 0;
	walker = (size_t*)ret->hpd_raw;

	ptr = (uint8_t*)(walker + ret->hpd_enum);
	if(align)
		ptr    = __ALIGN_PTR(walker + ret->hpd_enum, align);

	HEAP_POOL_DEBUG("ptr go %p to %p\n", walker + ret->hpd_enum, ptr);
	ret->hpd_offfirstelem = (off_t)((char*)ptr - (char*)ret);
	for(i = 0; i < ret->hpd_enum; ++i, ++walker, ptr+=chunksize+holesize) {
		*walker = i;
		*(ptr + size) = HEAP_POOL_MAGIC | HEAP_CHUNK_FREE;
	}

	return ret;

mapfailed:
	close(fd);
	return ret;
}

void heap_pool_delete_shmfile(char name[])
{
	shm_unlink(name);
}
