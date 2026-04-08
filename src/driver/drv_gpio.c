#include "drv_gpio.h"
#include "stm32f4xx_hal.h"
#include "user_utils.h"
#include "drv_touch.h"
#include "user_msg.h"

void GpioInit(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    SetGpioInput(GPIOA, GPIO_PIN_All & ~(GPIO_PIN_13 | GPIO_PIN_14), GPIO_NOPULL);
    SetGpioInput(GPIOB, GPIO_PIN_All, GPIO_NOPULL);
    SetGpioInput(GPIOC, GPIO_PIN_All, GPIO_NOPULL);
    SetGpioInput(GPIOD, GPIO_PIN_All, GPIO_NOPULL);

    SetGpioOutput(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);     //LCD CS
    SetGpioOutput(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);    //LORA CS
    SetGpioOutput(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);     //FLASH CS

    SetGpioOutput(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);   //LCD backlight
}

void SetGpioOutput(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    GPIO_InitTypeDef gpioInit = {0};

    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, PinState);
    gpioInit.Pin = GPIO_Pin;
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    gpioInit.Pull = GPIO_NOPULL;
    gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOx, &gpioInit);
}

void SetGpioInput(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t Pull)
{
    GPIO_InitTypeDef gpioInit = {0};

    gpioInit.Pin = GPIO_Pin;
    gpioInit.Mode = GPIO_MODE_INPUT;
    gpioInit.Pull = Pull;
    HAL_GPIO_Init(GPIOx, &gpioInit);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_8) {
        TouchPadIntHandler();
    }
}
