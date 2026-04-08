#include "stdio.h"
#include "save_error.h"
#include "drv_w25qxx.h"
#include "user_fs.h"
#include "flash_map.h"
#include "user_memory.h"
#include "user_fs.h"
#include "user_utils.h"

#define ERR_FILE             "0:err.log"

void SaveLastError(void)
{
    char *errStr = SRAM_MALLOC(SPI_FLASH_SIZE_ERR_INFO);
    W25qxx_ReadBytes((uint8_t *)errStr, SPI_FLASH_ADDR_ERR_INFO, SPI_FLASH_SIZE_ERR_INFO);
    if (CheckAllFF((uint8_t *)errStr, SPI_FLASH_SIZE_ERR_INFO)) {
        SRAM_FREE(errStr);
        return;
    }
    errStr[SPI_FLASH_SIZE_ERR_INFO - 1] = 0;
    char *pos = memchr(errStr, 0xFF, SPI_FLASH_SIZE_ERR_INFO);
    if (pos != NULL) {
        *pos = 0x00;
    }
    if (strlen(errStr) > 0) {
        int32_t ret = FsFileAppend(ERR_FILE, errStr, strlen(errStr));
        printf("save last error, ret=%ld\n", ret);
        printf("errStr=%s\n", errStr);
    }
    W25qxx_EraseAddr(SPI_FLASH_ADDR_ERR_INFO);
    SRAM_FREE(errStr);
}
