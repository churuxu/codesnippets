#include "circle_file.h"

#include "gtest/gtest.h"


TEST(circle_file, test_nodata){
    circle_file* f = circle_file_open("1.dat", "wb", 32);
    ASSERT_TRUE(f);
    circle_file_close(f);
}


TEST(circle_file, test_write1){
    int ret;
    circle_file* f = circle_file_open("2.dat", "wb", 32);
    ASSERT_TRUE(f);   
    ret = circle_file_write(f, "hello", 5);    
    EXPECT_EQ(5, ret);
    ret = circle_file_write(f, "world!", 6); 
    EXPECT_EQ(6, ret);   
    circle_file_close(f);
}

TEST(circle_file, test_write2){
    circle_file* f = circle_file_open("3.dat", "wb", 32);
    ASSERT_TRUE(f);   
    circle_file_write(f, "hello1", 6);    
    circle_file_write(f, "world1", 6);   
    circle_file_write(f, "hello2", 6);    
    circle_file_write(f, "world2", 6);      
    circle_file_write(f, "hello3", 6);    
    circle_file_write(f, "world3", 6); 
    circle_file_write(f, "hello4", 6);    
    circle_file_write(f, "world4", 6);     
    circle_file_close(f);
}


TEST(circle_file, test_read1){
    int ret;
    char buf[32];
    circle_file* f = circle_file_open("r1.dat", "wb", 32);
    ASSERT_TRUE(f);   
    ret = circle_file_write(f, "helloworld!", 11);    
    EXPECT_EQ(11, ret);
    circle_file_close(f);

    f = circle_file_open("r1.dat", "rb", 32);
    ASSERT_TRUE(f);
    circle_file_seek(f, -6, SEEK_END);    
    ret = circle_file_read(f, buf, 16); 
    EXPECT_EQ(6, ret);
    EXPECT_TRUE(0 == memcmp(buf, "world!", 6));

    circle_file_seek(f, -10, SEEK_CUR); 
    ret = circle_file_read(f, buf, 16); 
    EXPECT_EQ(10, ret);
    EXPECT_TRUE(0 == memcmp(buf, "elloworld!", 10));

    circle_file_close(f);
}

TEST(circle_file, test_read2){
    int ret;
    char buf[32];
    circle_file* f = circle_file_open("r2.dat", "wb", 32);
    ASSERT_TRUE(f);   
    ret = circle_file_write(f, "helloworld1", 11);    
    EXPECT_EQ(11, ret);
    ret = circle_file_write(f, "helloworld2", 11);    
    EXPECT_EQ(11, ret);
    ret = circle_file_write(f, "helloworld3", 11);    
    EXPECT_EQ(11, ret);    
    ret = circle_file_write(f, "helloworld4", 11);    
    EXPECT_EQ(11, ret);     
    circle_file_close(f);

    f = circle_file_open("r2.dat", "rb", 32);
    ASSERT_TRUE(f);
    circle_file_seek(f, -6, SEEK_END);    
    ret = circle_file_read(f, buf, 16); 
    EXPECT_EQ(6, ret);
    EXPECT_TRUE(0 == memcmp(buf, "world4", 6));

    circle_file_seek(f, -22, SEEK_CUR); 
    ret = circle_file_read(f, buf, 11); 
    EXPECT_EQ(11, ret);
    EXPECT_TRUE(0 == memcmp(buf, "helloworld3", 11));
    ret = circle_file_read(f, buf, 11); 
    EXPECT_EQ(11, ret);
    EXPECT_TRUE(0 == memcmp(buf, "helloworld4", 11));

    circle_file_close(f);
}

TEST(circle_file, test_wplus){
    int ret;
    char buf[32];
    circle_file* f = circle_file_open("w1.dat", "wb", 32);
    ASSERT_TRUE(f);   
    ret = circle_file_write(f, "helloworld!", 11);    
    EXPECT_EQ(11, ret);
    circle_file_close(f);

    f = circle_file_open("w1.dat", "wb+", 32);
    ASSERT_TRUE(f);   
    ret = circle_file_write(f, "helloworld1", 11);    
    EXPECT_EQ(11, ret);
    circle_file_seek(f, -11, SEEK_CUR);
    ret = circle_file_read(f, buf, 11); 
    EXPECT_EQ(11, ret);
    EXPECT_TRUE(0 == memcmp(buf, "helloworld1", 11));
    circle_file_close(f);

}

