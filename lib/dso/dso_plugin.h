#ifndef DSO_PLUGIN_H
#define DSO_PLUGIN_H



struct dso_plugin;


int
dso_plugin_init(struct dso_plugin *const, char *const, char *const);

void
dso_plugin_exit(struct dso_plugin *const);



#define dso_plugin_get_entry(dso, entry)                                            \
			entry = (typeof(entry)) ((typeof(entry)(*)(void))((dso)->dso_vsym))()  


#endif
