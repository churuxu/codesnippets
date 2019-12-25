#pragma once

#include <stdlib.h>
#include <stdint.h>


/*
数学表达式计算


运算符支持：+ - * / % << >>  | & ^ ( )

数值类型支持：float double int int64

数值示例： 1.0f  1.0  123  123L

计算结果类型:
double op double/float/int/int64 = double
float op float/int/int64 = float
int64 op int/int64 = int64
int op int = int

支持变量:
abc + a + 1

*/


#ifdef __cplusplus
extern "C" {
#endif


//数值类型
#define UNUMBER_TYPE_INT    1  
#define UNUMBER_TYPE_INT64  2 
#define UNUMBER_TYPE_FLOAT  3
#define UNUMBER_TYPE_DOUBLE 4


// 1=int 1L=int64  1.0=double  1.0f=float

//类型运算规则：有浮点数的，结果为浮点数；有更大的，结果取更大的；
// int + float = float,  float + double = double


//数值
typedef union {
    double val_double;
    float val_float;
    int64_t val_int64;
    int val_int;    
}unumber;

//变量获取函数，按变量名获取变量值，成功返回unumber类型，失败返回<0
typedef int (*math_var_getter)(unumber* result, const char* name);

//计算表达式，表达式中支持变量，变量值通过getter获得（可为NULL），计算成功返回unumber类型，失败返回<0
int math_calc(unumber* result, const char* exp, math_var_getter func);

//字符串转数值, 成功返回unumber类型，失败返回<0
//支持格式  123.0 123 123L 0x123 0x123L
int unumber_parse(unumber* result, const char* exp);

#ifdef __cplusplus
}
#endif

