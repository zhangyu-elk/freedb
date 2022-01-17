//
// Created by 张育 on 2022/1/9.
//

#ifndef FREEDB_ERROR_H
#define FREEDB_ERROR_H

#include <stdio.h>

enum {
    DB_ERR = -1,
    DB_OK
};

#define DIE(fmt, ...) do{ \
    fprintf(stderr, "%s,%d "fmt, __FILE__, __LINE__, ##__VA_ARGS__);       \
    exit(-1);                             \
} while(0)

#endif //FREEDB_ERROR_H
