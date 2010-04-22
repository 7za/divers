#include <stdio.h>



char line[] = "b6327000-b633a000 r-xp 00000000 08:01 21017      /usr/lib/libelf-0.8.13.so";

int main()
{
  int n;
  char *ptr = line;
  char adrstat[512], adrend[512], perm[5], dunno[512], lib[512];

  sscanf(ptr, "%[^-]-%[^ ] %[^ ] %[^/]%s", adrstat, adrend, perm, dunno, lib);
  puts(adrstat);
  puts(adrend);
  puts(perm);
  puts(dunno);
  puts(lib);

}
