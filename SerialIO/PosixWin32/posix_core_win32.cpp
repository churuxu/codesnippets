#ifdef _WIN32

#define POSIX_CORE_IMPL
#include "posix_core.h"


#include <memory>
#include <unordered_map>

#define printf(...)

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define TINY_POSIX_UWP
#endif


static LPCWSTR UTF8ToUTF16(const char* str, WCHAR* buf, int buflen) {
	int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, buf, buflen);
	if (len <= 0) {
		buf[0] = 0;		
	}
	return buf;
}


static void SetErrnoFromWSA(){
    int err = 0;
    int wsaerr = WSAGetLastError();
    switch (wsaerr){
        case WSAEWOULDBLOCK: err = EWOULDBLOCK;break;
        case WSATRY_AGAIN: err = EAGAIN;break;
        case WSAEADDRINUSE: err = EADDRINUSE;break;
        case WSAEALREADY: err = EALREADY;break;
        case WSAEBADF: err = EBADF;break;
        case WSAECONNRESET: err = ECONNRESET;break;
        case WSAECONNREFUSED: err = ECONNREFUSED;break;
        case WSAEFAULT: err = EFAULT;break;        
        case WSAEHOSTUNREACH: err = EHOSTUNREACH;break;
        case WSAENOBUFS: err = ENOBUFS;break;
        default:err = EIO;break;
    }
    if(err)errno = err;
}
static void SetErrnoFromWin32Error(){
    int err = 0;
    int win32err = GetLastError();
    switch (win32err){
        case ERROR_FILE_NOT_FOUND: err = ENOENT;break;   
        case ERROR_ACCESS_DENIED: err = EACCES;break;       
        default:err = EIO;break;
    }
    if(err)errno = err;
}

class Win32WSAStarter{
public:
    Win32WSAStarter(){
        WSADATA wsa;
        WSAStartup(MAKEWORD(2,2),&wsa);        
    }
};
Win32WSAStarter _win32_wsa_starter_;


class FileDescriptor{
public:
    virtual ~FileDescriptor(){};

    virtual ssize_t Read(void* buf, size_t len) = 0;

    virtual ssize_t Write(const void* buf, size_t len) = 0;

    virtual void Close() = 0;

    virtual int Control(int cmd, void* val) = 0;

    virtual off_t Seek(off_t pos, int where) = 0;

    //预检测，如果有事件，直接返回事件,如果没有，返回0
    virtual short PrePoll(short event) = 0;

    //事件等待后，调用这个
    virtual short PostPoll(short event) = 0;

    //获取event handle，用以等待事件发生
    virtual HANDLE GetEvent() = 0;

    //获取file handle， 用以原生API操作
    virtual HANDLE GetHandle() = 0;
};


//常规文件
class GenericFileDescriptor:public FileDescriptor{
public:
    GenericFileDescriptor(HANDLE file){        
        file_ = file;
        buf_ = NULL;
        buflen_ = 0;
        readed_ = 0;
        error_ = 0;
        event_ = 0;
        revent_ = 0;    
        flags_ = 0;
        memset(&op_, 0, sizeof(op_));
        op_.hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    }
    ~GenericFileDescriptor(){
        Close();
        if(op_.hEvent){
            CloseHandle(op_.hEvent);
        }        
        if(buf_)free(buf_);
    }
    HANDLE GetEvent(){        
        return op_.hEvent;
    }
    HANDLE GetHandle(){
        return file_;
    }
    
    ssize_t Read(void* buf, size_t len){
        if(flags_ & O_NONBLOCK){
            if(readed_){ //非阻塞，已读到结果
                if(readed_ > len){
                    memcpy(buf, buf_, len);
                    memmove(buf_, buf_ + len, readed_ - len);
                    readed_ -= len;                    
                }else{
                    memcpy(buf, buf_, readed_);
                    len = readed_;
                    readed_ = 0;                    
                } 
                return len;
            }else{ //非阻塞，未读到结果
                //BOOL bret = ReadFile(file_, buf_, buflen_, NULL, &op_);
                //int err = GetLastError();
                errno = EWOULDBLOCK;
                return -1;
            }
        }else{ //阻塞方式读
            DWORD rdlen = 0;
            BOOL bret = ReadFile(file_, buf, len, &rdlen, NULL);
            if(!bret){
                SetErrnoFromWin32Error();
                return -1;
            }
            return rdlen;
        }
    }

