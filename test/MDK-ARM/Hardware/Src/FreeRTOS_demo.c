/*
 * @Author: lilinng 2464532129@qq.com
 * @Date: 2026-02-12 18:52:37
 * @LastEditTime: 2026-02-23 13:48:50
 * @FilePath: \test_EIDEd:\MCU\stm32\stm32_practise\VS+HAL\stm32_hd_c\test\MDK-ARM\Hardware\Src\FreeRTOS_demo.c
 * @Description: 用于练习FreeRTOSapi
 */
#include "FreeRTOS.h"
#include "task.h"   //任务相关头文件,和FreeRTOS.h一样为必须包含的头文件
#include "queue.h"  //队列相关头文件
#include "semphr.h" //信号量相关头文件
#include "event_groups.h"   //事件标志组头文件
#include "timers.h"         //软件定时器头文件
#include "stm32f1xx_hal.h"
#include "stdio.h"
#include "gpio.h"

//软件定时器句柄定义
TimerHandle_t My_software_timer1Handle = NULL;
TimerHandle_t My_software_timer2Handle = NULL;

//启动任务的相关定义
#define START_TASK_STACK_SIZE 128
#define START_TASK_PRIORITY 1
TaskHandle_t start_task_handle;

//任务1的相关定义
#define START_TASK1_STACK_SIZE 128
#define START_TASK1_PRIORITY 2
TaskHandle_t task1_handle;

// //任务2的相关定义
// #define START_TASK2_STACK_SIZE 128
// #define START_TASK2_PRIORITY 3
// TaskHandle_t task2_handle;

// //任务3的相关定义
// #define START_TASK3_STACK_SIZE 128
// #define START_TASK3_PRIORITY 4
// TaskHandle_t task3_handle;

/**
 * @description: 软件定时器回调函数定义
 * @param {TimerHandle_t} xTimer
 * @return {void}
 */
static void My_software_timer1callback(TimerHandle_t xTimer);
static void My_software_timer2callback(TimerHandle_t xTimer);

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
// void task2(void *pvParameters);
// void task3(void *pvParameters);

/**
 * @description: 启动FreeRTOS
 * @return {*}
 */
void FreeRTOS_Start(void)
{
    /********************创建启动任务和必要量**************************/
        /**
     * @description: 创建一次性软件定时器
     * @param {char} *pcTimerName   名字
     * @param {TickType_t} xTimerPeriodInTicks  周期
     * @param {BaseType_t} xAutoReload  是否自动重载
     * @param {void} *pvTimerID         定时器id
     * @param {TimerCallbackFunction_t} pxCallbackFunction  回调函数
     * @return {TimerHandle_t}
     */
    My_software_timer1Handle = xTimerCreate(
                "My_software_timer1",
                (TickType_t)500,    //500个FreeRTOS的时钟节拍,由configTICK_RATE_HZ定义
                pdFALSE,            //一次性
                (void*)2,
                (TimerCallbackFunction_t)My_software_timer1callback
    );
    if(My_software_timer1Handle != NULL)
    {
        printf("software_timer1 create successfully\r\n");
    }
    My_software_timer2Handle = xTimerCreate(
                "My_software_timer2",
                (TickType_t)1000,    //1000个FreeRTOS的时钟节拍,由configTICK_RATE_HZ定义
                pdTRUE,            //周期
                (void*)1,
                (TimerCallbackFunction_t)My_software_timer2callback
    );
    if(My_software_timer2Handle != NULL)
    {
        printf("software_timer2 create successfully\r\n");
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
    // xTaskCreate((TaskFunction_t)task2,
    //                 (char*)"start_task2",
    //                 (uint32_t)START_TASK2_STACK_SIZE,
    //                 (void*)NULL,
    //                 (UBaseType_t)START_TASK2_PRIORITY,
    //                 (TaskHandle_t*)&task2_handle);
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
 * @description: 软件定时器回调函数实现
 * @param {TimerHandle_t} xTimer
 * @return {void}
 */
static void My_software_timer1callback(TimerHandle_t xTimer)
{
    static uint16_t cbcnt = 0;
    printf("timer1 cb count:%d\r\n",++cbcnt);
}
static void My_software_timer2callback(TimerHandle_t xTimer)
{
    static uint16_t cbcnt = 0;
    printf("timer2 cb count:%d\r\n",++cbcnt);
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
        if(key_value == KEY1)
        {
            //开启定时器
            /**
             * @description: 第一参数句柄，第二参数等待时间.详情见定义
             * @return {*}
             */
            res = xTimerStart(My_software_timer1Handle,portMAX_DELAY);
            if(res != pdFAIL)
            {
                printf("timer1 start\r\n");
            }
            res = xTimerStart(My_software_timer2Handle,portMAX_DELAY);
            if(res != pdFAIL)
            {
                printf("timer2 start\r\n");
            }
        }
        else if(key_value == KEY2)
        {
            //关闭定时器,与开启相同的形参列表
            res = xTimerStop(My_software_timer1Handle,portMAX_DELAY);
            if(res != pdFAIL)
            {
                printf("timer1 stop\r\n");
            }
            res = xTimerStop(My_software_timer2Handle,portMAX_DELAY);
            if(res != pdFAIL)
            {
                printf("timer2 stop\r\n");
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
// void task2(void *pvParameters)
// {
//     BaseType_t res;
//     uint32_t notify_value;
//     uint32_t out = 0;
//     while (1)
//     {       
//             /**
//              * @description: 
//              * @param {UBaseType_t} uxIndexToWaitOn //此参数无需填写
//              * @param {uint32_t} ulBitsToClearOnEntry   //进入前是否清零
//              * @param {uint32_t} ulBitsToClearOnExit    //退出前是否清零
//              * @param {uint32_t *} pulNotificationValue //通知值存放的地址
//              * @param {TickType_t} xTicksToWait         //阻塞时间
//              * @return {BaseType_t}
//              */
//         res = xTaskNotifyWait(
//                             0x00000006, //接收通知前是否清零,哪一位清零就置1
//                             0xFFFFFFFF, //接收通知后是否清零，置1清零
//                             &notify_value,
//                             portMAX_DELAY);
//         if(res == pdTRUE)
//         {
//             printf("task2 take value:%d\r\n",notify_value);
//         }

//         if(notify_value & EVENT_BIT0)   //判断接收
//         {
//             out |= notify_value;
//             printf("event1 take\r\n");
//         }
//         if(notify_value & EVENT_BIT1)
//         {
//             out |= notify_value;
//             printf("evevnt2 take\r\n");
//         }
//         printf("out:%#x",out);
//         vTaskDelay(1000);
//     }
// }

// void task3(void *pvParameters)
// {
//     while(1)
//     {

//         vTaskDelay(1000);
//     }
// }