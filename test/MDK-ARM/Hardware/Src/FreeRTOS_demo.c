/*
 * @Author: lilinng 2464532129@qq.com
 * @Date: 2026-02-12 18:52:37
 * @LastEditTime: 2026-02-23 16:38:26
 * @FilePath: \test_EIDEd:\MCU\stm32\stm32_practise\VS+HAL\stm32_hd_c\test\MDK-ARM\Hardware\Src\FreeRTOS_demo.c
 * @Description: 用于练习FreeRTOSapi
 */
#include "FreeRTOS.h"
#include "task.h"   //任务相关头文件,和FreeRTOS.h一样为必须包含的头文件
#include "queue.h"  //队列相关头文件
#include "semphr.h" //信号量相关头文件
#include "event_groups.h"   //事件标志组头文件
#include "timers.h"         //软件定时器头文件
#include "FreeRTOSConfig.h"
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

/**
 * @description: 启动FreeRTOS
 * @return {*}
 */
void FreeRTOS_Start(void)
{
    /********************创建启动任务和必要量**************************/
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
    //退出临界区代码
    taskEXIT_CRITICAL();

    //删除启动任务
    vTaskDelete(NULL);
}

/**
 * @description: 当KEY1按下申请内存，KEY2按下则释放内存并输出
 * @return {*}
 */
void task1(void *pvParameters)
{
    char* receive = NULL;
    KEY_ENUM key_value = NO_PRESS;
    while (1)
    {
        key_value = Key_Scan();
        if(key_value == KEY1)
        {
            /**
             * @description: 申请内存,单位Byte;实际大小为heap结构体+字节对齐损耗
             * @param {size_t} xWantedSize
             * @return {void*}
             */
            receive = (char*)pvPortMalloc(20);
            printf("Apply 20;Remaining %d\r\n",xPortGetFreeHeapSize());
        }
        if(key_value == KEY2)
        {
            vPortFree((void*)receive);
            printf("Release 20;Remain %d\r\n",xPortGetFreeHeapSize());
        }
        vTaskDelay(500);
    }
}
