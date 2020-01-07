#include "circle_file.h"

#include <string.h>

#define FILE_SIG 0xf1f2f3f4

#define FILE_FLAG_CIRCLED 0x01  //当前已经写到第二圈或以上


/*
文件状态场景有
                         maxsize
开头                         
开头----------结尾
开头-----------------------结尾
结尾--------------------------
------结尾--------------------
 
*/


typedef struct file_head{
    uint32_t sig;
    uint32_t flag; //选项
    uint32_t maxdata; //数据最大大小
    uint32_t end;  //结尾位置    
}file_head;


struct circle_file{
    FILE* fp;
    uint8_t* mem;
    int headsize;
    file_head head; //文件头信息    
    long pos; //当前读写位置
    long datasize; //当前文件数据内容大小
};


//打开文件
circle_file* circle_file_open(const char* name, const char* mode, int maxsize){
    FILE* fp = NULL;
    circle_file* f = NULL;
    size_t szret;
    uint8_t appendmode = 0;    
    uint8_t needwrite = 0;
    uint8_t needread = 0;    
    uint8_t readok = 0;
    const char* pmode = mode;
    f = (circle_file*)malloc(sizeof(circle_file));
    if(!f)return NULL;
    while(*pmode){
        switch (*pmode) {
        case 'r':needread = 1; break; 
        case 'w':needwrite = 1; break;
        case 'a':needread = 1;appendmode=1;mode="rb+"; break;     
        default:break;
        }       
        pmode ++;
    } 
    fp = fopen(name,mode);
    if(!fp){
        if(appendmode)fp = fopen(name, "wb+");        
    }
    if(!fp)return NULL;
    memset(f,0,sizeof(circle_file));
    f->headsize  = sizeof(file_head);
    f->fp = fp; 
    fseek(fp,0,SEEK_SET);
    if(needread){ //读或追加模式，需要先读原有头        
        szret = fread(&(f->head),1,f->headsize,fp);
        readok = (szret == f->headsize && f->head.sig == FILE_SIG); 
        if(readok){
            if(f->head.flag & FILE_FLAG_CIRCLED){
                f->datasize = f->head.maxdata;
            }else{
                f->datasize = f->head.end;
            }
        }
    }
    if(needwrite || (appendmode && !readok)){  //写模式 或 追加模式没有头， 需要新建头
        f->head.sig = FILE_SIG;
        f->head.maxdata = maxsize;        
        szret = fwrite(&(f->head),1,f->headsize,fp);        
        if(szret != f->headsize)goto error;        
    }
    if(appendmode && readok){ //追加模式，移动指针到结尾
        f->pos = f->head.end;
        fseek(fp, f->pos + f->headsize, SEEK_SET);
    }
    
    if(f->head.sig != FILE_SIG)goto error;
    return f;
error:
    if(f)free(f);
    if(fp)fclose(fp);
    return NULL;
}

//关闭文件
void circle_file_close(circle_file* f){
    if(f){
        if(f->fp)fclose(f->fp);
        free(f);
    }
}


//写数据
int circle_file_write(circle_file* f, const void* data, int len){
    size_t szret;    
    long pos = f->pos;
    long remain;
    long vremain;
    if(len <= 0 || len> f->head.maxdata)return -1;

    //当前位置到文件物理结尾还有多少
    remain = f->head.maxdata - pos; 
    //当前位置到文件抽象结尾还有多少
    if(f->head.end >= pos){ // ---ptr---end---
        vremain = f->head.end - pos;
    }else{ // --end---------ptr--
        vremain = f->head.maxdata - pos + f->head.end;
    }

    
    if(remain >= len){ //物理文件结尾足够        
        szret = fwrite(data,  1, len, f->fp);
        if(szret != len)return -1;
        pos += len;        
    }else{ //物理文件结尾不足        
        szret = fwrite(data,  1, remain, f->fp);
        if(szret != remain)return -1;
        fseek(f->fp, f->headsize, SEEK_SET); //回到开头再继续写
        f->head.flag |= FILE_FLAG_CIRCLED;         
        szret = fwrite((uint8_t*)data + remain,  1, len - remain, f->fp);
        if(szret != (len - remain))return -1;
        pos = len - remain;
    }   
    if(vremain >= len){ //抽象结尾足够

    }else{ //抽象结尾不够，结尾后移
        f->head.end = pos;
    }
    fseek(f->fp, 0, SEEK_SET);
    szret = fwrite(&(f->head),1,f->headsize,f->fp);
    if(szret != f->headsize)return -3;    
    f->pos = pos;
    fseek(f->fp, pos + f->headsize, SEEK_SET);
    if(f->datasize < f->head.maxdata){
        f->datasize += len;
        if(f->datasize > f->head.maxdata)f->datasize = f->head.maxdata;
    }
    return len;
}

//读数据
int circle_file_read(circle_file* f, void* buf, int buflen){
    size_t szret;    
    long pos = f->pos;
    long remain;
    long vremain;
    int rdlen;
    long totalsize;
    if(buflen <= 0)return -1;
    totalsize = (f->datasize < f->head.maxdata)?f->datasize:f->head.maxdata;
    //当前位置到文件物理结尾还有多少
    remain = totalsize - pos; 
    //当前位置到文件抽象结尾还有多少
    if(f->head.end >= pos){ // ---ptr---end---
        vremain = f->head.end - pos;
    }else{ // --end---------ptr--
        vremain = f->head.maxdata - pos + f->head.end;
    }
    if(vremain > buflen){ //文件抽象数据足够
        rdlen = buflen;
    }else{ //文件抽象数据不足
        rdlen = vremain;
    }
    
    if(remain >= rdlen){ //物理文件结尾足够
        szret = fread(buf,  1, rdlen, f->fp);
        pos += rdlen;
    }else{ //物理文件结尾不足
        szret = fread(buf,  1, remain, f->fp);
        if(szret != remain)return -1;
        fseek(f->fp, f->headsize, SEEK_SET); //回到开头再继续读        
        szret = fread((uint8_t*)buf + remain,  1, rdlen - remain, f->fp);
        if(szret != (rdlen - remain))return -1;
        pos = (rdlen - remain);
    }     
    f->pos = pos;
    return rdlen;
}


//移动当前读写指针位置
int circle_file_seek(circle_file* f, long pos, int where){
    long maxdatasize;
    long newpos;
    maxdatasize = (f->head.flag & FILE_FLAG_CIRCLED)?f->head.maxdata:f->datasize;
    //maxdatasize -= f->headsize;
    if(where == SEEK_END){
        if(pos > 0)return -1; //超过文件尾        
        if(maxdatasize + pos < 0)return -1; //超过文件头
        newpos = f->head.end + pos;
        if(newpos < 0)newpos = f->head.maxdata + newpos;        
    }else if(where == SEEK_CUR){ 
        if(pos > maxdatasize || pos < (0 - maxdatasize))return -1;
        newpos = f->pos + pos;
        if(newpos < 0)newpos = f->head.maxdata + newpos;
        if(newpos > f->head.maxdata)newpos = newpos - f->head.maxdata;
    }else{
        return -1;
    }
    f->pos = newpos;
    return fseek(f->fp, f->headsize + newpos, SEEK_SET);
}