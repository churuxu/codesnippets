#include "unistd.h"
#include "poll.h"
#include "gtest/gtest.h"

TEST(posix, file){

    int fd,ret;
    char buf[64];

    fd = open("not_exist.txt", O_RDONLY);
    if(fd>=0){
        printf("test error\n");
        return ;
    }
    if(errno != ENOENT){
        printf("errno error\n");
        return ;
    }

    
    fd = open("test.txt", O_WRONLY|O_CREAT|O_TRUNC);
    if(fd<0){
        perror("open for write error\n");
        return ;
    }
    ret = write(fd, "hello", 5);
    if(ret != 5){
        perror("write file error\n");
        return ;
    }
    close(fd);

    fd = open("test.txt", O_RDONLY);
    if(fd<0){
        perror("open for read error\n");
        return ;
    }

    ret = lseek(fd, 0, SEEK_END);
    if(ret != 5){
        perror("lseek file error\n");
        return ;
    } 

    ret = lseek(fd, 1, SEEK_SET);

    ret = read(fd, buf, 64);
    if(ret != 4){
        perror("read file error\n");
        return ;
    }
    buf[ret] = 0;
    printf("file readed:%s\n", buf);

    close(fd);

}