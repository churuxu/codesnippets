
#include "base64codec.h"
#include <string.h>

#include "gtest/gtest.h"


TEST(base64codec, encode){
    uint8_t mem[] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF};
    int ret;
    char buf[32];  

    ret = base64_encode("", 0, buf, 32);    
    EXPECT_EQ(0, ret);
    EXPECT_STREQ("", buf);

    ret = base64_encode(mem, 8, buf, 32);    
    EXPECT_EQ(12, ret);
    EXPECT_STREQ("EjRWeJCrze8=", buf);  
}


TEST(base64codec, decode){    
    int ret;    
    uint8_t mem[] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF};
    const char* data = "EjRWeJCrze8=";
    int len = strlen(data);
    char buf[32];  

    ret = base64_decode(data, len, buf, 32);    
    EXPECT_EQ(8, ret);
    EXPECT_TRUE(memcmp(buf, mem, 8) == 0);

}


