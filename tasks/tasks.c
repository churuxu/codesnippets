#include "tasks.h"

#ifndef MAX_TASKS
#define MAX_TASKS 64
#endif

typedef struct task_item{
    task_context* ctx;
    task_function func;
}task_item;

typedef struct task_scheduler{
    int count;    
    task_item tasks[MAX_TASKS];
}task_scheduler;


static task_scheduler sched_;

clock_t task_scheduler_tick(){
    return clock();
}

int task_scheduler_count(){
    task_scheduler* sched = &sched_;    
    return sched->count;
}

static void remove_finish_task(){
    task_scheduler* sched = &sched_;
    int i;
    int count;
    i = 0;
    count = sched->count;
    while(1){
        if(i >= count)break;
        if(sched->tasks[i].ctx->status == TASK_STATUS_FINISH){
            if(i != (count - 1)){
                sched->tasks[i].ctx = sched->tasks[count - 1].ctx;
                sched->tasks[i].func = sched->tasks[count - 1].func;
            }else{
                count --;
            }
        }else{
            i ++;            
        }
    }
    sched->count = count;
}

int task_scheduler_exec(){
    int i;
    int delay;
    int count;
    task_scheduler* sched = &sched_;
    
    count = sched->count;
    delay = 100000;
    for(i=0;i<count;i++){ //运行每个任务
        sched->tasks[i].func(sched->tasks[i].ctx);
        if(sched->tasks[i].ctx->delay < delay){ //记录最小超时时间
            delay = sched->tasks[i].ctx->delay;
        }
    }
    if(count != sched->count) delay = 0; //运行中，添加了任务
    if(delay<0)delay = 0;
    remove_finish_task();
    return delay;
}

int task_scheduler_add(task_function func, task_context* ctx){    
    int count;
    task_scheduler* sched = &sched_;

    if(!func || !ctx)return -1;
    ctx->delay = 0;
    ctx->expire = 0;
    ctx->pos = NULL;
    ctx->status = 0;
    count = sched->count;    
    if((count + 1) >= MAX_TASKS)return -2;
    sched->tasks[count].ctx = ctx;
    sched->tasks[count].func = func;
    sched->count = count + 1;
    return 0;
}

void task_scheduler_cancel(task_context* ctx){
    
}


