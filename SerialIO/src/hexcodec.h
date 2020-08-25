#pragma once


/**
十六进制转换
*/

#ifdef __cplusplus
extern "C" {
#endif

#define HEX_ENCODE_FLAG_UPPERCASE 0 //大写字母
#define HEX_ENCODE_FLAG_LOWERCASE 1 //小写字母
#define HEX_ENCODE_FLAG_SPACING   2 //每个字节之间留空格
#define HEX_ENCODE_FLAG_ELLIPSIS  4 //超过显示省略号, 注:只用于界面显示，无法decode

//编码，返回编码后字符串长度
int hex_encode(const void* data, int datalen, char* buf, int buflen);

//编码，返回编码后字符串长度, 支持指定选项
int hex_encode_with_flag(const void* data, int datalen, char* buf, int buflen, int flag);

//解码，返回解码后字节长度，失败返回0
int hex_decode(const char* str, int strlen, void* buf, int buflen);



#ifdef __cplusplus
}
#endif
