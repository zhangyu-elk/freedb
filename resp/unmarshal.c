//
// Created by 张育 on 2022/1/12.
//

#include <string.h>
#include "resp.h"
#include "../src/zmalloc.h"
#include "../util/util.h"

static int procItem(respFSM *fsm, const_bstring buffer);
static int processLineItem(respFSM *fsm, const_bstring buffer);

//移动到下一个task
static int moveNextTask(respFSM *fsm, const_bstring buffer, int len) {
    fsm->ridx ++;
    if (fsm->ridx >= 16) {
        fsm->err = bformat("Protocol error: nesting level exceed limit");
        return -1;
    }

    struct tagbstring nextBuffer;
    btfromblk(nextBuffer, bdata(buffer) + len, blength(buffer) - len);

    int ret = procItem(fsm, &nextBuffer);
    if (ret < 0) {
        return ret;
    }
    return len + ret;
}

//移动到上一个task
static int movePreTask(respFSM *fsm, const_bstring buffer, int len) {
    fsm->ridx --;

    //task中不使用指针主要是为来避免多次分配内存之类的
    if (fsm->ridx < 0) {
        fsm->value = zcalloc(sizeof(respItem));
        memcpy(fsm->value, &fsm->tasks[0].item, sizeof(respItem));
        memset(&fsm->tasks[0].item, 0,  sizeof(respItem));
        return len;
    }

    respReadTask *cur = &fsm->tasks[fsm->ridx + 1];
    respReadTask *pre = &fsm->tasks[fsm->ridx];

    //目前仅有多行字符串一种类型
    pre->item.array->argv[pre->elementIdx++] = cur->item;
    memset(cur, 0,  sizeof(respItem));

    //对于聚合类型已处理完毕，则退回到上一个
    if (pre->elementIdx == pre->elementCnt) {
        return movePreTask(fsm, buffer, len);
    }

    return moveNextTask(fsm, buffer, len);
}

//处理行元素
static int processLineItem(respFSM *fsm, const_bstring buffer) {
    respReadTask *cur = &fsm->tasks[fsm->ridx];

    //如果没有一行(\r\n)则直接结束，等待新的buffer进来
    int end = bstrchr(buffer, '\r');
    if (end <= 0 || (end + 1) >= blength(buffer)) {
        return 0;
    }

    switch (cur->item.type) {
        case RESP_STRING:
            cur->item.string = blk2bstr(bdata(buffer) + 1, end - 1);
            break;
        case RESP_ERROR:
            cur->item.err = blk2bstr(bdata(buffer) + 1, end - 1);
            break;
    }

    return movePreTask(fsm, buffer, end + 2);
}

//处理多行元素；多行元素指的是可能跨多行的元素
static int processBulkLineItem(respFSM *fsm, const_bstring buffer) {
    respReadTask *cur = &fsm->tasks[fsm->ridx];

    //如果没有一行(\r\n)则直接结束，等待新的buffer进来
    int end = bstrchr(buffer, '\r');
    if (end <= 0 || (end + 1) >= blength(buffer)) {
        return 0;
    }

    //解析出长度
    long long ll = 0;
    if (0 == string2ll(bdata(buffer) + 1, end - 1, &ll) || ll > 1024 * 1024) {
        fsm->err = bformat("Protocol error: invalid length");
        return -1;
    }

    //将解析长度和内容放在一次，否则会复杂很多；长度不够的情况直接返回
    int contentLen = (int )ll;
    int len = contentLen + 2 + end + 2;
    if (len > blength(buffer)) {
        return 0;
    }

    //多行字符串也是字符串，直接放string里面就可以来
    cur->item.string = blk2bstr(bdata(buffer) + end + 2, contentLen);
    if (cur->item.string == NULL) {
        fsm->err = bformat("Protocol error: out of memory");
        return -1;
    }
    cur->item.type = RESP_STRING;

    return movePreTask(fsm, buffer, len);
}


//处理数组之类的聚合元素
static int processAggregateItem(respFSM *fsm, const_bstring buffer) {
    respReadTask *cur = &fsm->tasks[fsm->ridx];

    //如果没有一行(\r\n)则直接结束，等待新的buffer进来
    int end = bstrchr(buffer, '\r');
    if (end <= 0 || (end + 1) >= blength(buffer)) {
        return 0;
    }

    long long ll = 0;
    if (0 == string2ll(bdata(buffer) + 1, end - 1, &ll) || ll > 1024 * 1024) {
        fsm->err = bformat("Protocol error: invalid length");
        return -1;
    }

    cur->item.array = zcalloc(sizeof(respArray) + sizeof(respItem) * ll);
    cur->item.array->argc = (int )ll;
    cur->elementCnt = (int )ll;
    cur->elementIdx = 0;

    if (cur->elementCnt > 0) {
        return moveNextTask(fsm, buffer, end + 2);
    }
    return movePreTask(fsm, buffer, end + 2);
}

static int procItem(respFSM *fsm, const_bstring buffer) {
    if (blength(buffer) <= 0) {
        return 0;
    }
    respReadTask *cur = &fsm->tasks[fsm->ridx];
    if (cur->item.type <= 0) {
        switch (bdata(buffer)[0]) {
            case RESP_STRING_FC:
                cur->item.type = RESP_STRING;
                break;
            case RESP_BULK_STRING_FC:
                cur->item.type = RESP_BULK_STRING;
                break;
            case RESP_INTEGER_FC:
                cur->item.type = RESP_INTEGER;
                break;
            case RESP_ARRAY_FC:
                cur->item.type = RESP_ARRAY;
                break;
            case RESP_ERROR_FC:
                cur->item.type = RESP_ERROR;
                break;
            default:
                fsm->err = bformat("Protocol error: unsupported type");
                return -1;
        }
    }

    switch(cur->item.type) {
        case RESP_STRING:
        case RESP_ERROR:
        case RESP_INTEGER:
            return processLineItem(fsm, buffer);
        case RESP_BULK_STRING:
            return processBulkLineItem(fsm, buffer);
        case RESP_ARRAY:
            return processAggregateItem(fsm, buffer);
    }

    return 0;
}

//外部保证buffer和len必须是有值的
respItem * respUnmarshal(respFSM *fsm, const_bstring buffer, int *len) {
    //检查数据
    if (!buffer || !len) {
        fsm->err = bformat("param error\n");
        return NULL;
    }
    if (blength(buffer) <= 0) {
        return NULL;
    }

    *len = procItem(fsm, buffer);

    //如果所有task处理完毕则value会有值，否则回复NULL
    return fsm->value;
}

void respFsmRest(respFSM *fsm) {
    memset(fsm, 0, sizeof(respFSM));
}