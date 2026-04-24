
#ifndef _DRV_ADC_H
#define _DRV_ADC_H

#include "stdint.h"
#include "stdbool.h"

#include "stm32f4xx_hal.h"


extern ADC_HandleTypeDef hadc1;

void AdcInit(void);
uint32_t GetHardwareVersionAdcValue(void);
uint32_t GetBatteryAdcValue(void);

#endif
