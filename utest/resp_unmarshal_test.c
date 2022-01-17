//
// Created by 张育 on 2022/1/15.
//

#include <stdlib.h>
#include <string.h>
#include "../cutest/CuTest.h"
#include "../resp/resp.h"
#include "../src/dbEngine.h"
#include "../src/error.h"
#include "../src/zmalloc.h"

void testRespMarshal1(CuTest *tc) {
    respFSM fsm;
    respFsmRest(&fsm);


    int len = 0;
    const char* str = "+6xxssgd\r\n";
    respItem *value = respUnmarshal(&fsm, blk2bstr(str, strlen(str)), &len);
    CuAssertPtrNotNull(tc, value);
    CuAssertIntEquals(tc, value->type, RESP_STRING);
    CuAssertTrue(tc, 0 == bcmp(bdata(value->string), "6xxssgd", strlen("6xxssgd")));
}

void testRespMarshal2(CuTest *tc) {
    respFSM fsm;
    respFsmRest(&fsm);


    int len = 0;
    const char* str = "$8\r\nxxss\r\ngd\r\n";
    respItem *value = respUnmarshal(&fsm, blk2bstr(str, strlen(str)), &len);
    CuAssertPtrNotNull(tc, value);
    CuAssertIntEquals(tc, value->type, RESP_STRING);
    CuAssertTrue(tc, 0 == bcmp(bdata(value->string), "xxss\r\ngd", strlen("xxss\r\ngd")));
}

void testRespMarshal3(CuTest *tc) {
    respFSM fsm;
    respFsmRest(&fsm);

    int len = 0;
    const char* str = "*2\r\n$8\r\nxxss\r\ngd\r\n+5aaaaa\r\n";
    respItem *value = respUnmarshal(&fsm, blk2bstr(str, strlen(str)), &len);
    CuAssertPtrNotNull(tc, value);
    CuAssertIntEquals(tc, value->type, RESP_ARRAY);
    CuAssertIntEquals(tc, value->array->argc, 2);
    CuAssertTrue(tc, 0 == bcmp(bdata(value->array->argv[0].string), "xxss\r\ngd", strlen("xxss\r\ngd")));
    CuAssertTrue(tc, 0 == bcmp(bdata(value->array->argv[1].string), "5aaaaa", strlen("5aaaaa")));
}

void testRespMarshal4(CuTest *tc) {
    respFSM fsm;
    respFsmRest(&fsm);

    int len = 0;
    const char* str1 = "*2\r\n$8\r\nxxss\r";
    const char* str2 = "$8\r\nxxss\r\ngd\r\n+5aaaaa\r\n";
    respItem *value = respUnmarshal(&fsm, blk2bstr(str1, strlen(str1)), &len);
    CuAssertIntEquals(tc, 4, len);

    value = respUnmarshal(&fsm, blk2bstr(str2, strlen(str2)), &len);
    CuAssertPtrNotNull(tc, value);
    CuAssertIntEquals(tc, value->type, RESP_ARRAY);
    CuAssertIntEquals(tc, value->array->argc, 2);
    CuAssertTrue(tc, 0 == bcmp(bdata(value->array->argv[0].string), "xxss\r\ngd", strlen("xxss\r\ngd")));
    CuAssertTrue(tc, 0 == bcmp(bdata(value->array->argv[1].string), "5aaaaa", strlen("5aaaaa")));
}

void testRespMarshal5(CuTest *tc) {
    respFSM fsm;
    respFsmRest(&fsm);

    int len = 0;
    const char* str1 = "*2\r\n$8\r\nxxss\r";
    const char* str2 = "$8\r\nxxss\rgd\r\n+5aaaaa\r\n";
    respItem *value = respUnmarshal(&fsm, blk2bstr(str1, strlen(str1)), &len);
    CuAssertIntEquals(tc, 4, len);

    value = respUnmarshal(&fsm, blk2bstr(str2, strlen(str2)), &len);
    CuAssertIntEquals(tc, -1, len);
}

void testRespMarshal6(CuTest *tc) {
    respString string = bfromcstr("abcde");

    respItem item = {};
    item.type = RESP_STRING;
    item.string = string;

    bstring buffer = bfromcstr("");
    int ret = respMarshal(&item, buffer);
    CuAssertIntEquals(tc, 0, ret);


    respFSM fsm;
    int len = 0;
    respFsmRest(&fsm);
    respItem *value = respUnmarshal(&fsm, buffer, &len);
    CuAssertPtrNotNull(tc, value);
    CuAssertIntEquals(tc, value->type, RESP_STRING);
    CuAssertTrue(tc, 0 == bcmp(bdata(value->string), "abcde", strlen("abcde")));
}

void testRespMarshal7(CuTest *tc) {
    respString string1 = bfromcstr("abcde");
    respString string2 = bfromcstr("12345");

    respItem item;
    item.type = RESP_ARRAY;
    item.array = zcalloc(sizeof(respArray) + 2 * sizeof(respItem));
    item.array->argc = 2;
    item.array->argv[0].type = RESP_STRING;
    item.array->argv[0].string = string1;
    item.array->argv[1].type = RESP_STRING;
    item.array->argv[1].string = string2;

    bstring buffer = bfromcstr("");
    int ret = respMarshal(&item, buffer);
    CuAssertIntEquals(tc, 0, ret);

    respFSM fsm;
    int len = 0;
    respFsmRest(&fsm);
    respItem *value = respUnmarshal(&fsm, buffer, &len);
    CuAssertPtrNotNull(tc, value);
    CuAssertIntEquals(tc, value->type, RESP_ARRAY);
    CuAssertIntEquals(tc, value->array->argc, 2);
    CuAssertTrue(tc, 0 == bcmp(bdata(value->array->argv[0].string), "abcde", strlen("abcde")));
    CuAssertTrue(tc, 0 == bcmp(bdata(value->array->argv[1].string), "12345", strlen("12345")));
}

void testRespMarshal8(CuTest *tc) {
    respItem item;
    item.type = RESP_ARRAY;
    item.array = zcalloc(sizeof(respArray) + 2 * sizeof(respItem));
    item.array->argc = 0;

    bstring buffer = bfromcstr("");
    int ret = respMarshal(&item, buffer);
    CuAssertIntEquals(tc, 0, ret);

    respFSM fsm;
    int len = 0;
    respFsmRest(&fsm);
    respItem *value = respUnmarshal(&fsm, buffer, &len);
    CuAssertPtrNotNull(tc, value);
    CuAssertIntEquals(tc, value->type, RESP_ARRAY);
    CuAssertIntEquals(tc, value->array->argc, 0);
}


CuSuite* respMarshalSuite() {
    CuSuite* suite = CuSuiteNew();
    CuSuiteAdd(suite, CuTestNew("解析单行字符串", testRespMarshal1));
    CuSuiteAdd(suite, CuTestNew("解析多行字符串", testRespMarshal2));
    CuSuiteAdd(suite, CuTestNew("解析字符串数组", testRespMarshal3));
    CuSuiteAdd(suite, CuTestNew("多次调用的结果", testRespMarshal4));
    CuSuiteAdd(suite, CuTestNew("多行字符串长度异常的情况", testRespMarshal5));

    CuSuiteAdd(suite, CuTestNew("序列化字符串", testRespMarshal6));
    CuSuiteAdd(suite, CuTestNew("序列化数组", testRespMarshal7));
    CuSuiteAdd(suite, CuTestNew("序列化空数组", testRespMarshal8));
    return suite;
}