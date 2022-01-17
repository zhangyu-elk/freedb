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
#include "command.h"
#include "server.h"
#include "client.h"
#include "error.h"
#include "zmalloc.h"
#include "znet.h"
#include "config.h"

//接收连接并注册到事件驱动中去
void acceptProcHandler(driver_t *driver, int fd, void *data, int mask) {
    server_t    *server = data;
    while (1) {
        struct sockaddr_in client_address;
        socklen_t address_len;
        int rfd = accept(fd, (struct sockaddr *)&client_address, &address_len);
        if (rfd == -1) {
            if (!netErrorAgain()) {
                perror("accept socket");
            }
            return;
        }

        client_t *client = clientNew(rfd, server);

        //设置为非阻塞模式然后注册到事件驱动中
        if (fdSetNonBlocking(rfd) ||
            DB_OK != driverRegEvent(driver, rfd, EV_IO_READABLE, commandParseProc, client)) {
            perror("register fail");
            close(rfd);
        }
    }}

server_t *server;

//目前仅监听http的端口
void serverMain() {
    //数据库引擎
    dbEngine *engine = dbEngineOpen("./");
    if (engine == NULL) {
        DIE("dbEngineOpen fail, %s", strerror(errno));
    }

    //io驱动
    driver_t *driver = driverNew(65535);
    if (driver == NULL)  {
        DIE("driverNew fail, %s", strerror(errno));
    }

    //读取配置
    config_t *config = load_config("");
    if (config == NULL) {
        DIE("load_config fail, %s", strerror(errno));
    }

    server = zcalloc(sizeof(server_t));
    server->driver = driver;
    server->cfg = config;
    server->engine = engine;

    //初始化命令哈希表
    if (0 != hashmap_create(16, &server->commandTable)) {
        DIE("hashmap_create fail, %s", strerror(errno));
    }
    for (int i = 0; i < sizeof commandTable/ sizeof(command_t); i++) {
        command_t *command = &commandTable[i];
        hashmap_put(&server->commandTable, command->name, strlen(command->name),  command);
    }


    int fd = zlisten(server->cfg->port);
    if (fd < 0 ) {
        DIE("listen fail, %s", strerror(errno));
    }

    int ret = driverRegEvent(server->driver, fd, EV_IO_READABLE, acceptProcHandler, server);
    if (ret != DB_OK) {
        DIE("register listen socket fail, %s", strerror(errno));
    }

    driverRun(server->driver);
}

