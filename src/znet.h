//
// Created by 张育 on 2022/1/9.
//

#ifndef FREEDB_ZNET_H
#define FREEDB_ZNET_H

//目前在freedb中全部使用非阻塞io和水平触发模式来处理；此文件主要是封装一些基础网络函数用，以z开头主要是怕同名

//将fd设置为非阻塞模式，0表示失败
int fdSetNonBlocking(int fd);

//判断网络调用的错误是否致命；是致命错误则回复非0
int netErrorAgain();

/*
 * 监听any地址的port端口同时设置为非阻塞fd；错误情况下返回-1，否则返回文件描述符
 * */
int zlisten(short port);

#endif //FREEDB_ZNET_H
