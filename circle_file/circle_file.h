#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

/*
循环利用的文件
写文件，超过一定大小时，又从文件开头开始写

读文件时，应从文件结尾开始读（因为文件开头可能已经被写覆盖掉了）

*/


#ifdef __cplusplus
extern "C" {
#endif


typedef struct circle_file circle_file;

//打开文件  
//读r  覆盖写w  覆盖读写w+ 追加读写a+  
//maxsize指定文件最大不超过大小  
circle_file* circle_file_open(const char* name, const char* mode, int maxsize);

//关闭文件
void circle_file_close(circle_file* f);

//移动当前读写指针位置 (支持SEEK_END SEEK_CUR, 不支持SEEK_SET)
int circle_file_seek(circle_file* f, long pos, int where);

//写数据
int circle_file_write(circle_file* f, const void* data, int len);

//读数据
int circle_file_read(circle_file* f, void* buf, int buflen);


#ifdef __cplusplus
}
#endif


