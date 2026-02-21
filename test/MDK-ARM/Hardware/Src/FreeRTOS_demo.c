/*
 * @Author: lilinng 2464532129@qq.com
 * @Date: 2026-02-12 18:52:37
 * @LastEditTime: 2026-02-21 14:39:27
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
 * @description: 用于按键扫描，当检测到按键KEY1被按下时，将发送任务通知
 * @return {*}
 */
void task1(void *pvParameters)
{
    BaseType_t res;
    KEY_ENUM key_value = NO_PRESS;
    while (1)
    {
        key_value = Key_Scan();
        if(key_value == KEY1 || key_value == KEY2)
        {
            //发送任务通知
            /**
             * @description: 
             * @param {TaskHandle_t}*** xTaskToNotify 接收任务句柄
             * @param {UBaseType_t} uxIndexToNotify tskDEFAULT_INDEX_TO_NOTIFY  //此参数无需填写
             * @param {uint32_t}*** ulValue    通知值
             * @param {eNotifyAction}*** eAction   发送行为
             * @param {uint32_t *} pulPreviousNotificationValue NULL    // 此参数无需填写
             * @return {*}
             */
            xTaskNotify(task2_handle,key_value,eSetValueWithoutOverwrite);
            if(res == pdPASS)
            {
                printf("task1 send successfully:[%d]\r\n",key_value);
            }
        }
        vTaskDelay(500);
    }
}
/**
 * @description: 用于接收任务通知，并打印相关提示信息
 * @param {void} *pvParameters
 * @return {*}
 */
void task2(void *pvParameters)
{
    BaseType_t res;
    uint32_t notify_value;
    while (1)
    {       
            /**
             * @description: 
             * @param {UBaseType_t} uxIndexToWaitOn //此参数无需填写
             * @param {uint32_t} ulBitsToClearOnEntry   //进入前是否清零
             * @param {uint32_t} ulBitsToClearOnExit    //退出前是否清零
             * @param {uint32_t *} pulNotificationValue //通知值存放的地址
             * @param {TickType_t} xTicksToWait         //阻塞时间
             * @return {BaseType_t}
             */
        res = xTaskNotifyWait(
                            0x00000000, //接收通知前是否清零,哪一位清零就置1
                            0xFFFFFFFF, //接收通知后是否清零，置1清零
                            &notify_value,
                            portMAX_DELAY);
        if(res == pdTRUE)
        {
            printf("task2 take value:%d\r\n",notify_value);
        }

    }
}
void task3(void *pvParameters)
{
    while(1)
    {

        vTaskDelay(1000);
    }
}