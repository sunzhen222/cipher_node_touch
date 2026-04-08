#include "user_delay.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

static volatile uint32_t g_timerTick = 0;

uint32_t actor = 10000000;

void UserDelay(uint32_t ms)
{
    if (osKernelGetState() == osKernelRunning) {
        osDelay(ms);
    } else {
        HAL_Delay(ms);
    }
}

void UserDelayUs(uint32_t us)
{
    volatile uint32_t i, tick, countPerUs;
    countPerUs = 100;
    for (tick = 0; tick < us; tick++) {
        for (i = 0; i < countPerUs; i++) {
            PretendDoingSomething(i);
        }
    }
}

void PretendDoingSomething(uint32_t i)
{
    if (i > actor) {
        printf("!!\n");
    }
}