    ssize_t Write(const void* buf, size_t len){        
        DWORD wrlen = 0;
        BOOL bret;
        if(flags_ & O_NONBLOCK){ //非阻塞文件，阻塞写
            OVERLAPPED op = {0};
            bret = WriteFile(file_, buf, len, &wrlen, &op);
            if(!bret && GetLastError() != ERROR_IO_PENDING){
                printf("write file error %d", GetLastError());
                SetErrnoFromWin32Error();
                return -1;
            }
            bret = GetOverlappedResult(file_, &op, &wrlen, TRUE);
            if(!bret){
                printf("write file getresult error %d", GetLastError());
                SetErrnoFromWin32Error();
                return -1;
            }
        }else{ //普通文件阻塞写       
            bret = WriteFile(file_, buf, len, &wrlen, NULL);
            if(!bret){
                SetErrnoFromWin32Error();
                return -1;
            }
        }
        return wrlen;
    }

    virtual void Close(){
        CloseHandle(file_);
    }
    virtual int Control(int cmd, void* val){
        if(cmd == F_GETFL){
            return flags_;
        }
        if(cmd == F_SETFL){
            flags_ = (int)(uintptr_t)val;
            if(flags_ & O_NONBLOCK){
                if(!buf_)buf_ = (CHAR*)malloc(4096);
                buflen_ = 4096;
                BOOL bret = ReadFile(file_, buf_, buflen_, NULL, &op_);
                if(!bret){
                    int err = GetLastError();
                    if(err != ERROR_IO_PENDING){
                        return -1;
                    }else{
                        reading_ = true;
                    }
                }
            }
            return 0;
        }
        return -1;
    }

    virtual off_t Seek(off_t pos, int where){
        LARGE_INTEGER oldpos, newpos;
        int method = FILE_BEGIN;
        if(where == SEEK_END)method = FILE_END;
        if(where == SEEK_CUR)method = FILE_CURRENT;
        oldpos.QuadPart = pos;        
        if(!SetFilePointerEx(file_, oldpos, &newpos, method)){
            SetErrnoFromWin32Error();
            return -1;
        }
        return newpos.QuadPart;
    }

    virtual short PrePoll(short event){
        if(!(flags_ & O_NONBLOCK))return event;
        if(event & POLLOUT)return POLLOUT;
        if(readed_)return POLLIN;
        if(reading_)return 0;
        BOOL bret = ReadFile(file_, buf_, buflen_, &readed_, &op_);
        if(!bret){
            int err = GetLastError();
            //printf("PrePollError %d != %d\n", err,ERROR_IO_PENDING);
            if(err != ERROR_IO_PENDING){
                return POLLERR;
            }else{
                reading_ = true; 
            }
        }               
        if(readed_)return POLLIN;
        return 0;
    }
    
    virtual short PostPoll(short event){ 
        if(!(flags_ & O_NONBLOCK))return event;     
        BOOL bret = GetOverlappedResult(file_, &op_, &readed_, FALSE);
        if(!bret){            
            if(GetLastError() != ERROR_IO_PENDING && GetLastError() != ERROR_IO_INCOMPLETE){
                error_ = GetLastError();
                revent_ = POLLERR;
            }
        }else{
            reading_ = false; 
            revent_ = POLLIN;   
            //ReadFile(file_, buf_, buflen_, NULL, &op_);
        }
        ResetEvent(op_.hEvent);
        return revent_;
    }  

private:    
    HANDLE file_;   
    OVERLAPPED op_;
    CHAR* buf_;
    DWORD buflen_;
    DWORD readed_;
    bool reading_;
    int error_;
    short event_;
    short revent_;    
    int flags_;
};

