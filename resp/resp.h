//
// Created by 张育 on 2022/1/12.
//

#ifndef FREEDB_RESP_H
#define FREEDB_RESP_H

#include <inttypes.h>
#include "../third/bstrlib/bstrlib.h"

//类似于redis的resp交互协议,支持流式解析协议处理

#define RESP_STRING_FC      '+'
#define RESP_BULK_STRING_FC '$'
#define RESP_INTEGER_FC     ':'
#define RESP_ARRAY_FC       '*'
#define RESP_ERROR_FC       '-'

enum {
    RESP_UNKNOWN,
    RESP_STRING,
    RESP_BULK_STRING,
    RESP_ERROR,
    RESP_INTEGER,
    RESP_ARRAY,
};

//目前仅支持字符串、整数、数组
typedef bstring respString;
typedef long long respInteger;
typedef struct respArray_st respArray;

//item有多种类型
typedef struct respItem_st {
    int     type;
    union {
        respInteger integer;
        respString  string;
        respString  err;
        respArray   *array;
    };
} respItem;

//对于数组而言，数组元素是不确定的；目前不要求数组中每个元素的类型一致
struct respArray_st {
    int         argc;
    respItem    argv[0];
} ;

typedef struct respReadTask_st {
    int         elementIdx;     //如果是数组的话当前是第几个元素
    int         elementCnt;     //如果是数组的话共有多少个元素
    respItem    item;
} respReadTask;

typedef struct respFSM_st {
    int             ridx;           //当前的读取任务进行到第几个
    bstring         err;            //错误信息
    respItem        *value;          //值
    respReadTask    tasks[16];      //因为涉及到递归的问题，所以需要一个数组来记录任务；最多允许16级嵌套
} respFSM;

//重置协议状态机，无论上一次成功还是失败、每次调用前都需要重置一下；包括最终释放
void respFsmRest(respFSM *fsm);

/*
 * 解析字符串缓存区，允许多次调用多次传入buffer来解析一个buffer；exptype表示期待类型，默认传0即可
 * len表示处理的字节数，-1表示出现错误
 * @return  返回解析后的结构体；如果返回nil且len不等于-1则表示还需要再次调用
*/
respItem* respUnmarshal(respFSM *fsm, const_bstring buffer, int *len);

/*
 * 序列化
*/
int respMarshal(respItem *item, bstring buffer);


#endif //FREEDB_RESP_H
