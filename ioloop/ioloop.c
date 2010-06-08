#include "ioloop.h"
#define _GNU_SOURCE
#include <search.h>
#include <stddef.h>

static struct
{
    struct hsearch_data htab;
    int             epfd; 
    int             epfd_count
}   _iol_manager = { .epfd = -1, .epfd_count = 0};




static int
iol_initlib(int max_events)
{
    int ret;
    _iol_manager.epfd_count = max_events;
    _iol_manager.epfd  = epoll_create(IOLOOP_MAX_EV);
    if(_iol_manager.epfd <= 0){
        return -1;
    }   
    ret = hcreate_r(max_events, &_iol_manager.htab);
    return -(!ret);
}


int
iol_add_event(struct ioloop_event_desc *ref)
{
    int ret;
    struct epoll_event *ev;
    struct entry *entryret = NULL;
    struct entry  entry;
    if(ref == NULL || ref->iol_fd <= 0){
        return -1;
    }
    snprintf(ref->iol_internal, sizeof(ref->iol_internal),"%d", ref->iol_fd);

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
    entry->key  = ((struct ioloop_event_desc*)ev->data.data)->iol_internal;
    entry->data = ev;
    ret = hsearch_r(entry, ENTER, &entryret, &_iol_manager.htab);
    if(!ret){
        goto hsearch_err;
    }

    return 0;

hsearch_err:
    epoll_ctl(_iol_manager.epfd, EPOLL_CTL_DEL, ref->iol_fd, ev);
epoll_err:
    free(ev->data.data);
alloc_data_err:
    free(ev);
alloc_err:
    return -1;
}



