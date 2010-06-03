#include "sym_resolver.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void *toto;

void f1()
{
    printf("hello\n");
    toto = __builtin_return_address(0);
printf("%p\n", toto);
    
}



int main()
{
    struct sym_resolv_desc desc;
    int i = 0;

    void *un[]    = {f1, f1 + 5,malloc, sym_resolv_get_funcname_by_addr, toto};

    struct sym_resolv res[5];

    if(sym_resolv_open(&desc)< 0){
	printf("unable to open procfile\n");
        return 0;
    }
f1();
un[4] = toto;

    sym_resolv_get_funcname_by_addr(&desc, un, res, 5);
    for(i = 0; i < 5; i++)
      printf("search=%p true = %p name = %s\n", un[i], res[i].sr_symaddr, res[i].sr_symname);

    sym_resolv_close(&desc);
return 0;

}
