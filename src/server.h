//
// Created by 张育 on 2022/1/9.
//

#ifndef FREEDB_SERVER_H
#define FREEDB_SERVER_H

#include "../bees/driver.h"
#include "config.h"

typedef struct server_st {
    driver_t *driver;       //io驱动
    config_t  *cfg;          //配置
} server_t;

//新建一个服务器
server_t *server_new(const char* path);

//关闭同时销毁资源
void server_close(server_t *);

//启动服务主循环
void server_run(server_t *server);

#endif //FREEDB_SERVER_H
