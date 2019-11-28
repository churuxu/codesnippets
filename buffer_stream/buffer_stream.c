#include "buffer_stream.h"
#include <string.h>


struct _buffer_stream{
    void* mode_arg;
    uint32_t mode; //当前模式
    uint32_t data_size; //当前一条数据长度
    uint32_t buffer_size; //当前缓冲区总数据长度
    uint32_t buffer_max; //缓冲区大小
    uint8_t buffer[];
};


buffer_stream* buffer_stream_create(int bufsize){
    int memsize = bufsize + sizeof(buffer_stream);
    buffer_stream* s = (buffer_stream*)malloc(memsize);
    if(s){
        s->buffer_max = bufsize;
        s->mode = 0;
        s->buffer_size = 0;
        s->data_size = 0;
    }
    return s;
} 

//计算下一条数据长度
static void buffer_stream_calc_data(buffer_stream* s){
    if(s->mode == BUFFER_STREAM_MODE_FIX){ //固定长度
        int len = (int)(intptr_t)s->mode_arg;
        if(s->buffer_size >= len){
            s->data_size = len;
        }else{
            s->data_size = 0;
        }
    }else if(s->mode == BUFFER_STREAM_MODE_DYNAMIC){ //动态长度
        int len = 0;
        buffer_stream_calc_func func = (buffer_stream_calc_func)s->mode_arg;
        if(func){
            len = func(s->buffer, s->buffer_size);
            if(len<0){ //数据有误，清空数据
                s->data_size = 0;
                s->buffer_size = 0;
                return;
            }
        }
        if(s->buffer_size >= len){
            s->data_size = len;
        }else{
            s->data_size = 0;
        }
    }else if(s->mode == BUFFER_STREAM_MODE_SPLIT){ //字符分隔
        int ch = (int)(intptr_t)s->mode_arg;
        uint8_t* finddata = (uint8_t*)memchr(s->buffer, ch, s->buffer_size);
        if(finddata){
            s->data_size = finddata - &(s->buffer[0]) + 1;
        }else{
            s->data_size = 0;
        }
    }else{ //默认方式
        s->data_size = s->buffer_size;
    }
}


void buffer_stream_destroy(buffer_stream* s){
    free(s);
}

buffer_stream* buffer_stream_init(void* mem, int memsize){
    buffer_stream* s = (buffer_stream*)mem;
    int bufsize = memsize - sizeof(buffer_stream);
    if(bufsize > 0){
        s->buffer_max = bufsize;
        s->mode = 0;
        s->buffer_size = 0;
        s->data_size = 0;
        return s;
    }
    return NULL;
}

//设置模式
int buffer_stream_set_mode(buffer_stream* s, int mode, void* arg){
    if(mode < 0 || mode > 4)return -1;
    s->mode = mode;
    s->mode_arg = arg;
    return 0;
}

//是否为空
int buffer_stream_is_empty(buffer_stream* s){
    return s->buffer_size == 0;
}

//清空缓存
int buffer_stream_clear(buffer_stream* s){
    s->buffer_size = 0;
    s->data_size = 0;
    return 0;
}

//将任意数据放到buffer_stream里
//buffer已满则返回-1
int buffer_stream_push(buffer_stream* s, const void* data, int len){
    uint32_t remain = s->buffer_max - s->buffer_size;
    if(len<=0)return -2;
    if(remain > len){
        memcpy(s->buffer + s->buffer_size, data, len);
        s->buffer_size += len;
        return len;
    }
    return -1;
}

//分配一段buffer，用于写入任意数据，不能调用pop函数
void* buffer_stream_prepare(buffer_stream* s, int len){
    uint32_t remain = s->buffer_max - s->buffer_size;
    if(len<=0)return NULL;
    if(remain > len){ 
        s->buffer_size += len;  
        return s->buffer + s->buffer_size;
    }
    return NULL;    
}

//移除当前一条数据
int buffer_stream_pop(buffer_stream* s, int len){
    int remain;
    if(len >= s->buffer_size){
        s->buffer_size = 0;
    }else{
        remain = s->buffer_size - len;
        memmove(s->buffer, s->buffer + len, remain);
        s->buffer_size = remain;
    }
    s->data_size = 0;
    return 0;
}


//获取当前数据，和数据长度
void* buffer_stream_fetch(buffer_stream* s, int* plen){
    int len;
    buffer_stream_calc_data(s);
    len = s->data_size;
    if(plen)*plen = len;
    return len?s->buffer:NULL;
}
