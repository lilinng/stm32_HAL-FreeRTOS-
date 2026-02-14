/*
 * @Author: lilinng 2464532129@qq.com
 * @Date: 2026-02-12 18:52:37
 * @LastEditTime: 2026-02-14 21:48:53
 * @FilePath: \test (工作区)d:\MCU\stm32\stm32_practise\VS+HAL\stm32_hd_c\test\MDK-ARM\Hardware\Src\FreeRTOS_demo.c
 * @Description: 
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
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

//队列句柄
QueueHandle_t little_queue_handle;
QueueHandle_t big_queue_handle;
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
    //在创建任务之前创建队列
    //1、创建小队列
    /**
     * @description: 第一个参数是队列长度，第二个参数是队列元素的大小
     * @return 类型为QueueHandle_t的队列句柄，创建失败返回NULL
     */
    little_queue_handle = xQueueCreate(2,sizeof(KEY_ENUM));
    if(little_queue_handle != NULL)
    {
        printf("little queue create successfully\r\n");
    }
    else
    {
        printf("little queue create failed\r\n");
    }
    //创建大队列
    //数据大存放指针即可
    big_queue_handle = xQueueCreate(1,sizeof(char*));
    if(big_queue_handle != NULL)
    {
        printf("big queue create successfully\r\n");
    }
    else
    {
        printf("big queue create failed\r\n");
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
 * @description: 按下key1或者Key2,将键值拷贝到小队列,按下key3,将大数据拷贝到大队列里.
 * @param {void} *pvParameters
 * @return {*}
 */
void task1(void *pvParameters)
{
    KEY_ENUM key_value = NO_PRESS;
    while (1)
    {
        key_value = Key_Scan();
        if(key_value == KEY1 || key_value == KEY2)
        {
            //将键值拷贝到小队列
            /**
             * @description: 发送数据到队列
             * @param {QueueHandle_t} xQueue:队列句柄
             * @param {void} *pvItemToQueue:要发送到队列的数据的指针,会将数据复制到队列存储区
             * @param {TickType_t} xTicksToWait:如果队列已满,等待的最大时间,单位为tick,如果为0则不等待直接返回,如果为portMAX_DELAY则一直等待直到发送成功
             * @return {BaseType_t} pdPASS:成功发送到队列,pdFAIL:队列已满无法发送
             */
            if(xQueueSend(little_queue_handle,&key_value,portMAX_DELAY) == pdPASS)
            {
                printf("send key value %d to little queue successfully\r\n",key_value);
            }
            else
            {
                printf("send key value %d to little queue failed\r\n",key_value);
            }

        }
        else if(key_value == KEY3)
        {
            //将大数据拷贝到大队列里
            char *big_data = "This is a big data";
            if(xQueueSend(big_queue_handle,&big_data,portMAX_DELAY) == pdPASS)
            {
                printf("send big data to big queue successfully\r\n");
            }
            else
            {
                printf("send big data to big queue failed\r\n");
            }
        }
        vTaskDelay(500);
    }
}
/**
 * @description: 读取小队列的消息并且输出
 * @param {void} *pvParameters
 * @return {*}
 */
void task2(void *pvParameters)
{
    KEY_ENUM received_buffer;
    BaseType_t res;
    while (1)
    {
        printf("task2 running...\r\n");
        /**
         * @description: 接收队列消息
         * @param {QueueHandle_t} xQueue:句柄
         * @param {void} *pvBuffer:接收缓冲区地址
         * @param {TickType_t} xTicksToWait:等待时间
         * @return {BaseType_t} pdPASS:成功接收,pdFAIL:队列为空无法接收
         */
        res = xQueueReceive(little_queue_handle,&received_buffer,portMAX_DELAY);
        if(res == pdPASS)
        {
            //阻塞等待
            printf("received key value %d from little queue successfully\r\n",received_buffer);
        }
        else
        {
            printf("failed to receive key value from little queue\r\n");
        }
    }
}

/**
 * @description: 接收大队列的消息并且输出
 * @param {void} *pvParameters
 * @return {*}
 */
void task3(void *pvParameters)
{
    char *received_buffer;
    BaseType_t res;
    while (1)
    {
        printf("task3 running...\r\n");
        //大队列存储的是字符串指针的地址(char**),所以接收时需要指针
        res = xQueueReceive(big_queue_handle,&received_buffer,portMAX_DELAY);
        if(res == pdPASS)
        {
            printf("received big data from big queue successfully: %s\r\n",received_buffer);
        }
        else
        {
            printf("failed to receive big data from big queue\r\n");
        }
    }
}