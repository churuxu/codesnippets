#include "config_manager.h"

#include "gtest/gtest.h"


static const char* json1 = "{\"status\":12,\"message\":\"success\",\"data\":{\"title\":{\"id\":123,\"name\":\"hello\"},\"content\":[{\"id\":456,\"value\":\"world\"},{\"id\":789,\"value\":\"aaaa\"}]}}";



//从内存加载配置
TEST(config_manager, generic){   
    const char* str; 
    int i;
    config* cfg = config_load(json1);
    EXPECT_TRUE(cfg != NULL);
    str = config_get_string(cfg, "message");
    EXPECT_STREQ("success", str);    
    str = config_get_string(cfg, "data");
    EXPECT_TRUE(NULL == str);
    str = config_get_string(cfg, "111");
    EXPECT_TRUE(NULL == str);
    str = config_get_string(NULL, "111");
    EXPECT_TRUE(NULL == str);  
    str = config_get_string(cfg, "data/title/name");    
    EXPECT_STREQ("hello", str);
    str = config_get_string(cfg, "data/content/0/value");    
    EXPECT_STREQ("world", str);
    str = config_get_string(cfg, "data/content/1/value");    
    EXPECT_STREQ("aaaa", str);
    str = config_get_string(cfg, "data/content/2/value");    
    EXPECT_TRUE(NULL == str);  
    i = config_get_integer(cfg, "status");
    EXPECT_EQ(12, i);
    i = config_get_integer(cfg, "data/title/id");
    EXPECT_EQ(123, i);
    i = config_get_integer(cfg, "data/content/0/id");
    EXPECT_EQ(456, i);
    i = config_get_integer(cfg, "data/content/2/id");
    EXPECT_EQ(0, i);

}


//从文件加载配置测试
TEST(config_manager, file){   
    const char* str; 
    int i;
    config* cfg;
    const char* file = "config.tmp";
    FILE* f = fopen(file, "wb");
    ASSERT_TRUE(f);
    fwrite(json1, 1, strlen(json1), f);
    fclose(f);

    cfg = config_load(file);
    remove(file);

    EXPECT_TRUE(cfg != NULL);
    str = config_get_string(cfg, "message");
    EXPECT_STREQ("success", str);    
    str = config_get_string(cfg, "data");
    EXPECT_TRUE(NULL == str);
    str = config_get_string(cfg, "111");
    EXPECT_TRUE(NULL == str);
    str = config_get_string(NULL, "111");
    EXPECT_TRUE(NULL == str);  
    str = config_get_string(cfg, "data/title/name");    
    EXPECT_STREQ("hello", str);
    str = config_get_string(cfg, "data/content/0/value");    
    EXPECT_STREQ("world", str);
    str = config_get_string(cfg, "data/content/1/value");    
    EXPECT_STREQ("aaaa", str);
    str = config_get_string(cfg, "data/content/2/value");    
    EXPECT_TRUE(NULL == str);  
    i = config_get_integer(cfg, "status");
    EXPECT_EQ(12, i);
    i = config_get_integer(cfg, "data/title/id");
    EXPECT_EQ(123, i);
    i = config_get_integer(cfg, "data/content/0/id");
    EXPECT_EQ(456, i);
    i = config_get_integer(cfg, "data/content/2/id");
    EXPECT_EQ(0, i);

}


//遍历子节点测试
TEST(config_manager, child){ 
    const char* str; 
    
    config* cfg;    

    cfg = config_load(json1);
    EXPECT_TRUE(NULL != cfg);
    
    //遍历object
    str = config_enum_child_key(cfg, "data/title", 0);
    EXPECT_TRUE(strcmp(str, "id") == 0 || strcmp(str, "name") == 0);
    str = config_enum_child_key(cfg, "data/title", 1);
    EXPECT_TRUE(strcmp(str, "id") == 0 || strcmp(str, "name") == 0);
    str = config_enum_child_key(cfg, "data/title", 2);
    EXPECT_TRUE(NULL == str);

    //遍历array
    str = config_enum_child_key(cfg, "data/content", 0);
    EXPECT_TRUE(strcmp(str, "0") == 0 || strcmp(str, "1") == 0);
    str = config_enum_child_key(cfg, "data/content", 1);
    EXPECT_TRUE(strcmp(str, "0") == 0 || strcmp(str, "1") == 0);
    str = config_enum_child_key(cfg, "data/content", 2);
    EXPECT_TRUE(NULL == str);

    //遍历根节点
    str = config_enum_child_key(cfg, "", 0);
    EXPECT_TRUE(strcmp(str, "status") == 0 || strcmp(str, "message") == 0|| strcmp(str, "data") == 0);
    str = config_enum_child_key(cfg, "", 1);
    EXPECT_TRUE(strcmp(str, "status") == 0 || strcmp(str, "message") == 0|| strcmp(str, "data") == 0);
    str = config_enum_child_key(cfg, NULL, 2);
    EXPECT_TRUE(strcmp(str, "status") == 0 || strcmp(str, "message") == 0|| strcmp(str, "data") == 0);
    str = config_enum_child_key(cfg, NULL, 3);
    EXPECT_TRUE(NULL == str);    
}



//错误测试
TEST(config_manager, err){     
    config* cfg;    

    cfg = config_load("");
    EXPECT_TRUE(NULL == cfg);  
    cfg = config_load(NULL);
    EXPECT_TRUE(NULL == cfg);  
    cfg = config_load("{123}");
    EXPECT_TRUE(NULL == cfg);  
}


