#include "event_emitter.h"
#include <string.h>
#include <stdio.h>


#define IS_BY_ID(em) (em->maxname <= 0)


typedef struct listener{
    void* ctx;
    event_full_handler handler;       
}listener;

typedef struct id_listener{
    void* ctx;
    event_full_handler handler;
    event_id_t num;       
}id_listener;

typedef struct name_listener{
    void* ctx;
    event_full_handler handler;
    char multi; //多级事件
    char name[];       
}name_listener;


struct event_emitter{
    int count;
    int maxcount;
    int maxname;
    int listenersz;
    //listener* listeners; 
    //listener memory
};

static int start_with(const char* str1, const char* str2){
    while(1){        
        if(!*str2)return 1;        
        if(*str1 != *str2)return 0;
        if(!*str1)return 0;
        str1 ++;
        str2 ++;
    }
    return 0;
}

static listener* get_listener(event_emitter* em, int index){
    uint8_t* mem = (uint8_t*)em;   
    int off = sizeof(event_emitter) + (index * em->listenersz);
    return (listener* )(mem + off);
}

//将指定的内存初始化位事件触发器
event_emitter* event_emitter_init(void* mem, int sz, int maxname){
    event_emitter* ret;
    int listenersize;
    int maxcount;
    if(!mem || sz < 32)return NULL;
    if(maxname <= 0) {
        listenersize = sizeof(id_listener);
    }else{
        listenersize = sizeof(name_listener) + maxname;
    }
    maxcount = (sz - sizeof(event_emitter)) / listenersize;
    if(maxcount < 1)return NULL;
    ret = (event_emitter* )mem;
    ret->maxcount = maxcount;
    ret->maxname = maxname;
    ret->count = 0;
    ret->listenersz = listenersize;
    return ret;
}


//触发普通事件，有订阅者返回1，无订阅者返回0
int event_emitter_emit(event_emitter* em, event_id_t name){
    return event_emitter_emit_data(em, name, NULL, 0 );
}

//订阅事件, 返回订阅id
int event_emitter_add_listener(event_emitter* em, event_id_t name, event_full_handler handler, void* ctx){
    if(em->count >= em->maxcount)return -1;
    if(!handler)return -2;
    if(IS_BY_ID(em)){ //按整数订阅
        id_listener* l = (id_listener*)get_listener(em, em->count);
        l->handler = handler;
        l->ctx = ctx;
        l->num = name;
        em->count ++;
        return 0;
    }else{ //按字符串订阅
        int len;
        name_listener* l = (name_listener*)get_listener(em, em->count);
        len = strlen(name);
        if(len >= em->maxname)return -1;
        l->handler = handler;
        l->ctx = ctx;
        if(len > 2 && name[len-1] == '#' && name[len-2] == '/'){ //订阅多级
            memcpy(l->name, name, len - 2);
            l->name[len - 2] = 0;
            l->multi = 1;
        }else{ //订阅单级
            memcpy(l->name, name, len + 1);
            l->multi = 0;
        }        
        em->count ++;
        return 0;
    }  
    return -3;  
}

//触发事件（带数据），有订阅者返回非0，无订阅者返回0
int event_emitter_emit_data(event_emitter* em, event_id_t name, void* data, int len){
    int i;
    int ret = 0;
    if(IS_BY_ID(em)){
        for(i=0;i<em->count;i++){
            id_listener* l = (id_listener*)get_listener(em, i);
            if(l->num == name){
                l->handler(l->ctx, data, len, name);
                ret ++;
            }
        }
    }else{        
        for(i=0;i<em->count;i++){
            name_listener* l = (name_listener*)get_listener(em, i);
            if(l->multi){
                if(start_with(name, l->name)){
                    l->handler(l->ctx, data, len, name);
                }                
            }else{
                if(strcmp(l->name,name) == 0){
                    l->handler(l->ctx, data, len, name);
                }
            }
        }
    }
    return ret;      
}




