#include "drv_power_switch.h"
#include "drv_gpio.h"

#define POWER_LCD_GPIO                  GPIOA
#define POWER_LCD_PIN                   GPIO_PIN_4
#define POWER_WIFI_GPIO                 GPIOB
#define POWER_WIFI_PIN                  GPIO_PIN_11


void PowerSwitchInit(void)
{
    SetGpioOutput(POWER_LCD_GPIO, POWER_LCD_PIN, GPIO_PIN_SET);
    SetGpioOutput(POWER_WIFI_GPIO, POWER_WIFI_PIN, GPIO_PIN_SET);
}

void PowerSwitchSetSource(PowerSource_t source, bool on)
{
    GPIO_PinState pinState = on ? GPIO_PIN_SET : GPIO_PIN_RESET;
    switch (source) {
    case POWER_SOURCE_LCD:
        HAL_GPIO_WritePin(POWER_LCD_GPIO, POWER_LCD_PIN, pinState);
        break;
    case POWER_SOURCE_WIFI:
        HAL_GPIO_WritePin(POWER_WIFI_GPIO, POWER_WIFI_PIN, pinState);
        break;
    default:
        break;
    }
}

