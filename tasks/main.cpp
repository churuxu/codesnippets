#include <stdio.h>
#include "tasks.h"
#include <Windows.h>



static task_context ctx1_;
static task_context ctx2_;


void test(task_context* ctx){
    task_enter(ctx);
    printf("test 1\n");
    task_sleep(1000);
    printf("test 2\n");
    task_sleep(1000);
    printf("test 3\n");
    task_leave();
}




void test_task1(task_context* ctx){
    static int count;
    task_enter(ctx);
    count = 0;
    while(1){
        
        count ++;
        printf("hello %d\n",count);
        if(count >= 5)break;        
        task_sleep(1000);
    }

    task_leave();
}

void test_task2(task_context* ctx){
    static int count;
    task_enter(ctx);
    count = 0;
    while(1){
        
        count ++;
        printf("world %d\n",count);
        if(count >= 3)break;
        task_sleep(1500);
    }
    task_leave();
}

int main(){
    int delay = 0;
    task_scheduler* sched;
    char mem[128];

    task_scheduler_set_clock(clock);

    //自行调度任务函数
    int finish = 0;
    printf("user scheduler ...\n");
    memset(&ctx1_,0,sizeof(task_context));
    memset(&ctx2_,0,sizeof(task_context));
    while(!finish){
        test_task1(&ctx1_);
        test_task2(&ctx2_);
        finish = task_is_finish(&ctx1_) && task_is_finish(&ctx2_);
        Sleep(50);
    }

    //托管调度任务函数
    printf("engine scheduler ...\n");
    sched = task_scheduler_init(mem, 128);
    task_scheduler_add(sched, test_task1, &ctx1_);
    task_scheduler_add(sched, test_task2, &ctx2_);

    delay = 0;
    while(task_scheduler_count(sched)){
        Sleep(delay);
        delay = task_scheduler_exec(sched);
    }
    return 0;
}


