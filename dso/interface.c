#include "interface.h"

#include <stdio.h>

static void
display()
{
  printf("hello plugin\n");
}

static struct zestruct obj = {display};


struct zestruct*
getzestruct()
{
  return &obj;
}
