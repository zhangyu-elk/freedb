//
// Created by 张育 on 2022/1/16.
//

#ifndef FREEDB_CLIENT_H
#define FREEDB_CLIENT_H

#include "../third/bstrlib/bstrlib.h"
#include "../resp/resp.h"

typedef struct client_st {
    server_t    *server;        //服务器
    respFSM     fsm;
    bstring     buffer;         //接收数据缓冲区
    size_t      pos;            //处理到的下标
    bstring     err;            //错误消息
    bstring     replyBuffer;    //回复缓冲区
    size_t      replyPos;       //回复到的下标
    int         fd;             //连接的文件描述符
} client_t;

client_t* clientNew(int fd, server_t *server);
void clientFree(client_t *client);


void commandParseProc(driver_t *driver, int fd, void *data, int mask);


//注意为了方便易用性，内部统一会销毁数据；item会销毁真正的值不包括item自身
//回复错误消息
void addReplyError(client_t *client, bstring err);
//回复普通字符串消息
void addReplyString(client_t *client, bstring string);
//回复错误消息
void addReplyError(client_t *client, bstring err);
//回复oK
void addReplyOK(client_t *client);
//回复RESp协议消息
void addReply(client_t *client, respItem *item);
#endif //FREEDB_CLIENT_H
