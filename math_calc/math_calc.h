#pragma once

#include <stdlib.h>
#include <stdint.h>


/*
数学表达式计算

运算类型：+ - * / % << >>  | & ^ ( )

数值类型：double int int64

计算结果类型:
double op int = double
double op int64 = double
double op double = double
int64 op int64 = int64
int op int64 = int64
int op int = int


*/


#ifdef __cplusplus
extern "C" {
#endif


//数值类型
#define UNUMBER_TYPE_DOUBLE 1
#define UNUMBER_TYPE_INT    2
#define UNUMBER_TYPE_INT64  3

//数值
typedef union {
    double val_double;
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

