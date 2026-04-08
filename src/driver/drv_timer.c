#include "drv_timer.h"
#include "string.h"
#include "stdio.h"
#include "user_assert.h"

#define TIMER1_DMA_BUFFER_SIZE          20

static void Timer1DmaInit(void);
static void DmaXferCplt(DMA_HandleTypeDef *hdma);

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
DMA_HandleTypeDef hdma_tim1_ch1;
static volatile bool g_timer1DmaWorking = false;
static uint16_t g_timer1DmaBuffer[TIMER1_DMA_BUFFER_SIZE], g_timer1DmaChangeBuffer[TIMER1_DMA_BUFFER_SIZE];
uint32_t g_timer1DmaLength;

void Timer1Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    __HAL_RCC_TIM1_CLK_ENABLE();
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 167;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 19999;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    ASSERT(HAL_TIM_Base_Init(&htim1) == HAL_OK);
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    ASSERT(HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) == HAL_OK);
    ASSERT(HAL_TIM_PWM_Init(&htim1) == HAL_OK);
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    ASSERT(HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) == HAL_OK);
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 1000;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    ASSERT(HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) == HAL_OK);
    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 0;
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
    ASSERT(HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) == HAL_OK);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM1 GPIO Configuration
    PA8     ------> TIM1_CH1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    Timer1DmaInit();
    Timer1Start();
    SetTimer1Duty(0);
}


void Timer1Start(void)
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
}

void SetTimer1Duty(uint16_t duty)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, duty);
}

void SetTimer1Prescaler(uint16_t prescaler)
{
    __HAL_TIM_SET_PRESCALER(&htim1, prescaler);
}

void SetTimer1Period(uint16_t period)
{
    __HAL_TIM_SET_AUTORELOAD(&htim1, period);
}

void Timer1DmaStart(uint16_t *pData, uint32_t length)
{
    if (g_timer1DmaWorking == false) {
        ASSERT(length <= TIMER1_DMA_BUFFER_SIZE);
        g_timer1DmaLength = length;
        memcpy(g_timer1DmaBuffer, pData, length * sizeof(uint16_t));
        memcpy(g_timer1DmaChangeBuffer, pData, length * sizeof(uint16_t));
        g_timer1DmaWorking = true;
    } else {
        //allready working, change value only
        __disable_irq();
        memcpy(g_timer1DmaChangeBuffer, pData, length * sizeof(uint16_t));
        __enable_irq();
    }

}

void Timer1DmaStop(void)
{
    g_timer1DmaWorking = false;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        HAL_IncTick();
    } else if (htim->Instance == TIM3) {
        if (g_timer1DmaWorking) {
            memcpy(g_timer1DmaBuffer, g_timer1DmaChangeBuffer, g_timer1DmaLength * sizeof(uint16_t));
            HAL_DMA_Start_IT(htim1.hdma[TIM_DMA_ID_CC1], (uint32_t)g_timer1DmaBuffer, (uint32_t)&htim1.Instance->CCR1, g_timer1DmaLength);
            __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_CC1);
        }
    } else if (htim->Instance == TIM4) {
    }
}

static void Timer1DmaInit(void)
{
    __HAL_RCC_DMA2_CLK_ENABLE();

    hdma_tim1_ch1.Instance = DMA2_Stream1;
    hdma_tim1_ch1.Init.Channel = DMA_CHANNEL_6;
    hdma_tim1_ch1.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_tim1_ch1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tim1_ch1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_tim1_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_tim1_ch1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_tim1_ch1.Init.Mode = DMA_NORMAL;
    hdma_tim1_ch1.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_tim1_ch1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    hdma_tim1_ch1.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;
    hdma_tim1_ch1.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_tim1_ch1.Init.PeriphBurst = DMA_PBURST_SINGLE;
    ASSERT(HAL_DMA_Init(&hdma_tim1_ch1) == HAL_OK);

    __HAL_LINKDMA(&htim1, hdma[TIM_DMA_ID_CC1], hdma_tim1_ch1);

    HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
    htim1.hdma[TIM_DMA_ID_CC1]->XferCpltCallback = DmaXferCplt;
}

//Timer3, for dshot resend
void Timer3Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    __HAL_RCC_TIM3_CLK_ENABLE();
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 20;      //4MHz
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 499;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    ASSERT(HAL_TIM_Base_Init(&htim3) == HAL_OK);
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    ASSERT(HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) == HAL_OK);
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    ASSERT(HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) == HAL_OK);
    HAL_TIM_Base_Start_IT(&htim3);

    HAL_NVIC_SetPriority(TIM3_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void SetTimer3Period(uint16_t period)
{
    __HAL_TIM_SET_AUTORELOAD(&htim3, period);
}

//Timer4, for UART2 Rx timeout (20ms)
void Timer4Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    __HAL_RCC_TIM4_CLK_ENABLE();
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 8399;    // 84MHz / 8400 = 10kHz
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 199;         // 10kHz / 200 = 50Hz (20ms)
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    ASSERT(HAL_TIM_Base_Init(&htim4) == HAL_OK);
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    ASSERT(HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) == HAL_OK);
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    ASSERT(HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) == HAL_OK);

    HAL_NVIC_SetPriority(TIM4_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
}

void Timer4Clear(void)
{
    __HAL_TIM_SET_COUNTER(&htim4, 0);
}

void Timer4Start(void)
{
    __HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_UPDATE);
    HAL_TIM_Base_Start_IT(&htim4);
}

void Timer4Stop(void)
{
    HAL_TIM_Base_Stop_IT(&htim4);
}

static void DmaXferCplt(DMA_HandleTypeDef *hdma)
{
    if (hdma == htim1.hdma[TIM_DMA_ID_CC1]) {
        __HAL_TIM_DISABLE_DMA(&htim1, TIM_DMA_CC1);
        //if (g_timer1DmaWorking) {
        //    // Restart DMA transfer
        //    memcpy(g_timer1DmaBuffer, g_timer1DmaChangeBuffer, g_timer1DmaLength * sizeof(uint16_t));
        //    HAL_DMA_Start_IT(htim1.hdma[TIM_DMA_ID_CC1], (uint32_t)g_timer1DmaBuffer, (uint32_t)&htim1.Instance->CCR1, g_timer1DmaLength);
        //    __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_CC1);
        //} else {
        //    printf("DMA work over\n");
        //}
    }
}

//Timer5, for 1kHz PWM output on PA0
void Timer5Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    __HAL_RCC_TIM5_CLK_ENABLE();

    // Configure TIM5 for 1kHz PWM
    // APB1 clock = 84MHz, Prescaler = 83, Period = 999
    // Timer clock = 84MHz / (83+1) = 1MHz
    // PWM frequency = 1MHz / (999+1) = 1kHz
    htim5.Instance = TIM5;
    htim5.Init.Prescaler = 83;
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim5.Init.Period = 999;
    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    ASSERT(HAL_TIM_Base_Init(&htim5) == HAL_OK);

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    ASSERT(HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) == HAL_OK);
    ASSERT(HAL_TIM_PWM_Init(&htim5) == HAL_OK);

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    ASSERT(HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) == HAL_OK);

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    ASSERT(HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_1) == HAL_OK);

    // Configure GPIO PA0 for TIM5_CH1
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM5 GPIO Configuration
    PA0     ------> TIM5_CH1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    Timer5Start();
    SetTimer5Duty(0);
}

void Timer5Start(void)
{
    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);
}

void SetTimer5Duty(uint16_t duty)
{
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, duty);
}

