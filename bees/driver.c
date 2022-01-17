//
// Created by 张育 on 2022/1/9.
//

#include <stdlib.h>
#include "driver.h"
#include "kqueue.ch"

driver_t *driverNew(size_t size) {
    driver_t *driver = zcalloc(sizeof(driver_t));
    evApi *api = evApiCreate(size);
    if (!api) {
        return NULL;
    }

    driver->api = api;
    driver->fired = zcalloc(sizeof(firedEv) * size);
    driver->events = zcalloc(sizeof(firedEv) * size);
    driver->size = size;
    return driver;
}

static int driverProcessEvents(driver_t *driver) {
    firedEv *fired = driver->fired;
    int nevents = evApiPool(driver->api, NULL, fired);
    for (int i = 0; i < nevents; i++) {
        ioEvent *event = &driver->events[fired->fd];

        if (fired->mask & event->mask & EV_IO_READABLE && event->rproc) {
            event->rproc(driver, fired->fd, event->data, fired->mask);
        }
        if (fired->mask & event->mask & EV_IO_WRITEABLE && event->wproc) {
            event->wproc(driver, fired->fd, event->data, fired->mask);
        }
    }
    return nevents;
}

int driverRegEvent(driver_t *driver, int fd, int mask,
                   ioProc_f *proc, void *data) {

    if (fd >= driver->size) {
        return -1;
    }
    ioEvent *ev = &driver->events[fd];

    if (evApiAddEvent(driver->api, fd, mask) == -1)
        return -1;
    ev->mask |= mask;
    if (mask & EV_IO_WRITEABLE) ev->wproc = proc;
    if (mask & EV_IO_READABLE) ev->rproc = proc;
    ev->data = data;
    return 0;
}

void driverDelEvent(driver_t *driver, int fd, int mask) {
    if (fd >= driver->size) return;
    ioEvent *ev = &driver->events[fd];
    if (ev->mask == EV_IO_NONE) return;

    evApiDelEvent(driver->api, fd, mask);
    ev->mask = ev->mask & (~mask);
}

//预先将listen的句柄注册进来，再启动
void driverRun(driver_t *driver) {
    while (!driver->stop) {
        driverProcessEvents(driver);
    }
}

void driver_stop(driver_t *driver) {
    driver->stop = 1;
}

void driver_close(driver_t *driver) {
    //停止
    driver_stop(driver);

    evApiFree(driver->api);
    zfree(driver->events);
    zfree(driver->fired);
    zfree(driver);
}





