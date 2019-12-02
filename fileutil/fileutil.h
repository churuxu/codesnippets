#pragma once

/**
文件实用函数
 * 
 */

#include <stdint.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif


//加载整个文件
//buflen in|out
int file_load(const char* name, void* buf, size_t* buflen);

//保存整个文件
int file_save(const char* name, const void* buf, size_t buflen);

//写文件
int file_write(const char* name, size_t offset, const void* buf, size_t buflen);



#ifdef __cplusplus
}
#endif

