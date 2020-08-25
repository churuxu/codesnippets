#pragma once

/*

user layer:
http://pubs.opengroup.org/onlinepubs/9699919799/


功能：
文件
Socket
串口
目录
时钟
事件
动态库

*/

#define _FILE_OFFSET_BITS 64  //支持4G以上文件


#ifdef _WIN32
#include "posix_core_win32.h"
#endif

//stdc headers
#include <stdlib.h> //for size_t
#include <stdint.h>
#include <stdio.h> //for SEEK_SET
#include <string.h> //for malloc free
#include <errno.h>
#include <signal.h> 
#include <time.h> 

#include <sys/types.h> 

//generic posix headers
#if defined(__linux__)
#include <unistd.h> //open close read write
#include <poll.h>  //poll
#include <fcntl.h> //fcntl
#include <termios.h> 
#include <sys/socket.h> 
#include <netdb.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__linux__)


//============== generic ===============

#ifndef F_GETFL
#define F_GETFL 0
#define F_SETFL 1
#endif

#ifndef O_NONBLOCK
#define O_NONBLOCK 0x00010000
#endif

#ifndef POLLIN
#define POLLIN  1 //有数据可读时触发
#define POLLOUT 2 //网络连接上时触发
#define POLLHUP 4 //网络连接断开时触发
#define POLLERR 8  //io出错
#define POLLPRI 16 //高优先数据到来
#endif


#ifndef USE_SYSTEM_POLLFD
struct pollfd {
    int fd;
    short events;     
    short revents;
};
#endif

int posix_open(const char* pathname, int flags, ...);
int posix_close(int fd);
int posix_fcntl(int fd, int cmd, ...);
ssize_t posix_read(int fd, void * buf, size_t count);
ssize_t posix_write(int fd, const void* buf, size_t count);
off_t posix_lseek(int fd, off_t offset, int where);
int posix_poll(struct pollfd* fds, unsigned int nfds, int timeout);



//========== socket ===========
#ifndef USE_SYSTEM_SOCKADDR
struct sockaddr{
    unsigned short sa_family;
    char sa_data[14];  
};
#endif
#ifndef USE_SYSTEM_SOCKLEN
typedef int socklen_t;
#endif

int posix_socket(int af, int type, int proto);
int posix_connect(int fd, const struct sockaddr* addr, socklen_t addrlen);
int posix_accept(int fd, struct sockaddr* addrbuf, socklen_t* addrlen);
int posix_bind(int fd, const struct sockaddr* addr, socklen_t addrlen);
int posix_listen(int fd, int cap);
ssize_t posix_recv(int fd, void* buf, size_t count, int flags);
ssize_t posix_send(int fd, const void* buf, size_t count, int flags);
ssize_t posix_recvfrom(int fd, void* buf, size_t buflen, int flags, struct sockaddr* addrbuf, socklen_t* addrlen);
ssize_t posix_sendto(int fd, const void* buf, size_t buflen, int flags, const struct sockaddr* addr, socklen_t addrlen);

#ifndef USE_SYSTEM_ADDRINFO
struct addrinfo{
    int ai_flags;                 /* Input flags.  */
    int ai_family;                /* Protocol family for socket.  */
    int ai_socktype;              /* Socket type.  */
    int ai_protocol;              /* Protocol for socket.  */
    socklen_t ai_addrlen;         /* Length of socket address.  */
    struct sockaddr *ai_addr;     /* Socket address for socket.  */
    char *ai_canonname;           /* Canonical name for service location.  */
    struct addrinfo *ai_next;     /* Pointer to next in list.  */   
};
int posix_getaddrinfo(const char* host, const char* port, const struct addrinfo* hint, struct addrinfo** ai);
void posix_freeaddrinfo(struct addrinfo* ai);
#endif

//============ sys =============
#ifndef USE_SYSTEM_TIMEVAL
struct _timeval_64{
    uint64_t tv_sec;
    uint64_t tv_usec;
};
#define timeval _timeval_64
#endif

#ifndef USE_SYSTEM_TIMEZONE
struct timezone{
    int tz_minuteswest;
    int tz_dsttime;
};

#endif

#ifndef USE_SYSTEM_CLOCK
clock_t posix_clock(void);
#endif
#ifndef USE_SYSTEM_TIME
time_t posix_time(time_t* out);
#endif
int posix_gettimeofday(struct timeval* tv, struct timezone* tz);
int posix_settimeofday(const struct timeval* tv, const struct timezone* tz);

unsigned int posix_sleep(unsigned int seconds);
unsigned int posix_usleep(unsigned int micro_seconds);


//========== dlfcn ===========
void* posix_dlopen(const char* name, int flags);
int posix_dlclose(void* handle);
void* posix_dlsym(void* handle, const char* funcname);




//============== 串口 ================


//数据位
#define CS7   0x070000
#define CS8   0x080000
#define CSIZE 0xff0000

//停止位
#define CSTOPB 0x01000000 //有=2 无=1

//校验位   PARENB|PARODD=奇校验  PARENB=偶校验
#define PARENB 0x02000000 
#define PARODD 0x04000000

//修改串口参数
#define TCSANOW 0
#define TCSADRAIN 1
#define TCSAFLUSH 2


int posix_cfsetispeed(struct termios* attr, speed_t t);
int posix_cfsetospeed(struct termios* attr, speed_t t);
int posix_tcgetattr(int fd, struct termios* attr);
int posix_tcsetattr(int fd, int opt, const struct termios* attr);



//============== renames ===============
#ifndef POSIX_CORE_IMPL

#define open posix_open
#define close posix_close
#define poll posix_poll
#define read posix_read
#define write posix_write
#define fcntl posix_fcntl
#define lseek posix_lseek

#define socket posix_socket
#define connect posix_connect
#define listen posix_listen
#define accept posix_accept
#define bind posix_bind
#define sendto posix_sendto
#define recvfrom posix_recvfrom
#define recv posix_recv
#define send posix_send



#ifndef USE_SYSTEM_ADDRINFO
#define getaddrinfo posix_getaddrinfo
#define freeaddrinfo posix_freeaddrinfo
#endif

#ifndef USE_SYSTEM_CLOCK
#define clock posix_clock
#endif
#ifndef USE_SYSTEM_TIME
#define time posix_time
#endif
#define gettimeofday posix_gettimeofday
#define settimeofday posix_settimeofday

#define sleep posix_sleep
#define usleep posix_usleep


#define cfsetispeed posix_cfsetispeed
#define cfsetospeed posix_cfsetospeed
#define tcgetattr posix_tcgetattr
#define tcsetattr posix_tcsetattr

#endif

#endif //defined(__linux__)

#ifdef __cplusplus
}
#endif

