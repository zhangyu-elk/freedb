//
// Created by 张育 on 2022/1/11.
//

#ifndef FREEDB_COMMAND_H
#define FREEDB_COMMAND_H

#include "server.h"
#include "client.h"
#include "../third/bstrlib/bstrlib.h"
#include "../resp/resp.h"

//回复null的时候表示成功、其余无论错误信息还是回复信息统一放在item里面


//put命令

//set命令
void setCommand(client_t *client, int argc, respItem *argv);

//执行命令然后返回一个item，目前直接引用resp库，简单做
typedef void commandProc(client_t *client, int argc, respItem *argv);
typedef struct command_st {
    const char*     name;       //命令，仅支持固定的几个命令
    commandProc     *proc;      //回调函数，参数类型比较难做说明先内部自己做
} command_t;

extern command_t commandTable[2];

#endif //FREEDB_COMMAND_H
