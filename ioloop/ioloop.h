#include <stdint.h>
#include <stddef.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <stdint.h>

typedef ssize_t (*ioloop_func_t)(int fd, void*);
typedef void    (*ioloop_clean_t)(void *);

struct ioloop_event_desc
{
    int			iol_fd;
    uint32_t	iol_ev;
	uint32_t	iol_rev;	
    
	void *iol_ctx;

    ioloop_func_t   iol_func;
    ioloop_clean_t  iol_clean;
};


int iol_del_event(int);

int iol_add_event(struct ioloop_event_desc *const);

void iol_main_loop(void);


#define IOLOOP_INITIALIZER          \
    {                               \
        .iol_fd         =   -1,     \
        .iol_ev			=   0,      \
		.iol_rev		=	0,		\
		.iol_ctx		=	NULL,	\
        .iol_func       =   NULL,   \
        .iol_clean      =   NULL,   \
    }


#define DECLARE_IOLOOP_EVENT_DESC(name)                 \
    struct ioloop_event_desc name = IOLOOP_INITIALIZER;

