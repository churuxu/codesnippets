#include <string.h>
#include <stdio.h>
#include "math_calc.h"



#define ERROR_NOT_NUMBER    -1
#define ERROR_NUMBER_FORMAT -2
#define ERROR_ARG -3
 
#define EXPECT_NUMBER   0
#define EXPECT_OPERATOR 1

#define CALC_STACK_MAX 64
#define VALUE_STR_MAX 33

typedef struct calc_item{    
    char isopt;
    char opt;
    char numtype;
    unumber num;
}calc_item;

typedef struct calc_stack{
    int count;     
    calc_item items[CALC_STACK_MAX];
}calc_stack;

typedef struct token_status{
    const char* next;
    char opt;
    char isopt;
    char expect;    
    char numstr[VALUE_STR_MAX];
}token_status;

//字符串转数值, 成功返回unumber类型，失败返回<0
int unumber_parse(unumber* result, const char* exp){
    int64_t ival;
    double dval;
    char* end = NULL;
    int isreal = 0;
    int isi64 = 0;
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
        isreal = (strchr(exp, '.') != NULL);
        if(isreal){ //实数
            dval = strtod(exp, &end);
        }else{ //整数
            ival = strtoll(exp, &end, 10);
        }
    }else if(exp[0] == '.'){ // 0.几，省略0的
        isreal = 1;
        dval = strtod(exp, &end);
    }else{ //非数字
        result->val_int64 = 0;
        return ERROR_NOT_NUMBER;
    }

    if(isreal){ //实数        
        if(end && end[0]){ //有后缀 = float
            if((end[0] != 'f' && end[0] != 'F') || end[1] != 0){ //错误后缀
                result->val_int64 = 0;
                return ERROR_NUMBER_FORMAT; 
            }else{ //正确后缀
                result->val_float = (float)(isnegative?(0.0-dval):dval);
                return UNUMBER_TYPE_FLOAT;
            }              
        }else{ //没有后缀 = double
            result->val_double = (isnegative?(0.0-dval):dval);
            return UNUMBER_TYPE_DOUBLE;
        }        
    }else{ //整数
        if(end && end[0]){ //有后缀 = int64
            if((end[0] != 'L' && end[0] != 'l') || end[1] != 0){
                result->val_int64 = 0;
                return ERROR_NUMBER_FORMAT;                 
            }
            isi64 = 1;
        }
        if(ival > (int64_t)INT32_MAX)isi64 = 1; //超过int最大大小也是 int64
        if(isi64){
            result->val_int64 = isnegative?(0-ival):ival;;
            return UNUMBER_TYPE_INT64;
        }else{
            result->val_int = isnegative?(0-ival):ival;
            return UNUMBER_TYPE_INT;
        }
    }
    return ERROR_NUMBER_FORMAT;
}


//获取运算符优先级
static int get_operator_level(char ch){
    switch (ch){
    case '*':
    case '/':  
    case '%':  
        return 3;        
    case '+':
    case '-':
        return 4;
    case 'r': //右移
    case 'l': //左移
        return 5;
    case '&':  
        return 8;
    case '^':  
        return 9;   
    case '|':  
        return 10;
    default:
        return 0;
    }
}

//判断是否数值
static int is_value_char(char ch){
    if(ch>='0' && ch<='9')return 1;
    if(ch>='a' && ch<='z')return 1;
    if(ch>='A' && ch<='Z')return 1;
    if(ch=='_')return 1;
    if(ch=='.')return 1;
    return 0;
}


//跳过空白字符
static const char* skip_blank(const char* exp){
    const char* ret = exp;
    char ch;
    if(!exp)return NULL;
    do{
        ch = *ret;
        if(ch != ' ' && ch !=  '\t')break;
        ret ++;
    }while (ch);
    return ret;
}


