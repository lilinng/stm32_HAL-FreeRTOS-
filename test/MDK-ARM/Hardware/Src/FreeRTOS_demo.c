#include "FreeRTOS.h"
#include "task.h"
#include "stm32f1xx_hal.h"
#include "stdio.h"

//启动任务的相关定义
#define START_TASK_STACK_SIZE 128
#define START_TASK_PRIORITY 1
TaskHandle_t start_task_handle;

//任务1的相关定义
#define START_TASK1_STACK_SIZE 128
#define START_TASK1_PRIORITY 2
TaskHandle_t start_task1_handle;

//任务2的相关定义
#define START_TASK2_STACK_SIZE 128
#define START_TASK2_PRIORITY 3
TaskHandle_t start_task2_handle;

//任务3的相关定义
#define START_TASK3_STACK_SIZE 128
#define START_TASK3_PRIORITY 4
TaskHandle_t start_task3_handle;

void start_task(void *pvParameters);

void start_task1(void *pvParameters);

void start_task2(void *pvParameters);

void start_task3(void *pvParameters);

unsigned char Key_Scan(void)
{
    if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_13) == GPIO_PIN_RESET)
    {
        HAL_Delay(10);
        if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_13) == GPIO_PIN_RESET)
        {
            return 1;
        }
    }
    else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_14) == GPIO_PIN_RESET)
    {
        HAL_Delay(10);
        if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_14) == GPIO_PIN_RESET)
        {
            return 2;
        }
    }
    else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_15) == GPIO_PIN_RESET)
    {
        HAL_Delay(10);
        if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_15) == GPIO_PIN_RESET)
        {
            return 3;
        }
    }
    else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_10) == GPIO_PIN_RESET)
    {
        HAL_Delay(10);
        if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_10) == GPIO_PIN_RESET)
        {
            return 4;
        }
    }
    return 0;
}

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
    xTaskCreate((TaskFunction_t)start_task1,
                    (char*)"start_static_task1",
                    (uint32_t)START_TASK1_STACK_SIZE,
                    (void*)NULL,
                    (UBaseType_t)START_TASK1_PRIORITY,
                    (TaskHandle_t*)&start_task1_handle);

    //创建任务2
    xTaskCreate((TaskFunction_t)start_task2,
                    (char*)"start_static_task2",
                    (uint32_t)START_TASK2_STACK_SIZE,
                    (void*)NULL,
                    (UBaseType_t)START_TASK2_PRIORITY,
                    (TaskHandle_t*)&start_task2_handle);

    //创建任务3
    xTaskCreate((TaskFunction_t)start_task3,
                    (char*)"start_static_task3",
                    (uint32_t)START_TASK3_STACK_SIZE,
                    (void*)NULL,
                    (UBaseType_t)START_TASK3_PRIORITY,
                    (TaskHandle_t*)&start_task3_handle);

    //删除启动任务
    vTaskDelete(NULL);

    //退出临界区代码
    taskEXIT_CRITICAL();
}
void start_task1(void *pvParameters)
{
    while (1)
    {
        //任务1的代码
        printf("This is task1\r\n");
        HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_6);
        vTaskDelay(500);
    }
}
void start_task2(void *pvParameters)
{
    while (1)
    {
        //任务2的代码
        printf("This is task2\r\n");
        HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_7);
        vTaskDelay(1000);
    }
}
void start_task3(void *pvParameters)
{
    char task_info[200]={0};
    unsigned char key_value=0;
    while (1)
    {
        //任务3的代码
        printf("This is task3\r\n");
        key_value=Key_Scan();
        switch(key_value)
        {
            case 1:
            {
                //挂起任务1
                vTaskSuspend(start_task1_handle);
                printf(">>>>>suspend task1<<<<<\r\n");
                break;
            }
            case 2:
            {
                //恢复任务1
                vTaskResume(start_task1_handle);
                printf(">>>>>resume task1<<<<<\r\n");
                break;
            }
            case 3:
            {
                //挂起和使能调度器
                vTaskSuspendAll();
                printf(">>>>>suspend all tasks<<<<<\r\n");
                break;
            }
            case 4:
            {
                //恢复和使能调度器
                xTaskResumeAll();
                printf(">>>>>resume all tasks<<<<<\r\n");
                break;
            }
            default:
                break;
        }
        //打印任务状态
        vTaskList(task_info);
        printf("%s\r\n",task_info);
        //不加延时则低优先级的任务一直不能实现调度,无法实现LED闪烁
        vTaskDelay(500);
    }
}