class SocketFileDescriptor:public FileDescriptor{
public:
    SocketFileDescriptor(SOCKET sock){
        int len = sizeof(int);
        hevent_ = CreateEvent(NULL,FALSE,FALSE,NULL);
        sock_ = sock;
        error_ = 0;
        event_ = 0;
        flags_ = 0;
        binded_ = false;
        getsockopt(sock, SOL_SOCKET, SO_TYPE, (char*)&type_, &len);        
    }
    ~SocketFileDescriptor(){
        if(hevent_)CloseHandle(hevent_);
        closesocket(sock_);        
    }
    HANDLE GetEvent(){
        return hevent_;
    }    
    HANDLE GetHandle(){
        return (HANDLE)sock_;
    }
    void SetType(int type){
        type_ = type;
    }
    void SetBinded(){
        binded_ = true;    
    }

    ssize_t Read(void* buf, size_t len){
        ssize_t ret = recv(sock_, (char*)buf, len, 0);
        if(ret < 0)SetErrnoFromWSA();
        return ret;
    }

    ssize_t Write(const void* buf, size_t len){
        ssize_t ret = send(sock_, (const char*)buf, len, 0);
        if(ret < 0)SetErrnoFromWSA();
        return ret;        
    }

    virtual void Close(){
        closesocket(sock_);
    }
    virtual int Control(int cmd, void* val){
        if(cmd == F_GETFL){
            return flags_;
        }
        if(cmd == F_SETFL){
            flags_ = (int)(uintptr_t)val;
            if(flags_ & O_NONBLOCK){
	            u_long nblock = 1;
	            if(ioctlsocket(sock_, FIONBIO, &nblock)){
                    return -1;
                }    
            }
            return 0;
        }
        return -1;
    }
    virtual off_t Seek(off_t pos, int where){
        return -1;
    }



    virtual short PrePoll(short event){  
        if(type_ == SOCK_DGRAM) { //udp
            if(event & POLLOUT) return POLLOUT; //直接可写
            if(!binded_)return 0; //未绑定，不可读
        } 
        if(event_ != event){ //当前监听事件不同 
	        long wsaev = FD_CLOSE;
	        if (event & POLLOUT) {
		        wsaev |= FD_WRITE;
		        wsaev |= FD_CONNECT;
	        }
	        if (event & POLLIN) {
		        wsaev |= FD_READ;
		        wsaev |= FD_ACCEPT;               
	        }
	        if (event & POLLPRI) {		        
		        wsaev |= FD_OOB;                
	        }
            if(WSAEventSelect(sock_, hevent_, wsaev)){
                return POLLERR;
            }else{
                event_ = event;
            }
        }
        return 0;
    }


    virtual short PostPoll(short event){  
        short revent = 0;      
        WSANETWORKEVENTS wsaevents;
        if(type_ == SOCK_DGRAM) { //udp
            if(event & POLLOUT) return POLLOUT; //直接可写
            if(!binded_)return 0; //未绑定，不可读
        }         
		if (0 == WSAEnumNetworkEvents(sock_, hevent_, &wsaevents)) {
            if(wsaevents.lNetworkEvents & FD_READ){
                revent |= (wsaevents.iErrorCode[FD_READ_BIT])?POLLERR:POLLIN;
            }
            if(wsaevents.lNetworkEvents & FD_WRITE){
                revent |= (wsaevents.iErrorCode[FD_WRITE_BIT])?POLLERR:POLLOUT;
            }
            if(wsaevents.lNetworkEvents & FD_OOB){
                revent |= (wsaevents.iErrorCode[FD_OOB_BIT])?POLLERR:POLLIN;
            }
            if(wsaevents.lNetworkEvents & FD_ACCEPT){
                revent |= (wsaevents.iErrorCode[FD_ACCEPT_BIT])?POLLERR:POLLIN;
            }
            if(wsaevents.lNetworkEvents & FD_CONNECT){
                revent |= (wsaevents.iErrorCode[FD_CONNECT_BIT])?POLLERR:POLLOUT;
            }
            if(wsaevents.lNetworkEvents & FD_CLOSE){
                revent |= (wsaevents.iErrorCode[FD_CLOSE_BIT])?POLLERR:POLLIN;
            }
		}
		else {
			revent = POLLERR;			
		}
        return revent;
    }  

private:    
    SOCKET sock_;   
    HANDLE hevent_; 
    int error_;
    short event_;   
    int flags_;
    int type_;
    bool binded_;
};

