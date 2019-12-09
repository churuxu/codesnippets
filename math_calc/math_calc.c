#include <string.h>
#include "math_calc.h"



#define ERROR_NOT_NUMBER    -1
#define ERROR_NUMBER_FORMAT -2
#define ERROR_ARG -3

typedef struct calc_item{    
    char op;
    char type;
    unumber num;
}calc_item;

//字符串转数值, 成功返回unumber类型，失败返回<0
int unumber_parse(unumber* result, const char* exp){
    int64_t ival;
    double dval;
    char* end = NULL;
    char* isreal;
    int isi64;
    int isnegative = 0;
    if(!exp || !result)return ERROR_ARG;
    if(exp[0] == '-'){ //负数
        isnegative = 1;
        exp ++;
    }
    if (exp[0] == '0' && exp[1] == 'x'){ //十六进制   
        isreal = 0;
        ival = strtoll(exp + 2, &end, 16);
    }else if (exp[0] >= '0' && exp[0] <= '9'){ //十进制
        isreal = strchr(exp, '.');
        if(isreal){ //实数
            dval = strtod(exp, &end);
        }else{ //整数
            ival = strtoll(exp, &end, 10);
        }
    }else{ //非数字
        result->val_int64 = 0;
        return ERROR_NOT_NUMBER;
    }

    if(isreal){ //实数
        result->val_double = isnegative?(0.0-dval):dval;
        if(end && end[0]){ //有后缀
            if((end[0] != 'f' && end[0] != 'F') || end[1] != 0){
                result->val_int64 = 0;
                return ERROR_NUMBER_FORMAT; 
            }               
        }
        return UNUMBER_TYPE_DOUBLE;
    }else{ //整数
        if(end && end[0]){ //有后缀
            if((end[0] != 'L' && end[0] != 'l') || end[1] != 0){
                result->val_int64 = 0;
                return ERROR_NUMBER_FORMAT;                 
            }
            isi64 = 1;
        }
        if(ival > (int64_t)INT32_MAX)isi64 = 1;
        if(isi64){
            result->val_int64 = isnegative?(0-ival):ival;;
            return UNUMBER_TYPE_INT64;
        }else{
            result->val_int = isnegative?(0-ival):ival;
            return UNUMBER_TYPE_INT;
        }
    }
}

/*
获取操作数
获取操作符

当前操作数

如果当前运算符优先级最高，出栈，运算，结果入栈
1 + 2 * 当前运算符优先级比前一个高，不计算
1 * 2 +  当前运算符优先级比前一个低 或 相等， 计算
( 1 + 2 * 3 )  当前运算符为 尾括号，计算

*/



//计算表达式，表达式中支持变量，变量值通过getter获得（可为NULL），计算成功返回unumber类型，失败返回<0
int math_calc(unumber* result, const char* exp, math_var_getter func){
    char strbuf[32];
    calc_item  

    if(!result || !exp)return ERROR_ARG;

    return -1;
}





