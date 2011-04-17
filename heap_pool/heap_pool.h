#ifndef HEAP_POOL_H
#define HEAP_POOL_H
#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>


//#define DBG
#ifdef DBG
#include <stdio.h>
# define HEAP_POOL_DEBUG(...)  printf(__VA_ARGS__);
#else
# define HEAP_POOL_DEBUG(...)  do{}while(0)
#endif



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

/*!
 * @brief : shm descriptor
 * @field : hpd_next : contains 0 if there are not linked descriptor
 * 	it is usable only on runtime, because adresse became unvalid after
 * 	munmap. When we try to open existing shm, we check hpd_next, and if 
 * 	it is not equal to 0, we try to open other descriptor with same name + postfix number
 *	the shm descriptor has the following organization
 *	+-------------------+----------------+-------------------+
 *	| misc_information  |array of index  |  buffers elements |
 *	+-------------------+----------------+-------------------+
 *
 *	The array index is used for managing allocation and liberation.
 *	For instance, when we want a new element, the first step is to
 *	get the first_free field in the misc_information. Then we get value
 *	in array_index[first_free]. This contains index of the buffer element we
 *	can get.
 *
 * @field : hpd_esize : size of one elem
 * @field : hpd_enum  : number of elem 
 * @field : hpd_holesize  : hole we have to put between element, due to alignment queried
 * @field : hpd_firstfree : this contains index of the array containing index of elem
 * @field : hpd_offfirst_elem : this is the offset we have to add to heap_pool_desc_addr to 
 * 	get the address of the element
 * @field : hpd_raw : flexible array which mark the start of the raw
 *
 */
struct heap_pool_desc {
	struct   heap_pool_desc *hpd_next;
#ifdef HEAP_POOL_DBHOOK
	void* (*hpdbg_on_alloc)(void*);
	void* (*hpdbg_on_free)(void*);
	void* (*hpdbg_on_dblfree)(void*);
	void* (*hpdbg_on_allocerr)(void*);
#endif
	size_t hpd_szel;
	size_t hpd_nrel;

	size_t hpd_nralloc;

	size_t hpd_firstfree;

	size_t hpd_szck;
	size_t hpd_holesize;

	size_t   hpd_allocsize;
	off_t	 hpd_offfirstelem;
	uint8_t  hpd_raw[];
};

#define HEAP_POOL_ADDR_IN_BNDS(p,addr)	\
		((addr)>(char*)p && (addr) < (char*)p + (p)->hpd_allocsize) 

/*
 * this inline function give the address of the heap_pool_desc, according 
 * the value of hpd_raw
 */
static inline void* heap_pool_raw_to_desc(char raw[])
{
	return raw - (size_t) ((struct heap_pool_desc*)NULL)->hpd_raw;
}

/*!
 * @brief : destroy (ie, munmap + unlink) an opened shm
 */
void heap_pool_destroy(struct heap_pool_desc *desc, char name[]);

/*!
 * @brief : close (ie munmap) an opened shm
 */
void heap_pool_close(struct heap_pool_desc *desc);

/*!
 * @brief : unlink an shm, even if we have no descriptor using it
 */
void heap_pool_delete_shmfile(char name[]);

/*!
 * @brief : open an existing shm. return ERR_PTR if this shm doesn't exist
 */
struct heap_pool_desc* heap_pool_open(char name[]);


/*!
 * @brief : create an shm and descriptor mananging it.
 * This function can failed if shm file already exist, so we have to check
 * return value with IS_ERR
 */
struct heap_pool_desc* heap_pool_create(char name[],
					size_t const nb,
					size_t const size,
					size_t const align);

/*!
 * @brief : open an shm if exists, otherwise, create it
 */
struct heap_pool_desc* heap_pool_create_if_needed(char name[],
						size_t const nb,
						size_t const size,
						size_t const align);
#ifdef   HEAP_POOL_OVERFLOW_DBG
typedef  uint32_t chunk_extra_t;	
# define HEAP_POOL_MAGIC 0xcacacaca
#else
typedef  uint8_t  chunk_extra_t;
# define HEAP_POOL_MAGIC 0xca 
#endif


/* TODO : this macro should be more efficient */
#define HEAP_POOL_GET_NTH_CHUNK(p, nth)	\
	((char*)p+(p)->hpd_offfirstelem + nth *	(p)->hpd_szck)

static inline size_t heap_pool_nb_free(struct heap_pool_desc *p)
{
	return p->hpd_nrel - p->hpd_nralloc;
}


__attribute__((always_inline))
static inline void* heap_pool_alloc(struct heap_pool_desc *p)
{
	size_t *array_index;
	size_t old_ff;
	if(!(heap_pool_nb_free(p))) {
		return  (void*)(unsigned long)-ENOMEM;
	}
	old_ff = p->hpd_firstfree;
	array_index  = (size_t*)(p->hpd_raw);

	p->hpd_firstfree = array_index[old_ff];
	
	++p->hpd_nralloc;

	return HEAP_POOL_GET_NTH_CHUNK(p, old_ff);
}


__attribute__((always_inline))
static inline void heap_pool_free(struct heap_pool_desc *p, void *addr)
{
	size_t index_index = *(size_t*)(addr - sizeof(size_t));
	size_t *array_index  = (size_t*)(p->hpd_raw);
	array_index[index_index] = p->hpd_firstfree;
	p->hpd_firstfree = index_index;
	--p->hpd_nralloc;
}



#endif

