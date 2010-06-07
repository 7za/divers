#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#define __USE_GNU
#include <sys/ucontext.h>
#include <ucontext.h>
#include <dlfcn.h>

#include "dbg_print/dbgprint.h"
#include "sym_resolv/sym_resolver.h"
#include "gnucext/gnuc_extension.h"


static const char*
sig_looker_sigsegvcode_itoa(int code)
{
    static char const *const tab[] =  { [SEGV_MAPERR] = "addr_not_mapped", 
                                        [SEGV_ACCERR] = "addr_not_allowed" 
                                };
    if(code == SEGV_MAPERR || code == SEGV_ACCERR){
        return tab[code];
    }
    return "code_unknown";
}

static void
sig_looker_on_segv (int signum, siginfo_t * info, void *__ptr)
{
    ucontext_t *ucontext = (ucontext_t *) __ptr;
    greg_t      gip = 0;
    void *ptr = NULL;
	struct sym_resolv_desc desc;

    if(ucontext){
        gip = ucontext->uc_mcontext.gregs[14];
    }
    
    dbg_printf(stderr, "signal=%s;", sys_siglist[signum]);
    dbg_printf(stderr, " code=%s;" ,sig_looker_sigsegvcode_itoa(info->si_code));
    dbg_printf(stderr, " addr_access=%p;", info->si_addr);
    dbg_printf(stderr, " sender=%d", info->si_pid);

    if(gip){
        ptr = (void*)gip; 
    } else {
        ptr = __builtin_return_address(0);
    }
	if(!sym_resolv_open(&desc)){
		void *addr[] = {ptr};
		struct sym_resolv res[1];
		sym_resolv_get_funcname_by_addr(&desc, addr, res, 1);
		dbg_printf(stderr, " in=[%s,%p]\n", res[0].sr_symname, res[0].sr_symaddr);
		sym_resolv_close(&desc);
	}
}


static void __attribute__ ((constructor))
sig_looker_init(void)
{
    struct sigaction action;
    
    memset (&action, 0, sizeof (action));
    
    action.sa_sigaction = sig_looker_on_segv;
    action.sa_flags     = (int)(SA_SIGINFO | SA_RESETHAND);
    
    if (sigaction (SIGSEGV, &action, NULL) < 0){
        dbg_printf(stderr, "unable to register segv callback\n");
    }
    dbg_printf(stdout, "segv callback register\n");
}

void test()
{
    int *ptr = NULL;
    puts(__func__);

    *ptr = 4;
}

int main()
{
    printf("coucou\n");
    dbg_printf(stdout, "plop\n");
    test();
    return 0;
}
