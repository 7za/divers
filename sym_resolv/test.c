#include "sym_resolver.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define __USE_GNU
#include <dlfcn.h>

void *toto;

static void f1()
{
    printf("hello\n");
    toto = __builtin_return_address(0);
    printf("%p\n", toto);
}

#include <sys/time.h>


int main(void)
{
    struct sym_resolv_desc desc;
    int i = 0;
    Dl_info info;
    struct timeval ts1, ts2, resu;

    void *un[]    = {f1, f1 + 5,malloc, sym_resolv_addr, toto};

    struct sym_resolv res[5];

    if(sym_resolv_open(&desc)< 0){
	printf("unable to open procfile\n");
        return 0;
    }

    f1();

    un[4] = toto;

    gettimeofday(&ts1, 0);
    sym_resolv_addr(&desc, un, res, 5);
    for(i = 0; i < 5; i++)
        printf("search=%p true = %p name = %s\n", un[i], res[i].sr_symaddr, res[i].sr_symname);
    gettimeofday(&ts2, 0);

    timersub(&ts1, &ts2, &resu);
    printf("%ld %ld\n", resu.tv_sec, resu.tv_usec);
	Elf32_Sym *ptr = sym_resolv_symbol(&desc, "toto");


    printf("test with dladdr\n");

    gettimeofday(&ts1, 0);
    for(i = 0; i < 5; i++){
        if(dladdr(un[i], &info)){
            printf("serach=%p find %p -> %s\n",un[i], info.dli_saddr, info.dli_sname);
        } else {
            printf("%p not found\n", un[i]);
        }
    }
    gettimeofday(&ts2, 0);
    timersub(&ts1, &ts2, &resu);
    printf("%ld %ld\n", resu.tv_sec, resu.tv_usec);


	if(ptr){
		printf("value=%p size=%u info=%u\n", ptr->st_value, ptr->st_size, ptr->st_info);
	}
	

    sym_resolv_close(&desc);
    return 0;
}
