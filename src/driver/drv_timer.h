
#ifndef _DRV_TIMER_H
#define _DRV_TIMER_H

#include "stdint.h"
#include "stdbool.h"
#include "stm32f4xx_hal.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;

void Timer1Init(void);
void Timer1Start(void);
void SetTimer1Duty(uint16_t duty);
void SetTimer1Prescaler(uint16_t prescaler);
void SetTimer1Period(uint16_t period);
void Timer1DmaStart(uint16_t *pData, uint32_t Length);
void Timer1DmaStop(void);
void Timer3Init(void);
void SetTimer3Period(uint16_t period);
void Timer4Init(void);
void Timer4Clear(void);
void Timer4Start(void);
void Timer4Stop(void);
void Timer5Init(void);
void Timer5Start(void);
void SetTimer5Duty(uint16_t duty);

#endif
