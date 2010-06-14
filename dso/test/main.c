#include "plugin.h"
#include "../dso_plugin.h"
#include <stdio.h>


int main()
{
	struct dso_plugin_desc    plugin;
	struct plugin_test  *test  = NULL;
	struct plugin_test  *test2 = NULL;
	struct plugin_test* (*ptr)(void) = NULL;

	if(dso_plugin_init(&plugin, "./libplugintest.so", "get_plugintest") < 0){
		return (0);
	}

	ptr  = dso_plugin_get_entry(&plugin, ptr);
	test = ptr(); 
	test->display(__FILE__);

	dso_plugin_exit(&plugin);
	return 0;
}
