
#ifndef _DRV_UART_H
#define _DRV_UART_H

#include "stdint.h"
#include "stdbool.h"
#include "stm32f4xx_hal.h"

typedef void (*UartRxCallback_t)(uint8_t *pData, uint32_t length);

extern UART_HandleTypeDef huart1, huart2;

void Uart1Init(void);
void Uart1Start(void);
void Uart2Init(void);
void Uart2Start(void);

#endif
