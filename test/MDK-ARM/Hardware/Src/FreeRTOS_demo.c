#include "FreeRTOS.h"
#include "task.h"
#include "stm32f1xx_hal.h"
#include "stdio.h"
#include "gpio.h"

//启动任务的相关定义
#define START_TASK_STACK_SIZE 128
#define START_TASK_PRIORITY 1
TaskHandle_t start_task_handle;

//任务1的相关定义
#define START_TASK1_STACK_SIZE 128
#define START_TASK1_PRIORITY 2
TaskHandle_t start_task1_handle;

void start_task(void *pvParameters);

void task1(void *pvParameters);


void FreeRTOS_Start(void)
{
    //创建一个启动任务
    xTaskCreate((TaskFunction_t)start_task,
                    (char*)"start_static_task",
                    (uint32_t)START_TASK_STACK_SIZE,
                    (void*)NULL,
                    (UBaseType_t)START_TASK_PRIORITY,
                    (TaskHandle_t*)&start_task_handle);
                    
    // 启动调度器(静态开启对应宏后会自动创建空闲任务和软件定时器任务)
    /* 两个分配函数(未开启configKERNEL_PROVIDED_STATIC_MEMORY时需要用户提供实现)
    vApplicationGetIdleTaskMemory & vApplicationGetTimerTaskMemory */
    vTaskStartScheduler();
}
void start_task(void *pvParameters)
{
    //临界区:保护其中的代码不会被打断
    //进入临界区代码,在后面还需要有退出
    taskENTER_CRITICAL();
    //创建任务1
    xTaskCreate((TaskFunction_t)task1,
                    (char*)"start_task1",
                    (uint32_t)START_TASK1_STACK_SIZE,
                    (void*)NULL,
                    (UBaseType_t)START_TASK1_PRIORITY,
                    (TaskHandle_t*)&start_task1_handle);

    //删除启动任务
    vTaskDelete(NULL);

    //退出临界区代码
    taskEXIT_CRITICAL();
}
void task1(void *pvParameters)
{
    KEY_ENUM key_value;
    while (1)
    {
        key_value = Key_Scan();
        
        if(key_value == KEY1)
        {
            /*关中断*/
            portDISABLE_INTERRUPTS();
            printf(">>>>Close IT<<<<\r\n");
        }
        else if(key_value == KEY2)
        {
            /*开中断*/
            portENABLE_INTERRUPTS();
            printf(">>>>Open IT<<<<\r\n");
        }
        //此时不能延时，延时会失能和使能调度器和中断，导致现象不明确
        //原因：延时是在使当前任务进入阻塞队列，触发下一个任务，此为链表操作不能被打断
        // vTaskDelay(100);
    }
}
