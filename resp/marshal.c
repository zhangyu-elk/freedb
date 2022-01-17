//
// Created by 张育 on 2022/1/15.
//

#include "resp.h"


#define ASSERT_RETURN_NULL(v) do{ if (0 != (v)) {    \
    return -1;                                                \
} }while(0);

static int replySep(bstring buffer) {
    return bcatcstr(buffer, "\r\n");
}

static int marshalItem(bstring buffer, respItem *item);

//序列化单行，应当需要支持但行字符串、数字、error
static int marshalString(bstring buffer, respItem *item) {
    //符号+长度
    ASSERT_RETURN_NULL(bformata(buffer, "%c", RESP_ERROR_FC));
    //添加真正的内容；目前仅支持错误信息
    ASSERT_RETURN_NULL(bconcat(buffer, item->err));
    //添加换行符
    ASSERT_RETURN_NULL(replySep(buffer));

    return 0;
}

//序列化多行字符串
static int marshalBulkString(bstring buffer, respItem *item) {
    //符号+长度
    ASSERT_RETURN_NULL(bformata(buffer, "%c%d", RESP_BULK_STRING_FC, blength(item->string)));
    //添加换行符
    ASSERT_RETURN_NULL(replySep(buffer));

    //添加真正的内容
    ASSERT_RETURN_NULL(bconcat(buffer, item->string));
    //添加换行符
    ASSERT_RETURN_NULL(replySep(buffer));

    return 0;
}

//序列化数组
static int marshalArray(bstring buffer, respItem *item) {
    //符号+长度
    ASSERT_RETURN_NULL(bformata(buffer, "%c%d", RESP_ARRAY_FC,item->array->argc));
    //添加换行符
    ASSERT_RETURN_NULL(replySep(buffer));

    for (int i = 0; i < item->array->argc; i++) {
        ASSERT_RETURN_NULL(marshalItem(buffer, &item->array->argv[i]));
    }
    return 0;
}

static int marshalItem(bstring buffer, respItem *item) {
    switch (item->type) {
        case RESP_ERROR:
            return marshalString(buffer, item);
        case RESP_STRING:
        case RESP_BULK_STRING:
            return marshalBulkString(buffer, item);
        case RESP_ARRAY:
            return marshalArray(buffer, item);
    }
    return -1;
}



//序列化不需要支持流式传输，一次性将整个传进来就可以l；失败的情况只有内存不够的情况
int respMarshal(respItem *item, bstring buffer){
    if (item == NULL || buffer == NULL) {
        return -1;
    }

    return marshalItem(buffer, item);
}