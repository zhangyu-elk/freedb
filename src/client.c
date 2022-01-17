//
// Created by 张育 on 2022/1/16.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include "znet.h"
#include "command.h"
#include "server.h"
#include "client.h"
#include "zmalloc.h"

//向客户端回复消息
static int reply(client_t *client);

//客户端回复数据处理器
static void replyBufferProc(driver_t *driver, int fd, void *data, int mask) {
    client_t *client = data;
    int ret = reply(client);
    if (ret == -1) {
        clientFree(client);
    }
}

//如果fd暂时无法写入会注册到事件驱动中处理
static int reply(client_t *client) {
    //将replybuffer中的数据返回给客户端；先尝试直接写入/写入失败则注册事件来写入
    while (blength(client->replyBuffer) > client->replyPos) {
        ssize_t len = write(client->fd, bdata(client->replyBuffer) + client->replyPos,
                        blength(client->replyBuffer) - client->replyPos);
        if (len == -1) {
            if (!netErrorAgain()) {
                return -1;
            }
            if (0 != driverRegEvent(client->server->driver, client->fd, EV_IO_WRITEABLE, replyBufferProc, client)) {
                return -1;
            }
            break;
        }

        client->replyPos += len;
    }

    //初步任务字符串长度不会很多，没1k进行一次移动操作
    if (client->replyPos > 1024) {
        bassignblk(client->replyBuffer, bdata(client->replyBuffer) + client->replyPos,
                   blength(client->replyBuffer) - client->replyPos);
        client->replyPos = 0;
    }
    if (blength(client->replyBuffer) == client->replyPos) {
        driverDelEvent(client->server->driver, client->fd, EV_IO_WRITEABLE);
    }
    return 0;
}

void addReplyString(client_t *client, bstring string) {
    respItem item;
    item.type = RESP_STRING;
    item.string = string;

    respMarshal(&item, client->replyBuffer);
    bdestroy(string);
    reply(client);
}

void addReplyOK(client_t *client) {
    struct tagbstring okString;
    btfromblk(okString, "OK", 2);

    respItem item;
    item.type = RESP_STRING;
    item.string = &okString;

    respMarshal(&item, client->replyBuffer);
    reply(client);
}

void addReplyError(client_t *client, bstring err) {
    respItem item;
    item.type = RESP_ERROR;
    item.err = err;

    respMarshal(&item, client->replyBuffer);
    bdestroy(err);
    reply(client);
}

//回复错误后销毁数据
static void replyErrorAndClose(client_t *client) {
    addReplyError(client, client->err);
    client->err = NULL;

    clientFree(client);
}


//执行命令
static int commandExec(client_t *client, respItem *item) {
    if (item->type != RESP_ARRAY) {
        addReplyError(client, bformat("Command error: expect a array"));
        return -1;
    }

    if (item->array->argc < 1 && item->array->argv[0].type != RESP_STRING) {
        addReplyError(client, bformat("Command error: expect cmd string"));
        return -1;
    }

    bstring key = item->array->argv[0].string;
    command_t *command = hashmap_get(&client->server->commandTable, bdata(key), blength(key));
    if (command == 0) {
        addReplyError(client, bformat("Command error: unsupported %s", bdata(key)));
        return -1;
    }

    command->proc(client, item->array->argc, item->array->argv);
    return 0;
}

//解析命令
static int commandParseAndExec(client_t *conn) {
    int     len = 0;

    struct tagbstring buffer;
    btfromblk(buffer, bdata(conn->buffer) + conn->pos, blength(conn->buffer) - conn->pos);
    respItem *item = respUnmarshal(&conn->fsm, &buffer, &len);
    if (len < 0) {
        return - 1;
    }

    //处理了1k再做数据的迁移
    conn->pos += len;
    if (conn->pos > 1024) {
        bassignblk(conn->buffer, bdata(conn->buffer) + conn->pos, blength(conn->buffer) - conn->pos);
    }

    if (item != 0) {
        respFsmRest(&conn->fsm);
    }
    if (item && 0 != commandExec(conn, item)) {
        return -1;
    }
    return 0;
}

//命令解析处理器
void commandParseProc(driver_t *driver, int fd, void *data, int mask) {
    client_t *client = data;
    while (1) {
        char buffer[1024] = { };
        int len = recv(fd, buffer, 1024, 0);
        if (len <= 0) {
            //在返回0和错误不能继续的情况下删除事件并关闭句柄
            if (len == 0 || !netErrorAgain()) {
                clientFree(client);
            }
            return;
        }

        bcatblk(client->buffer, buffer, len);
        if (commandParseAndExec(client) == 0) {
            continue;
        }

        //命令解析失败回复错误关闭连接
        replyErrorAndClose(client);
        break;
    }
}

client_t* clientNew(int fd, server_t *server) {
    client_t *client = zcalloc(sizeof(client_t));

    client->server = server;
    respFsmRest(&client->fsm);
    client->fd = fd;
    client->buffer = bfromcstr("");
    client->replyBuffer = bfromcstr("");

    return client;
}

void clientFree(client_t *client) {
    if (!client)  return;

    //取消事件注册
    driverDelEvent(client->server->driver, client->fd, EV_IO_READABLE | EV_IO_WRITEABLE);

    //关闭句柄
    close(client->fd);

    //清除状态机中的数据
    respFsmRest(&client->fsm);

    //销毁缓冲区
    bdestroy(client->buffer);
    bdestroy(client->replyBuffer);
    bdestroy(client->err);

    //释放资源
    zfree(client);
}