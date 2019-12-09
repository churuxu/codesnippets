#include "gtest.h"
#include "math_calc.h"



TEST(math_calc, parse_ok){
    unumber num;
    int type;

    type = unumber_parse(&num, "123.0");
    EXPECT_EQ(type, UNUMBER_TYPE_DOUBLE);
    EXPECT_EQ(123.0, num.val_double);
    type = unumber_parse(&num, "0.0");
    EXPECT_EQ(type, UNUMBER_TYPE_DOUBLE);
    EXPECT_EQ(0.0, num.val_double);
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
}