#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "hexcodec.h"

/*

*/

#define BUF_SIZE 2048

static int fd1_;
static int fd2_;
static char dev_name_[64];
static char dev_name2_[64];
static char action_;
static char codec_;
static int baud_;
static char parity_;
static int stopbits_;
static char buf_[BUF_SIZE];

static void show_help(const char* appname){
    printf("Serial Input Output Tool\n");
    printf("  usage:      %s <R|W|M[H]> <serial> [baud] [parity] [stopbits] \n", appname);
    printf("  usage:      %s T <serial1> [serial2]\n", appname);
    printf("  action:     R=read, W=write, M=monitor, T=self test, H means in hex format\n");
    printf("  serial:     the serial dev file name\n");
    printf("  baud:       speed such as 9600, default:115200\n");
    printf("  parity:     N=none, E=even, O=odd, default:N\n");
    printf("  stopbits:   1 or 2, default:1\n");
    printf("  example:    %s RH COM3 115200 N 1\n", appname);
}



//使用全局配置参数，打开串口，返回fd
static int serial_open(const char* name, int verbose){
    int ret;
    int fd = 0;
    struct termios attr;
    speed_t sp;
  
    fd = open(name, O_RDWR);
    if(fd<0){
        if(verbose) perror("open error\n");        
        return -1;
    }

    ret = tcgetattr(fd, &attr);
    if(ret){
        if(verbose)perror("tcgetattr error\n");
        goto error;
    }

    switch (baud_)    {
    case 1200: sp = B1200; break;
    case 2400: sp = B2400; break;
    case 4800: sp = B4800; break;
    case 9600: sp = B9600; break;
    case 19200: sp = B19200; break;
    case 38400: sp = B38400; break;
    case 57600: sp = B57600; break;
    case 115200: sp = B115200; break;
    default:
        if(verbose)printf("not support baud %d\n", baud_);
        goto error;  
    }

    cfsetispeed(&attr, sp);
    cfsetospeed(&attr, sp);

    attr.c_cflag &= ~CSIZE;
    attr.c_cflag |= CS8;

    if(parity_ == 'N'){
        attr.c_cflag &= ~PARENB;
        attr.c_cflag &= ~PARODD;
    }else if(parity_ == 'E'){
        attr.c_cflag |= PARENB;
        attr.c_cflag &= ~PARODD;
    }else if(parity_ == 'O'){
        attr.c_cflag |= PARENB;
        attr.c_cflag |= PARODD;
    }else{
        if(verbose)printf("not support parity %c\n", parity_);
        goto error;
    }

    if(stopbits_ == 1){
        attr.c_cflag &= ~CSTOPB;
    }else if(stopbits_ == 2){
        attr.c_cflag |= CSTOPB;
    }else{
        if(verbose)printf("not support stopbits %d\n", stopbits_);
        goto error;        
    }
    ret = tcsetattr(fd, TCSANOW,  &attr);
    if(ret){
        if(verbose)printf("tcsetattr error\n");
        goto error;         
    }  
    
    return fd;
error:
    if(fd>0)close(fd);
    return -1;
}

//读取串口输入
static int serial_read(int fd, void* buf, int len){
    return read(fd, buf, len);
}

//写串口
static int serial_write(int fd, const void* buf, int len){
    return write(fd, buf, len);
}

//读取控制台输入
static int console_read(void* buf, int len){
    return fread(buf, 1, len, stdin);
}

//写控制台
static int console_write(const void* buf, int len){
    fwrite(buf, 1, len, stdout);
    fwrite("\n", 1, 1, stdout);
    return len;
}


//监控操作
static int action_read(int monitor){
    char buf[BUF_SIZE];
    int ret;
    int fd;
    fd = serial_open(dev_name_, 1);
    if(fd<=0)return -1;   
    while(1){
        ret = serial_read(fd, buf, BUF_SIZE);
        if(ret < 0){            
            close(fd);
            if(!monitor)return -1;
            while(1){
                fd = serial_open(dev_name_, 0);
                if(fd > 0)break;
                sleep(3);
            }
        }
        if(ret == 0){
            continue;
        }
        console_write(buf, ret);        
    }
    return -1;
}


//写操作
static int action_write(){
    char buf[BUF_SIZE];
    int ret;
    int len;
    int fd;
    fd = serial_open(dev_name_, 1);
    if(fd<0)return -1;
    while(1){
        ret = console_read(buf, BUF_SIZE);
        if(ret < 0){
            printf("console read error");
            return -1;
        }
        if(ret == 0){
            continue;
        }
        len = serial_write(fd, buf, ret);
        if(len != ret){
            printf("serial write error");
            return -1;
        }
        break;             
    }
    return 0;
}



//自发自收测试
static int action_test(){
    int fd1, fd2;
    char buf[BUF_SIZE];
    int len1, len2;
    int ret;       

    printf("open %s ...\n", dev_name_);
    fd1 = serial_open(dev_name_, 1);
    if(fd1 < 0)return -1;
    
    if(dev_name2_[0]){
        printf("open %s ...\n", dev_name2_);
        fd2 = serial_open(dev_name2_, 1);        
        if(fd2 < 0)return -1;        
    }else{
        fd2 = fd1;
    }
    fd1_ = fd1;
    fd2_ = fd2;

       
    printf("write ...\n");
    snprintf(buf, BUF_SIZE, "hello %d", rand());
    len1 = strlen(buf);
    ret = serial_write(fd1, buf, len1);
    if(ret != len1){
        printf("serial write error");
        return -1;        
    }
  
    printf("read ...\n");
    len2 = serial_read(fd2, buf_, BUF_SIZE);     
    printf("writed : %s\n", buf);
    printf("readed : %s\n", buf_);
    if(len1 == len2 && memcmp(buf, buf_, len1) == 0){
        printf("matched\n");
        return 0;
    }
    return 1;
}


int main(int argc, char* argv[]){
    const char* str;
    if(argc < 4){ 
        show_help(argv[0]);
        return 1;
    }


    str = argv[1];
    action_ = str[0];
    codec_ = str[1];

    snprintf(dev_name_, 64, "%s", argv[2]);
    
    if(argc > 3){
        if(action_ == 'T'){
            snprintf(dev_name2_, 64, "%s", argv[3]);
            baud_ = 115200;
        }else{
            baud_ = strtol(argv[3],NULL,10);
        }        
        if(!baud_){
            show_help(argv[0]);
            return -1;
        }
    }else{
        baud_ = 115200;
    }
    
    if(argc > 4){
        str = argv[4];
        if(str[1] != 0){
            show_help(argv[0]);
            return -1;
        }
        parity_ = str[0];
    }else{
        parity_ = 'N';
    }

    if(argc > 5){
        stopbits_ = strtol(argv[5],NULL,10);
    }else{ 
        stopbits_ = 1;
    }

    if(action_ == 'R'){
        return action_read(0);
    }
    if(action_ == 'W'){
        return action_write();
    }
    if(action_ == 'M'){
        return action_read(1);
    }
    if(action_ == 'T'){
        return action_test();
    }
    printf("not support action %s\n", argv[1]);
    return 1;
}



