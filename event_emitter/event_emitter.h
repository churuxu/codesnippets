#pragma once

#include <stdlib.h>
#include <stdint.h>


/*
事件触发器
订阅事件
触发事件

事件名支持整数或字符串

字符串事件名支持多级：
订阅 a/#
a/111 a/222 都会触发

*/


#ifdef __cplusplus
extern "C" {
#endif

//事件ID （字符串）
typedef const char* event_id_t;

//使用整数作为事件ID 
#define EVENT_ID(num) ((char*)(uintptr_t)(num))

//事件回调函数
typedef void (*event_handler)(void* ctx);

//事件回调函数（带数据）
typedef void (*event_data_handler)(void* ctx, void* data, int len);

//事件回调函数（带数据,带事件名）
typedef void (*event_full_handler)(void* ctx, void* data, int len, const char* name);

typedef struct event_emitter event_emitter;

//将指定的内存初始化为事件触发器
//maxname==0 按整数id触发事件， maxname!=0,按字符串触发事件,字符串事件名不超过maxname
event_emitter* event_emitter_init(void* mem, int sz, int maxname);

//订阅事件, 成功返回0
int event_emitter_add_listener(event_emitter* em, event_id_t name, event_full_handler handler, void* ctx);

//触发事件（不带数据），有订阅者返回非0，无订阅者返回0
int event_emitter_emit(event_emitter* em, event_id_t name);

//触发事件（带数据），有订阅者返回非0，无订阅者返回0
int event_emitter_emit_data(event_emitter* em, event_id_t name, void* data, int len);

//订阅事件
#define event_emitter_on(e,n,h,c) event_emitter_add_listener(e,n,(event_full_handler)h,c)


#ifdef __cplusplus
}
#endif


