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
#ifdef DBG
#include <assert.h>
#endif

#ifndef __HEAP_POOL_ALLOC_STEP
# define __HEAP_POOL_ALLOC_STEP	(5)
#endif

#ifndef __HEAP_POOL_ALLOC_CHUNK_STEP
# define __HEAP_POOL_ALLOC_CHUNK_STEP	(10)
#endif

#if  defined(_BSD_SOURCE) || _XOPEN_SOURCE >= 500
# define heap_pool_getpagesize()	getpagesize()
#else
# define heap_pool_getpagesize()	sysconf(_SC_PAGESIZE)
#endif


#define __ALIGN(x, y) (((x) + (y) - 1) & ~((y) - 1))
#define __ALIGN_PTR(addr, align) ((void*)__ALIGN((unsigned long)(addr), (align)))
#define HEAP_CHUNK_FREE (0)
#define HEAP_CHUNK_USED (1)

void heap_pool_destroy(struct heap_pool_desc *desc, char name[])
{
	(void)name;
	munmap(desc, desc->hpd_allocsize);
#ifdef HP_HAVE_SHM
	shm_unlink(name);
#endif
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
#ifdef HP_HAVE_SHM
	struct stat st;
	int ret;
	ret = __heap_pool_get_stat(name, &st);
	/* dont exist */
	if(ret < 0) {
		return ERR_PTR(-errno);	
	}
	return __heap_pool_open(name, st.st_size);

#else
	(void)name;
	return NULL;
#endif
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

#define is_power_of_two(val)	((val) == 1 || !((val) & (val - 1))) 

static size_t __compute_new_alignment(size_t const curr_align)
{
	size_t taille_index = sizeof(size_t);
	size_t new_align = curr_align;
	if(taille_index > curr_align) {
		new_align += taille_index - curr_align;
		new_align =  __ALIGN(new_align, curr_align);
	}
	HEAP_POOL_DEBUG("alignment queried is %zu, new alignment with size_t is %zu\n",
			curr_align, new_align);
	return new_align;
}

struct heap_pool_desc* heap_pool_create(char name[],
					size_t const nb,
					size_t const size,
					size_t const _align)
{
	size_t allocsize;
	size_t align;
	size_t chunksize;
	size_t holesize = 0;
	size_t pagesize;
	struct heap_pool_desc *ret = NULL;
	int fd = 0;
	size_t  *walker, i;
	uint8_t *ptr;
	unsigned int mapflag;

	HEAP_POOL_DEBUG("Starting with %d %d %d\n", nb, size, _align);
	if(_align && !is_power_of_two(_align)) { 
		return ERR_PTR(-EINVAL);
	}
	align = (!_align) ? sizeof(size_t) : __compute_new_alignment(_align);

	pagesize  = (size_t)heap_pool_getpagesize();
	chunksize = size + CHUNK_EXTRA_SIZE; 
	holesize = sizeof(size_t);
	if(_align > 1) {
		size_t next_val = chunksize + holesize;
		size_t diff     = __ALIGN(next_val, _align) - next_val;
		holesize += diff;
	}

	allocsize = sizeof(*ret) + align - holesize +  
		nb*(sizeof(size_t) + chunksize + holesize);
	allocsize = __ALIGN(allocsize, pagesize);

	HEAP_POOL_DEBUG("need %zu pages\n", allocsize / pagesize);
	HEAP_POOL_DEBUG("allocsize is %zu\n", allocsize);
	HEAP_POOL_DEBUG("holesize is %zu\n", holesize);

#ifdef HP_HAVE_SHM
	fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0666);
	if(unlikely(fd < 0)) {
		return ERR_PTR(-errno);
	}
	ftruncate(fd, allocsize);

	mapflag = MAP_SHARED
#else
	(void)name;
	mapflag = MAP_ANONYMOUS | MAP_PRIVATE;
#endif

	ret = mmap(NULL, allocsize, PROT_READ | PROT_WRITE, 
		mapflag, fd, 0);
	if(ret == MAP_FAILED) {
		goto mapfailed;
	}
#ifdef HP_HAVE_SHM
	close(fd);
#endif
	ret->hpd_next = NULL;
	ret->hpd_szel = size;
	ret->hpd_nrel = 
		((allocsize - align - sizeof(*ret)) /
		 (sizeof(size_t) + holesize + chunksize));
	ret->hpd_nralloc = 0;

	HEAP_POOL_DEBUG("numelem is %zu\n", ret->hpd_nrel);

	ret->hpd_firstfree = 0;

	ret->hpd_szck = holesize + ret->hpd_szel + CHUNK_EXTRA_SIZE; 

	ret->hpd_holesize  = holesize;
	ret->hpd_allocsize = allocsize;

	walker = (size_t*)ret->hpd_raw;
	ptr = (uint8_t*)(walker + ret->hpd_nrel);
	HEAP_POOL_DEBUG("ptr before allocating hole %p\n", ptr);
	uint8_t *ptr2 = __ALIGN_PTR(ptr + align, align);
	HEAP_POOL_DEBUG("ptr after allocating hole %p\n", ptr2 - ptr);
	ptr = ptr2;


	HEAP_POOL_DEBUG("ptr go %p to %p\n", walker + ret->hpd_nrel, ptr);
	ret->hpd_offfirstelem = (off_t)((char*)ptr - (char*)ret);
#define INDEX_INDEX_INDEXNEXT (1)

	for(i = 0; i < ret->hpd_nrel; ++i, walker++, ptr+=chunksize+holesize) {
		*walker     = (i+1);
#ifdef HEAP_POOL_OVERFLOW_DBG
		*(ptr + size) = HEAP_POOL_MAGIC | HEAP_CHUNK_FREE;
#endif
		*(size_t*)(ptr - sizeof(size_t)) = i;
		
#ifdef DBG
		if(_align)
			assert((unsigned long)ptr % _align == 0);
#endif

	}

	HEAP_POOL_DEBUG("-------------------------\n");
mapfailed:

#ifdef HP_HAVE_SHM
	close(fd);
#endif
	return ret;
}

void heap_pool_delete_shmfile(char name[])
{
#ifdef HP_HAVE_SHM
	shm_unlink(name);
#endif
	(void)name;
}
