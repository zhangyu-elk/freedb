//
// Created by 张育 on 2022/1/8.
//

#ifndef FREEDB_DBENGINE_H
#define FREEDB_DBENGINE_H

#include <lmdb.h>

typedef struct dbEngine_st {
    MDB_env         *env;
    MDB_dbi         dbi;
} dbEngine;

//打开db引擎句柄
dbEngine* dbEngineOpen(const char *root);

//清空数据库表
int db_engine_drop(dbEngine *engine);

//关闭引擎句柄
void db_engine_close(dbEngine *engine);

//查询数据
int dbEngineGet(dbEngine *engine, const void *key, size_t lkey, void **value, size_t *lvalue);

//写入数据
int dbEngineSet(dbEngine *engine, const void *key, size_t lkey, const void *value, size_t lvalue);

//移除数据
int dbEngineRemove(dbEngine *engine, const void *key, size_t lkey);

#endif //FREEDB_DBENGINE_H
