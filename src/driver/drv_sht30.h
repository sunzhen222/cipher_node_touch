#ifndef _DRV_SHT30_H
#define _DRV_SHT30_H

#include "stdint.h"
#include "stdbool.h"

typedef struct {
    int16_t temperatureCx10;
    uint16_t humidityRhx10;
} Sht30Data_t;

void Sht30Init(void);
bool Sht30Read(Sht30Data_t *data);

#endif
