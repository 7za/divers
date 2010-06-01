#include "heap_looker.h"

#ifdef HEAP_LOOKER_TRACE_ON

#include <stdio.h>
#include <execinfo.h>
#include <stdlib.h>

#define __USE_GNU
#include <dlfcn.h>

static	FILE *bufffp = NULL; 


void
heap_looker_exit(void)
{
	if(bufffp){
		fclose(bufffp);
		bufffp = NULL;
	}
}


void
heap_looker_init(FILE *fp)
{
	bufffp = fp;
}



#define HEAP_LOOKER_WRITE_ALLOCBACKTRACE(addr, len)								\
	do{																			\
		fprintf(bufffp, "action=alloc;addr=%p;size=%zd;callby=[", addr, len);	\
		fprintf(bufffp, "%p]\n", __builtin_return_address(1));					\
	}while(0)


#define HEAP_LOOKER_WRITE_FREEBACKTRACE(addr)					\
	do{															\
		fprintf(bufffp, "action=free;addr=%p;stack=[", addr);	\
		fprintf(bufffp, "%p]\n", __builtin_return_address(1));	\
	}while(0)



void*
malloc(size_t len)
{
	static void* (*malloc_ptr)(size_t) = NULL;
	void* ptr = NULL;

	if(malloc_ptr == NULL){
		malloc_ptr = dlsym(RTLD_NEXT, "malloc");
	}
	if(malloc_ptr){
		ptr = malloc_ptr(len);
	}

	if(bufffp){
		HEAP_LOOKER_WRITE_ALLOCBACKTRACE(ptr, len);
	}
	return ptr;
}

void*
calloc(size_t a, size_t b)
{

	static void* (*calloc_ptr)(size_t, size_t) = NULL;
	void* ptr = NULL;

	if(calloc_ptr == NULL){
		calloc_ptr = dlsym(RTLD_NEXT, "calloc");
	}
	if(calloc_ptr){
		ptr = calloc_ptr(a, b);
	}

	if(bufffp){
		HEAP_LOOKER_WRITE_ALLOCBACKTRACE(ptr, a*b);
	}
	return ptr;
}

void*
realloc(void *base, size_t newsize)
{
	static void* (*realloc_ptr)(void*, size_t) = NULL;
	void* ptr = NULL;

	if(realloc_ptr == NULL){
		realloc_ptr = dlsym(RTLD_NEXT, "realloc");
	}
	if(realloc_ptr){
		ptr = realloc_ptr(base, newsize);
	}

	if(bufffp){
		HEAP_LOOKER_WRITE_ALLOCBACKTRACE(ptr, newsize);
	}
	return ptr;
}

void
free(void *base)
{
	static void (*free_ptr)(void*) = NULL;

	if(free_ptr == NULL){
		free_ptr = dlsym(RTLD_NEXT, "free");
	}
	if(free_ptr){
		free_ptr(base);
	}
	if(bufffp){
		HEAP_LOOKER_WRITE_FREEBACKTRACE(base);
	}
}

#else

void
heap_looker_exit(void) {}

void
heap_looker_init(FILE *fp) {}



#endif