//获取下一段
static int next_token(token_status* stat){
    char ch;
    int i;
    const char* next = skip_blank(stat->next); 
    const char* end;
    if(!next)return 1;
    if(*next == 0)return 1;  
    ch = *next;
    
    if (stat->expect == EXPECT_NUMBER){ //下一段是数字
        if(ch == '('){ 
            end = next + 1;
            stat->isopt = 1;
            stat->opt = ch;
            stat->next = end;
            return 0;
        }else{
            if(!is_value_char(ch) && ch != '-')return -1;
            stat->numstr[0] = next[0];
            for(i=1;i<(VALUE_STR_MAX-1);i++){
                if(!next[i]){ //已经结尾
                    end = NULL;
                    break;
                }else{ //未结尾
                    if(is_value_char(next[i])){ //是值类型的字符
                        stat->numstr[i] = next[i];
                    }else{
                        end = &next[i];                        
                        break;
                    }
                }
            }            
            stat->isopt = 0;
            stat->numstr[i] = 0;
            stat->next = end;
            stat->expect = EXPECT_OPERATOR;
            return 0;
        }
    }else{ //下一段是操作符号
        if(is_value_char(ch))return -1;
        if(ch == '(')return -1;
        end = next + 1;
        if(ch == '>' && *end == '>'){ //右移
            end ++;
            stat->opt = 'r';
        }else if(ch == '<' && *end == '<'){ //左移
            end ++;
            stat->opt = 'l';
        }else{
            stat->opt = ch;
        }
        stat->isopt = 1;
        stat->next = end;
        if(ch != ')')stat->expect = EXPECT_NUMBER;
        return 0;        
    }
}

//单次整数运算
static int calc_int(int a, char opt, int b, int* result){
    switch (opt){
    case '/': if(!b)return -1; *result = a / b; return 0;
    case '%': if(!b)return -1; *result = a % b; return 0;
    case '*': *result = a * b; return 0;
    case '+': *result = a + b; return 0;
    case '-': *result = a - b; return 0;
    case '&': *result = a & b; return 0;
    case '^': *result = a ^ b; return 0;
    case '|': *result = a | b; return 0;
    case 'r': *result = a >> b; return 0;
    case 'l': *result = a << b; return 0;    
    default:return -1;
    }
}
//单次整数运算
static int calc_int64(int64_t a, char opt, int64_t b, int64_t* result){
    switch (opt){    
    case '/': if(!b)return -1; *result = a / b; return 0;
    case '%': if(!b)return -1; *result = a % b; return 0;
    case '*': *result = a * b; return 0;
    case '+': *result = a + b; return 0;
    case '-': *result = a - b; return 0;
    case '&': *result = a & b; return 0;
    case '^': *result = a ^ b; return 0;
    case '|': *result = a | b; return 0;
    case 'r': *result = a >> b; return 0;
    case 'l': *result = a << b; return 0;    
    default:return -1;
    }
}
//单次浮点数运算
static int calc_float(float a, char opt, float b, float* result){
    switch (opt){
    case '*': *result = a * b; return 0;
    case '/': *result = a / b; return 0;    
    case '+': *result = a + b; return 0;
    case '-': *result = a - b; return 0;
    default:return -1;
    }
}
//单次浮点数运算
static int calc_double(double a, char opt, double b, double* result){
    switch (opt){
    case '*': *result = a * b; return 0;
    case '/': *result = a / b; return 0;    
    case '+': *result = a + b; return 0;
    case '-': *result = a - b; return 0;
    default:return -1;
    }
}

