
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../third/bstrlib/bstrlib.h"
#include "../resp/resp.h"
#include "../src/zmalloc.h"

int main() {
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof server_addr);

    server_addr.sin_len = sizeof(struct sockaddr_in);
    server_addr.sin_family = AF_INET;//Address families AF_INET互联网地址簇
    server_addr.sin_port = htons(9000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //创建socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);//SOCK_STREAM 有连接
    if (server_socket == -1) {
        return -1;
    }

    int ret = connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        perror("connect fail");
        return -1;
    }

    printf("connect success");

    while (1) {
        char buffer[1024] = { 0 };
        scanf("%[^\n]" , buffer );

        bstring cmd = bfromcstr(buffer);
        struct bstrList *list = bsplit(cmd, ' ');

        respItem item;
        item.type = RESP_ARRAY;
        item.array = zcalloc(sizeof(respArray) + list->mlen * sizeof(respItem));
        item.array->argc = list->qty;
        for (int i = 0; i < list->qty; i++) {
            item.array->argv[i].type = RESP_STRING;
            item.array->argv[i].string = list->entry[i];
        }

        bstring string = bfromcstr("");
        respMarshal(&item, string);
        zfree(item.array);

        ssize_t len = write(server_socket, bdata(string), blength(string));
        if (len != blength(string)) {
            perror("write err1");
            return -1;
        }


        respFSM fsm;
        respFsmRest(&fsm);

        while (1) {
            ssize_t len = read(server_socket, buffer, 1024);
            if (len <= 0) {
                perror("write err2");
                return -1;
            }
            int    tmplen;
            struct tagbstring tmpstring;
            btfromblk(tmpstring, buffer, len);
            respItem *item = respUnmarshal(&fsm, &tmpstring, &tmplen);
            if (tmplen < 0) {
                printf("%s\n", bdata(fsm.err));
                return -1;
            }
            printf("recv: %s\n", buffer);
            if (item != NULL) {
                break;
            }
        }

        bdestroy(string);
        bdestroy(cmd);
        bstrListDestroy(list);
    }

}