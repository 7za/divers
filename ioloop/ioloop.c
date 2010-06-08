#include "ioloop.h"
#define _GNU_SOURCE
#include <search.h>
#include <stddef.h>

static struct
{
    void			*root;
    int             epfd; 
}   _iol_manager = { .root = NULL, .epfd = -1};


#define IOLOOP_MAXEV 1024

static int
iol_initlib(void)
{
    int ret = 0;
    _iol_manager.epfd  = epoll_create(IOLOOP_MAXEV);
    if(_iol_manager.epfd <= 0){
        ret =  -1;
    }   
    return ret;
}


int
iol_add_event(struct ioloop_event_desc *ref)
{
    int ret;
    struct epoll_event *ev;
    if(ref == NULL || ref->iol_fd <= 0){
        return -1;
    }

    ev = malloc(sizeof(*ev));

    if(ev == NULL){
        goto alloc_err;
    }

    ev->data.data   = malloc(sizeof(*ref));
    if(ev->data.data == NULL){
        goto alloc_data_err;
    }

    memcpy(ev->data.data, ref, sizeof(*ref));

    ev->events      = ref->iol_flags;

    ret = epoll_ctl(_iol_manager.epfd, EPOLL_CTL_ADD, ref->iol_fd, ev);

    if(ret){
        goto epoll_err;
    }
	if(!tsearch((void*)ev, &_iol_manager.root, _iol_cmp)){
		goto tsearch_err;
	}

    return 0;

tsearch_err:
    epoll_ctl(_iol_manager.epfd, EPOLL_CTL_DEL, ref->iol_fd, ev);
epoll_err:
    free(ev->data.data);
alloc_data_err:
    free(ev);
alloc_err:
    return -1;
}



