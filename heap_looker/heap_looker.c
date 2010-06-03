#include <stdio.h>

#ifndef __unused
# define __unused __attribute__((unused))
#endif


#ifdef HEAP_LOOKER_TRACEON
#include <execinfo.h>
#include <stdlib.h>

#define __USE_GNU
#include <dlfcn.h>

static	FILE *_fp = NULL; 


void
__heap_looker_exit(void)
{}


void
__heap_looker_init(FILE *fp)
{
	_fp = fp ? fp : stdout;
}


#define HEAP_LOOKER_WRITE_ALLOCBACKTRACE(addr, len)							        \
	do{																				\
        Dl_info dinfo;                                                              \
        void *__ptr;                                                                \
        __ptr = __builtin_return_address(0);                                        \
		fprintf(_fp, "[action=alloc;start=%p; len=%zd; call=", addr, len);          \
        if(dladdr(__ptr, &dinfo) && dinfo.dli_sname){                               \
            fprintf(_fp, "%s]\n", dinfo.dli_sname);                                 \
        } else{                                                                     \
            fprintf(_fp, "%p]\n", __ptr);                                           \
        }                                                                           \
	}while(0)


#define HEAP_LOOKER_WRITE_FREEBACKTRACE(addr)										\
	do{																				\
        Dl_info dinfo;                                                              \
        void *__ptr = __builtin_return_address(0);                                  \
		fprintf(_fp, "[action=free;start=%p; call=", addr);                         \
        if(dladdr(__ptr, &dinfo) && dinfo.dli_sname){                               \
            fprintf(_fp, "%s]\n", dinfo.dli_sname);                                 \
        } else{                                                                     \
            fprintf(_fp, "%p]\n", __ptr);                                           \
        }                                                                           \
	}while(0)


#define __load_func_if(ptr)                     \
    do{                                         \
        if(!ptr){                               \
            ptr = dlsym(RTLD_NEXT, __func__);   \
        }                                       \
    }while(0)                                   \

void*
malloc(size_t len)
{
    static void* (*malloc_ptr)(size_t) = NULL;
	void *ret = NULL;
    
    __load_func_if(malloc_ptr);

    if(malloc_ptr){
        ret = malloc_ptr(len);
	    if(_fp && ret){
		    HEAP_LOOKER_WRITE_ALLOCBACKTRACE(ret, len);
	    }
    }
	return ret;
}


void*
calloc(size_t a, size_t b)
{
    static void* (*calloc_ptr)(size_t, size_t) = NULL;
	void *ret = NULL;
    
    __load_func_if(calloc_ptr);

    if(calloc_ptr){
        ret = calloc_ptr(a, b);
	    if(_fp && ret){
		    HEAP_LOOKER_WRITE_ALLOCBACKTRACE(ret, a * b);
	    }
    }
	return ret;
}

void*
realloc(void *base, size_t a)
{
    static void* (*realloc_ptr)(void*, size_t) = NULL;
	void *ret = NULL;
    
    __load_func_if(realloc_ptr);

    if(realloc_ptr){
        ret = realloc_ptr(base, a);
	    if(_fp && ret){
		    HEAP_LOOKER_WRITE_ALLOCBACKTRACE(ret, a);
	    }
    }
    return ret;
}

void
free(void *base)
{
    static void (*free_ptr)(void*) = NULL;
    
    __load_func_if(free_ptr);

    if(free_ptr){
        free_ptr(base);
        if(_fp){
            HEAP_LOOKER_WRITE_FREEBACKTRACE(base);
        }
    }
}

#else


static inline void __heap_looker_init(FILE *fp __unused) {}

static inline void __heap_looker_exit(void) {}


#endif


extern typeof(__heap_looker_exit)
heap_looker_exit __attribute__((alias("__heap_looker_exit")));

extern typeof(__heap_looker_init)
heap_looker_init __attribute__((alias("__heap_looker_init")));


