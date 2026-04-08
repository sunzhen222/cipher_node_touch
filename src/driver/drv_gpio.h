
#ifndef _DRV_GPIO_H
#define _DRV_GPIO_H

#include "stdint.h"
#include "stdbool.h"
#include "stm32f4xx_hal.h"

#define LED_ON()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)
#define LED_OFF()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)

void GpioInit(void);
void SetGpioOutput(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
void SetGpioInput(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t Pull);

#endif
