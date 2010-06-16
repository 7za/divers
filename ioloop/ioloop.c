#include "ioloop.h"
#include <assert.h>
#define __USE_GNU
#include <search.h>
#include <stddef.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>

static struct
{
    void    *root;
    int     epfd; 
	int     quit;
}   _iol_manager = { .root = NULL, .epfd = -1, .quit = 0};


static int
_iol_cmp(const void* a1, const void* a2)
{
	struct ioloop_event_desc *d1 = ((const struct epoll_event*)a1)->data.ptr;
	struct ioloop_event_desc *d2 = ((const struct epoll_event*)a2)->data.ptr;

	return d1->iol_fd - d2->iol_fd;
}


#define IOLOOP_MAXEV 1024

__attribute__((constructor))
static void
iol_initlib(void)
{
    _iol_manager.epfd  = epoll_create(IOLOOP_MAXEV);
}


static void
_iol_free_node(void *ptr)
{
    struct epoll_event *ev;

    if(ptr != NULL){
        struct ioloop_event_desc *dsc;
        ev = ptr;
	    dsc = (struct ioloop_event_desc*)ev->data.ptr;

	    epoll_ctl(_iol_manager.epfd, EPOLL_CTL_DEL, dsc->iol_fd, 0);
        close(dsc->iol_fd);
	    free(ev->data.ptr);
	    free(ev);
	    ev = NULL;
    }
}

__attribute__((destructor))
static void
iol_exitlib(void)
{
	tdestroy(_iol_manager.root, _iol_free_node);
}



void
iol_stop_loop()
{
	_iol_manager.quit = 1;
}



int
iol_main_loop()
{
	struct epoll_event vec[IOLOOP_MAXEV];
	int count , i;

    /* wait for io events*/
	count = epoll_wait(_iol_manager.epfd, vec, IOLOOP_MAXEV, -1);
    if(count <= 0){
        return -1;
    }
	for( i = 0; i < count; ++i ){
		struct ioloop_event_desc *curr = (struct ioloop_event_desc*)vec[i].data.ptr;
		if(curr && curr->iol_func){
			curr->iol_rev = vec[i].events;
			if(curr->iol_func(curr->iol_fd, curr) <= 0){
				iol_del_event(curr->iol_fd);
			}
        }
	}
    return i;
}

static int
_iol_del_internal_event(int fd, struct epoll_event *ev)
{
	if(ev){
        struct ioloop_event_desc *d = ev->data.ptr;
		tdelete(ev, &_iol_manager.root, _iol_cmp);

		epoll_ctl(_iol_manager.epfd, EPOLL_CTL_DEL, fd, ev); 

        if(d && d->iol_clean){
            d->iol_clean(d);
		    free(ev->data.ptr);
            ev->data.ptr = NULL;
        }
    
		free(ev);
        ev = NULL;
		return 0;
	}
	return -1;
}


int
iol_del_event(int fd)
{
	struct ioloop_event_desc tmpdesc = {.iol_fd = fd};
	struct epoll_event       tmpev; 
	struct epoll_event **ev = NULL;

	tmpev.data.ptr = &tmpdesc;

	ev = tfind(&tmpev, &_iol_manager.root, _iol_cmp);
    if(ev){
	    return _iol_del_internal_event(fd, *ev);
    }
    return -1;
}



int
iol_add_event( struct ioloop_event_desc *const ref)
{
    int ret;
    struct epoll_event *ev;

    if(ref == NULL || ref->iol_fd <= 0){
        return -1;
    }

    ev = calloc(1, sizeof(*ev));

    if(ev == NULL){
        goto  iol_add_alloc_err;
    }

    ev->data.ptr   = calloc(1, sizeof(*ref));
    if(ev->data.ptr == NULL){
        goto iol_add_alloc_data_err;
    }
	memcpy(ev->data.ptr, ref, sizeof(*ref));

    ev->events      = ref->iol_ev;

    ret = epoll_ctl(_iol_manager.epfd, EPOLL_CTL_ADD, ref->iol_fd, ev);

    if(ret){
        goto iol_add_epoll_err;
    }

	if(!tsearch(ev, &_iol_manager.root, _iol_cmp)){
		goto iol_add_tsearch_err;
	}

    return 0;

iol_add_tsearch_err:
    epoll_ctl(_iol_manager.epfd, EPOLL_CTL_DEL, ref->iol_fd, ev);
iol_add_epoll_err:
    free(ev->data.ptr);
iol_add_alloc_data_err:
    free(ev);
iol_add_alloc_err:
    return -1;
}



