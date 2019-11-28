#include "gtest.h"
#include "buffer_stream.h"


//默认模式基本测试
TEST(buffer_stream, test_default){
    int len;
    void* data;
    buffer_stream* s = buffer_stream_create(64);
    ASSERT_TRUE(s);

    //写入两次，一次读取
    buffer_stream_push(s, "hello", 5);
    buffer_stream_push(s, "world", 5);
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(10, len);
    EXPECT_TRUE(data);
    EXPECT_TRUE(0==memcmp(data, "helloworld", 10));
    buffer_stream_pop(s, len);

    //无数据时，预期获取到NULL
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(0, len);
    EXPECT_TRUE(data == NULL);

    //写入两次，一次读取
    buffer_stream_push(s, "hello1", 6);
    buffer_stream_push(s, "world1", 6);
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(12, len);
    EXPECT_TRUE(data);
    EXPECT_TRUE(0==memcmp(data, "hello1world1", 10));
    buffer_stream_pop(s, len);

    buffer_stream_destroy(s);
}


//fix模式基本测试
TEST(buffer_stream, test_fix){
    int len;
    void* data;
    buffer_stream* s = buffer_stream_create(64);
    ASSERT_TRUE(s);

    //每次固定读4字节
    buffer_stream_set_fix_mode(s, 4);

    //写入数据
    buffer_stream_push(s, "hello", 5);
    buffer_stream_push(s, "world", 5);

    //读第1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(4, len);
    EXPECT_TRUE(data);
    EXPECT_TRUE(0 == memcmp(data, "hell", 4));
    buffer_stream_pop(s, len);
    //读第2次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(4, len);
    EXPECT_TRUE(data);
    EXPECT_TRUE(0 == memcmp(data, "owor", 4));
    buffer_stream_pop(s, len);
    //读第3次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(0, len);
    EXPECT_TRUE(data == NULL);
    //再写一次
    buffer_stream_push(s, "hello", 5);
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(4, len);
    EXPECT_TRUE(data);
    EXPECT_TRUE(0 == memcmp(data, "ldhe", 4));
    buffer_stream_pop(s, len);
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(0, len);
    EXPECT_TRUE(data == NULL);

    buffer_stream_destroy(s);
}

//计算动态长度，例如AT+5hello
static int calc_testat_length(void* data, int len){   
    char* str = (char*)data;
    if(len<4)return 0; //数据不足
    if(str[0] == 'A' && str[1] == 'T' && str[2] == '+'){ 
        //数据足够
        return 4 + (str[3] - '0');
    }    
    //数据有误
    return -1;
}


//dynamic模式基本测试
TEST(buffer_stream, test_dynamic){
    int len;
    void* data;
    buffer_stream* s = buffer_stream_create(64);
    ASSERT_TRUE(s);

    //每次读动态长度
    buffer_stream_set_dynamic_mode(s, calc_testat_length);

    //写入数据
    buffer_stream_push(s, "AT+5helloAT+6world!A", 20);
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(9, len);
    EXPECT_TRUE(data);
    EXPECT_TRUE(0 == memcmp(data, "AT+5hello", 9));
    buffer_stream_pop(s, len);
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(10, len);
    EXPECT_TRUE(data);
    EXPECT_TRUE(0 == memcmp(data, "AT+6world!", 10));
    buffer_stream_pop(s, len);    
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(0, len);
    EXPECT_TRUE(data == NULL);
    //写入数据
    buffer_stream_push(s, "T+3bye", 6);
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(7, len);
    EXPECT_TRUE(data);
    EXPECT_TRUE(0 == memcmp(data, "AT+3bye", 7));
    buffer_stream_pop(s, len);    
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(0, len);
    EXPECT_TRUE(data == NULL);
    //写入错误数据
    buffer_stream_push(s, "T+3bye", 6);   
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(0, len);
    EXPECT_TRUE(data == NULL);
    //写入数据
    buffer_stream_push(s, "AT+3bye", 7);
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(7, len);
    EXPECT_TRUE(data);
    EXPECT_TRUE(0 == memcmp(data, "AT+3bye", 7));
    buffer_stream_pop(s, len); 


    buffer_stream_destroy(s);
}


//split模式基本测试
TEST(buffer_stream, test_split){
    int len;
    void* data;
    buffer_stream* s = buffer_stream_create(64);
    ASSERT_TRUE(s);

    //每次读取一行
    buffer_stream_set_split_mode(s, '\n');

    //写入数据
    buffer_stream_push(s, "hello\nworld\nhaha", 16);
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(6, len);
    EXPECT_TRUE(data);
    EXPECT_TRUE(0 == memcmp(data, "hello\n", 6));
    buffer_stream_pop(s, len);
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(6, len);
    EXPECT_TRUE(data);
    EXPECT_TRUE(0 == memcmp(data, "world\n", 6));
    buffer_stream_pop(s, len);    
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(0, len);
    EXPECT_TRUE(data == NULL);
    //写入数据
    buffer_stream_push(s, " bye\n", 6);
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(9, len);
    EXPECT_TRUE(data);
    EXPECT_TRUE(0 == memcmp(data, "haha bye\n", 9));
    buffer_stream_pop(s, len);    
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(0, len);
    EXPECT_TRUE(data == NULL);    

    buffer_stream_destroy(s);
}


//模式切换测试
TEST(buffer_stream, test_change_mode){
    int len;
    void* data;
    buffer_stream* s = buffer_stream_create(64);
    ASSERT_TRUE(s);

    //写入数据
    buffer_stream_push(s, "recv 5\nhellorecv 6\nworld!aaa", 26);
    //读取一行
    buffer_stream_set_split_mode(s, '\n');
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(7, len);
    EXPECT_TRUE(data);
    EXPECT_TRUE(0 == memcmp(data, "recv 5\n", 7));
    buffer_stream_pop(s, len);
    //读取固定长度
    buffer_stream_set_fix_mode(s, 5);
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(5, len);
    EXPECT_TRUE(data);
    EXPECT_TRUE(0 == memcmp(data, "hello", 5));
    buffer_stream_pop(s, len);
    //读取一行
    buffer_stream_set_split_mode(s, '\n');
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(7, len);
    EXPECT_TRUE(data);
    EXPECT_TRUE(0 == memcmp(data, "recv 6\n", 7));
    buffer_stream_pop(s, len);
    //读取固定长度
    buffer_stream_set_fix_mode(s, 6);
    //读1次
    data = buffer_stream_fetch(s, &len);
    EXPECT_EQ(6, len);
    EXPECT_TRUE(data);
    EXPECT_TRUE(0 == memcmp(data, "world!", 6));
    buffer_stream_pop(s, len);   

    buffer_stream_destroy(s);
}

