#include "battery.h"

#include "drv_adc.h"
#include "stm32f4xx_hal.h"
#include "stdio.h"
#include "ui_msg.h"

#define BATTERY_PERCENT_MIN_MV      3600U
#define BATTERY_PERCENT_MAX_MV      4000U

#define BATTERY_ADC_REF_MV          3300U
#define BATTERY_ADC_MAX_VALUE       4095U
#define BATTERY_DIVIDER_NUMERATOR   2U
#define BATTERY_DIVIDER_DENOMINATOR 1U

#define USB_DETECT_PORT             GPIOC
#define USB_DETECT_PIN              GPIO_PIN_10

#define CHRG_PORT                   GPIOC
#define CHRG_PIN                    GPIO_PIN_11

void BatteryInit(void)
{
    GPIO_InitTypeDef gpioInit = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    gpioInit.Pin = USB_DETECT_PIN;
    gpioInit.Mode = GPIO_MODE_IT_RISING_FALLING;
    gpioInit.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(USB_DETECT_PORT, &gpioInit);
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    gpioInit.Pin = CHRG_PIN;
    gpioInit.Mode = GPIO_MODE_INPUT;
    gpioInit.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(CHRG_PORT, &gpioInit);
}

uint32_t GetBatteryVoltageMv(void)
{
    uint32_t adcValue = GetBatteryAdcValue();
    uint32_t voltageMv;

    voltageMv = (adcValue * BATTERY_ADC_REF_MV * BATTERY_DIVIDER_NUMERATOR)
                / (BATTERY_ADC_MAX_VALUE * BATTERY_DIVIDER_DENOMINATOR);
    return voltageMv;
}

uint32_t GetBatteryPercent(void)
{
    uint32_t voltageMv = GetBatteryVoltageMv();

    if (voltageMv <= BATTERY_PERCENT_MIN_MV) {
        return 0;
    }
    if (voltageMv >= BATTERY_PERCENT_MAX_MV) {
        return 100;
    }

    return (voltageMv - BATTERY_PERCENT_MIN_MV) * 100U
           / (BATTERY_PERCENT_MAX_MV - BATTERY_PERCENT_MIN_MV);
}

BatteryStatus GetBatteryStatus(void)
{
    GPIO_PinState usbDetect = HAL_GPIO_ReadPin(USB_DETECT_PORT, USB_DETECT_PIN);
    GPIO_PinState chrg = HAL_GPIO_ReadPin(CHRG_PORT, CHRG_PIN);

    if (usbDetect == GPIO_PIN_RESET) {
        return BATTERY_NORMAL;
    }

    if (chrg == GPIO_PIN_RESET) {
        return BATTERY_CHARGING;
    }

    return BATTERY_CHARGE_COMPLETE;
}

void SendBatteryInfoToUi(void)
{
    uint32_t batteryPercent = GetBatteryPercent();
    BatteryStatus batteryStatus = GetBatteryStatus();
    printf("battery percent:%lu%%, status:%d\n", batteryPercent, batteryStatus);
    SendUiMsg(UI_MSG_CODE_BATTERY_PERCENT, &batteryPercent, sizeof(batteryPercent));
    SendUiMsg(UI_MSG_CODE_CHARGING, &batteryStatus, sizeof(batteryStatus));
}

void PrintBatteryInfo(void)
{
    uint32_t voltageMv = GetBatteryVoltageMv();
    uint32_t percent = GetBatteryPercent();
    BatteryStatus status = GetBatteryStatus();

    printf("Battery Voltage: %lu mV, Percent: %lu%%, Status: %s\r\n",
           voltageMv, percent,
           status == BATTERY_NORMAL ? "Normal" :
           (status == BATTERY_CHARGING ? "Charging" : "Charge Complete"));
}
