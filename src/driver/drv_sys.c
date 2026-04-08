#include "drv_sys.h"
#include "stdio.h"
#include "stm32f4xx_hal.h"
#include "software_version.h"
#include "hardware_version.h"

uint32_t HAL_RCC_GetSysClockFreq(void);
uint32_t HAL_RCC_GetHCLKFreq(void);
uint32_t HAL_RCC_GetPCLK1Freq(void);
uint32_t HAL_RCC_GetPCLK2Freq(void);


void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

}

void SystemClockConfig(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    ASSERT(HAL_RCC_OscConfig(&RCC_OscInitStruct) == HAL_OK);

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    ASSERT(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) == HAL_OK);
}



void PrintSystemInfo(void)
{
    printf("esc master\n");
    printf("compiler:");
#ifdef __ARMCC_VERSION
    printf("ARMCC\n");
#elif defined(__ICCARM__)
    printf("ICCARM\n");
#elif defined(__GNUC__)
    printf("GNUC\n");
#endif
    printf("sys clock:%luHz\n", HAL_RCC_GetSysClockFreq());
    printf("hclk:%luHz\n", HAL_RCC_GetHCLKFreq());
    printf("pclk1:%luHz\n", HAL_RCC_GetPCLK1Freq());
    printf("pclk2:%luHz\n", HAL_RCC_GetPCLK2Freq());
    printf("build time:%s\n", GetBuildTime());
    printf("software version=%s\n", GetSoftwareVersionString());
    printf("hardware version=%s\n", GetHardwareVersionString());
}
