//
// Created by 张育 on 2022/1/8.
//

#ifndef FREEDB_DB_ENGINE_H
#define FREEDB_DB_ENGINE_H

#include <lmdb.h>

typedef struct db_engine_st {
    MDB_env         *env;
    MDB_dbi         dbi;
}db_engine_t;

//打开db引擎句柄
db_engine_t* db_engine_open(const char *root);

//清空数据库表
int db_engine_drop(db_engine_t *engine);

//关闭引擎句柄
void db_engine_close(db_engine_t *engine);

//查询数据
int db_engine_get(db_engine_t *engine, const void *key, const uint32_t lkey, void **value, uint32_t *lvalue);

//写入数据
int db_engine_put(db_engine_t *engine, const void *key, const uint32_t lkey, const void *value, const uint32_t lvalue);

//移除数据
int db_engine_remove(db_engine_t *engine, const void *key, const uint32_t lkey);

#endif //FREEDB_DB_ENGINE_H
