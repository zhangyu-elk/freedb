//
// Created by 张育 on 2022/1/14.
//

#ifndef FREEDB_UTIL_H
#define FREEDB_UTIL_H

//将字符串转为长整型，失败返回0、成功返回1
int string2ll(const char *s, size_t slen, long long *value);

#endif //FREEDB_UTIL_H