//获取指定类型的值
static float unumber_get_float(unumber* n, int type){
    switch (type){
    case UNUMBER_TYPE_FLOAT:return (float)n->val_float;
    case UNUMBER_TYPE_DOUBLE:return (float)n->val_double;
    case UNUMBER_TYPE_INT64:return (float)(n->val_int64);
    case UNUMBER_TYPE_INT:return (float)(n->val_int);    
    default:return 0.0f;
    }
}
static double unumber_get_double(unumber* n, int type){
    switch (type){
    case UNUMBER_TYPE_DOUBLE:return (double)n->val_double;
    case UNUMBER_TYPE_FLOAT:return (double)n->val_float;
    case UNUMBER_TYPE_INT64:return (double)(n->val_int64);
    case UNUMBER_TYPE_INT:return (double)(n->val_int);    
    default:return 0.0;
    }
}
static int unumber_get_int(unumber* n, int type){
    switch (type){
    case UNUMBER_TYPE_INT:return (int)(n->val_int);
    case UNUMBER_TYPE_INT64:return (int)(n->val_int64);    
    case UNUMBER_TYPE_FLOAT:return (int)n->val_float;
    case UNUMBER_TYPE_DOUBLE:return (int)n->val_double;   
    default:return 0;
    }
}
static int64_t unumber_get_int64(unumber* n, int type){
    switch (type){
    case UNUMBER_TYPE_INT64:return (int64_t)(n->val_int64);
    case UNUMBER_TYPE_INT:return (int64_t)(n->val_int);         
    case UNUMBER_TYPE_FLOAT:return (int64_t)n->val_float;
    case UNUMBER_TYPE_DOUBLE:return (int64_t)n->val_double;   
    default:return 0;
    }
}

//栈里取两个数和一个运算符，计算一次, 结果放回栈里
static int stack_calc_once(calc_stack* st){
    calc_item* item1; 
    calc_item* item2;
    calc_item* op;
    int count;
    unumber result;
    int ret;
    char resulttype;
    if(st->count < 3)return 0;
    count = st->count;
    item2 = &st->items[count - 1];
    op = &st->items[count - 2];
    item1 = &st->items[count - 3];
    if(item1->isopt || item2->isopt || !op->isopt)return -1;

    if(item1->numtype == UNUMBER_TYPE_DOUBLE || item2->numtype == UNUMBER_TYPE_DOUBLE){
        //任意一个操作数为double,则结果为double        
        resulttype = UNUMBER_TYPE_DOUBLE;
        ret = calc_double(unumber_get_double(&item1->num, item1->numtype), op->opt, unumber_get_double(&item2->num, item2->numtype), &result.val_double);
    }else if(item1->numtype == UNUMBER_TYPE_FLOAT || item2->numtype == UNUMBER_TYPE_FLOAT){
        //没有一个double,  任意一个操作数为float,则结果为float
        resulttype = UNUMBER_TYPE_FLOAT;
        ret = calc_float(unumber_get_float(&item1->num, item1->numtype), op->opt, unumber_get_float(&item2->num, item2->numtype), &result.val_float);
    }else if(item1->numtype == UNUMBER_TYPE_INT64 || item2->numtype == UNUMBER_TYPE_INT64){
        //没有一个double/float,  任意一个操作数为int64, 则结果为int64
        resulttype = UNUMBER_TYPE_INT64;
        ret = calc_int64(unumber_get_int64(&item1->num, item1->numtype), op->opt, unumber_get_int64(&item2->num, item2->numtype), &result.val_int64);
    }else{
        //都是int，则结果为int
        resulttype = UNUMBER_TYPE_INT;
        ret = calc_int(unumber_get_int(&item1->num, item1->numtype), op->opt, unumber_get_int(&item2->num, item2->numtype), &result.val_int);
    }
    if(ret < 0)return ret;
    item1->numtype = resulttype;
    item1->num = result;
    st->count = st->count - 2;
    return 0;
}


//获取到的段添加到栈中
static int stack_push_token(calc_stack* st, token_status* stat, math_var_getter func){
    calc_item* item;
    int ret;
    if(st->count >= CALC_STACK_MAX)return -1;
    item = &st->items[st->count];
    item->isopt = stat->isopt;
    if(item->isopt){
        item->opt = stat->opt;
    }else{
        ret = unumber_parse(&item->num, stat->numstr);
        if(func && ret == ERROR_NOT_NUMBER){
            ret = func(&item->num, stat->numstr);
            if(ret == 0 || ret > 3)return -1;
        }
        if(ret<0)return ret;
        item->numtype = ret;        
    }
    st->count ++;
    return 0;
}



