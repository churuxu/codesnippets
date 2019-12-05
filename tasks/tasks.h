#pragma once

/**
任务调度

*/


#include <time.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif


#define TASK_STATUS_YIELD   0
#define TASK_STATUS_FINISH  1


#define _TASK_CONCAT2(s1, s2) s1##s2
#define _TASK_CONCAT(s1, s2) _TASK_CONCAT2(s1, s2)
#define _TASK_LABLE  _TASK_CONCAT(task_pos_, __LINE__)

//单个任务运行状态
typedef struct task_context{
    void* pos;
    clock_t expire;
	int delay;
    int status;    
}task_context;

//任务调度器
typedef struct task_scheduler task_scheduler;

//时钟函数
typedef clock_t (*task_clock_function)();

//任务运行函数
typedef void (*task_function)(task_context* ctx);

//获取当前时钟
clock_t task_scheduler_tick();

//设置时钟函数
void task_scheduler_set_clock(task_clock_function clockfn);

#define TASK_SCHEDULER_SIZE(maxtask) ((maxtask * sizeof(void*) * 2) + 32)

//将指定的内存初始化为调度器
task_scheduler* task_scheduler_init(void* mem, int memsize);

//获取任务数量
int task_scheduler_count(task_scheduler* s);

//运行任务，返回到下一次运行的时间间隔
int task_scheduler_exec(task_scheduler* s);

//添加任务，成功返回0，任务已满则返回-1
int task_scheduler_add(task_scheduler* s, task_function func, task_context* ctx);


//任务是否已结束
#define task_is_finish(ctx) ((ctx)->status == TASK_STATUS_FINISH)

//取消任务
#define task_cancel(ctx) (ctx)->status = TASK_STATUS_FINISH


//进入任务代码块
#define task_enter(ctx) \
    task_context* _current_task_ = ctx;\
    if(ctx->pos)goto *(ctx->pos);\

//退出任务代码块
#define task_leave() \
    _current_task_->status = TASK_STATUS_FINISH;\
    task_yield(); \
    return;\

 //让出任务执行
#define task_yield() \
    _current_task_->pos = &&_TASK_LABLE;\
    return;\
_TASK_LABLE: 

 //退出任务
#define task_exit() \
    _current_task_->status = TASK_STATUS_FINISH;\
    _current_task_->pos = NULL; \
    return;\


 //等待条件，条件true则继续运行
#define task_wait(cond) \
    while(!(cond)){ \
        task_yield();\
    }


//等待条件，条件true或超时，则继续运行，通过status判断是否超时
#define task_timed_wait(cond, ms) \
    _current_task_->expire = task_scheduler_tick() + ms; \
    while(!(cond)){ \
        clock_t now = task_scheduler_tick(); \
        _current_task_->delay = _current_task_->expire - now; \
        if(_current_task_->delay<=0){\
            _current_task_->delay = 0xfffff;\
            break;\
        }\
        task_yield();\
    }

//等待固定时间，之后继续运行
#define task_sleep(ms)  \
    task_timed_wait(0, ms);



#ifdef __cplusplus
}
#endif
