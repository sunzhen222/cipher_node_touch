#include "drv_trng.h"
#include "stdio.h"
#include "stm32f4xx_hal.h"

RNG_HandleTypeDef hrng;

void TrngInit(void)
{
    hrng.Instance = RNG;
    __HAL_RCC_RNG_CLK_ENABLE();
    /* RNG interrupt Init */
    HAL_RNG_Init(&hrng);
    HAL_NVIC_SetPriority(HASH_RNG_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(HASH_RNG_IRQn);
}

uint32_t TrngGetRandomNumber(void)
{
    return HAL_RNG_GetRandomNumber(&hrng);
}
