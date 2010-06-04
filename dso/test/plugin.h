#ifndef PLUGIN_TEST
#define PLUGIN_TEST

struct plugin_test
{
  void (*display)(char *ptr);
};

struct plugin_test*
get_plugintest(void);

#endif

