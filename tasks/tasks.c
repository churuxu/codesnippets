#include "tasks.h"


typedef struct task_item{
    task_context* ctx;
    task_function func;
}task_item;

struct task_scheduler{
    int max_count;
    int count;    
    task_item tasks[];
};


static task_clock_function clock_;

//获取当前时钟
clock_t task_scheduler_tick(){
    if(!clock_)return 0;
    return clock_();
}

//设置时钟函数
void task_scheduler_set_clock(task_clock_function clockfn){
    clock_ = clockfn;
}


int task_scheduler_count(task_scheduler* sched){       
    return sched->count;
}

static void remove_finish_task(task_scheduler* sched){    
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

//初始化
task_scheduler* task_scheduler_init(void* mem, int memsize){    
    task_scheduler* sched;
    if(memsize < (sizeof(task_scheduler) + sizeof(task_item)))return NULL;
    sched = (task_scheduler*)mem;
    sched->max_count = (memsize - sizeof(task_scheduler)) / sizeof(task_item);
    sched->count = 0;
    return sched;
}


int task_scheduler_exec(task_scheduler* sched){
    int i;
    int delay;
    int count;    
    
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
    remove_finish_task(sched);
    return delay;
}

int task_scheduler_add(task_scheduler* sched, task_function func, task_context* ctx){    
    int count;    

    if(!func || !ctx)return -2;
    count = sched->count;    
    if((count + 1) >= sched->max_count)return -1;
    ctx->delay = 0;
    ctx->expire = 0;
    ctx->pos = NULL;
    ctx->status = 0;
    sched->tasks[count].ctx = ctx;
    sched->tasks[count].func = func;
    sched->count = count + 1;
    return 0;
}




