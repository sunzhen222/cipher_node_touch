
#ifndef _DRV_POWER_SWITCH_H
#define _DRV_POWER_SWITCH_H

#include "stdint.h"
#include "stdbool.h"

typedef enum {
    POWER_SOURCE_LCD = 0,
    POWER_SOURCE_WIFI,
} PowerSource_t;

void PowerSwitchInit(void);
void PowerSwitchSetSource(PowerSource_t source, bool on);

#endif
