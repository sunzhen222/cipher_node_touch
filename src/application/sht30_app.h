#ifndef _SHT30_APP_H
#define _SHT30_APP_H

#include "stdbool.h"
#include "drv_sht30.h"

void Sht30AppInit(void);
void Sht30AppRefresh(bool printLog);

#endif