TEST(circle_file, test_aplus1){
    int ret;
    char buf[32];
    circle_file* f = circle_file_open("w1.dat", "wb", 32);
    ASSERT_TRUE(f);   
    ret = circle_file_write(f, "helloworld1", 11);    
    EXPECT_EQ(11, ret);
    circle_file_close(f);

    f = circle_file_open("w1.dat", "ab+", 32);
    ASSERT_TRUE(f);   
    ret = circle_file_write(f, "helloworld2", 11);    
    EXPECT_EQ(11, ret);
    circle_file_seek(f, -22, SEEK_CUR);

    ret = circle_file_read(f, buf, 11); 
    EXPECT_EQ(11, ret);
    EXPECT_TRUE(0 == memcmp(buf, "helloworld1", 11));
    ret = circle_file_read(f, buf, 11); 
    EXPECT_EQ(11, ret);
    EXPECT_TRUE(0 == memcmp(buf, "helloworld2", 11));

    circle_file_close(f);

}


TEST(circle_file, test_aplus2){
    int ret;
    char buf[32];
    circle_file* f = circle_file_open("w2.dat", "wb", 32);
    ASSERT_TRUE(f);   
    ret = circle_file_write(f, "helloworld1", 11);    
    EXPECT_EQ(11, ret);
    ret = circle_file_write(f, "helloworld2", 11);    
    EXPECT_EQ(11, ret);
    ret = circle_file_write(f, "helloworld3", 11);    
    EXPECT_EQ(11, ret);
    ret = circle_file_write(f, "helloworld4", 11);    
    EXPECT_EQ(11, ret);    
    circle_file_close(f);

    f = circle_file_open("w2.dat", "ab+", 32);
    ASSERT_TRUE(f);   
    ret = circle_file_write(f, "helloworld5", 11);    
    EXPECT_EQ(11, ret);

    circle_file_seek(f, -22, SEEK_CUR);
    ret = circle_file_read(f, buf, 11); 
    EXPECT_EQ(11, ret);
    EXPECT_TRUE(0 == memcmp(buf, "helloworld4", 11));
    ret = circle_file_read(f, buf, 11); 
    EXPECT_EQ(11, ret);
    EXPECT_TRUE(0 == memcmp(buf, "helloworld5", 11));

    circle_file_close(f);

}


TEST(circle_file, test_aplus3){
    int ret;
    char buf[32];
    remove("w3.dat");
    circle_file* f = circle_file_open("w3.dat", "ab+", 32);
    ASSERT_TRUE(f);   
    ret = circle_file_write(f, "helloworld1", 11);    
    EXPECT_EQ(11, ret);
    ret = circle_file_write(f, "helloworld2", 11);    
    EXPECT_EQ(11, ret);
    ret = circle_file_write(f, "helloworld3", 11);    
    EXPECT_EQ(11, ret);
    ret = circle_file_write(f, "helloworld4", 11);    
    EXPECT_EQ(11, ret);    
    circle_file_close(f);

    f = circle_file_open("w3.dat", "ab+", 32);
    ASSERT_TRUE(f);   
    ret = circle_file_write(f, "helloworld5", 11);    
    EXPECT_EQ(11, ret);

    circle_file_seek(f, -22, SEEK_CUR);
    ret = circle_file_read(f, buf, 11); 
    EXPECT_EQ(11, ret);
    EXPECT_TRUE(0 == memcmp(buf, "helloworld4", 11));
    ret = circle_file_read(f, buf, 11); 
    EXPECT_EQ(11, ret);
    EXPECT_TRUE(0 == memcmp(buf, "helloworld5", 11));

    circle_file_close(f);

}

TEST(circle_file, test_clean){
    int ret = 0;
    ret = remove("1.dat");
    ret = remove("2.dat");
    ret = remove("3.dat");
    ret = remove("r1.dat");
    ret = remove("r2.dat");
    ret = remove("w1.dat");
    ret = remove("w2.dat");
    ret = remove("w3.dat");
    (void)ret;
    //EXPECT_EQ(0, ret);
}
