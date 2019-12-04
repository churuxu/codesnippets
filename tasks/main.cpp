#include <stdio.h>
#include "tasks.h"
#include <Windows.h>

void test(task_context* ctx){
    task_enter(ctx);
    printf("hello 1\n");
    task_yield();
    printf("hello 2\n");
    task_yield();
    printf("hello 3\n");
    task_leave();
}

static task_context ctx1_;
static task_context ctx2_;


void test_task1(task_context* ctx){
    static int count;
    task_enter(ctx);

    while(1){
        
        count ++;
        printf("hello %d\n",count);
        if(count >= 8)break;        
        task_sleep(1000);
    }

    task_leave();
}

void test_task2(task_context* ctx){
    static int count;
    task_enter(ctx);

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
    task_scheduler_add(test_task1, &ctx1_);
    task_scheduler_add(test_task2, &ctx2_);

    while(task_scheduler_count()){
        Sleep(delay);
        delay = task_scheduler_exec();        
    }
    return 0;
}


