#include "plugin.h"
#include "../dso_plugin.h"
#include <stdio.h>


int main()
{
	struct dso_plugin_desc    plugin;
	struct plugin_test  *test = NULL;

	if(dso_plugin_init(&plugin, "./libplugintest.so", "get_plugintest") < 0){
		return (0);
	}

	test = dso_plugin_get_entry(&plugin, *test);

	test->display(__FILE__);

	dso_plugin_exit(&plugin);
	return 0;
}
