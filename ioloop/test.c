#include "ioloop.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


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
    printf("cleaning up for %d\n", d1->iol_fd);
    puts("here");
    fflush(stdin);
}



static ssize_t
f1cbk(int fd __attribute__((unused)), void *data)
{
    struct ioloop_event_desc *d1 = data;
    ssize_t ret     = -1;

    if( d1 && (d1->iol_rev == EPOLLOUT)){
        if(fputc('\n', (FILE*)d1->iol_ctx) == 1){
            ret = 1;
        }
    } 
    return ret;
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


void f1()
{
    FILE *fp;
    DECLARE_IOLOOP_EVENT_DESC(f1iol);
    
    fp = popen("ct-ng powerpc-405-linux-gnu", "w");
    f1iol.iol_ctx   = fp;
    f1iol.iol_fd    = fileno(fp);
    f1iol.iol_func  = f1cbk;
    f1iol.iol_ev    = EPOLLOUT | EPOLLRDHUP;
    f1iol.iol_clean = fcbk_clean;
    printf("%s -> fd = %d\n", __func__, f1iol.iol_fd);

    if(iol_add_event(&f1iol) < 0){
        printf("gros echec\n");
        exit(1);
    }
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


int main()
{
    printf("pid = %d\n", getpid());
    f1();
    f2();

    iol_main_loop();

    puts("\n\n\n");
    return 0;
}
