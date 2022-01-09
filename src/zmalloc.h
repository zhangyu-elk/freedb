//
// Created by 张育 on 2022/1/9.
//

#ifndef FREEDB_ZMALLOC_H
#define FREEDB_ZMALLOC_H

//分配空间并清空
void* zcalloc(size_t size);
//释放空间
void zfree(void *buf);

#endif //FREEDB_ZMALLOC_H
