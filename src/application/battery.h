
#ifndef _BATTERY_H
#define _BATTERY_H

#include "stdint.h"
#include "stdbool.h"

typedef enum {
    BATTERY_NORMAL = 0,
    BATTERY_CHARGING,
    BATTERY_CHARGE_COMPLETE,
} BatteryStatus;

void BatteryInit(void);
uint32_t GetBatteryPercent(void);
uint32_t GetBatteryVoltageMv(void);
BatteryStatus GetBatteryStatus(void);
void SendBatteryInfoToUi(void);
void PrintBatteryInfo(void);

#endif
