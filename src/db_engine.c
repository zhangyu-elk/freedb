//
// Created by 张育 on 2022/1/8.
//

#include <stdio.h>
#include <memory.h>
#include "db_engine.h"
#include "error.h"
#include "zmalloc.h"

#define MDBE(expr) do{ \
    int ret = expr;                                                             \
    if (ret != MDB_SUCCESS) {                                                   \
        printf("%s,%d mdb error: %s\n", __FILE__, __LINE__, mdb_strerror(ret)); \
        goto EXIT;                                                              \
    }                                                                           \
}while(0)

#define C_CLOSE(obj, close) do{ \
    if (obj) {                  \
        close(obj);             \
        obj = NULL;             \
    }                           \
}while(0)

db_engine_t* db_engine_open(const char *root) {
    if (root == NULL) {
        return NULL;
    }

    MDB_env *env = NULL;
    MDB_txn *txn = NULL;
    MDB_dbi  dbi;

    //创建引擎环境
    MDBE(mdb_env_create(&env));
    //设置允许的数据库个数
    MDBE(mdb_env_set_maxdbs(env, 16));
    //打开数据库文件
    MDBE(mdb_env_open(env, root, 0, 0664));
    //打开指定的数据库
    MDBE(mdb_txn_begin(env, NULL, 0, &txn));
    MDBE(mdb_dbi_open(txn, "db1", MDB_CREATE, &dbi));
    MDBE(mdb_txn_commit(txn));

    db_engine_t *engine = zcalloc(sizeof(engine));
    engine->env = env;
    engine->dbi = dbi;
    return engine;

    EXIT:
    C_CLOSE(txn, mdb_txn_abort);
    C_CLOSE(env, mdb_env_close);
    return NULL;
}

int db_engine_put(db_engine_t *engine, const void *key, const uint32_t lkey, const void *value, const uint32_t lvalue) {
    int ret = DB_OK;

    MDB_txn *txn = NULL;
    MDBE(mdb_txn_begin(engine->env, NULL, 0, &txn));

    MDB_val dbkey, dbvalue;

    dbkey.mv_size = lkey;
    dbkey.mv_data = (void *)key;
    dbvalue.mv_size = lvalue;
    dbvalue.mv_data = (void *)value;

    MDBE(mdb_put(txn, engine->dbi, &dbkey, &dbvalue, 0));
    MDBE(mdb_txn_commit(txn));

    return ret;

    EXIT:
    C_CLOSE(txn, mdb_txn_abort);
    return ret;
}

int db_engine_get(db_engine_t *engine, const void *key, const uint32_t lkey, void **value, uint32_t *lvalue) {
    int ret = DB_OK;

    MDB_txn *txn = NULL;
    MDBE(mdb_txn_begin(engine->env, NULL, 0, &txn));

    MDB_val dbkey, dbvalue;

    dbkey.mv_size = lkey;
    dbkey.mv_data = (void *)key;

    MDBE(mdb_get(txn, engine->dbi, &dbkey, &dbvalue));

    *lvalue = dbvalue.mv_size;
    *value = zcalloc(dbvalue.mv_size);
    memcpy(*value, dbvalue.mv_data, dbvalue.mv_size);

    EXIT:
    C_CLOSE(txn, mdb_txn_abort);
    return ret;
}

int db_engine_remove(db_engine_t *engine, const void *key, const uint32_t lkey) {
    int ret = DB_OK;

    MDB_txn *txn = NULL;
    MDBE(mdb_txn_begin(engine->env, NULL, 0, &txn));

    MDB_val dbkey, dbvalue;

    dbkey.mv_size = lkey;
    dbkey.mv_data = (void *)key;

    MDBE(mdb_del(txn, engine->dbi, &dbkey, NULL));

    EXIT:
    C_CLOSE(txn, mdb_txn_abort);
    return ret;
}

int db_engine_drop(db_engine_t *engine) {
    MDB_txn *txn = NULL;
    MDBE(mdb_txn_begin(engine->env, NULL, 0, &txn));
    MDBE(mdb_drop(txn, engine->dbi, 0));
    MDBE(mdb_txn_commit(txn));
    //清除dbi，避免释放的时候报错
    engine->dbi = 0;
    return DB_OK;

    EXIT:
    mdb_txn_abort(txn);
    return DB_OK;
}

void db_engine_close(db_engine_t *engine) {
    mdb_dbi_close(engine->env, engine->dbi);
    C_CLOSE(engine->env, mdb_env_close);
}
