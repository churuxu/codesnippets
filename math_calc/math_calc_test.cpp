#include "gtest.h"
#include "math_calc.h"
#include <map>
#include <string>


//普通解析
TEST(math_calc, parse_ok){
    unumber num;
    int type;
    
    type = unumber_parse(&num, "1");
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(1, num.val_int);
    type = unumber_parse(&num, "123.0");
    EXPECT_EQ(type, UNUMBER_TYPE_DOUBLE);
    EXPECT_EQ(123.0, num.val_double);
    type = unumber_parse(&num, "0.0");
    EXPECT_EQ(type, UNUMBER_TYPE_DOUBLE);
    EXPECT_EQ(0.0, num.val_double);
    type = unumber_parse(&num, ".12");
    EXPECT_EQ(type, UNUMBER_TYPE_DOUBLE);
    EXPECT_EQ(0.12, num.val_double);    
    type = unumber_parse(&num, "-0.0");
    EXPECT_EQ(type, UNUMBER_TYPE_DOUBLE);
    EXPECT_EQ(0.0, num.val_double);
    type = unumber_parse(&num, "-123.45");
    EXPECT_EQ(type, UNUMBER_TYPE_DOUBLE);
    EXPECT_EQ(-123.45, num.val_double);
    type = unumber_parse(&num, "0");
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(0, num.val_int);
    type = unumber_parse(&num, "001");
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(001, num.val_int);
    type = unumber_parse(&num, "-000");
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(0, num.val_int);
    type = unumber_parse(&num, "123");
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(123, num.val_int);
    type = unumber_parse(&num, "123L");
    EXPECT_EQ(type, UNUMBER_TYPE_INT64);
    EXPECT_EQ(123L, num.val_int64);    
    type = unumber_parse(&num, "0x123");
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(0x123, num.val_int);
    type = unumber_parse(&num, "0x123L");
    EXPECT_EQ(type, UNUMBER_TYPE_INT64);
    EXPECT_EQ(0x123L, num.val_int64); 
    type = unumber_parse(&num, "1234567890123");
    EXPECT_EQ(type, UNUMBER_TYPE_INT64);
    EXPECT_EQ(1234567890123, num.val_int64);
    type = unumber_parse(&num, "0xabcdefABCDEF");
    EXPECT_EQ(type, UNUMBER_TYPE_INT64);
    EXPECT_EQ(0xabcdefABCDEF, num.val_int64);    
    type = unumber_parse(&num, "0xabcdL");
    EXPECT_EQ(type, UNUMBER_TYPE_INT64);
    EXPECT_EQ(0xabcdL, num.val_int64);
    type = unumber_parse(&num, "-123");
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(-123, num.val_int);
    type = unumber_parse(&num, "-123L");
    EXPECT_EQ(type, UNUMBER_TYPE_INT64);
    EXPECT_EQ(-123L, num.val_int64); 
    type = unumber_parse(&num, "-1234567890123");
    EXPECT_EQ(type, UNUMBER_TYPE_INT64);
    EXPECT_EQ(-1234567890123, num.val_int64); 
    type = unumber_parse(&num, "-0xabcdef123456");
    EXPECT_EQ(type, UNUMBER_TYPE_INT64);
    EXPECT_EQ(-0xabcdef123456, num.val_int64); 

}


//错误解析
TEST(math_calc, parse_error){
    unumber num;
    int type;

    type = unumber_parse(&num, NULL);
    EXPECT_TRUE(type<0);
    type = unumber_parse(&num, "");
    EXPECT_TRUE(type<0);
    type = unumber_parse(&num, "abc");
    EXPECT_TRUE(type<0);
    type = unumber_parse(&num, "123a");
    EXPECT_TRUE(type<0);
    type = unumber_parse(&num, "a123");
    EXPECT_TRUE(type<0);
    type = unumber_parse(&num, "0x12.3");
    EXPECT_TRUE(type<0);
    type = unumber_parse(&num, "0a123");
    EXPECT_TRUE(type<0);
    type = unumber_parse(&num, "0L123");
    EXPECT_TRUE(type<0);
    type = unumber_parse(&num, "123L1");
    EXPECT_TRUE(type<0);
    type = unumber_parse(&num, "1.2.3");
    EXPECT_TRUE(type<0);
    type = unumber_parse(&num, "123f");
    EXPECT_TRUE(type<0);
    type = unumber_parse(&num, "--123");
    EXPECT_TRUE(type<0);

    //type = unumber_parse(&num, "1234567890123456789012345678901234567890");
    //EXPECT_TRUE(type<0);        
}


