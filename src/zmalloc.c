//
// Created by 张育 on 2022/1/9.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

void* zcalloc(size_t size) {
    void *buf = malloc(size);
    if (buf) {
        memset(buf, 0, size);
        return buf;
    }
    //后续如果后续要可能需要封装成宏定义，否则无法打印出行好
    printf("out of memory, malloc [%ld] fail\n", size);
}