typedef std::shared_ptr<FileDescriptor>  FileDescriptorPtr;

static std::unordered_map<int, FileDescriptorPtr> fd_table_;
static int fd_index_;


static int FileDescriptorAdd(FileDescriptorPtr ptr){
    if(!ptr)return -1;    
    //find unused fd 
    while(1){
        fd_index_ ++;
        if(fd_index_ <= 0)fd_index_ = 1;
        auto it = fd_table_.find(fd_index_);
        if(it == fd_table_.end()){
            break;
        }
    }
    //add
    fd_table_[fd_index_] = ptr;
    return fd_index_;
}

static FileDescriptorPtr FileDescriptorGet(int fd){
    auto it = fd_table_.find(fd);
    if(it != fd_table_.end()){
        return it->second;
    }
    return nullptr;
}

static void FileDescriptorDel(int fd){
    fd_table_.erase(fd);
}
int FileDescriptorAddSocket(SOCKET f){
    if(f == INVALID_SOCKET)return -1;
    FileDescriptorPtr ptr = std::make_shared<SocketFileDescriptor>(f);
    return FileDescriptorAdd(ptr);
}

SOCKET FileDescriptorGetSocket(int fd){
    FileDescriptorPtr ptr = FileDescriptorGet(fd);
    if(ptr){
        return (SOCKET) ptr->GetHandle();
    }
    return INVALID_SOCKET;
}

HANDLE FileDescriptorGetHandle(int fd){
    FileDescriptorPtr ptr = FileDescriptorGet(fd);
    if(ptr){
        return ptr->GetHandle();
    }
    return NULL;
}
int FileDescriptorAddHandle(HANDLE f){
    if(!f || f == INVALID_HANDLE_VALUE)return -1;
    FileDescriptorPtr ptr = std::make_shared<GenericFileDescriptor>(f);
    return FileDescriptorAdd(ptr);
}



int posix_poll(struct pollfd* pfds, unsigned int nfds, int timeout){
    HANDLE hevents[64];
    int maxev = ((nfds>64)?64:nfds);
    int i;
    int count = 0;
    DWORD ret;
    for(i = 0; i<maxev; i++){
        FileDescriptorPtr ptr = FileDescriptorGet(pfds[i].fd);
        if(!ptr)return -1;
        pfds[i].revents = ptr->PrePoll(pfds[i].events);
        if(pfds[i].revents){
            count ++;
        }else{
            hevents[i] = ptr->GetEvent();
        }
    }
    if(count){
        return count;
    }
    if(maxev == 0){
        SleepEx(timeout, TRUE);   
        return 0;
    }
    ret = WaitForMultipleObjectsEx(maxev, hevents, FALSE, timeout, TRUE);
    printf("wait %d return %d %d\n", timeout,ret, GetLastError());

    for(i = 0; i<maxev; i++){
        FileDescriptorPtr ptr = FileDescriptorGet(pfds[i].fd);
        if(!ptr)return -1;
        pfds[i].revents = ptr->PostPoll(pfds[i].events);
        if(pfds[i].revents){
            count ++;
        }     
    }
    return ret>0?ret:count;
/*
    if(ret >= WAIT_OBJECT_0 && ret < (WAIT_OBJECT_0 + maxev)){
        i = ret - WAIT_OBJECT_0;
        FileDescriptorPtr ptr = FileDescriptorGet(pfds[i].fd);
        pfds[i].revents = ptr->PostPoll(pfds[i].events);
        return 1;
    }
    return -1;
*/
}