//普通计算
TEST(math_calc, calc_normal){
    unumber num;
    int type;

    type = math_calc(&num, "1 + 2", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(3, num.val_int);

    type = math_calc(&num, "1 >> 2", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(0, num.val_int);

    type = math_calc(&num, "1 << 2", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(4, num.val_int);

    type = math_calc(&num, "1 + 2 + 3 + 4", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(10, num.val_int); 

    type = math_calc(&num, "1 + 2 * 3 + 4", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(11, num.val_int); 

    type = math_calc(&num, "1 + 2 / 3 + 4", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(5, num.val_int);

    type = math_calc(&num, "10 + 2 - 3 * 4 / 2", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(6, num.val_int);
}


//计算复杂表达式
TEST(math_calc, calc_hard){
    unumber num;
    int type;

    type = math_calc(&num, "(1+2-3*4/5+6)>>(7&8)|(9^(10-11*12))", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ((1+2-3*4/5+6)>>(7&8)|(9^(10-11*12)), num.val_int);
}

//计算中转换类型
TEST(math_calc, calc_type){
    unumber num;
    int type;

    type = math_calc(&num, "1 / 2.0", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_DOUBLE);
    EXPECT_EQ(0.5, num.val_double);

    type = math_calc(&num, "-1 + 2L", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT64);
    EXPECT_EQ(1, num.val_int64);
    
    type = math_calc(&num, "1L + 2.0", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_DOUBLE);
    EXPECT_EQ(3.0, num.val_double);

}

//特殊计算
TEST(math_calc, calc_special){
    unumber num;
    int type;

    type = math_calc(&num, "-3", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(-3, num.val_int);

    type = math_calc(&num, "1+-2", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(-1, num.val_int);
    
    type = math_calc(&num, "-1 + 2", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(1, num.val_int); 

    type = math_calc(&num, "-0", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(0, num.val_int); 

    type = math_calc(&num, ".12", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_DOUBLE);
    EXPECT_EQ(0.12, num.val_double);
}

//括号计算
TEST(math_calc, calc_quot){
    unumber num;
    int type;
     
    type = math_calc(&num, "(3 + 3) / 3 + 4", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(6, num.val_int);

    type = math_calc(&num, "1 + 2 * (3 + 4)", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(15, num.val_int); 

    type = math_calc(&num, "(1 + 2)", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(3, num.val_int);

    type = math_calc(&num, "(3)", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(3, num.val_int);

    type = math_calc(&num, "1 + 2 * (3)", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(7, num.val_int);

    type = math_calc(&num, "1 + ((2 * 3) - 4) * 5", NULL);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(11, num.val_int);      
}


//变量处理
static std::map<std::string,std::string> vars_;

static void set_var(const char* name, const char* val){
    if(name && val)vars_[name] = val;
}

static int get_var(unumber* result, const char* name){
    if(!name)return -1;
    auto it = vars_.find(name);
    if(it != vars_.end()){
        return unumber_parse(result, it->second.c_str());
    }
    return -1;
}

//变量计算
TEST(math_calc, calc_var){
    unumber num;
    int type;
     
    set_var("a", "123");
    set_var("b", "234");
    set_var("ccc", "3");

    type = math_calc(&num, "a + b", get_var);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(357, num.val_int);    

    type = math_calc(&num, "3 / ccc", get_var);
    EXPECT_EQ(type, UNUMBER_TYPE_INT);
    EXPECT_EQ(1, num.val_int);
}


//错误计算
TEST(math_calc, calc_error){
    unumber num;
    int type;
    type = math_calc(&num, "1.2.3", NULL);
    EXPECT_TRUE(type<0);     
    type = math_calc(&num, "(3 + 3) / 3 + ", NULL);
    EXPECT_TRUE(type<0);
    type = math_calc(&num, "1 + +2 * (3 + 4)", NULL);
    EXPECT_TRUE(type<0);
    type = math_calc(&num, "(1 + 2", NULL);
    EXPECT_TRUE(type<0);
    type = math_calc(&num, "1 + 2 +", NULL);
    EXPECT_TRUE(type<0);
    type = math_calc(&num, "1 + 2 )", NULL);    
    EXPECT_TRUE(type<0);
    type = math_calc(&num, "1 2 3", NULL);    
    EXPECT_TRUE(type<0);
    type = math_calc(&num, "1)", NULL);    
    EXPECT_TRUE(type<0);
    type = math_calc(&num, "1+1+1)", NULL);    
    EXPECT_TRUE(type<0);
    type = math_calc(&num, "1+1()", NULL);    
    EXPECT_TRUE(type<0);
    type = math_calc(&num, "1+()1", NULL);    
    EXPECT_TRUE(type<0); 
    type = math_calc(&num, "1+a", NULL);    
    EXPECT_TRUE(type<0);     
    type = math_calc(&num, "1?2", NULL);    
    EXPECT_TRUE(type<0);
    type = math_calc(&num, "1|2.0", NULL);    
    EXPECT_TRUE(type<0); 
    type = math_calc(&num, "1/0", NULL);    
    EXPECT_TRUE(type<0); 
    type = math_calc(&num, "1%0", NULL);    
    EXPECT_TRUE(type<0);    
    type = math_calc(&num, "1%(3-3)", NULL);    
    EXPECT_TRUE(type<0);
    type = math_calc(&num, "1234567890123456789012345678901234567890 + 1", NULL);    
    EXPECT_TRUE(type<0);    
}



