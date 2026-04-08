#include "user_assert.h"
#include "stdio.h"
#include "string.h"
#include "stm32f4xx_hal.h"
#include "flash_map.h"
#include "background_task.h"
#include "drv_w25qxx.h"
#include "hardware_version.h"
#include "software_version.h"

void ShowAssert(const char *file, uint32_t line)
{
    char str[256] = {0};
    uint32_t flashWriteIndex = 0;

    printf("assert,file=%s\nline=%lu\n", file, line);

    sprintf(str, "hardware version: %s, software version: %s, build time: %s\n", GetHardwareVersionString(), GetSoftwareVersionString(), GetBuildTime());
    W25qxx_WriteBytes((uint8_t *)str, SPI_FLASH_ADDR_ERR_INFO + flashWriteIndex, strlen(str));
    flashWriteIndex += strlen(str);
    sprintf(str, "assert,file=%s\nline=%lu\n\n", file, line);
    W25qxx_WriteBytes((uint8_t *)str, SPI_FLASH_ADDR_ERR_INFO + flashWriteIndex, strlen(str));
    NVIC_SystemReset();
}


void assert_failed(uint8_t *file, uint32_t line)
{
    ShowAssert((char *)file, line);
}

