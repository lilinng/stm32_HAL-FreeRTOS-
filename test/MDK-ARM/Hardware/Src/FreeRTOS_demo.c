/*
 * @Author: lilinng 2464532129@qq.com
 * @Date: 2026-02-12 18:52:37
 * @LastEditTime: 2026-02-17 21:11:18
 * @FilePath: \test (工作区)d:\MCU\stm32\stm32_practise\VS+HAL\stm32_hd_c\test\MDK-ARM\Hardware\Src\FreeRTOS_demo.c
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

//信号量句柄,也是队列句柄
QueueHandle_t   User_Counting_handle;

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

/**
 * @description: 启动FreeRTOS
 * @return {*}
 */
void FreeRTOS_Start(void)
{
    UBaseType_t count = 0;
    //在创建任务之前创建信号量
    //  xSemaphoreCreateBinary();   此函数创建时没有释放Binary
    /**
     * @description: 第一个参数最大计数值，第二个初始计数值
     * @return 判断句柄是否为NULL,即可判断是否创建成功
     */
    User_Counting_handle = xSemaphoreCreateCounting(100,0);   //此函数创建后主动释放一次Binarys
    //判断是否成功创建        
    if(User_Counting_handle != NULL)
    {
        //创建成功获取计数值并且输出
        count = uxSemaphoreGetCount(User_Counting_handle);
        printf("count:%d\r\n",count);
    }
    else
    {
        printf("Create counting failed\r\n");
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

    //退出临界区代码
    taskEXIT_CRITICAL();

    //删除启动任务
    vTaskDelete(NULL);
}
/**
 * @description: 用于按键扫描，当检测到KEY1按下时,释放计数信号量
 * @param {void} *pvParameters
 * @return {*}
 */
void task1(void *pvParameters)
{
    BaseType_t res;
    KEY_ENUM key_value = NO_PRESS;
    while (1)
    {
        key_value = Key_Scan();
        if(key_value == KEY1)
        {
            //释放计数信号量
            res = xSemaphoreGive(User_Counting_handle);
            if(res == pdPASS)
            {
                printf("Give successfully\r\n");
            }
            else 
            {
                printf("Give failed\r\n");
            }
        }
        vTaskDelay(500);
    }
}
/**
 * @description: 获取计数信号量,当成功获取信号量后输出信息
 * @param {void} *pvParameters
 * @return {*}
 */
void task2(void *pvParameters)
{
    BaseType_t current_count = 0;
    BaseType_t res;
    while (1)
    {
        res = xSemaphoreTake(User_Counting_handle,portMAX_DELAY);
        if(res == pdPASS)
        {
            printf("take successfully\r\n");
        }
        else 
        {
            printf("take failed\r\n");
        }
        current_count = uxSemaphoreGetCount(User_Counting_handle);
        printf("current_count:%d\r\n",current_count);
        vTaskDelay(1000);
    }
}