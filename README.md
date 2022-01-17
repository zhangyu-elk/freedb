# freedb
> 给予lmdb实现的一个kv数据库

## 


## 单元测试
使用cutest做单元测试，utest目录下即为单元测试代码

## 事件驱动库
目前仅实现来mac平台上给予kqueue的事件驱动库，

TODO：
1. 定时器实现

## 命令模块

### kv引擎
1. put
2. set

## TODO
1. 性能测试
2. 线程私有错误码定义
3. index模块
4. count模块
5. 多进程读写分离
6. sdk
8. 主备同步
9. 事务机制
10. 命令支持
11. 字符串内存池优化实现