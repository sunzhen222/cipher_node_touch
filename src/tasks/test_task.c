#include "test_task.h"
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"
#include "user_utils.h"
#include "drv_uart.h"

static void TestTask(void *pvParameter);

osThreadId_t g_testTaskId;

void CreateTestTask(void)
{
    const osThreadAttr_t testTaskAttributes = {
        .name = "TestTask",
        .priority = osPriorityNormal,
        .stack_size = 1024,
    };
    g_testTaskId = osThreadNew(TestTask, NULL, &testTaskAttributes);
}

static void TestTask(void *pvParameter)
{
    UNUSED(pvParameter);
    while (1) {
        osDelay(1000);
    }
}

