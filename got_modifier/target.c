int main(void)
{
  printf("%d\n", getpid());
  int i = 0;
  for(;;)
  {
	printf("%d\n", i++);
	sleep(5);
  }
}
 
