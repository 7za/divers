#ifndef HEAP_POOL_H
#define HEAP_POOL_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>


#ifndef unlikely 
# define unlikely(x)	(__builtin_expect((x), 0))
#endif

#ifndef likely 
# define likely(x)	(__builtin_expect((x), 1))
#endif

#define MAX_ERRNO 4095
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

static inline void* ERR_PTR(long error)
{
	return (void *) error;
}

static inline long PTR_ERR(const void *ptr)
{
	return (long) ptr;
}

static inline long IS_ERR(const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)ptr);
}

static inline long IS_ERR_OR_NULL(const void *ptr)
{
	return !ptr || IS_ERR_VALUE((unsigned long)ptr);
}

struct heap_pool_desc {
	struct   heap_pool_desc *hpd_next;
#ifdef HEAP_POOL_DBHOOK
	void* (*hpdbg_on_alloc)(void*);
	void* (*hpdbg_on_free)(void*);
	void* (*hpdbg_on_dblfree)(void*);
	void* (*hpdbg_on_allocerr)(void*);
#endif
	size_t   hpd_esize;
	size_t   hpd_enum;
	size_t   hpd_holesize;
	size_t   hpd_firstfree;
	size_t   hpd_allocsize;
	uint8_t  *hpd_firstelem;
	uint8_t  hpd_raw[];
};

static inline void* heap_pool_raw_to_desc(char raw[])
{
	return raw - (size_t) ((struct heap_pool_desc*)NULL)->hpd_raw;
}

typedef struct {
	void	*addr;
	size_t	index;
} heap_pool_addr_t;


void* heap_pool_alloc(struct heap_pool_desc *p, heap_pool_addr_t *ret);

void heap_pool_free(struct heap_pool_desc *p, heap_pool_addr_t addr);

void heap_pool_destroy(struct heap_pool_desc *desc, char name[]);

void heap_pool_close(struct heap_pool_desc *desc);

struct heap_pool_desc* heap_pool_open(char name[]);

struct heap_pool_desc* heap_pool_create(char name[],
					size_t const nb,
					size_t const size,
					size_t const align);

struct heap_pool_desc* heap_pool_create_if_needed(char name[],
						size_t const nb,
						size_t const size,
						size_t const align);


#endif
