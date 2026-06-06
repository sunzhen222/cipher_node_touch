#ifndef _SIMULATOR_DRV_W25QXX_H
#define _SIMULATOR_DRV_W25QXX_H

#include <stdint.h>

void W25qxx_EraseChip(void);
void W25qxx_WriteBytes(uint8_t *pBuffer, uint32_t addr, uint32_t size);

#endif
