#include "startup_task.h"
#include "startup.h"
#include "cmsis_os2.h"
#include "user_utils.h"

static void StartupTask(void *pvParameter);

osThreadId_t g_startupTaskId;


void CreateStartupTask(void)
{
    const osThreadAttr_t cmdTaskAttributes = {
        .name = "StartupTask",
        .priority = osPriorityRealtime7,
        .stack_size = 8192,
    };
    g_startupTaskId = osThreadNew(StartupTask, NULL, &cmdTaskAttributes);
}

static void StartupTask(void *pvParameter)
{
    UNUSED(pvParameter);
    Startup();
    osThreadExit();
}

