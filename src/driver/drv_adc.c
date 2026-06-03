#include "drv_adc.h"
#include "stdio.h"
#include "user_utils.h"
#include "test_cmd.h"
#include "rtos_expand.h"
#include "drv_timer.h"
#include "stm32f4xx_ll_adc.h"

#define SAMPLE_COUNT    100

static void Adc1Init(void);
static void AdcTestFunc(int argc, char *argv[]);
static uint32_t GetAdcValue(uint32_t channel, uint32_t samplingTime);

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
    __HAL_RCC_GPIOB_CLK_ENABLE();

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
    PB1     ------> ADC1_IN9
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

uint32_t GetHardwareVersionAdcValue(void)
{
    return GetAdcValue(ADC_CHANNEL_11, ADC_SAMPLETIME_3CYCLES);
}

uint32_t GetBatteryAdcValue(void)
{
    return GetAdcValue(ADC_CHANNEL_9, ADC_SAMPLETIME_3CYCLES);
}

int32_t GetMcuTemperatureCx10(void)
{
    uint32_t tempAdcValue = GetAdcValue(ADC_CHANNEL_TEMPSENSOR, ADC_SAMPLETIME_480CYCLES);
    uint32_t vrefAdcValue = GetAdcValue(ADC_CHANNEL_VREFINT, ADC_SAMPLETIME_480CYCLES);
    int32_t tempAdcValueAtCalVref;
    int32_t cal1 = (int32_t)(*TEMPSENSOR_CAL1_ADDR);
    int32_t cal2 = (int32_t)(*TEMPSENSOR_CAL2_ADDR);

    if ((vrefAdcValue == 0U) || (cal2 == cal1)) {
        return 0;
    }

    tempAdcValueAtCalVref = (int32_t)((tempAdcValue * (uint32_t)(*VREFINT_CAL_ADDR) + (vrefAdcValue / 2U))
                                      / vrefAdcValue);

    return (((tempAdcValueAtCalVref - cal1) * (TEMPSENSOR_CAL2_TEMP - TEMPSENSOR_CAL1_TEMP) * 10)
            / (cal2 - cal1)) + (TEMPSENSOR_CAL1_TEMP * 10);
}

static uint32_t GetAdcValue(uint32_t channel, uint32_t samplingTime)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    uint32_t sum = 0;

    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = samplingTime;
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
        uint32_t hw_value;
        uint32_t battery_value;
        int32_t temperatureCx10;

        hw_value = GetHardwareVersionAdcValue();
        battery_value = GetBatteryAdcValue();
        temperatureCx10 = GetMcuTemperatureCx10();
        printf("GetHardwareVersionAdcValue=%lu\n", hw_value);
        printf("GetBatteryAdcValue=%lu\n", battery_value);
        printf("GetMcuTemperature=%ld.%ldC\n", (long)(temperatureCx10 / 10),
               (long)(temperatureCx10 >= 0 ? temperatureCx10 % 10 : -(temperatureCx10 % 10)));
    } else {
        printf("adc test input err\n");
    }
}