ssize_t posix_read(int fd, void * buf, size_t count){
    FileDescriptorPtr ptr = FileDescriptorGet(fd);
    if(ptr)return ptr->Read(buf, count);
    return -1;
}
ssize_t posix_write(int fd, const void* buf, size_t count){
    FileDescriptorPtr ptr = FileDescriptorGet(fd);
    if(ptr)return ptr->Write(buf, count);
    return -1;
}

int posix_close(int fd){
    FileDescriptorDel(fd);
    return 0;
}
int posix_open(const char* pathname, int flags, ...){
    int acc = 0;
    int mode = 0;
	WCHAR buf[MAX_PATH];
    WCHAR comname[64];
    int attr = FILE_ATTRIBUTE_NORMAL;
    if(flags & O_NONBLOCK){
        attr |= FILE_FLAG_OVERLAPPED;
    }
    if(flags & O_RDWR){
        acc |= (GENERIC_READ | GENERIC_WRITE);
    }
    if(flags & O_WRONLY){
        acc |= GENERIC_WRITE;
    } 
    if(!acc)acc = GENERIC_READ;
    
    HANDLE file = NULL;
    if(flags & O_CREAT){
        mode = CREATE_ALWAYS;
    }else{
        mode = OPEN_EXISTING;
    }
	LPCWSTR wname = UTF8ToUTF16(pathname, buf, MAX_PATH);

    if(memcmp(wname, L"COM", 3*sizeof(WCHAR)) == 0){
        //COM10以上需要以\\\\.\\COM10这样的路径打开
        int len = wcslen(wname);
        if(len < 16){
            comname[0] = '\\';
            comname[1] = '\\';
            comname[2] = '.';
            comname[3] = '\\';
            memcpy(&comname[4], wname, (len+1)*sizeof(WCHAR));
            wname = comname;
        }        
    }

#ifdef TINY_POSIX_UWP
	CREATEFILE2_EXTENDED_PARAMETERS param;
	ZeroMemory(&param, sizeof(param));
	param.dwSize = sizeof(param);
	param.dwFileAttributes = attr;	
	file = CreateFile2(wname, acc, 0, mode, &param);
#else
    file = CreateFileW(wname, acc, 0, NULL, mode, attr, NULL);
#endif
    if(!file || file == INVALID_HANDLE_VALUE){
        SetErrnoFromWin32Error();
        return -1; 
    }
    int fd = FileDescriptorAddHandle(file);    
    FileDescriptorPtr ptr = FileDescriptorGet(fd);
    if(ptr){
        if(ptr->Control(F_SETFL, (void*)(uintptr_t)flags)){
            FileDescriptorDel(fd);
            return -1;
        }        
    }
    return fd;
}
int posix_fcntl(int fd, int cmd, ...){
    va_list ap;
    void* value; 
    int ret = -1;   
    FileDescriptorPtr ptr = FileDescriptorGet(fd);
    if(ptr){
        va_start(ap, cmd);
        value = va_arg(ap, void*);
        ret = ptr->Control(cmd, value);
        va_end(ap);
    }   
    return ret;
}
off_t posix_lseek(int fd, off_t offset, int where){
    FileDescriptorPtr ptr = FileDescriptorGet(fd);
    if(ptr){
        return ptr->Seek(offset, where);
    }   
    return -1;    
}

