#ifdef USE_GTEST

#include "hexcodec.h"
#include <string.h>

#include "gtest/gtest.h"


TEST(hexcodec, encode){
    uint8_t mem[] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF};
    int ret;
    char buf[32];  

    ret = hex_encode(mem, 8, buf, 32);    
    EXPECT_EQ(16, ret);
    EXPECT_STREQ("1234567890ABCDEF", buf);

    ret = hex_encode_with_flag(mem, 8, buf, 32, HEX_ENCODE_FLAG_SPACING);    
    EXPECT_EQ(23, ret);
    EXPECT_STREQ("12 34 56 78 90 AB CD EF", buf);

    ret = hex_encode_with_flag(mem, 8, buf, 32, HEX_ENCODE_FLAG_LOWERCASE);    
    EXPECT_EQ(16, ret);
    EXPECT_STREQ("1234567890abcdef", buf);

    ret = hex_encode_with_flag(mem, 8, buf, 32, HEX_ENCODE_FLAG_UPPERCASE);    
    EXPECT_EQ(16, ret);
    EXPECT_STREQ("1234567890ABCDEF", buf);    

    ret = hex_encode_with_flag(mem, 8, buf, 15, HEX_ENCODE_FLAG_ELLIPSIS);    
    EXPECT_EQ(13, ret);
    EXPECT_STREQ("1234567890...", buf);   

    ret = hex_encode_with_flag(mem, 8, buf, 20, HEX_ENCODE_FLAG_SPACING|HEX_ENCODE_FLAG_ELLIPSIS);    
    EXPECT_EQ(18, ret);
    EXPECT_STREQ("12 34 56 78 90 ...", buf); 

}


TEST(hexcodec, decode){    
    int ret;
    uint8_t buf[32];  
    uint8_t mem[] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF};

    ret = hex_decode("1234567890ABCDEF", 16, buf, 32);   
    EXPECT_EQ(8, ret);
    EXPECT_EQ(0, memcmp(mem, buf, 8));

    ret = hex_decode("1234567890abcdef", 16, buf, 32);   
    EXPECT_EQ(8, ret);
    EXPECT_EQ(0, memcmp(mem, buf, 8));

    ret = hex_decode("12 34 56 78 90 AB CD EF", 23, buf, 32);   
    EXPECT_EQ(8, ret);
    EXPECT_EQ(0, memcmp(mem, buf, 8));

    ret = hex_decode("12\n34\r56\t78 90 aB CD ef", 23, buf, 32);   
    EXPECT_EQ(8, ret);
    EXPECT_EQ(0, memcmp(mem, buf, 8));

}

#endif
