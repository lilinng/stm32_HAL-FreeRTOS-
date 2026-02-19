/*
 * @Author: lilinng 2464532129@qq.com
 * @Date: 2026-02-12 18:52:37
 * @LastEditTime: 2026-02-19 13:06:01
 * @FilePath: \test_EIDEd:\MCU\stm32\stm32_practise\VS+HAL\stm32_hd_c\test\MDK-ARM\Hardware\Src\FreeRTOS_demo.c
 * @Description: 用于练习FreeRTOSapi
 */
#include "FreeRTOS.h"
#include "task.h"   //任务相关头文件,和FreeRTOS.h一样为必须包含的头文件
#include "queue.h"  //队列相关头文件
#include "semphr.h" //信号量相关头文件
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
#define START_TASK2_PRIORITY 3
TaskHandle_t task2_handle;

//任务3的相关定义
#define START_TASK3_STACK_SIZE 128
#define START_TASK3_PRIORITY 4
TaskHandle_t task3_handle;

//信号量句柄,也是队列句柄
QueueHandle_t   User_handle;

/**
 * @description: 启动任务，用于创建其他进程并在创建完成后删除自身
 * @param {void} *pvParameters
 * @return {*}
 */
void start_task(void *pvParameters);

/**
 * @description: 以下函数都对应任务实现
 * @param {void} *pvParameters
 * @return {*}
 */
void task1(void *pvParameters);

void task2(void *pvParameters);

void task3(void *pvParameters);

/**
 * @description: 启动FreeRTOS
 * @return {*}
 */
void FreeRTOS_Start(void)
{
    //创建互斥信号量,创建后默认释放一次信号量
    User_handle = xSemaphoreCreateMutex();
    if(User_handle == NULL)
    {
        printf("mutex create failed\r\n");
    }
    else
    {
        printf("mutex create successfully\r\n");
    }
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
                    (uint32_t)START_TASK2_STACK_SIZE,
                    (void*)NULL,
                    (UBaseType_t)START_TASK2_PRIORITY,
                    (TaskHandle_t*)&task2_handle);
    xTaskCreate((TaskFunction_t)task3,
                    (char*)"start_task3",
                    (uint32_t)START_TASK3_STACK_SIZE,
                    (void*)NULL,
                    (UBaseType_t)START_TASK3_PRIORITY,
                    (TaskHandle_t*)&task3_handle);
    //退出临界区代码
    taskEXIT_CRITICAL();

    //删除启动任务
    vTaskDelete(NULL);
}
/**
 * @description: 低优先级任务，对比高优先级先占用信号量的时间久一点
 * @param {void} *pvParameters
 * @return {*}
 */
void task1(void *pvParameters)
{
    BaseType_t res;
    while (1)
    {
        //获取并输出信号量
        res = xSemaphoreTake(User_handle,portMAX_DELAY);
        if(res == pdPASS)
        {
            printf("low priority task semaphor successfully\r\n");
        }
        else
        {
            printf("low priority task semaphor failed\r\n");

        }
        //执行其他逻辑(延时)
        printf("task 1 running\r\n");
        HAL_Delay(3000);

        res = 0;

        //释放信号量
        res = xSemaphoreGive(User_handle);
        if(res == pdPASS)
        {
            printf("task 1 Give successfully\r\n");
        }
        else 
        {
            printf("task 1 Give failed\r\n");
        }
        vTaskDelay(1000);
    }
}
/**
 * @description: 中等优先级任务，简单的应用任务
 * @param {void} *pvParameters
 * @return {*}
 */
void task2(void *pvParameters)
{
    while (1)
    {
        printf("task 2 running\r\n");
        HAL_Delay(1500);
        printf("task 2 finish HAL_Delay\r\n");
        vTaskDelay(1000);
    }
}
void task3(void *pvParameters)
{
    BaseType_t res;
    while(1)
    {
        //获取并输出信号量
        res = xSemaphoreTake(User_handle,portMAX_DELAY);
        if(res == pdPASS)
        {
            printf("High priority task semaphor successfully\r\n");
        }
        else
        {
            printf("High priority task semaphor failed\r\n");

        }
        //执行其他逻辑(延时)
        printf("task 3 running\r\n");
        HAL_Delay(3000);

        res = 0;
        
        //释放信号量
        res = xSemaphoreGive(User_handle);
        if(res == pdPASS)
        {
            printf("task 3 Give successfully\r\n");
        }
        else 
        {
            printf("task 3 Give failed\r\n");
        }
        vTaskDelay(1000);
    }
}