#ifndef _SIMULATOR_LORA_H
#define _SIMULATOR_LORA_H

#include <stdbool.h>
#include <stdint.h>

#define LLCC68_LORA_SF_5 5
#define LLCC68_LORA_SF_6 6
#define LLCC68_LORA_SF_7 7
#define LLCC68_LORA_SF_8 8
#define LLCC68_LORA_SF_9 9
#define LLCC68_LORA_SF_10 10
#define LLCC68_LORA_SF_11 11

#define LLCC68_LORA_BANDWIDTH_125_KHZ 4
#define LLCC68_LORA_BANDWIDTH_250_KHZ 5
#define LLCC68_LORA_BANDWIDTH_500_KHZ 6

uint8_t LoraSettings(void);
const char *LoraGetWirelessStatus(void);
bool LoraTxBusy(void);

#endif
