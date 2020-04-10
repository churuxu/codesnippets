#pragma once



/*
格式化报文组装/解析

解析格式例如 "u8 u16 $Ua|f32 $Ub|f32 $Uc|f32 u16"


组装格式例如 "$ADDR|u8 03 01 00 02 00 $CRC16|U16"


类型
u8 u16 u32 u64  //无符号整数，小端
i8 i16 i32 i64  //整数，小端
U8 U16 U32 U64  //无符号整数，大端
I8 I16 I32 I64  //整数，大端
f32 f64 //浮点数
D*  //字符串形式整数
D1 D2 D3 ... //字符串形式整数，并指定字符数
H*  //字符串形式整数(十六进制)
H1 H2 H3 ... //字符串形式整数(十六进制)，并指定字符数
B1 B2 B3 ... //固定长度二进制
C1 C2 C3 ... //bcd码，固定长度

*/



#include <stdint.h>
#include <stdlib.h>


#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

#ifdef __cplusplus
extern "C" {
#endif

//解析结果数据类型
#define PACKET_VALUE_INTEGER 1
#define PACKET_VALUE_NUMBER  2
#define PACKET_VALUE_BINARY  3

typedef int64_t packet_integer_t;
typedef float packet_number_t;
typedef void* packet_binary_t;

//解析出来的单项值
typedef struct packet_value{
    int type; //类型
    int len; //长度（只有binary才有）
    union {
        packet_integer_t int_val;
        packet_number_t num_val;        
        packet_binary_t bin_val;
    };    
}packet_value;


//回调函数
typedef int (*packet_callback)(void* udata, const char* name, packet_value* val);

//解析结果回调函数, val=解析结果
//成功返回0， 失败返回非0


//解析函数, 成功返回0，失败返回非0
int packet_parse(const char* format, void* buf, int buflen, packet_callback cb, void* udata);


//组装报文回调函数, 
// val->len = 已有长度 val->bin_val = 已有报文数据
// 需要给val赋值，以将值传递给组装函数
// 成功返回0  失败返回非0

//组装报文,成功返回报文长度，失败返回<0
int packet_build(const char* format, void* buf, int buflen, packet_callback cb, void* udata);



#ifdef __cplusplus
}
#endif

