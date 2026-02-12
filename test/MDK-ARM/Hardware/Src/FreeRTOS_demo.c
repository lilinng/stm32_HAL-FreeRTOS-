#include "FreeRTOS.h"
#include "task.h"
#include "stm32f1xx_hal.h"
#include "stdio.h"

//启动任务的相关定义
#define START_TASK_STACK_SIZE 128
#define START_TASK_PRIORITY 1
TaskHandle_t start_task_handle;
//堆栈大小和深度相同
StackType_t start_task_stack[START_TASK_STACK_SIZE];
StaticTask_t start_task_tcb;

//任务1的相关定义
#define START_TASK1_STACK_SIZE 128
#define START_TASK1_PRIORITY 2
TaskHandle_t start_task1_handle;
StackType_t start_task1_stack[START_TASK1_STACK_SIZE];
StaticTask_t start_task1_tcb;//静态任务控制块

//任务2的相关定义
#define START_TASK2_STACK_SIZE 128
#define START_TASK2_PRIORITY 3
TaskHandle_t start_task2_handle;
StackType_t start_task2_stack[START_TASK2_STACK_SIZE];
StaticTask_t start_task2_tcb;

//任务3的相关定义
#define START_TASK3_STACK_SIZE 128
#define START_TASK3_PRIORITY 4
TaskHandle_t start_task3_handle;
StackType_t start_task3_stack[START_TASK3_STACK_SIZE];
StaticTask_t start_task3_tcb;

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
    return 0;
}

void FreeRTOS_Start(void)
{
    //创建一个启动任务，使用静态内存分配方式创建
    start_task_handle = xTaskCreateStatic((TaskFunction_t)start_task,
                    (char*)"start_static_task",
                    (uint32_t)START_TASK_STACK_SIZE,
                    (void*)NULL,
                    (UBaseType_t)START_TASK_PRIORITY,
                    (StackType_t*)start_task_stack,//任务栈的地址
                    (StaticTask_t*)&start_task_tcb);//TCB的地址
                    
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
    //使用静态创建三个任务
    //创建任务1
    start_task1_handle = xTaskCreateStatic((TaskFunction_t)start_task1,
                    (char*)"start_static_task1",
                    (uint32_t)START_TASK1_STACK_SIZE,
                    (void*)NULL,
                    (UBaseType_t)START_TASK1_PRIORITY,
                    (StackType_t*)start_task1_stack,
                    (StaticTask_t*)&start_task1_tcb);

    //创建任务2
    start_task2_handle = xTaskCreateStatic((TaskFunction_t)start_task2,
    (char*)"start_static_task2",
    (uint32_t)START_TASK2_STACK_SIZE,
    (void*)NULL,
    (UBaseType_t)START_TASK2_PRIORITY,
    (StackType_t*)start_task2_stack,
    (StaticTask_t*)&start_task2_tcb);

    //创建任务3
    start_task3_handle = xTaskCreateStatic((TaskFunction_t)start_task3,
    (char*)"start_static_task3",
    (uint32_t)START_TASK3_STACK_SIZE,
    (void*)NULL,
    (UBaseType_t)START_TASK3_PRIORITY,
    (StackType_t*)start_task3_stack,
    (StaticTask_t*)&start_task3_tcb);

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
        printf("This is static task1\r\n");
        HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_6);
        vTaskDelay(500);
    }
}
void start_task2(void *pvParameters)
{
    while (1)
    {
        //任务2的代码
        printf("This is static task2\r\n");
        HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_7);
        vTaskDelay(1000);
    }
}
void start_task3(void *pvParameters)
{
    unsigned char key_value=0;
    while (1)
    {
        //任务3的代码
        printf("This is static task3\r\n");
        key_value=Key_Scan();
        if(key_value==1)
        {
            //判断是否已经删除过，避免重复执行删除
            if(start_task1_handle != NULL)  //被删除的句柄需要被赋值为NULL
            {
                printf("KEY1 Pressed\r\n");
                vTaskDelete(start_task1_handle);
                start_task1_handle = NULL;
            }
        }
        else if(key_value==2)
        {
            if(start_task2_handle != NULL)  //被删除的句柄会被赋值为NULL
            {
                printf("KEY2 Pressed\r\n");
                vTaskDelete(start_task2_handle);
                start_task2_handle = NULL;
            }
        }
        //不加延时则低优先级的任务一直不能实现调度,无法实现LED闪烁
        vTaskDelay(500);
    }
}