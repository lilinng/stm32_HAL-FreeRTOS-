/*
 * @Author: lilinng 2464532129@qq.com
 * @Date: 2026-02-12 18:52:37
 * @LastEditTime: 2026-02-14 00:46:42
 * @FilePath: \test_EIDEd:\MCU\stm32\stm32_practise\VS+HAL\stm32_hd_c\test\MDK-ARM\Hardware\Src\FreeRTOS_demo.c
 * @Description: 
 */
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
TaskHandle_t task1_handle;

//任务2的相关定义
#define START_TASK2_STACK_SIZE 128
#define START_TASK2_PRIORITY 2
TaskHandle_t task2_handle;

void start_task(void *pvParameters);

void task1(void *pvParameters);

void task2(void *pvParameters);

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
                    (TaskHandle_t*)&task1_handle);

    //创建任务2
    xTaskCreate((TaskFunction_t)task2,
                    (char*)"start_task2",
                    (uint32_t)START_TASK2_STACK_SIZE*2,
                    (void*)NULL,
                    (UBaseType_t)START_TASK2_PRIORITY,
                    (TaskHandle_t*)&task2_handle);

    //退出临界区代码
    taskEXIT_CRITICAL();

    //删除启动任务
    vTaskDelete(NULL);
}
void task1(void *pvParameters)
{
    while (1)
    {
        printf("Task1 running...\r\n");
        HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_6);
        vTaskDelay(500);
    }
}
void task2(void *pvParameters)
{
    char task_info[100];
    while (1)
    {
        printf("Task2 running...\r\n");
        vTaskGetRunTimeStats(task_info);
        printf("%s\r\n",task_info);
        vTaskDelay(1000);
    }
}