/*
如果当前运算符优先级最高，出栈，运算，结果入栈
1 + 2 * 当前运算符优先级比前一个高，不计算
1 * 2 +  当前运算符优先级比前一个低 或 相等， 计算

*/

//当前是否需要计算
static int is_need_calc(calc_stack* st, token_status* stat){
    calc_item* op;
    calc_item* item1; 
    calc_item* item2;    
    char oldop;
    char newop;
    if(st->count < 3)return 0;
    if(!stat->isopt)return 0;
    op = &st->items[st->count - 2];
    if(!op->isopt)return 0;
    item2 = &st->items[st->count - 1];    
    item1 = &st->items[st->count - 3]; 
    if(item1->isopt || item2->isopt)return 0;
    oldop = op->opt;
    newop = stat->opt;
    if(get_operator_level(oldop) <= get_operator_level(newop)){
        //新的优先级低，则需要计算
        return 1;
    }
    return 0;
}

//计算括号内的
static int calc_until_quot(calc_stack* st){
    calc_item* op;
    calc_item* item;
    int ret ;    
    if(st->count < 2)return -1;
    while(1){
        op = &st->items[st->count - 2];
        item = &st->items[st->count - 1];
        if(op->isopt && op->opt == '('){
            op->isopt = item->isopt;
            op->numtype = item->numtype;
            op->num = item->num;
            st->count --;
            return 0;
        } 
        if(st->count<2)return -1;       
        ret = stack_calc_once(st);
        if(ret<0)return ret;        
    }    
    return 0;
}


//计算表达式，表达式中支持变量，变量值通过getter获得（可为NULL），计算成功返回unumber类型，失败返回<0
int math_calc(unumber* result, const char* exp, math_var_getter func){
    token_status stat;
    int ret = 0;  
    calc_item* r;  
    calc_stack st;
    if(!result || !exp)return ERROR_ARG;
    memset(&stat,0,sizeof(stat));
    stat.next = exp;
    stat.expect = EXPECT_NUMBER;
    st.count = 0;
    r = &st.items[0];
    r->numtype = UNUMBER_TYPE_INT;
    r->num.val_int = 0;
    //printf("exp: %s\n",exp);
    while(ret == 0){
        ret = next_token(&stat); //获取下一段
        if(ret < 0)return -1; 
        if(ret == 1)break; //已经结束
        /*
        if(stat.isopt){
            printf(" %c\n",stat.opt);
        }else{
            printf(" %s\n",stat.numstr);
        }*/
        
        if(stat.isopt && stat.opt == ')'){ //如果是), 则计算直到(
            ret = calc_until_quot(&st);
            if(ret < 0)return -1;
        }else{
            //判断运算符优先级，如果需要计算则计算
            while(is_need_calc(&st, &stat)){
                ret = stack_calc_once(&st);
                if(ret < 0)return -1;
            }
            //不需要计算，则将新的段添加栈中
            ret = stack_push_token(&st, &stat, func);
            if(ret < 0)return -1;
        }
    }
    //剩余还有数值，进行计算
    while(st.count >= 3){
        ret = stack_calc_once(&st);
        if(ret < 0)return -1;
    }
    if(st.count != 1)return -1; //结果数量不对
    switch (r->numtype){
    case UNUMBER_TYPE_DOUBLE: result->val_double = r->num.val_double; return UNUMBER_TYPE_DOUBLE;
    case UNUMBER_TYPE_FLOAT: result->val_float = r->num.val_float; return UNUMBER_TYPE_FLOAT;
    case UNUMBER_TYPE_INT64: result->val_int64 = r->num.val_int64; return UNUMBER_TYPE_INT64;
    default: result->val_int = r->num.val_int; return UNUMBER_TYPE_INT;
    }
    return -1;
}





