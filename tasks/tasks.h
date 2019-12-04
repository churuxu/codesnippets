#pragma once

/**
任务调度

多任务并行运行
1个大任务 = 无限运行多个小任务



*/
#include <time.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef size_t co_pos_t;
typedef clock_t task_time_t;

#define CO_RESULT_OK     1
#define CO_RESULT_YIELD  0

#define _TASK_CONCAT2(s1, s2) s1##s2
#define _TASK_CONCAT(s1, s2) _TASK_CONCAT2(s1, s2)
#define _TASK_LABLE  _TASK_CONCAT(task_pos_, __LINE__)

#define _TASK_POS_INIT(s) s = NULL
#define _TASK_POS_SET(s) s = &&_TASK_LABLE; _TASK_LABLE:
#define _TASK_POS_JMP(s) goto *s

#define TASK_STATUS_YIELD   0
#define TASK_STATUS_FINISH  1


typedef struct task_context{
    void* pos;
    clock_t expire;
	int delay;
    int status;    
}task_context;

typedef clock_t (*task_clock_function)();

typedef void (*task_function)(task_context* ctx);


clock_t task_scheduler_tick();

int task_scheduler_init(task_clock_function clockfn);

int task_scheduler_count();

int task_scheduler_exec();

int task_scheduler_add(task_function func, task_context* ctx);

void task_scheduler_cancel(task_context* ctx);


#ifdef DOXYGEN

#define task_enter(ctx) //进入任务代码块
#define task_leave()  //退出任务代码块

#define task_yield() //让出任务执行
#define task_exit()  //退出任务
#define task_wait(cond)  //等待条件，条件true则继续运行
#define task_timed_wait(cond, ms)   //等待条件，条件true或超时，则继续运行，通过status判断是否超时
#define task_sleep(ms)  //等待固定时间，之后继续运行
#else

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
        task_time_t now = task_scheduler_tick(); \
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


#endif

#ifdef __cplusplus
}
#endif
