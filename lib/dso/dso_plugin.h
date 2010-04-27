#ifndef DSO_PLUGIN_H
#define DSO_PLUGIN_H


struct dso_plugin_t
{
    void *dso_vmem;
    void *dso_vsym;
};


#define dso_plugin_get_entry_sym(addr, dso) \
    do{                                                                       \
        (addr) = (typeof(addr)) ((typeof(addr) (*)(void))(dso)->dso_vsym)(); \
    }while(0)


int 
dso_plugin_init(struct dso_plugin_t *, char *const, char *const);

void
dso_plugin_exit(struct dso_plugin_t*);

#endif
