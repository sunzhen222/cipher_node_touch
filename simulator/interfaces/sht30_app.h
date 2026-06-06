#ifndef _SIMULATOR_SHT30_APP_H
#define _SIMULATOR_SHT30_APP_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int32_t temperatureCx10;
    uint32_t humidityRhx10;
} Sht30Data_t;

void Sht30AppRefresh(bool printLog);

#endif
