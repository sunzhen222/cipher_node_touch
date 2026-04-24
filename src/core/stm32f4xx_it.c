/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "stm32f4xx_hal.h"
#include "stm32f4xx_it.h"
#include "drv_adc.h"
#include "drv_uart.h"
#include "drv_timer.h"
#include "drv_spi.h"

extern TIM_HandleTypeDef htim2;
extern RNG_HandleTypeDef hrng;

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
    while (1) {
    }
}


/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
    while (1) {
    }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
    while (1) {
    }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
    while (1) {
    }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{

}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim3);
}


/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM4_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim4);
}


/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}

/**
  * @brief This function handles EXTI line1 interrupts.
  */
void EXTI1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

/**
  * @brief This function handles EXTI line2 interrupts.
  */
void EXTI2_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

/**
  * @brief This function handles EXTI line[9:5] interrupts.
  */
void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
}

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
}


/**
  * @brief This function handles HASH and RNG global interrupts.
  */
void HASH_RNG_IRQHandler(void)
{
    HAL_RNG_IRQHandler(&hrng);
}

/**
  * @brief This function handles DMA2 stream1 global interrupt.
  */
void DMA2_Stream1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(htim1.hdma[TIM_DMA_ID_CC1]);
}

/**
  * @brief This function handles DMA2 stream3 global interrupt (SPI1 TX).
  */
void DMA2_Stream3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_spi1_tx);
}

/**
  * @brief This function handles DMA1 stream5 global interrupt (SPI3 TX).
  */
void DMA1_Stream5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_spi3_tx);
}
