#include <stdio.h>
#include <execinfo.h>
#include <stdlib.h>

#define __USE_GNU
#include <dlfcn.h>

static	FILE *bufffp = NULL; 
static  void* (*malloc_ptr)(size_t) = NULL;
static  void  (*free_ptr)(void*)    = NULL;
static  void* (*realloc_ptr)(void*, size_t) = NULL;
static  void* (*calloc_ptr)(size_t, size_t) = NULL;


void
heap_looker_exit(void)
{
	if(bufffp){
		fclose(bufffp);
		bufffp = NULL;
	}
}


void
heap_looker_init(void)
{
	
	bufffp = tmpfile();

	if(bufffp == NULL){
		goto heap_looker_init_fopen_err;
	}
	
	malloc_ptr = dlsym(RTLD_NEXT, "malloc");
	if(malloc_ptr){
		goto heap_looker_init_dlsym_malloc_err;
	}

	calloc_ptr = dlsym(RTLD_NEXT, "calloc");
	if(calloc_ptr){
		goto heap_looker_init_dlsym_calloc_err;
	}

	realloc_ptr = dlsym(RTLD_NEXT, "realloc");
	if(realloc_ptr){
		goto heap_looker_init_dlsym_realloc_err;
	}

	free_ptr = dlsym(RTLD_NEXT, "free");
	if(free_ptr){
		goto heap_looker_init_dlsym_free_err;
	}

	return;

heap_looker_init_dlsym_free_err:
heap_looker_init_dlsym_realloc_err:
heap_looker_init_dlsym_calloc_err:
heap_looker_init_dlsym_malloc_err:
	printf("unable to lookup syms\n");
	fclose(bufffp);
	bufffp = NULL;
	return;
heap_looker_init_fopen_err:
	printf("unable to create tmp file\n");
}


#define HEAP_LOOKER_WRITE_ALLOCBACKTRACE(addrstart, len)							\
	do{																				\
		void *array[10];															\
		size_t size;																\
		char **strings;																\
		size_t i;																	\
		size = backtrace (array, sizeof(array)/sizeof(void*));						\
		strings = backtrace_symbols (array, size);									\
		fprintf(stdout, "allocation[start=%p; len=%z; backtrace[", addrstart, len); \
		fprintf(bufffp, "allocation[start=%p; len=%z; backtrace[", addrstart, len); \
		for(i = 1; i < size; i++){													\
			fprintf(stdout, "(%s)", strings[i]);									\
			fprintf(bufffp, "(%s)", strings[i]);									\
		}																			\
		fprintf(stdout,"]]\n");														\
		fprintf(bufffp,"]]\n");														\
		free(strings);																\
	}while(0)


#define HEAP_LOOKER_WRITE_FREEBACKTRACE(addr)										\
	do{																				\
		void *array[10];															\
		size_t size;																\
		char **strings;																\
		size_t i;																	\
		size = backtrace (array, sizeof(array)/sizeof(void*));						\
		strings = backtrace_symbols (array, size);									\
		fprintf(stdout, "liberation[start=%p; backtrace[", addr);					\
		fprintf(bufffp, "liberation[start=%p; backtrace[", addr);					\
		for(i = 1; i < size; i++){													\
			fprintf(bufffp, "(%s)", strings[i]);									\
		}																			\
		fprintf(stdout,"]]\n");														\
		fprintf(bufffp,"]]\n");														\
		free(strings);																\
	}while(0)



void*
malloc(size_t len)
{
	void* ptr = malloc_ptr(len);
	if(bufffp){
		HEAP_LOOKER_WRITE_ALLOCBACKTRACE(ptr, len);
	}
	return ptr;
}

void*
calloc(size_t a, size_t b)
{
	void* ptr = calloc_ptr(a, b);
	if(bufffp){
		HEAP_LOOKER_WRITE_ALLOCBACKTRACE(ptr, a*b);
	}
	return ptr;
}

void*
realloc(void *base, size_t newsize)
{
	void* ptr = realloc_ptr(base, newsize);
	if(bufffp){
		HEAP_LOOKER_WRITE_ALLOCBACKTRACE(ptr, newsize);
	}
	return ptr;
}

void
free(void *base)
{
	free_ptr(base);
	if(bufffp){
		HEAP_LOOKER_WRITE_FREEBACKTRACE(base);
	}
}


