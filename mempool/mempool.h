#pragma once

#include <stdlib.h>
#include <stdint.h>

/**
内存池
用于分配固定大小的对象所需内存

用法示例：
void* pool = malloc(mempool_buffer_size(4,32));
mempool_init(pool, 4, 32);
void* obj1 = mempool_alloc(pool);
void* obj2 = mempool_alloc(pool);

*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _mempool mempool; 

//循环遍历回调函数，返回非0则停止遍历
typedef int (*mempool_foreach_callback)(void* udata, void* element);

//计算内存池所需buffer大小
size_t mempool_buffer_size(size_t elementsize, size_t maxcount); 

//将指定的内存buffer初始化为内存池，所需大小可通过mempool_buffer_size计算
mempool* mempool_init(void* buffer, size_t elementsize, size_t maxcount);

//分配一个元素所需的内存
void* mempool_alloc(mempool* pool);

//释放一个元素所需的内存
void mempool_free(mempool* pool, void* element);

//遍历当前池，回调中不可调用alloc和free
//void mempool_foreach(mempool* pool, mempool_foreach_callback cb, void* udata);


#ifdef __cplusplus
}
#endif


