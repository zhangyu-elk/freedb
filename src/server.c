//
// Created by 张育 on 2022/1/9.
//

#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <memory.h>
#include <errno.h>
#include <unistd.h>
#include "server.h"
#include "processer.h"
#include "error.h"
#include "zmalloc.h"
#include "znet.h"
#include "config.h"

server_t *server_new(const char* path) {
    driver_t *driver = driver_new(65535);
    if (!driver)    return NULL;

    config_t *config = load_config(path);
    if (!config) {
        return NULL;
    }

    server_t *server = zcalloc(sizeof(server_t));
    server->driver = driver;
    server->cfg = config;
    return server;
}

void server_close(server_t *server) {
    driver_stop(server->driver);
    zfree(server);
}

static void echo_proc(driver_t *driver, int fd, void *data, int mask) {
    while (1) {
        char buffer[1024] = { };
        int rlen = recv(fd, buffer, 1024, 0);
        if (rlen == 0 || (rlen < 0 && net_fatal())) {
            if (rlen < 0) perror("recv fail");
            driver_delete_ioevent(driver, fd, EV_IO_READABLE | EV_IO_WRITEABLE);
            close(fd);
        }

        printf("recv: [%s]\n", buffer);

        int slen = send(fd, buffer, rlen, 0);
        if (slen < 0) {
            if (net_fatal()) {
                perror("send fail");
                driver_delete_ioevent(driver, fd, EV_IO_READABLE | EV_IO_WRITEABLE);
            }
            return;
        }
    }
}

void accept_proc(driver_t *driver, int fd, void *val, int mask) {
    accept_proc_data_t *data = val;
    while (1) {
        struct sockaddr_in client_address;
        socklen_t address_len;
        int rfd = accept(fd, (struct sockaddr *)&client_address, &address_len);
        if (rfd == -1) {
            return;
        }

        if (set_nonblock(rfd) == -1) {
            perror("set_nonblock fail");
            close(rfd);
            continue;
        }

        int ret = driver_register_ioevent(driver, rfd, data->mask, data->proc, NULL);
        if (ret != DB_OK) {
            printf("driver_register_ioevent fail, close fd\n");
            close(rfd);
        }
    }
}

//目前仅监听http的端口
void server_run(server_t *server) {
    int fd = zlisten(server->cfg->http_port);
    if (fd < 0 ) {
        DIE("listen fail, %s", strerror(errno));
    }

    accept_proc_data_t data = {EV_IO_READABLE, echo_proc};
    int ret = driver_register_ioevent(server->driver, fd, EV_IO_READABLE, accept_proc, &data);
    if (ret != DB_OK) {
        DIE("register listen socket fail, %s", strerror(errno));
    }

    driver_run(server->driver);
}

