
#ifndef _DRV_TRNG_H
#define _DRV_TRNG_H

#include "stdint.h"
#include "stdbool.h"

void TrngInit(void);
uint32_t TrngGetRandomNumber(void);

#endif
