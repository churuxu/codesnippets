#pragma once


#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef __GNUC__
//mingw
#include <unistd.h> 
#endif

#ifdef _MSC_VER
//vc
#if __SIZE_OF_POINTER__ == 64 
typedef int64_t ssize_t;
#else
typedef int32_t ssize_t;
#endif

#endif

#define B300    CBR_300
#define B600    CBR_600
#define B1200   CBR_1200
#define B2400   CBR_2400
#define B4800   CBR_4800
#define B9600   CBR_9600
#define B19200  CBR_19200
#define B38400  CBR_38400
#define B57600  CBR_57600
#define B115200 CBR_115200


typedef int speed_t;

struct termios{    
    uint32_t c_cflag;    
    DCB dcb;
};


#define USE_SYSTEM_SOCKADDR
#define USE_SYSTEM_ADDRINFO 
#define USE_SYSTEM_TIME
#define USE_SYSTEM_CLOCK
#define USE_SYSTEM_TIMEZONE

#ifdef _MSC_VER
#define USE_SYSTEM_POLLFD
#endif

#ifdef __MINGW__
#define USE_SYSTEM_SOCKLEN
#endif

#undef lseek


#ifdef __cplusplus
extern "C" {
#endif


int FileDescriptorAddSocket(SOCKET f);
int FileDescriptorAddHandle(HANDLE f);
SOCKET FileDescriptorGetSocket(int fd);
HANDLE FileDescriptorGetHandle(int fd);



#ifdef __cplusplus
}
#endif

