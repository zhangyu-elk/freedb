//
// Created by 张育 on 2022/1/9.
//

#ifndef FREEDB_CONFIG_H
#define FREEDB_CONFIG_H

typedef struct config_st {
    short           port;               //服务端口
    short           http_port;          //http服务端口，接收json命令
    short           management_port;    //管理http服务端口
} config_t;

config_t *load_config(const char* path);

#endif //FREEDB_CONFIG_H
