#ifndef _STATUS_BAR_H
#define _STATUS_BAR_H

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"

#define STATUS_BAR_HEIGHT               48

void CreateStatusBar(void);
void HandleStatusBarMsg(uint32_t code, void *data, uint32_t dataLen);

void SetStatusBarBatteryPercent(uint8_t percent);
void SetStatusBarCharging(bool charging);

#endif
