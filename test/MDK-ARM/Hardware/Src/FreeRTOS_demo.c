#include "FreeRTOS.h"
#include "task.h"
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
#define START_TASK2_PRIORITY 2
TaskHandle_t task2_handle;

void start_task(void *pvParameters);

void task1(void *pvParameters);

void task2(void *pvParameters);

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
                    (uint32_t)START_TASK2_STACK_SIZE*2,
                    (void*)NULL,
                    (UBaseType_t)START_TASK2_PRIORITY,
                    (TaskHandle_t*)&task2_handle);

    //退出临界区代码
    taskEXIT_CRITICAL();

    //删除启动任务
    vTaskDelete(NULL);
}
void task1(void *pvParameters)
{
    while (1)
    {
        printf("Task1 running...\r\n");
        HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_6);
        vTaskDelay(500);
    }
}
void task2(void *pvParameters)
{
    while (1)
    {
        printf("Task2 running...\r\n\r\n");

        //通过句柄获取任务优先级
        UBaseType_t priority = uxTaskPriorityGet(task1_handle);
        printf("Task1 Priority: %d\r\n\r\n", (uint32_t)priority);

        //修改任务优先级并输出
        vTaskPrioritySet(task1_handle, priority + 1);
        priority = uxTaskPriorityGet(task1_handle);
        printf("Task1 New Priority: %d\r\n\r\n", (uint32_t)priority);
        vTaskPrioritySet(task1_handle, priority - 1);

        //获取当前系统中任务的数量并输出
        UBaseType_t task_count = uxTaskGetNumberOfTasks();
        printf("Total Tasks: %d\r\n\r\n", (uint32_t)task_count);

        //获取所有任务状态信息
        TaskStatus_t arr[3]={0};
        uxTaskGetSystemState(arr, 3, NULL);
        printf("TaskName\t\tTaskNumber\t\tCurrentState\t\tCurrentPriority\t\tBasePriority\r\n");
        for(uint8_t i=0;i<3;i++)
        {
            printf("%s\t\t%d\t\t%d\t\t%d\t\t%d\r\n",
                    arr[i].pcTaskName,
                    arr[i].xTaskNumber,
                    arr[i].eCurrentState,
                    arr[i].uxCurrentPriority,
                    arr[i].uxBasePriority);
            if(i==2)
            {
                printf("\r\n");
            }
        }

        //获取单个任务的信息
        TaskStatus_t task_info = {0};
        //该函数后面两个参数第一个默认传pdTRUE，第二个传eInvalid(第二个属于使能getinfo的选项)
        vTaskGetInfo(task1_handle,
                    &task_info, 
                    pdTRUE, //需要查询堆栈历史剩余最小值
                    eInvalid);  //设置为eInvalid才会获取真正状态
        printf("name:%s\r\n", task_info.pcTaskName);
        printf("number:%d\r\n", task_info.xTaskNumber);
        printf("state:%d\r\n", task_info.eCurrentState);
        printf("priority:%d\r\n\r\n", task_info.uxCurrentPriority);

        //获取当前的任务句柄
        TaskHandle_t current_handle = xTaskGetCurrentTaskHandle();
        printf("Current Task Handle: %p\t\ttask2 Handle: %p\r\n\r\n", 
                current_handle,
                task2_handle);

        //通过任务名获取指定句柄
        current_handle = xTaskGetHandle("task1");
        printf("%p = xTaskGetHandle(\"task1\")\r\n\r\n",current_handle);

        //获取任务栈历史最小值(任务栈大小-历史最大值)
        StackType_t stack_remain_min = uxTaskGetStackHighWaterMark2(task2_handle);
        printf("task2 stack remain min: %d\r\n\r\n", (uint32_t)stack_remain_min);

        //获取任务状态
        eTaskState state = eTaskGetState(task2_handle);
        printf("task2 state: %d\r\n\r\n", (uint32_t)state); //查表见状态
        
        //以表格形式显示所有信息
        char str[100] = {0};
        vTaskList(str);
        printf("%s\r\n\r\n",str);
        vTaskDelay(1000);
    }
    
}