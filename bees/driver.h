//
// Created by 张育 on 2022/1/9.
//

#ifndef FREEDB_DRIVER_H
#define FREEDB_DRIVER_H

#include "ev.h"

struct evApi_st;
typedef struct evApi_st evApi;

struct firedEv_st;
typedef struct firedEv_st firedEv;


typedef struct ioEvent_st ioEvent;
//网络IO事件驱动库，主要包含两部分：1，IO；2，定时器
typedef struct driver_st {
    evApi   *api;           //事件驱动系统api
    ioEvent *events;        //注册的事件
    int     reg_ev_size;    //当前注册的事件个数
    firedEv *fired;         //触发的事件
    int     stop;           //是否停止
    size_t  size;          //最大支持的句柄数量，注意文件描述符不能超过这个的大小
}driver_t;

//io事件
typedef void ioProc_f(driver_t *driver, int fd, void *data, int mask);
struct ioEvent_st {
    int         mask;       //mask
    ioProc_f    *wproc;     //写回调
    ioProc_f    *rproc;     //读回调
    void*       data;       //回调数据
} ;

//工厂函数，用来创建io驱动句柄
driver_t *driver_new(size_t size);

//销毁函数
void driver_close(driver_t *driver);

//启动事件引擎
void driver_run(driver_t *driver);

//停止事件循环，注意只是打了标记还是得等事件线程自己退出
void driver_stop(driver_t *driver);

//删除io事件
void driver_delete_ioevent(driver_t *driver, int fd, int mask);

//注册一个io事件
int driver_register_ioevent(driver_t *driver, int fd, int mask,
                            ioProc_f *proc, void *data);

#endif //FREEDB_DRIVER_H
