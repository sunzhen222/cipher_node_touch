
#ifndef _DRV_SPI_H
#define _DRV_SPI_H

#include "stdint.h"
#include "stdbool.h"
#include "stm32f4xx_hal.h"

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;
extern DMA_HandleTypeDef hdma_spi1_tx;
extern DMA_HandleTypeDef hdma_spi3_tx;

void SpiInit(void);

#endif
