#include "dso_plugin.h"
#include <dlfcn.h>
#include <stddef.h>

#ifndef  likely
# define likely(x)       __builtin_expect((x),1)
#endif

#ifndef  unlikely
# define unlikely(x)     __builtin_expect((x),0)
#endif


#ifdef   DSO_PLUGIN_DBG
# define dso_plugin_trace() dlerror()
#else
# define dso_plugin_trace() do{}while(0) 
#endif

int
dso_plugin_init( struct dso_plugin_t *const ptr, 
                 char *const fname, 
                 char *const cname)
{
  if(unlikely(ptr   == NULL) ||
     unlikely(fname == NULL) ||
     unlikely(cname == NULL) ) {
      return -1;   
  }
  

  ptr->dso_vmem = dlopen(fname, RTLD_LAZY);
  if(unlikely(ptr->dso_vmem == NULL)){
      return -1;
  }

  ptr->dso_vsym = dlsym(ptr->dso_vmem, cname);
  if(ptr->dso_vsym == NULL){
      return -1;
  } 
  return 0;
}

void
dso_plugin_exit(struct dso_plugin_t *ptr)
{
  if(likely(ptr != NULL) && likely(ptr->dso_vmem != NULL)){
    dlclose(ptr->dso_vmem);
  }
}
