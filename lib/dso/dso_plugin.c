#include "dso_plugin.h"
#include <dlfcn.h>
#include <stddef.h>
#include <stdio.h>

#ifndef  likely
# define likely(x)       __builtin_expect((x),1)
#endif

#ifndef  unlikely
# define unlikely(x)     __builtin_expect((x),0)
#endif


void
dso_plugin_exit(struct dso_plugin *const p)
{
	if(likely(p != NULL && p->dso_vmem != NULL)){
		dlclose(p->dso_vmem);
	}
}



int
dso_plugin_init(struct dso_plugin *const p,
                char *const file,
                char *const ent)
{
	if(unlikely((p == NULL) || (file == NULL) || (ent == NULL))){
		return (-1);
	}

	p->dso_vmem = dlopen(file, RTLD_LAZY);
	if(p->dso_vmem == NULL){
		goto dso_plugin_init_dlopen_err;
	}

	p->dso_vsym = dlsym(p->dso_vmem, ent);
	if(p->dso_vsym == NULL){
		goto dso_plugin_init_dlsym_err;
	}

	return 0;

dso_plugin_init_dlsym_err:
	dlclose(p->dso_vmem);
dso_plugin_init_dlopen_err:
	fprintf(stderr,"%s\n", dlerror());
	return (-1);
}
