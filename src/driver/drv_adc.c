#include "drv_adc.h"
#include "stdio.h"
#include "user_utils.h"
#include "test_cmd.h"
#include "rtos_expand.h"
#include "drv_timer.h"

#define SAMPLE_COUNT    100

static void Adc1Init(void);
static void AdcTestFunc(int argc, char *argv[]);

ADC_HandleTypeDef hadc1;

void AdcInit(void)
{
    Adc1Init();
    RegisterTestCmd("adc:", AdcTestFunc);
}

static void Adc1Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = ENABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    ASSERT(HAL_ADC_Init(&hadc1) == HAL_OK);

    sConfig.Channel = ADC_CHANNEL_11;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    ASSERT(HAL_ADC_ConfigChannel(&hadc1, &sConfig) == HAL_OK);

    /**ADC1 GPIO Configuration
    PC1     ------> ADC1_IN11
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

uint32_t GetHardwareVersionAdcValue(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    uint32_t sum = 0;

    sConfig.Channel = ADC_CHANNEL_11;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    for (uint32_t i = 0; i < SAMPLE_COUNT; i++) {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 100);
        sum += HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);
    return (sum + (SAMPLE_COUNT / 2)) / SAMPLE_COUNT;
}

static void AdcTestFunc(int argc, char *argv[])
{
    VALUE_CHECK(argc, 1);
    if (strcmp(argv[0], "get") == 0) {
        uint32_t value;
        value = GetHardwareVersionAdcValue();
        printf("GetHardwareVersionAdcValue=%lu\n", value);
    } else {
        printf("adc test input err\n");
    }
}

