#ifndef _DRV_I2C_H
#define _DRV_I2C_H

#include "stdint.h"
#include "stdbool.h"
#include "stm32f4xx_hal.h"

extern I2C_HandleTypeDef hi2c1;

void I2cInit(void);
void I2cScan(void);
bool GetFirstI2cDeviceAddr(I2C_HandleTypeDef *hi2c, uint8_t *addr);

#endif
