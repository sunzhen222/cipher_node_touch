#include "rtos_expand.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "cmsis_os2.h"
#include "string.h"
#include "user_memory.h"

volatile uint32_t FreeRTOSRunTimeTicks;
uint32_t FreeRTOSRunTimeTicksBak;
TaskStatus_t *g_taskStatusArray;

uint32_t FindIndexFromName(const char *taskname);

void PrintTasksStatus(void)
{
#if (configUSE_TRACE_FACILITY > 0)
    uint32_t totalRunTime;
    UBaseType_t   arraySize;
    TaskStatus_t  *statusArray;
    UBaseType_t sizeOfStack;
    UBaseType_t totalSizeofStack = 0;

    static uint32_t taskRunTime = 0;
    uint32_t millionPercent;

    arraySize = uxTaskGetNumberOfTasks(); //Get the number of tasks
    statusArray = SramMalloc(arraySize * sizeof(TaskStatus_t));
    if (statusArray != NULL) { //Memory request successful
        arraySize = uxTaskGetSystemState((TaskStatus_t *)statusArray,
                                         (UBaseType_t)arraySize,
                                         (uint32_t *)&totalRunTime);
        printf("\n\n");
        printf("total task num :  %ld.\n", arraySize);
        printf("CurrentState enum: {eRunning = 0, eReady,eBlocked,eSuspended,eDeleted,eInvalid}\n");

        printf("    TaskName\t   Task Prio\t   CurrentState\t   SizeStk\t MinStk\t   FreeStkPercent\tCPUUsedPercent\t\t\tTaskNumber\n");
        for (uint8_t i = 0; i < arraySize; i++) {
            sizeOfStack = vTaskGetStackSize(xTaskGetHandle(statusArray[i].pcTaskName));
            totalSizeofStack += sizeOfStack * 4;
            taskRunTime = FindIndexFromName(statusArray[i].pcTaskName);
            millionPercent = (uint64_t)(statusArray[i].ulRunTimeCounter - taskRunTime) * 1000000 / (uint64_t)(totalRunTime - FreeRTOSRunTimeTicksBak);
            printf("%12s\t %9d\t %9d\t %8d\t %6d\t %8d\t %8d/%8d\t %8ld/1M %8ld\n",
                   statusArray[i].pcTaskName,
                   (int)statusArray[i].uxCurrentPriority,
                   (int)statusArray[i].eCurrentState,
                   (int)sizeOfStack * 4,
                   (int)statusArray[i].usStackHighWaterMark * 4,
                   (int)((float)(int)statusArray[i].usStackHighWaterMark / sizeOfStack * 100),
                   (int)(statusArray[i].ulRunTimeCounter - taskRunTime),
                   (int)(totalRunTime - FreeRTOSRunTimeTicksBak),
                   millionPercent,
                   statusArray[i].xTaskNumber);
        }

        printf("total allocated stack size :  %d(%d K).\n", (int)totalSizeofStack, (int)totalSizeofStack / 1024);
    }
    SramFree(statusArray);
#else
    printf(" Need to set configUSE_TRACE_FACILITY to 1.\n");
#endif
}

uint32_t FindIndexFromName(const char *taskname)
{
    uint8_t i;
    uint32_t arraySize;
    if (g_taskStatusArray == NULL) {
        return 0;
    }
    arraySize = uxTaskGetNumberOfTasks();
    for (i = 0; i < arraySize; i++) {
        if (strcmp(g_taskStatusArray[i].pcTaskName, taskname) == 0) {
            break;
        }
    }
    return g_taskStatusArray[i].ulRunTimeCounter;
}


void ClrRunTimeStats(void)
{
    uint32_t totalRunTime;
    UBaseType_t arraySize;

    arraySize = uxTaskGetNumberOfTasks();
    if (g_taskStatusArray) {
        SramFree(g_taskStatusArray);
    }
    g_taskStatusArray = SramMalloc(sizeof(TaskStatus_t) * arraySize);
    uxTaskGetSystemState((TaskStatus_t *)g_taskStatusArray,
                         (UBaseType_t)arraySize,
                         (uint32_t *)&totalRunTime);
    FreeRTOSRunTimeTicksBak = FreeRTOSRunTimeTicks;
}


uint32_t GetRunTimeCounter(void)
{
    if (osKernelGetState() == osKernelRunning) {
        FreeRTOSRunTimeTicks = osKernelGetTickCount();
    }
    return FreeRTOSRunTimeTicks;
}


void ConfigureTimerForRunTimeStats(void)
{
    FreeRTOSRunTimeTicks = 0;
}

#include "stm32f4xx_hal.h"

extern TIM_HandleTypeDef htim2;

//u32 is enough to calculate the time difference.
uint32_t GetMicroSecCount(void)
{
    uint32_t microSecCount;
    ASSERT(HAL_TIM_Base_Stop_IT(&htim2) == HAL_OK);
    microSecCount = osKernelGetTickCount() * 1000;
    microSecCount += __HAL_TIM_GET_COUNTER(&htim2);
    ASSERT(HAL_TIM_Base_Start_IT(&htim2) == HAL_OK);
    return microSecCount;
}

#define MAX_TASK_NUM    10
#define START_TASK_ID   2

void PrintTasksFireWaterData(void)
{
#if (configUSE_TRACE_FACILITY > 0)
    uint32_t tasksRunTime[MAX_TASK_NUM] = {0};
    static uint32_t lastTasksRunTime[MAX_TASK_NUM] = {0};

    UBaseType_t arraySize;
    TaskStatus_t *statusArray;
    uint32_t totalRunTime;

    arraySize = uxTaskGetNumberOfTasks(); //Get the number of tasks
    statusArray = SramMalloc(arraySize * sizeof(TaskStatus_t));
    arraySize = uxTaskGetSystemState((TaskStatus_t *)statusArray,
                                     (UBaseType_t)arraySize,
                                     (uint32_t *)&totalRunTime);

    for (uint8_t i = 0; i < arraySize; i++) {
        tasksRunTime[statusArray[i].xTaskNumber] = statusArray[i].ulRunTimeCounter - lastTasksRunTime[statusArray[i].xTaskNumber];
        if (tasksRunTime[statusArray[i].xTaskNumber] > TASKS_FIRE_WATER_TICK) {
            tasksRunTime[statusArray[i].xTaskNumber] = TASKS_FIRE_WATER_TICK;
        }
        lastTasksRunTime[statusArray[i].xTaskNumber] = statusArray[i].ulRunTimeCounter;
    }
    printf("tasks:");
    for (uint8_t i = START_TASK_ID; i < arraySize + START_TASK_ID; i++) {
        if (i == START_TASK_ID) {
            printf("%lu", TASKS_FIRE_WATER_TICK - tasksRunTime[i]);
        } else {
            printf(",%lu", tasksRunTime[i]);
        }
    }
    printf("\n");
    SramFree(statusArray);
#else
    printf(" Need to set configUSE_TRACE_FACILITY to 1.\n");
#endif
}
