//
// Created by 张育 on 2022/1/9.
//

#ifndef FREEDB_PROCESSER_H
#define FREEDB_PROCESSER_H

//事件处理器全部定义在这里


typedef struct accept_proc_data_st {
    int         mask;
    ioProc_f    *proc;
} accept_proc_data_t;
/*
 * 调用accept函数并将接收到的连接按照mask和proc添加到driver中去
 * */
void accept_proc(driver_t *driver, int fd, void *data, int mask);



#endif //FREEDB_PROCESSER_H