int posix_socket(int af, int type, int proto){
    SOCKET sock = socket(af, type, proto);    
    int ret = FileDescriptorAddSocket(sock);
    if(ret==-1)SetErrnoFromWSA();    
    return ret;    
}
int posix_connect(int fd, const struct sockaddr* addr, socklen_t addrlen){
    SOCKET sock = FileDescriptorGetSocket(fd);
    int ret = connect(sock, addr, addrlen);
    if(ret)SetErrnoFromWSA();
    return ret;
}
int posix_accept(int fd, struct sockaddr* addrbuf, socklen_t* addrlen){
    SOCKET sock = FileDescriptorGetSocket(fd);
    SOCKET client = accept(sock, addrbuf, addrlen);
    int ret = FileDescriptorAddSocket(client);
    if(ret==-1)SetErrnoFromWSA();
    return ret;
}
int posix_bind(int fd, const struct sockaddr* addr, socklen_t addrlen){
    SOCKET sock = FileDescriptorGetSocket(fd);
    int ret = bind(sock, addr, addrlen);
    if(ret){
        SetErrnoFromWSA();
        return ret;
    }
    auto ptr = std::dynamic_pointer_cast<SocketFileDescriptor>(FileDescriptorGet(fd));
    if(ptr){
        ptr->SetBinded();
    }
    return ret;    
}
int posix_listen(int fd, int cap){
    SOCKET sock = FileDescriptorGetSocket(fd);
    int ret = listen(sock, cap); 
    if(ret)SetErrnoFromWSA();
    return ret;         
}
ssize_t posix_recv(int fd, void* buf, size_t count, int flags){
    SOCKET sock = FileDescriptorGetSocket(fd);
    ssize_t ret = recv(sock, (char*)buf, count, flags); 
    if(ret<0)SetErrnoFromWSA();
    return ret;         
}
ssize_t posix_send(int fd, const void* buf, size_t count, int flags){
    SOCKET sock = FileDescriptorGetSocket(fd);
    ssize_t ret = send(sock, (char*)buf, count, flags);
    if(ret<0)SetErrnoFromWSA();
    return ret;               
}
ssize_t posix_recvfrom(int fd, void* buf, size_t buflen, int flags, struct sockaddr* addr, socklen_t* addrlen){
    SOCKET sock = FileDescriptorGetSocket(fd);
    ssize_t ret = recvfrom(sock, (char*)buf, buflen, flags, addr, addrlen); 
    if(ret<0)SetErrnoFromWSA();
    return ret;
}
ssize_t posix_sendto(int fd, const void* buf, size_t buflen, int flags, const struct sockaddr* addr, socklen_t addrlen){
    SOCKET sock = FileDescriptorGetSocket(fd);
    ssize_t ret = sendto(sock, (char*)buf, buflen, flags, addr, addrlen); 
    if(ret<0){
        SetErrnoFromWSA();
        return ret;        
    }
    
    auto ptr = std::dynamic_pointer_cast<SocketFileDescriptor>(FileDescriptorGet(fd));
    if(ptr){
        ptr->SetBinded();
    }    
    return ret;
}






//获取系统时间
int posix_gettimeofday(struct timeval* tv, struct timezone* tz){
	FILETIME ft;
	uint64_t t;
    uint64_t ms;
	GetSystemTimeAsFileTime(&ft);
	t = (uint64_t)ft.dwHighDateTime << 32 | ft.dwLowDateTime;	
	ms = (t / 10000 - 11644473600000ULL);
    tv->tv_sec = ms / 1000;
    tv->tv_usec = (ms % 1000) * 1000;
    return 0;
}

//设置系统时间
int posix_settimeofday(const struct timeval* tv, const struct timezone* tz){
    errno = EPERM;
    return -1;
}


//========== sys ===========

//
static void win32_hq_delay(unsigned int us){
	LARGE_INTEGER nTickPerSecond;
	LARGE_INTEGER nTick;
    LARGE_INTEGER nTimeoutTick;
    uint64_t delaytime;
    if(!QueryPerformanceFrequency(&nTickPerSecond)) goto error;
	if(!QueryPerformanceCounter(&nTick)) goto error;
    if(us < 50)goto error;
    us -= 35; //减少一些调用耗时
    delaytime = (us * nTickPerSecond.QuadPart) / (1000 * 1000);
    nTimeoutTick.QuadPart = nTick.QuadPart + delaytime;
    while(nTick.QuadPart < nTimeoutTick.QuadPart){
        SwitchToThread();
        if(!QueryPerformanceCounter(&nTick)) goto error;        
    }
    return;
error:
    SwitchToThread();
    return;
}

