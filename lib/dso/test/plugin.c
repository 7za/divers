#include "plugin.h"


void func(char *p)
{
  printf("hello plugin = %s\n", p);
}


static struct plugin_test test = {func};

struct plugin_test* get_plugintest()
{
  return &test;
}
