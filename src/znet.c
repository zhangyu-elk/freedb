//
// Created by 张育 on 2022/1/9.
//

#include <sys/fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <memory.h>
#include "znet.h"


//监听服务端口
int zlisten(short port) {
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof server_addr);

    server_addr.sin_len = sizeof(struct sockaddr_in);
    server_addr.sin_family = AF_INET;//Address families AF_INET互联网地址簇
    server_addr.sin_port = htons(port);
    //server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_addr.s_addr = INADDR_ANY;

    //创建socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);//SOCK_STREAM 有连接
    if (server_socket == -1) {
        return -1;
    }

    int bind_result = bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_result == -1) {
        return -1;
    }

    if (listen(server_socket, 1024) == -1) {
        return -1;
    }

    if (set_nonblock(server_socket) == -1) {
        return -1;
    }

    return server_socket;
}

int set_nonblock(int fd) {
    int val;
    if ((val = fcntl(fd, F_GETFL, 0)) < 0) {
        return -1;
    }

    val |= O_NONBLOCK;
    if (fcntl(fd,F_SETFL,val) < 0)
        return -1;

    return 0;
}

//判断网络错误是否致命
int net_fatal() {
    if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
        return 0;
    }
    return 1;
}