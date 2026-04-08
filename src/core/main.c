
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "startup.h"
#include "startup_task.h"
#include "drv_sys.h"

int main(void)
{
    HAL_Init();
    SystemClockConfig();
    osKernelInitialize();
    CreateStartupTask();
    osKernelStart();
}

