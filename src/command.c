//
// Created by 张育 on 2022/1/11.
//

#include "command.h"
#include "dbEngine.h"

//set命令
void setCommand(client_t *client, int argc, respItem *argv) {
    if (argc != 3) {
        return addReplyError(client, bformat("Command error: set expect %d argv but %d", 3, argc));
    }
    if (argv[1].type != RESP_STRING || argv[1].type != RESP_STRING) {
        return addReplyError(client, bformat("Command error: set expect key and value is string"));
    }

    bstring key = argv[1].string;
    bstring value = argv[2].string;

    int ret = dbEngineSet(client->server->engine, bdata(key), blength(key), bdata(value), blength(value));
    if (ret != 0) {
        return addReplyError(client, bformat("DB error: %d", ret));
    }

    return addReplyOK(client);
}

//get命令
void getCommand(client_t *client, int argc, respItem *argv) {
    if (argc != 2) {
        return addReplyError(client, bformat("Command error: get expect %d argv but %d", 3, argc));
    }
    if (argv[1].type != RESP_STRING) {
        return addReplyError(client, bformat("Command error: get expect key is string"));
    }

    void    *value = NULL;
    size_t  vlen = 0;

    int ret = dbEngineGet(client->server->engine, bdata(argv[1].string), blength(argv[1].string), &value, &vlen);
    if (ret != 0) {
        return addReplyError(client, bformat("DB error: %d", ret));
    }

    return addReplyString(client, blk2bstr(value, (int )vlen));
}

command_t commandTable[] = {
        {
            "set",
                setCommand,
        },
        {
            "get",
                getCommand,
        }
};