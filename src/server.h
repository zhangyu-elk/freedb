//
// Created by 张育 on 2022/1/9.
//

#ifndef FREEDB_SERVER_H
#define FREEDB_SERVER_H

#include "../bees/driver.h"
#include "dbEngine.h"
#include "config.h"
#include "../third/hashmap.h/hashmap.h"

typedef struct server_st {
    driver_t            *driver;        //io驱动
    dbEngine            *engine;        //数据库引擎
    config_t            *cfg;           //配置
    struct hashmap_s    commandTable;   //命令
} server_t;

//启动服务
void serverMain();

#endif //FREEDB_SERVER_H
