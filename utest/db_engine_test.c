//
// Created by 张育 on 2022/1/8.
//

#include <stdlib.h>
#include <string.h>
#include "../cutest/CuTest.h"
#include "../src/dbEngine.h"
#include "../src/error.h"

//lmdb是内嵌数据库所以不打桩，直接测
void test_db_engine_open(CuTest *tc) {
    dbEngine *engine = dbEngineOpen("./");
    CuAssertPtrNotNull(tc, engine);

    int ret = db_engine_drop(engine);
    CuAssertIntEquals(tc, ret, DB_OK);

    db_engine_close(engine);
}

//测试写入、查询、删除
void test_db_engine_put(CuTest *tc) {
    dbEngine *engine = dbEngineOpen("./");
    CuAssertPtrNotNull(tc, engine);

    char *key = {"key"};
    char *value = {"value"};

    int ret = dbEngineSet(engine, key, strlen(key), value, strlen(value));
    CuAssertIntEquals(tc, ret, DB_OK);

    char *rvalue = NULL;
    uint32_t rlvalue = 0;
    ret = dbEngineGet(engine, key, strlen(key), (void **) &rvalue, &rlvalue);
    CuAssertIntEquals(tc, ret, DB_OK);
    CuAssertIntEquals(tc, rlvalue, strlen(value));
    CuAssertTrue(tc, 0 == strcmp(rvalue, value));

    ret = dbEngineRemove(engine, key, strlen(key));
    CuAssertIntEquals(tc, ret, DB_OK);

    ret = db_engine_drop(engine);
    CuAssertIntEquals(tc, ret, DB_OK);

    db_engine_close(engine);
}

CuSuite* db_engine_open_suite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_db_engine_open);
    SUITE_ADD_TEST(suite, test_db_engine_put);
    return suite;
}