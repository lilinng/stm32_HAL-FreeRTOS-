/*
 * @Author: lilinng 2464532129@qq.com
 * @Date: 2026-02-12 18:52:37
 * @LastEditTime: 2026-02-19 15:01:36
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
QueueHandle_t   queue1_handle = NULL;
QueueHandle_t   binary_handle = NULL;
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
    //创建一个队列一个信号量
    queue1_handle = xQueueCreate(2,sizeof(KEY_ENUM));
    //默认不释放信号量
    binary_handle = xSemaphoreCreateBinary();
    if(queue1_handle != NULL && binary_handle != NULL)
    {
        printf("queue1 & binary create successfully\r\n");
    }
    else
    {
        printf("create failed\r\n");
    }
    //创建队列集
    /**
     * @description: 参数为传入的队列个数
     * @return {QueueSetHandle_t}
     */
    User_handle = xQueueCreateSet(2);
    if(User_handle != NULL)
    {
        printf("queue set create successfully\r\n");
    }
    //将队列添加到队列集
    /**
     * @description: 第一个参数为需要添加的队列，第二个为目标队列集
     * @return {BaseType_t}
     */
    BaseType_t res1 = 0;
    BaseType_t res2 = 0;
    //添加之前队列必须为空
    res1 = xQueueAddToSet(queue1_handle,User_handle);  
    res2 = xQueueAddToSet(binary_handle,User_handle);
    if(res1 == pdPASS && res2 == pdPASS)
    {
        printf("add successfully\r\n");
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
 * @description: 用于按键扫描，当key1按下，往队列写入数据；当key2按下，释放二值信号量
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
            res = xQueueSend(queue1_handle,&key_value,portMAX_DELAY);
            if(res == pdPASS)
            {
                printf("send successfully\r\n");
            }
        }
        else if(key_value == KEY2)
        {
            res = xSemaphoreGive(binary_handle);
            if(res == pdPASS)
            {
                printf("give successfully\r\n");
            }
            else
            {
                printf("give failed\r\n");
            }
        }
        vTaskDelay(1000);
    }
}
/**
 * @description: 读取队列集中的消息并输出
 * @param {void} *pvParameters
 * @return {*}
 */
void task2(void *pvParameters)
{
    QueueSetMemberHandle_t member_handle;
    uint8_t receive;
    BaseType_t res;
    while (1)
    {
       /**
        * @description: 查看哪个队列有数据来了;如果是多个队列数据同时就绪，多次调用得到每个就绪队列的句柄
        * @return {QueueSetMemberHandle_t}
        */
       member_handle = xQueueSelectFromSet(User_handle,portMAX_DELAY);
       //根据队列去获取数据
       if(member_handle == queue1_handle)
       {
            //读取队列数据
            res = xQueueReceive(queue1_handle,&receive,portMAX_DELAY);
            if(res == pdPASS)
            {
                printf("take queue1:%d\r\n",receive);
            }
            else
            {
                printf("take failed\r\n");
            }
       }
       else if(member_handle == binary_handle)
       {
            //获取信号量
            res = xSemaphoreTake(binary_handle,portMAX_DELAY);
            if(res == pdPASS)
            {
                printf("get binary successfully\r\n");
            }
            else
            {
                printf("get faild\r\n");
            }
       }
       else
       {
            printf("queue set return is NULL\r\n");
       }
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