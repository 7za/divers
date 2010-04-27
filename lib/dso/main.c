#include <stdio.h>

#include "dso_plugin.h"
#include "interface.h"


int main()
{
  struct dso_plugin_t plugin;
  struct zestruct *zestruct = NULL;

  dso_plugin_init(&plugin, "libinterface.so", "getzestruct");

  dso_plugin_get_entry_sym(zestruct, &plugin);

  zestruct->display();

  dso_plugin_exit(&plugin);

}
