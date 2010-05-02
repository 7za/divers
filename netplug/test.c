#include <stdio.h>

int main()
{
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0)
	  printf("error\n");
  else {
    printf("ok\n");
	close(sock);
  }

}
