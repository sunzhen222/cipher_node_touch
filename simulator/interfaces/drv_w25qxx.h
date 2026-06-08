#ifndef _SIMULATOR_DRV_W25QXX_H
#define _SIMULATOR_DRV_W25QXX_H

#include <stdint.h>
#include <stdbool.h>

bool W25qxx_Init(void);
void W25qxx_EraseChip(void);
void W25qxx_EraseAddr(uint32_t addr);
void W25qxx_WriteBytes(uint8_t *pBuffer, uint32_t addr, uint32_t size);
void W25qxx_ReadBytes(uint8_t *pBuffer, uint32_t addr, uint32_t size);

#endif
