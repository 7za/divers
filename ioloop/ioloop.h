#include <stdint.h>
#include <stddef.h>
#include <epoll.h>
#include <sys/types.h>

typedef ssize_t (*ioloop_func_t)(int fd, void*);

struct ioloop_event_desc
{
    int      iol_fd;
    void    *iol_data;
    int      iol_flags;
    
    ioloop_func_t   iol_func;

    char     iol_internal[12];

};

#define IOLOOP_INITIALIZER          \
    {                               \
        .iol_fd         =   -1,     \
        .iol_data       =   NULL,   \
        .iol_flags      =   0,      \
        .iol_func       =   NULL,   \
        .iol_internal   =   {[0]=0},\
    }


