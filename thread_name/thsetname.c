#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sched.h>

#define MAX_NAME_LEN 16

static void set_thread_name(char *thread_name)
{
	char name[MAX_NAME_LEN + 1];	/* Name must be <= 16 characters + a null */

	strncpy(name, thread_name, MAX_NAME_LEN);
	name[MAX_NAME_LEN] = 0;
	prctl(PR_SET_NAME, (unsigned long)&name);
}

static void *input_thread(void *arg)
{
	set_thread_name("input_thread");
	while (1)
		sleep(10);
	return NULL;
}

static void *output_thread(void *arg)
{
	set_thread_name("output_thread");
	while (1)
		sleep(10);
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t i_t;
	pthread_t o_t;

	printf("Threads with names\n");

	pthread_create(&i_t, NULL, input_thread, NULL);
	pthread_create(&o_t, NULL, output_thread, NULL);

	/* Wait for both threads to finish */

	printf(	"now, read thread name with: "
		"ps -Leo pid,tid,class,rtprio,ni,pri,psr,pcpu,stat,wchan:14,comm\n");
	pthread_join(i_t, NULL);
	pthread_join(o_t, NULL);

	return 0;
}
