#ifndef DSO_PLUGIN_H
#define DSO_PLUGIN_H

struct dso_plugin
{
	void *dso_vmem;
	void *dso_vsym;
};

int
dso_plugin_init(struct dso_plugin *const, char *const, char *const);

void
dso_plugin_exit(struct dso_plugin *const);



#define dso_plugin_get_entry(dso, entry)                                            \
	    do{                                                                         \
			entry = (typeof(entry)) ((typeof(entry)(*)(void))((dso)->dso_vsym))();  \
		}while(0)


#endif
