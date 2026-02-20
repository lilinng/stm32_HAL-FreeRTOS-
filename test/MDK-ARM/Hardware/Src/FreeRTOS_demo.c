/*
 * @Author: lilinng 2464532129@qq.com
 * @Date: 2026-02-12 18:52:37
 * @LastEditTime: 2026-02-20 20:59:06
 * @FilePath: \test_EIDEd:\MCU\stm32\stm32_practise\VS+HAL\stm32_hd_c\test\MDK-ARM\Hardware\Src\FreeRTOS_demo.c
 * @Description: 用于练习FreeRTOSapi
 */
#include "FreeRTOS.h"
#include "task.h"   //任务相关头文件,和FreeRTOS.h一样为必须包含的头文件
#include "queue.h"  //队列相关头文件
#include "semphr.h" //信号量相关头文件
#include "event_groups.h"   //事件标志组头文件
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

//事件标志组句柄
EventGroupHandle_t  User_handle;

//事件标志组相关定义
#define EVENT_GROUP_BIT_0   (1<<0)
#define EVENT_GROUP_BIT_1   (1<<1)
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
    //事件标志组创建
    /**
     * @description: 创建事件标志组
     * @return: {EventGroupHandle_t} 返回句柄
     */
    User_handle = xEventGroupCreate();
    if(User_handle != NULL)
    {
        printf("event group create successfully\r\n");
    }
    else
    {
        printf("create failed\r\n");
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
    // xTaskCreate((TaskFunction_t)task3,
    //                 (char*)"start_task3",
    //                 (uint32_t)START_TASK3_STACK_SIZE,
    //                 (void*)NULL,
    //                 (UBaseType_t)START_TASK3_PRIORITY,
    //                 (TaskHandle_t*)&task3_handle);
    //退出临界区代码
    taskEXIT_CRITICAL();

    //删除启动任务
    vTaskDelete(NULL);
}
/**
 * @description: 读取按键按下键值，根据不同键值将事件标志组相应事件置1，模拟事件发生
 * @return {*}
 */
void task1(void *pvParameters)
{
    KEY_ENUM key_value = NO_PRESS;
    while (1)
    {
        key_value = Key_Scan();
        if(key_value == KEY1)
        {
            //key1按下，对bit0置1
            /**
             * @description: 第一个参数句柄，第二个参数置?
             * @return {EventBits_t}
             */
            xEventGroupSetBits(User_handle,EVENT_GROUP_BIT_0);
            printf("key1 pressed\r\n");
        }
        else if(key_value == KEY2)
        {
            //key2按下，对bit1置1
            xEventGroupSetBits(User_handle,EVENT_GROUP_BIT_1);
            printf("key2 pressed\r\n");
        }
        vTaskDelay(500);
    }
}
/**
 * @description: 同时等待事件标志组中的多个事件位，当这些事件位都置一的话执行相应的处理
 * @param {void} *pvParameters
 * @return {*}
 */
void task2(void *pvParameters)
{
    EventBits_t res;
    while (1)
    {       
        /**
         * @description: 
         * @param {EventGroupHandle_t} xEventGroup  句柄
         * @param {EventBits_t} uxBitsToWaitFor     需要等待的标志位
         * @param {BaseType_t} xClearOnExit         退出时是否清零所有标志位
         * @param {BaseType_t} xWaitForAllBits      是否等待所有标志位成立
         * @param {TickType_t} xTicksToWait         等待期间进入阻塞状态
         * @return {EventBits_t}
         */
        res = xEventGroupWaitBits(User_handle,
                            EVENT_GROUP_BIT_0|EVENT_GROUP_BIT_1,
                            pdTRUE,     //pdFALSE
                            pdTRUE,     //pdFALSE
                            portMAX_DELAY);
        printf("task2 event group:%#x\r\n",res);
    }
}
void task3(void *pvParameters)
{
    BaseType_t res;
    while(1)
    {

        vTaskDelay(1000);
    }
}