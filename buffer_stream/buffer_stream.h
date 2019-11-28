#pragma once

#include <stdlib.h>
#include <stdint.h>


/*
缓冲流，主要用途：
通信报文拆包，组包
读取一行
读取固定长度字节
fifo队列
等
*/


#ifdef __cplusplus
extern "C" {
#endif



#define BUFFER_STREAM_MODE_DEFAULT 0 //默认，有多少数据读多少数据
#define BUFFER_STREAM_MODE_FIX     1 //每次读取固定长度
#define BUFFER_STREAM_MODE_DYNAMIC 2 //每次读取动态长度，长度由函数计算
#define BUFFER_STREAM_MODE_SPLIT   3 //按分隔符读取，例如每次读取一行


//长度计算函数，返回单个报文长度
//返回0表示数据不足，返回-1表示数据有误
typedef int (*buffer_stream_calc_func)(void* data, int datalen);

typedef struct _buffer_stream buffer_stream;

//将指定的内存初始化为 buffer_stream（无须释放）
buffer_stream* buffer_stream_init(void* mem, int memsize);

//创建buffer_stream，(需要释放)
buffer_stream* buffer_stream_create(int bufsize);

//释放buffer_stream
void buffer_stream_destroy(buffer_stream* s);

//设置模式
int buffer_stream_set_mode(buffer_stream* s, int mode, void* arg);

//设置默认模式
#define buffer_stream_set_default_mode(s) buffer_stream_set_mode(s, BUFFER_STREAM_MODE_DEFAULT, NULL)
//设置固定长度模式，len指定长度
#define buffer_stream_set_fix_mode(s,len) buffer_stream_set_mode(s, BUFFER_STREAM_MODE_FIX, (void*)(uintptr_t)len)
//设置动态长度模式，func指定buffer_stream_calc_func，用于计算长度
#define buffer_stream_set_dynamic_mode(s,func) buffer_stream_set_mode(s, BUFFER_STREAM_MODE_DYNAMIC,  (void*)func)
//设置分隔符模式，ch指定分隔字符
#define buffer_stream_set_split_mode(s,ch) buffer_stream_set_mode(s, BUFFER_STREAM_MODE_SPLIT, (void*)(uintptr_t)ch)


//是否为空
int buffer_stream_is_empty(buffer_stream* s);

//清空缓存
int buffer_stream_clear(buffer_stream* s);

//将任意数据放到buffer_stream里
//buffer已满则返回-1
int buffer_stream_push(buffer_stream* s, const void* data, int len);

//分配一段buffer，用于写入任意数据，不能调用pop函数
void* buffer_stream_prepare(buffer_stream* s, int len);

//移除当前一段数据
int buffer_stream_pop(buffer_stream* s, int len);

//获取当前数据，和数据长度
void* buffer_stream_fetch(buffer_stream* s, int* len);



#ifdef __cplusplus
}
#endif

