#ifndef _SIMULATOR_BATTERY_H
#define _SIMULATOR_BATTERY_H

#include <stdint.h>

typedef enum {
    BATTERY_NORMAL = 0,
    BATTERY_CHARGING,
    BATTERY_CHARGE_COMPLETE,
} BatteryStatus;

uint32_t GetBatteryPercent(void);
uint32_t GetBatteryVoltageMv(void);
BatteryStatus GetBatteryStatus(void);

#endif
