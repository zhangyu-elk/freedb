//
// Created by 张育 on 2022/1/9.
//

//kqueue事件api封装

#include <sys/event.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "../src/zmalloc.h"
#include "ev.h"

typedef struct firedEv_st {
    int     fd;
    int     mask;
} firedEv_t;

typedef struct evApi_st {
    int     kqfd;
    size_t              esize;
    struct kevent       *events;
} evApi_t;

static evApi_t *evApiCreate(size_t size) {
    evApi_t *api = zcalloc(sizeof(evApi_t));
    api->events = zcalloc(sizeof(struct kevent) * size);
    api->esize = size;

    api->kqfd = kqueue();
    if (api->kqfd == -1) {
        zfree(api->events);
        zfree(api);
        return NULL;
    }

    return api;
}

static void evApiFree(evApi_t *api) {
    close(api->kqfd);
    zfree(api->events);
    zfree(api);
}

static int evApiAddEvent(evApi_t *api, int fd, int mask) {
    struct kevent ke;

    if (mask & EV_IO_READABLE) {
        EV_SET(&ke, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
        if (kevent(api->kqfd, &ke, 1, NULL, 0, NULL) == -1) return -1;
    }
    if (mask & EV_IO_WRITEABLE) {
        EV_SET(&ke, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        if (kevent(api->kqfd, &ke, 1, NULL, 0, NULL) == -1) return -1;
    }
    return 0;
}

static void evApiDelEvent(evApi_t *api, int fd, int mask) {
    struct kevent ke;

    if (mask & EV_IO_READABLE) {
        EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        kevent(api->kqfd, &ke, 1, NULL, 0, NULL);
    }
    if (mask & EV_IO_WRITEABLE) {
        EV_SET(&ke, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        kevent(api->kqfd, &ke, 1, NULL, 0, NULL);
    }
}

//启动事件循环，注意fired的大小必须大于创建时传入的大小，否则可能会崩溃
static int evApiPool(evApi_t *api, struct timeval *tvp, firedEv_t *fired) {
    int retval, numevents = 0;

    if (tvp != NULL) {
        struct timespec timeout;
        timeout.tv_sec = tvp->tv_sec;
        timeout.tv_nsec = tvp->tv_usec * 1000;
        retval = kevent(api->kqfd, NULL, 0, api->events, api->esize,
                        &timeout);
    } else {
        retval = kevent(api->kqfd, NULL, 0, api->events, api->esize,
                        NULL);
    }

    if (retval < 0) {
        perror("kqueue");
    }

    if (retval > 0) {
        int j;

        numevents = retval;
        for(j = 0; j < numevents; j++) {
            int mask = 0;
            struct kevent *e = api->events+j;

            if (e->filter == EVFILT_READ) mask |= EV_IO_READABLE;
            if (e->filter == EVFILT_WRITE) mask |= EV_IO_WRITEABLE;
            fired[j].fd = e->ident;
            fired[j].mask = mask;
        }
    }
    return numevents;
}

static char *aeApiName(void) {
    return "kqueue";
}




