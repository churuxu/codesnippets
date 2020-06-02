#pragma once


#include <stdint.h>

/**
base64编解码
*/

#ifdef __cplusplus
extern "C" {
#endif


//编码，返回编码后字符串长度
int base64_encode(const void* data, int len, char* buf, int buflen);

//解码，返回解码后字节长度，失败返回0
int base64_decode(const char* str, int len, void* buf, int buflen);



#ifdef __cplusplus
}
#endif
