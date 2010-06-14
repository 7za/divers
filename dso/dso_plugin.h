#ifndef DSO_PLUGIN_H
#define DSO_PLUGIN_H


#ifndef  likely
# define likely(x)       __builtin_expect((x),1)
#endif

#ifndef  unlikely
# define unlikely(x)     __builtin_expect((x),0)
#endif


struct dso_plugin_desc
{
	void *dso_vmem;
	void *dso_vsym;
};


int
dso_plugin_init(struct dso_plugin_desc *const, char *const, char *const);

void
dso_plugin_exit(struct dso_plugin_desc *const);



#define dso_plugin_get_entry(dso, entry)							\
            ({                                                      \
                typeof(entry) _ret = NULL;                          \
                if(likely((dso)!=NULL && (dso)->dso_vsym != NULL)){ \
			        _ret = (typeof(_ret))((dso)->dso_vsym);			\
                }                                                   \
                _ret;                                               \
             })

#define dso_plugin_get_sym(dso, getter, symname)					\
	({																\
		typeof(getter) _ret = NULL;									\
		if(likely(dso)){											\
			dso->dso_vsym = dlsym(dso->dso_vmem, symname);			\
			_ret = dso_plugin_get_entry(dso, getter);				\
		}															\
		_ret;														\
	 })



#endif
