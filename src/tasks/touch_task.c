#include "touch_task.h"
#include "stdio.h"
#include "string.h"
#include "cmsis_os2.h"
#include "user_utils.h"

osThreadId_t g_touchTaskHandle;
osSemaphoreId_t g_touchSem = NULL;
TouchStatus_t g_touchStatus = {0};

static void TouchTask(void *argument);
static void TouchIntHandler(void);


void CreateTouchTask(void)
{
    const osThreadAttr_t touchTaskAttributes = {
        .name = "TouchTask",
        .stack_size = 1024 * 1,
        .priority = (osPriority_t) osPriorityHigh7,
    };
    g_touchTaskHandle = osThreadNew(TouchTask, NULL, &touchTaskAttributes);
}


TouchStatus_t *GetTouchStatus(void)
{
    return &g_touchStatus;
}


static void TouchTask(void *argument)
{
    uint32_t waitTime;
    UNUSED(argument);
    g_touchSem = osSemaphoreNew(20, 0, NULL);
    TouchInit(TouchIntHandler);
    osDelay(50);

    while (1) {
        waitTime = g_touchStatus.touch ? 1000 : osWaitForever;
        osSemaphoreAcquire(g_touchSem, waitTime);
        osKernelLock();
        TouchGetStatus(&g_touchStatus);
        osKernelUnlock();
    }
}

static void TouchIntHandler(void)
{
    osSemaphoreRelease(g_touchSem);
}

