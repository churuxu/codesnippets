#include <stdio.h>
#include "fileutil.h"




//加载整个文件
//buflen in|out
int file_load(const char* name, void* buf, size_t* buflen){
    FILE* fd = fopen(name, "rb");
    int ret = -1;
    int len;
    if(fd){
        len = (int)fread(buf, 1, *buflen, fd); 
        if(len>0){
            *buflen = (size_t)len; 
            ret = 0;           
        }
        fclose(fd);
    } 
    return ret;   
}

//保存整个文件
int file_save(const char* name, const void* buf, size_t buflen){
    FILE* fd = fopen(name, "wb");
    int ret = -1;
    int len;
    if(fd){
        len = (int)fwrite(buf, 1, buflen, fd); 
        if(len == buflen){
            ret = 0;
        }
        fclose(fd);
    } 
    return ret;
}


int file_write(const char* name, size_t offset, const void* buf, size_t buflen){
    FILE* fd = NULL;
    int ret = -1;
    int len,i;
    size_t cur;
    size_t dur;
    char zero = 0;
    fd = fopen(name, "rb+");
    if(fd){
        fseek(fd, 0, SEEK_END);
        cur = ftell(fd);
        if(cur < offset){ //当前文件比offset小, 则填充0
            dur = offset - cur;
            for(i=0;i<dur;i++){
                len = fwrite(&zero, 1, 1, fd);
                if(len != 1)goto clean;
            }
        }else{
            fseek(fd, offset, SEEK_SET);
        }
        len = (int)fwrite(buf, 1, buflen, fd); 
        if(len == buflen){
            ret = 0;
        }        
    }     
clean:  
    if(fd)fclose(fd);
    return ret;
}


