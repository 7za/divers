#include "ioloop.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

static void
fcbk_clean(void *d)
{
    struct ioloop_event_desc *d1 = d;
    FILE *fp;

    if(d1){
        fp = d1->iol_ctx;
        if(fp){
            pclose(fp);
            d1->iol_ctx = NULL;
        }
    }
    printf("clean-up fd %d\n", d1->iol_fd);
}


static ssize_t
f2cbk(int fd __attribute__((unused)), void *data)
{
    struct ioloop_event_desc *d1 = data;
    ssize_t ret = -1;
    if( d1 && (d1->iol_rev == EPOLLIN)){
        char buff[512];
        ssize_t nbread = read(d1->iol_fd, buff, 512);
        ret = nbread;
    }
    return ret;
}


void f2()
{
    FILE *fp;
    DECLARE_IOLOOP_EVENT_DESC(f2iol);
    
    fp = popen("for i in $(seq 1 10); do echo $i; done; exit", "r");
    f2iol.iol_clean = fcbk_clean;
    f2iol.iol_ev    = EPOLLIN | EPOLLRDHUP;
    f2iol.iol_ctx   = fp;
    f2iol.iol_fd    = fileno(fp);
    f2iol.iol_func  = f2cbk;
    printf("%s -> fd = %d\n", __func__, f2iol.iol_fd);

    if(iol_add_event(&f2iol) < 0){
        printf("gros echec 2\n");
        exit(1);
    }
}

void f3()
{
    puts(__func__);
    f2();
}


void*
th_cbk(void *data)
{
    struct timespec spec = { 10, 5};
    printf("dans le thread\n");
    pthread_yield();
    nanosleep( &spec, NULL);
    printf("reload\n");

    f3();
    pthread_exit(NULL);
}


int main()
{
    pthread_t thread;
    pthread_attr_t attr;
    int ret;
    printf("pid = %d\n", getpid());
    f2();
    f2();
    f2();
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread, &attr, th_cbk, NULL);

    while(1){
        ret = iol_main_loop();
    }

    puts("\n\n\n");
    return 0;
}