unsigned int posix_sleep(unsigned int seconds){
    unsigned int ms = seconds * 1000;
    Sleep(ms);
    return 0;
}

unsigned int posix_usleep(unsigned int micro_seconds){
    unsigned int ms = micro_seconds / 1000;    
    if(ms > 10){ //时间较长，直接用Sleep
        Sleep(ms);
    }else{ //时间较短，采用高精度等待
        win32_hq_delay(micro_seconds);
    }     
    return 0;
}

//============= dlfcn ================
void* posix_dlopen(const char* name, int flags){
#ifdef TINY_POSIX_UWP
	WCHAR buf[MAX_PATH];
	LPCWSTR wname = UTF8ToUTF16(name, buf, MAX_PATH);
	return (void*)LoadPackagedLibrary(wname, flags);
#else
	return (void*)LoadLibraryA(name);
#endif    
}

int posix_dlclose(void* handle){
    FreeLibrary((HMODULE)handle);   
    return 0;
}
void* posix_dlsym(void* handle, const char* funcname){
    return (void*)GetProcAddress((HMODULE)handle, funcname);
}


//============= termios ================

int posix_cfsetispeed(struct termios* attr, speed_t t){
    attr->dcb.BaudRate = t;
    return 0;
}
int posix_cfsetospeed(struct termios* attr, speed_t t){
    attr->dcb.BaudRate = t;
    return 0;
}
int posix_tcgetattr(int fd, struct termios* attr){	
	attr->dcb.DCBlength = sizeof(DCB);
    HANDLE file = FileDescriptorGetHandle(fd);
	if(!GetCommState(file, &attr->dcb))return -1;
    attr->c_cflag = 0;    
    //数据位长度
    if(attr->dcb.ByteSize == 7){
        attr->c_cflag |= CS7;
    }else{
        attr->c_cflag |= CS8;
    }
    //校验位
    if(attr->dcb.Parity){
        attr->c_cflag |= PARENB; 
        if(attr->dcb.Parity == ODDPARITY)attr->c_cflag |= PARODD;  
    }
    //停止位
    if(attr->dcb.StopBits == TWOSTOPBITS){
        attr->c_cflag |= CSTOPB;
    }
    return 0;
}
int posix_tcsetattr(int fd, int opt, const struct termios* attr){
    DCB dcb;
    int wordlen;
    HANDLE file = FileDescriptorGetHandle(fd);
    memcpy(&dcb, &attr->dcb, sizeof(DCB));
    dcb.DCBlength = sizeof(DCB);
    //数据位长度
    wordlen = (attr->c_cflag & CSIZE);
    if(wordlen == CS7){
        dcb.ByteSize = 7;
    }else{
        dcb.ByteSize = 8;
    }
    //校验位
    if(attr->c_cflag & PARENB){
        if(attr->c_cflag & PARODD){
            dcb.Parity = ODDPARITY;
        }else{
            dcb.Parity = EVENPARITY;
        }
    }else{
        dcb.Parity = NOPARITY;
    }
    //停止位
    if(attr->c_cflag & CSTOPB){
        dcb.StopBits = TWOSTOPBITS;
    }else{
        dcb.StopBits = ONESTOPBIT;
    } 
    if(!SetCommState(file, &dcb))return -1;

    //默认超时设置
    
	COMMTIMEOUTS CommTimeOuts;
	CommTimeOuts.ReadIntervalTimeout = 50;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 0;   
	CommTimeOuts.WriteTotalTimeoutMultiplier = 50;
	CommTimeOuts.WriteTotalTimeoutConstant = 2000;
	if(!SetCommTimeouts(file, &CommTimeOuts))return -1;
    
    if(!SetCommMask(file,  EV_BREAK|EV_ERR))return -1;

    //初始化设置
    if(!SetupComm(file, 4096, 4096))return -1;

    return 0;
}

#endif


