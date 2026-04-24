#include "drv_gpio.h"
#include "stm32f4xx_hal.h"
#include "user_utils.h"
#include "drv_touch.h"
#include "user_msg.h"
#include "drv_button.h"

void GpioInit(void)
{
    GPIO_InitTypeDef gpioInit = {0};
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
    SetGpioOutput(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);   //LED

    //PC9, LLCC68 DIO1.
    gpioInit.Pin = GPIO_PIN_9;
    gpioInit.Mode = GPIO_MODE_IT_RISING;
    gpioInit.Pull = GPIO_NOPULL;
    gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &gpioInit);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    //PA1, Button
    gpioInit.Pin = GPIO_PIN_1;
    gpioInit.Mode = GPIO_MODE_IT_FALLING;
    gpioInit.Pull = GPIO_PULLUP;
    gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpioInit);
    HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);

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
    } else if (GPIO_Pin == GPIO_PIN_9) {
        PubValueMsg(BACKGROUND_MSG_LORA_IRQ, 0);
    } else if (GPIO_Pin == GPIO_PIN_10) {
        PubValueMsg(BACKGROUND_MSG_REFRESH_BATTERY, 0);
    } else if (GPIO_Pin == GPIO_PIN_1) {
        ButtonIntHandler();
    }
}
