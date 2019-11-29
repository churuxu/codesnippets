#include "mempool.h"
#include <stdio.h>
//池大小 1~256 ，所需next指针 uint8_t
//257~65536  所需next指针 uint16_t
//65537~n 所需next指针 uint32_t

struct _mempool{
    size_t elsz; //元素大小
    size_t indexsz; //索引大小
    size_t maxcount; //最大元素个数
    size_t top;
    //size_t first;
    //size_t last;
    size_t lastf;
    size_t* next;
    //size_t* prev;
    uint8_t* buffer;    
};

#if 0
static void mempool_debug_print(mempool* p,const char* opt){
    size_t i;
    printf("%s first:%d last:%d f:%d top:%d next:",opt, (int)p->first,(int)p->last,(int)p->lastf,(int)p->top);
    for(i=0;i<p->top;i++){
        printf("%d ",(int)(p->next[i]));
    }
    printf("\n");
}
#else
#define mempool_debug_print(a,b)
#endif

//计算内存池所需buffer大小
size_t mempool_buffer_size(size_t elementsize, size_t maxcount){
    if(maxcount<0xff){
        return sizeof(mempool) + elementsize * maxcount + sizeof(uint8_t) * maxcount;
    }else if(maxcount<0xffff){
        return sizeof(mempool) + elementsize * maxcount + sizeof(uint16_t) * maxcount;
    }
    return sizeof(mempool) + elementsize * maxcount + sizeof(void*) * maxcount;
}

//将指定的内存buffer初始化为内存池，所需大小可通过mempool_buffer_size计算
mempool* mempool_init(void* buffer, size_t elementsize, size_t maxcount){
    mempool* ret = (mempool*)buffer;
    int indexsz;
    if(maxcount<0xff){
        indexsz = 1;
    }else if(maxcount<0xffff){
        indexsz = 2;
    }else{
        indexsz = sizeof(void*);
    }
    ret->indexsz = indexsz;
    ret->elsz = elementsize;
    ret->maxcount = maxcount;
    //ret->first = 0;
    //ret->last = 0;
    ret->lastf = (size_t)-1;
    ret->top = 0;
    ret->next = (size_t*) ((uint8_t*)ret + sizeof(mempool));
    ret->next[0] = 0;
    ret->buffer = (uint8_t*)ret + sizeof(mempool) + maxcount * indexsz;  
    return ret;  
}

static size_t get_next(mempool* p, int index){
    size_t ret;
    if(p->indexsz == 1){
        ret = ((uint8_t*)(p->next))[index];
        if(ret == 0xff)ret = (size_t)-1;
    }if(p->indexsz == 2){
        ret = ((uint16_t*)(p->next))[index];
        if(ret == 0xffff)ret = (size_t)-1;
    }else{
        ret = p->next[index];
    }
    return ret;
}
static void set_next(mempool* p, int index, size_t val){    
    if(p->indexsz == 1){
        if(val == (size_t)-1){
            ((uint8_t*)(p->next))[index] = 0xff;
        }else{
            ((uint8_t*)(p->next))[index] = val;
        }        
    }if(p->indexsz == 2){
        if(val == (size_t)-1){
            ((uint16_t*)(p->next))[index] = 0xffff;
        }else{
            ((uint16_t*)(p->next))[index] = val;
        }
    }else{
        p->next[index] = val;
    }
}

void* mempool_alloc(mempool* p){
    size_t index;
    if(p->lastf != (size_t)-1){
        index = p->lastf;
        p->lastf = get_next(p, index);        
    }else{    
        index = p->top;
        if(index >= p->maxcount)return NULL;
        p->top ++;
    }
    //p->last = p->next[index]; 
    mempool_debug_print(p, "after alloc: ");      
    return p->buffer + index * p->elsz;     
}


void mempool_free(mempool* p, void* element){
    
    size_t index = ((uint8_t*)element - p->buffer) / p->elsz;
    set_next(p, index, p->lastf);    
    p->lastf = index;
    mempool_debug_print(p, "after free:  ");     
}

/*
//分配一个元素所需的内存
void* mempool_alloc(mempool* p){
    size_t index;
    index = p->next[p->last];
    p->last = index;
    p->top ++;
    p->next[index] = p->top;
    //p->last = p->next[index]; 
    mempool_debug_print(p, "after alloc: ");      
    return p->buffer + index * p->elsz;     
}

//释放一个元素所需的内存

void mempool_free(mempool* p, void* element){
    size_t index;
    size_t temp;
    index = ((uint8_t*)element - p->buffer) / p->elsz;
    //index.next = 尾.next
    //尾.next = 头
    //头 = index.next
    temp = p->next[p->last];
    p->next[p->last] = p->first;
    p->first = p->next[index];
    p->next[index] = temp;
    mempool_debug_print(p, "after free:  ");     
}*/


